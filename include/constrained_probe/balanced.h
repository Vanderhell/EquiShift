#ifndef CONSTRAINED_PROBE_BALANCED_H
#define CONSTRAINED_PROBE_BALANCED_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t count;   // number of channels
    int32_t step;     // perturbation step size
} bp_config_t;

typedef struct {
    uint16_t expected;
    uint16_t seen;
    uint16_t best_index;
    uint16_t worst_index;
    int32_t best;
    int32_t worst;
    int32_t second_best;
    bool has_second;
} bp_stream_t;

typedef struct {
    uint16_t best_index;
    uint16_t worst_index;
    int64_t spread;
    int64_t top_two_margin;
} bp_decision_t;

bool bp_validate(const bp_config_t *cfg);
bool bp_delta(const bp_config_t *cfg, uint16_t probe, uint16_t axis, int32_t *out);
bool bp_response_sum(const bp_config_t *cfg, const int32_t *responses, int32_t *sum);
bool bp_score(const bp_config_t *cfg, const int32_t *responses, int32_t *scores);
bool bp_score_i64(const bp_config_t *cfg, const int32_t *responses, int64_t *scores);
bool bp_max_safe_step(uint16_t count, uint16_t probe, const int32_t *x,
                     const int32_t *minimum, const int32_t *maximum,
                     int32_t requested, int32_t *safe);
/* Apply the transition between distinct balanced probes by changing two channels. */
bool bp_transition(uint16_t count, uint16_t previous, uint16_t next, int32_t step,
                  uint16_t *old_axis, int32_t *old_change,
                  uint16_t *new_axis, int32_t *new_change);
bool bp_stream_begin(bp_stream_t *stream, uint16_t count);
bool bp_stream_update(bp_stream_t *stream, int32_t response);
bool bp_stream_finish(const bp_stream_t *stream, bp_decision_t *decision);

#ifdef __cplusplus
}
#endif

#endif // CONSTRAINED_PROBE_BALANCED_H
