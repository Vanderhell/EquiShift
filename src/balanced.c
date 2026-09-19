#include "internal/balanced.h"

#include <stddef.h>

bool bp_validate(const bp_config_t *cfg)
{
    uint32_t magnitude, factor, low_product, high_product;
    if (cfg == NULL || cfg->count < 2 || cfg->step == 0) return false;
    /* The cold component -step must fit int32_t, and the hot component
       (N-1)*step must fit too. Compute the product in 16-bit limbs to avoid
       both signed overflow and 64-bit multiply/divide helper calls on M0. */
    magnitude = (uint32_t)cfg->step;
    if (cfg->step < 0) magnitude = 0u - magnitude;
    if (magnitude > (uint32_t)INT32_MAX) return false;
    factor = (uint32_t)cfg->count - 1u;
    low_product = (magnitude & UINT32_C(0xffff)) * factor;
    high_product = (magnitude >> 16) * factor + (low_product >> 16);
    return high_product <= ((uint32_t)INT32_MAX >> 16);
}

bool bp_delta(const bp_config_t *cfg, uint16_t probe, uint16_t axis,
              int32_t *out)
{
    int32_t value;
    if (!bp_validate(cfg) || out == NULL || probe >= cfg->count ||
        axis >= cfg->count) return false;
    /* Validation proves both the multiply and negation are representable. */
    value = probe == axis
        ? (int32_t)(cfg->count - 1u) * cfg->step
        : -cfg->step;
    *out = value;
    return true;
}

bool bp_score_i64(const bp_config_t *cfg, const int32_t *responses,
                  int64_t *scores)
{
    int64_t total = 0;
    uint16_t i;
    if (!bp_validate(cfg) || responses == NULL || scores == NULL) return false;
    for (i = 0; i < cfg->count; ++i) total += (int64_t)responses[i];
    for (i = 0; i < cfg->count; ++i)
        scores[i] = (int64_t)cfg->count * (int64_t)responses[i] - total;
    return true;
}
