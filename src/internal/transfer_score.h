#ifndef CONSTRAINED_PROBE_TRANSFER_SCORE_H
#define CONSTRAINED_PROBE_TRANSFER_SCORE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Internal optional non-hot-path decoder; not stable public API. Requires
   count>=2, reference<count, and count-element non-overlapping response and
   output arrays. responses[reference] is ignored. Scores are int64, retain
   response scaling, and sum to zero. Output is unchanged on failure. */
bool cp_transfer_score(uint16_t count, uint16_t reference, int32_t baseline,
                       const int32_t *responses, int64_t *scores);

#ifdef __cplusplus
}
#endif

#endif
