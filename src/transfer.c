#include "constrained_probe/transfer.h"
#include <limits.h>
#include <stddef.h>

typedef enum {
    CP_SEQ_IDLE = 0,
    CP_SEQ_ACTIVE,
    CP_SEQ_COMPLETE,
    CP_SEQ_FAILED,
    CP_SEQ_ABORTED,
    CP_SEQ_RESTORED
} cp_seq_status_t;

#define SEQ_VALUES(s) ((int32_t *)(s)->_reserved_pointers[0])
#define SEQ_BASELINE(s) ((int32_t *)(s)->_reserved_pointers[1])
#define SEQ_MINIMUM(s) ((const int32_t *)(s)->_reserved_pointers[2])
#define SEQ_MAXIMUM(s) ((const int32_t *)(s)->_reserved_pointers[3])
#define SEQ_OPERATIONS(s) ((const cp_transfer_operation_t *)(s)->_reserved_pointers[4])
#define SEQ_COUNT(s) ((s)->_reserved_counters[0])
#define SEQ_OPERATION_COUNT(s) ((s)->_reserved_counters[1])
#define SEQ_NEXT(s) ((s)->_reserved_counters[2])

bool cp_transfer_validate(uint16_t count, uint16_t reference, int32_t step)
{
    return count >= 2 && reference < count && step != 0 && step != INT32_MIN;
}

bool cp_transfer_delta(uint16_t count, uint16_t reference, uint16_t channel,
                       uint16_t axis, int32_t step, int32_t *out)
{
    if (!cp_transfer_validate(count, reference, step) || channel >= count ||
        channel == reference || axis >= count || out == NULL) return false;
    if (axis == channel) *out = step;
    else if (axis == reference) *out = (int32_t)-(int64_t)step;
    else *out = 0;
    return true;
}

bool cp_transfer_safe_range(uint16_t count, uint16_t source, uint16_t destination,
                           const int32_t *values, const int32_t *minimum,
                           const int32_t *maximum, int64_t *minimum_delta,
                           int64_t *maximum_delta)
{
    int64_t low, high, candidate;
    if (count < 2 || source >= count || destination >= count || source == destination ||
        values == NULL || minimum == NULL || maximum == NULL ||
        minimum_delta == NULL || maximum_delta == NULL || minimum_delta == maximum_delta) return false;
    if (minimum[source] > maximum[source] || minimum[destination] > maximum[destination] ||
        values[source] < minimum[source] || values[source] > maximum[source] ||
        values[destination] < minimum[destination] || values[destination] > maximum[destination])
        return false;

    /* source' = source-delta; destination' = destination+delta. */
    low = (int64_t)values[source] - maximum[source];
    candidate = (int64_t)minimum[destination] - values[destination];
    if (candidate > low) low = candidate;
    if (low < INT32_MIN) low = INT32_MIN;

    high = (int64_t)values[source] - minimum[source];
    candidate = (int64_t)maximum[destination] - values[destination];
    if (candidate < high) high = candidate;
    if (high > INT32_MAX) high = INT32_MAX;

    if (low > high) return false;
    *minimum_delta = low;
    *maximum_delta = high;
    return true;
}

bool cp_transfer_apply(uint16_t count, uint16_t source, uint16_t destination,
                       int32_t *values, const int32_t *minimum,
                       const int32_t *maximum, int32_t delta)
{
    int64_t source_after, destination_after;
    if (count < 2 || source >= count || destination >= count || source == destination ||
        values == NULL || minimum == NULL || maximum == NULL || delta == 0 ||
        minimum[source] > maximum[source] || minimum[destination] > maximum[destination] ||
        values[source] < minimum[source] || values[source] > maximum[source] ||
        values[destination] < minimum[destination] || values[destination] > maximum[destination])
        return false;
    source_after = (int64_t)values[source] - delta;
    destination_after = (int64_t)values[destination] + delta;
    if (source_after < minimum[source] || source_after > maximum[source] ||
        destination_after < minimum[destination] || destination_after > maximum[destination] ||
        source_after < INT32_MIN || source_after > INT32_MAX ||
        destination_after < INT32_MIN || destination_after > INT32_MAX) return false;
    values[source] = (int32_t)source_after;
    values[destination] = (int32_t)destination_after;
    return true;
}

bool cp_transfer_sequence_begin(cp_transfer_sequence_t *sequence,
                                uint16_t count, int32_t *values,
                                const int32_t *minimum, const int32_t *maximum,
                                int32_t *baseline,
                                const cp_transfer_operation_t *operations,
                                uint16_t operation_count)
{
    uint16_t i;
    if (sequence == NULL || count < 2 || operation_count == 0 || values == NULL ||
        minimum == NULL || maximum == NULL || baseline == NULL || operations == NULL ||
        values == baseline) return false;
    for (i = 0; i < count; ++i)
        if (minimum[i] > maximum[i] || values[i] < minimum[i] || values[i] > maximum[i]) return false;
    for (i = 0; i < operation_count; ++i)
        if (operations[i].source >= count || operations[i].destination >= count ||
            operations[i].source == operations[i].destination || operations[i].delta == 0) return false;

    for (i = 0; i < count; ++i) baseline[i] = values[i];
    sequence->_reserved_pointers[0] = values;
    sequence->_reserved_pointers[1] = baseline;
    sequence->_reserved_pointers[2] = (void *)minimum;
    sequence->_reserved_pointers[3] = (void *)maximum;
    sequence->_reserved_pointers[4] = (void *)operations;
    SEQ_COUNT(sequence) = count;
    SEQ_OPERATION_COUNT(sequence) = operation_count;
    SEQ_NEXT(sequence) = 0;
    sequence->_reserved_status = CP_SEQ_ACTIVE;
    return true;
}

bool cp_transfer_sequence_step(cp_transfer_sequence_t *sequence,
                               uint16_t expected_operation)
{
    const cp_transfer_operation_t *op;
    uint16_t next;
    if (sequence == NULL || sequence->_reserved_status != CP_SEQ_ACTIVE) return false;
    next = SEQ_NEXT(sequence);
    if (expected_operation != next || next >= SEQ_OPERATION_COUNT(sequence)) return false;
    op = &SEQ_OPERATIONS(sequence)[next];
    if (!cp_transfer_apply(SEQ_COUNT(sequence), op->source, op->destination,
                           SEQ_VALUES(sequence), SEQ_MINIMUM(sequence), SEQ_MAXIMUM(sequence),
                           op->delta)) {
        sequence->_reserved_status = CP_SEQ_FAILED;
        return false;
    }
    SEQ_NEXT(sequence) = (uint16_t)(next + 1u);
    return true;
}

bool cp_transfer_sequence_finish(cp_transfer_sequence_t *sequence)
{
    if (sequence == NULL || sequence->_reserved_status != CP_SEQ_ACTIVE ||
        SEQ_NEXT(sequence) != SEQ_OPERATION_COUNT(sequence)) return false;
    sequence->_reserved_status = CP_SEQ_COMPLETE;
    return true;
}

bool cp_transfer_sequence_restore(cp_transfer_sequence_t *sequence)
{
    uint16_t i;
    if (sequence == NULL || sequence->_reserved_status == CP_SEQ_IDLE ||
        SEQ_VALUES(sequence) == NULL || SEQ_BASELINE(sequence) == NULL || SEQ_COUNT(sequence) < 2)
        return false;
    for (i = 0; i < SEQ_COUNT(sequence); ++i)
        SEQ_VALUES(sequence)[i] = SEQ_BASELINE(sequence)[i];
    sequence->_reserved_status = CP_SEQ_RESTORED;
    return true;
}

bool cp_transfer_sequence_abort(cp_transfer_sequence_t *sequence)
{
    if (sequence == NULL || (sequence->_reserved_status != CP_SEQ_ACTIVE &&
        sequence->_reserved_status != CP_SEQ_FAILED &&
        sequence->_reserved_status != CP_SEQ_COMPLETE)) return false;
    if (!cp_transfer_sequence_restore(sequence)) return false;
    sequence->_reserved_status = CP_SEQ_ABORTED;
    return true;
}
