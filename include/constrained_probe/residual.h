#ifndef CONSTRAINED_PROBE_RESIDUAL_H
#define CONSTRAINED_PROBE_RESIDUAL_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* r = sum(probe_responses) - count * baseline_response.
   Interpret as a consistency diagnostic only for one complete balanced
   zero-sum probe set measured around this independent baseline. */
bool cp_sequence_residual(uint16_t count, const int32_t *probe_responses,
                          int32_t baseline_response, int64_t *residual);

#ifdef __cplusplus
}
#endif

#endif
