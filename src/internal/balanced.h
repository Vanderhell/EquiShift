#ifndef CONSTRAINED_PROBE_BALANCED_H
#define CONSTRAINED_PROBE_BALANCED_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Internal specialized balanced-frame primitive; not stable public API.
   For N channels and nonzero q, probe i has (N-1)q on axis i and -q on
   every other axis. Valid N is 2..UINT16_MAX; validate also requires every
   component to fit int32_t. */
typedef struct {
    uint16_t count;
    int32_t step;
} bp_config_t;

/* Requires count >= 2, nonzero step, and every component representable in
   int32_t. */
bool bp_validate(const bp_config_t *cfg);

/* Generate one component without materializing the N x N frame. Output is
   unchanged on failure; probe and axis must be less than count. */
bool bp_delta(const bp_config_t *cfg, uint16_t probe, uint16_t axis,
              int32_t *out);

/* Decode scores[i] = N*y[i] - sum(y), equal to q*N^2*(g[i]-mean(g)) for a
   stationary affine plant. Responses and outputs each contain count values
   and must not overlap. Wide outputs are written only on success. */
bool bp_score_i64(const bp_config_t *cfg, const int32_t *responses,
                  int64_t *scores);

#ifdef __cplusplus
}
#endif

#endif
