#include "internal/residual.h"
#include <stddef.h>

cp_residual_status_t cp_residual_begin(cp_residual_state_t *state,
                                       uint16_t count,
                                       int32_t baseline_response)
{
    if (state == NULL) return CP_RESIDUAL_INVALID_ARGUMENT;
    state->count = count;
    state->next_index = 0;
    state->baseline = baseline_response;
    state->residual_sum = 0;
    state->status = (uint8_t)CP_RESIDUAL_OK;
    state->active = 1;
    if (count < 2) {
        state->status = (uint8_t)CP_RESIDUAL_INVALID_ARGUMENT;
        state->active = 0;
        return CP_RESIDUAL_INVALID_ARGUMENT;
    }
    return CP_RESIDUAL_OK;
}

cp_residual_status_t cp_residual_update(cp_residual_state_t *state,
                                        uint16_t probe_index,
                                        int32_t response)
{
    cp_residual_status_t status;
    if (state == NULL) return CP_RESIDUAL_INVALID_ARGUMENT;
    if (state->active == 0) return CP_RESIDUAL_BAD_STATE;
    status = (cp_residual_status_t)state->status;
    if (status != CP_RESIDUAL_OK) return status;
    if (probe_index < state->next_index) {
        state->status = (uint8_t)CP_RESIDUAL_DUPLICATE_INDEX;
        state->active = 0;
        return CP_RESIDUAL_DUPLICATE_INDEX;
    }
    if (probe_index != state->next_index || probe_index >= state->count) {
        state->status = (uint8_t)CP_RESIDUAL_OUT_OF_ORDER;
        state->active = 0;
        return CP_RESIDUAL_OUT_OF_ORDER;
    }
    /* Accumulate y_i-baseline directly. Each difference is in
       [-4294967295,4294967295], so UINT16_MAX terms fit in int64_t. This also
       avoids a 64-bit multiply on Cortex-M0. */
    state->residual_sum += (int64_t)response - (int64_t)state->baseline;
    ++state->next_index;
    return CP_RESIDUAL_OK;
}

cp_residual_status_t cp_residual_finish(cp_residual_state_t *state,
                                        int64_t *residual)
{
    cp_residual_status_t status;
    if (state == NULL || residual == NULL) return CP_RESIDUAL_INVALID_ARGUMENT;
    if (state->active == 0) {
        status = (cp_residual_status_t)state->status;
        return status == CP_RESIDUAL_OK ? CP_RESIDUAL_BAD_STATE : status;
    }
    status = (cp_residual_status_t)state->status;
    if (status != CP_RESIDUAL_OK) {
        state->active = 0;
        return status;
    }
    if (state->next_index != state->count) {
        state->status = (uint8_t)CP_RESIDUAL_INCOMPLETE;
        state->active = 0;
        return CP_RESIDUAL_INCOMPLETE;
    }
    *residual = state->residual_sum;
    state->active = 0;
    return CP_RESIDUAL_OK;
}

bool cp_sequence_residual(uint16_t count, const int32_t *probe_responses,
                          int32_t baseline_response, int64_t *residual)
{
    uint16_t i;
    int64_t sum = 0;
    if (count < 2 || probe_responses == NULL || residual == NULL) return false;
    for (i = 0; i < count; ++i)
        sum += (int64_t)probe_responses[i] - (int64_t)baseline_response;
    *residual = sum;
    return true;
}
