#ifndef CONSTRAINED_PROBE_TRANSFER_SCORE_H
#define CONSTRAINED_PROBE_TRANSFER_SCORE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Optional non-hot-path decoder. Responses are indexed by channel;
   responses[reference] is ignored. Scores retain response scaling and sum to zero. */
bool cp_transfer_score(uint16_t count, uint16_t reference, int32_t baseline,
                       const int32_t *responses, int64_t *scores);

#ifdef __cplusplus
}
#endif

#endif
