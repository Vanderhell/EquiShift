#include "constrained_probe/balanced.h"
#include <limits.h>
#include <stddef.h>

bool bp_validate(const bp_config_t *cfg)
{
    int64_t hot;
    if (cfg == NULL || cfg->count < 2 || cfg->step == 0) return false;
    /* Widen before negation/multiplication; int64_t safely covers uint16_t * int32_t. */
    hot = (int64_t)(cfg->count - 1u) * (int64_t)cfg->step;
    return hot >= INT32_MIN && hot <= INT32_MAX &&
           -(int64_t)cfg->step >= INT32_MIN && -(int64_t)cfg->step <= INT32_MAX;
}

bool bp_delta(const bp_config_t *cfg, uint16_t probe, uint16_t axis, int32_t *out)
{
    int64_t value;
    if (!bp_validate(cfg) || out == NULL || probe >= cfg->count || axis >= cfg->count) return false;
    value = probe == axis ? (int64_t)(cfg->count - 1u) * cfg->step : -(int64_t)cfg->step;
    *out = (int32_t)value;
    return true;
}

bool bp_response_sum(const bp_config_t *cfg, const int32_t *responses, int32_t *sum)
{
    int64_t total = 0;
    uint16_t i;
    if (!bp_validate(cfg) || responses == NULL || sum == NULL) return false;
    for (i = 0; i < cfg->count; ++i) total += responses[i];
    if (total < INT32_MIN || total > INT32_MAX) return false;
    *sum = (int32_t)total;
    return true;
}

bool bp_score_i64(const bp_config_t *cfg, const int32_t *responses, int64_t *scores)
{
    int64_t total = 0;
    uint16_t i;
    if (!bp_validate(cfg) || responses == NULL || scores == NULL) return false;
    for (i = 0; i < cfg->count; ++i) total += responses[i];
    for (i = 0; i < cfg->count; ++i) scores[i] = (int64_t)cfg->count * responses[i] - total;
    return true;
}

bool bp_score(const bp_config_t *cfg, const int32_t *responses, int32_t *scores)
{
    int64_t total = 0, value;
    uint16_t i;
    if (!bp_validate(cfg) || responses == NULL || scores == NULL) return false;
    for (i = 0; i < cfg->count; ++i) total += responses[i];
    /* Validate every result before writing, so failure leaves output untouched. */
    for (i = 0; i < cfg->count; ++i) {
        value = (int64_t)cfg->count * responses[i] - total;
        if (value < INT32_MIN || value > INT32_MAX) return false;
    }
    for (i = 0; i < cfg->count; ++i)
        scores[i] = (int32_t)((int64_t)cfg->count * responses[i] - total);
    return true;
}

bool bp_max_safe_step(uint16_t count, uint16_t probe, const int32_t *x,
                      const int32_t *minimum, const int32_t *maximum,
                      int32_t requested, int32_t *safe)
{
    uint32_t i;
    int64_t limit, magnitude;
    if (count < 2 || probe >= count || x == NULL || minimum == NULL || maximum == NULL ||
        safe == NULL || requested == 0) return false;
    magnitude = requested < 0 ? -(int64_t)requested : requested;
    limit = magnitude;
    if (limit > INT32_MAX) limit = INT32_MAX;
    if (limit > INT32_MAX / (count - 1u)) limit = INT32_MAX / (count - 1u);
    for (i = 0; i < count; ++i) {
        int64_t coefficient = i == probe ? (int64_t)count - 1 : -1;
        int64_t available;
        if (minimum[i] > maximum[i] || x[i] < minimum[i] || x[i] > maximum[i]) return false;
        if (coefficient > 0) available = requested > 0 ? (int64_t)maximum[i] - x[i] : (int64_t)x[i] - minimum[i];
        else available = requested > 0 ? (int64_t)x[i] - minimum[i] : (int64_t)maximum[i] - x[i];
        if (available / (i == probe ? count - 1u : 1u) < limit)
            limit = available / (i == probe ? count - 1u : 1u);
    }
    if (limit <= 0) return false;
    *safe = requested < 0 ? (int32_t)-limit : (int32_t)limit;
    return true;
}

bool bp_transition(uint16_t count, uint16_t previous, uint16_t next, int32_t step,
                   uint16_t *old_axis, int32_t *old_change,
                   uint16_t *new_axis, int32_t *new_change)
{
    int64_t change;
    if (count < 2 || previous >= count || next >= count || previous == next || step == 0 ||
        old_axis == NULL || old_change == NULL || new_axis == NULL || new_change == NULL) return false;
    change = (int64_t)count * step;
    if (change <= INT32_MIN || change > INT32_MAX) return false;
    *old_axis = previous; *old_change = (int32_t)-change;
    *new_axis = next; *new_change = (int32_t)change;
    return true;
}

bool bp_stream_begin(bp_stream_t *stream, uint16_t count)
{
    if (stream == NULL || count < 2) return false;
    stream->expected=count; stream->seen=0; stream->best_index=0; stream->worst_index=0;
    stream->best=INT32_MIN; stream->worst=INT32_MAX; stream->second_best=INT32_MIN;
    stream->has_second=false;
    return true;
}

bool bp_stream_update(bp_stream_t *stream, int32_t response)
{
    uint16_t i;
    if (stream == NULL || stream->expected == 0 || stream->seen >= stream->expected) return false;
    i=stream->seen++;
    if (i==0) {
        stream->best=response; stream->worst=response;
        stream->best_index=0; stream->worst_index=0;
        return true;
    }
    if (response > stream->best) {
        stream->second_best=stream->best; stream->has_second=true;
        stream->best=response; stream->best_index=i;
    } else if (!stream->has_second || response > stream->second_best) {
        stream->second_best=response; stream->has_second=true;
    }
    if (response < stream->worst) { stream->worst=response; stream->worst_index=i; }
    return true;
}

bool bp_stream_finish(const bp_stream_t *stream, bp_decision_t *decision)
{
    if (stream == NULL || decision == NULL || stream->expected == 0 ||
        stream->seen != stream->expected || !stream->has_second) return false;
    decision->best_index=stream->best_index; decision->worst_index=stream->worst_index;
    decision->spread=(int64_t)stream->best-stream->worst;
    decision->top_two_margin=(int64_t)stream->best-stream->second_best;
    return true;
}
