#include "constrained_probe/residual.h"
#include <stddef.h>

bool cp_sequence_residual(uint16_t count, const int32_t *probe_responses,
                          int32_t baseline_response, int64_t *residual)
{
    uint16_t i;
    int64_t sum = 0;
    if (count < 2 || probe_responses == NULL || residual == NULL) return false;
    for (i = 0; i < count; ++i) sum += probe_responses[i];
    *residual = sum - (int64_t)count * baseline_response;
    return true;
}
