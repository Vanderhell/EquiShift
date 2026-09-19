#ifndef CONSTRAINED_PROBE_TRANSFER_H
#define CONSTRAINED_PROBE_TRANSFER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Stable API. See docs/transfer_api.md for the caller contract. */

/* One zero-sum transfer moves step on channel and -step on reference. */
bool cp_transfer_validate(uint16_t count, uint16_t reference, int32_t step);
bool cp_transfer_delta(uint16_t count, uint16_t reference, uint16_t channel,
                       uint16_t axis, int32_t step, int32_t *out);

/* Signed delta range for source -= delta, destination += delta. The result is
   intersected with int32_t and both channels' [minimum, maximum] headroom. */
bool cp_transfer_safe_range(uint16_t count, uint16_t source, uint16_t destination,
                            const int32_t *values, const int32_t *minimum,
                            const int32_t *maximum, int64_t *minimum_delta,
                            int64_t *maximum_delta);

/* Apply exactly one nonzero transfer. No element is written on failure. */
bool cp_transfer_apply(uint16_t count, uint16_t source, uint16_t destination,
                       int32_t *values, const int32_t *minimum,
                       const int32_t *maximum, int32_t delta);

typedef struct {
    uint16_t source;
    uint16_t destination;
    int32_t delta;
} cp_transfer_operation_t;

/* Caller owns every array and the state object. The state object and all arrays
   must be disjoint; arrays must be disjoint from one another and
   remain valid for the sequence lifetime; baseline, limits, and operations must
   not be modified. Calls on one sequence must be serialized. Baseline stores
   the exact begin state. */
typedef struct {
    void *_reserved_mutable_pointers[2];
    const void *_reserved_input_pointers[3];
    uint16_t _reserved_counters[3];
    int32_t _reserved_status;
} cp_transfer_sequence_t;

/* Begin snapshots values. A sequence must contain 1..UINT16_MAX operations. */
bool cp_transfer_sequence_begin(cp_transfer_sequence_t *sequence,
                                uint16_t count, int32_t *values,
                                const int32_t *minimum, const int32_t *maximum,
                                int32_t *baseline,
                                const cp_transfer_operation_t *operations,
                                uint16_t operation_count);
/* expected_operation must equal next_operation; duplicate/out-of-order calls
   fail without changing sequence or channel values. */
bool cp_transfer_sequence_step(cp_transfer_sequence_t *sequence,
                               uint16_t expected_operation);
bool cp_transfer_sequence_finish(cp_transfer_sequence_t *sequence);
/* Restore is safe after any applied prefix, completion, failure, or abort.
   Abort restores immediately. Reset/power-loss recovery requires caller-managed
   persistence and rebinding; this API does not make hardware writes atomic. */
bool cp_transfer_sequence_restore(cp_transfer_sequence_t *sequence);
bool cp_transfer_sequence_abort(cp_transfer_sequence_t *sequence);

#ifdef __cplusplus
}
#endif

#endif
