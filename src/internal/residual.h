#ifndef CONSTRAINED_PROBE_RESIDUAL_H
#define CONSTRAINED_PROBE_RESIDUAL_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Internal specialized diagnostic; not part of the stable public API.

   For N balanced zero-sum probes and an independent stationary baseline b,
   r = sum(y[i]-b). It is zero for an ideal affine plant. It is an aggregate
   consistency check, not fault attribution. Inputs contain at least count
   int32 values; count is 2..UINT16_MAX. All arithmetic fits int64_t. */
typedef enum {
    CP_RESIDUAL_OK = 0,
    CP_RESIDUAL_INVALID_ARGUMENT,
    CP_RESIDUAL_DUPLICATE_INDEX,
    CP_RESIDUAL_OUT_OF_ORDER,
    CP_RESIDUAL_INCOMPLETE,
    CP_RESIDUAL_BAD_STATE
} cp_residual_status_t;

typedef struct {
    uint16_t count;
    uint16_t next_index;
    int32_t baseline;
    int64_t residual_sum;
    uint8_t status;
    uint8_t active;
} cp_residual_state_t;

/* Initializes/restarts caller-owned state. Null returns INVALID_ARGUMENT.
   count<2 initializes inactive error state and returns INVALID_ARGUMENT. */
cp_residual_status_t cp_residual_begin(cp_residual_state_t *state,
                                       uint16_t count,
                                       int32_t baseline_response);
/* Must follow each expected index once in increasing order. A prior index is
   DUPLICATE_INDEX; a skipped/future/out-of-range index is OUT_OF_ORDER. A
   structural error ends the stream; later updates return BAD_STATE. */
cp_residual_status_t cp_residual_update(cp_residual_state_t *state,
                                        uint16_t probe_index,
                                        int32_t response);
/* Incomplete streams return INCOMPLETE. Result is written only on success.
   Successful finish ends the stream; a second finish returns BAD_STATE. */
cp_residual_status_t cp_residual_finish(cp_residual_state_t *state,
                                        int64_t *residual);

/* Batch equivalent; both pointers are required and must not overlap. Outputs
   are unchanged on failure. The response array has count elements. */
bool cp_sequence_residual(uint16_t count, const int32_t *probe_responses,
                          int32_t baseline_response, int64_t *residual);

#ifdef __cplusplus
}
#endif

#endif
