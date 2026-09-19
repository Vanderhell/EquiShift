#include "internal/transfer_score.h"
#include <stddef.h>

bool cp_transfer_score(uint16_t count, uint16_t reference, int32_t baseline,
                       const int32_t *responses, int64_t *scores)
{
    uint16_t i;
    int64_t total = 0;
    if (count < 2 || reference >= count || responses == NULL || scores == NULL) return false;
    for (i = 0; i < count; ++i)
        if (i != reference) total += (int64_t)responses[i] - baseline;
    for (i = 0; i < count; ++i) {
        int64_t relative = i == reference ? 0 : (int64_t)responses[i] - baseline;
        scores[i] = (int64_t)count * relative - total;
    }
    return true;
}
