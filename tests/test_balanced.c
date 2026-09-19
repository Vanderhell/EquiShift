#include "balanced.h"

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

int main(void)
{
    bp_config_t cfg = {3, 2};
    int32_t d, y[] = {11, 17, 23};
    int64_t score[3], replay[3];
    assert(!bp_validate(NULL));
    assert(!bp_validate(&(bp_config_t){0, 1}));
    assert(!bp_validate(&(bp_config_t){1, 1}));
    assert(!bp_validate(&(bp_config_t){2, 0}));
    assert(bp_validate(&cfg));
    assert(bp_delta(&cfg, 0, 0, &d) && d == 4);
    assert(bp_delta(&cfg, 0, 1, &d) && d == -2);
    assert(!bp_delta(&cfg, 3, 0, &d));
    assert(!bp_delta(&cfg, 0, 3, &d));
    assert(!bp_delta(&cfg, 0, 0, NULL));
    assert(!bp_delta(NULL, 0, 0, &d));
    assert(bp_score_i64(&cfg, y, score));
    assert(score[0] == -18 && score[1] == 0 && score[2] == 18);
    assert(bp_score_i64(&cfg, y, replay));
    for (unsigned i = 0; i < 3; ++i) assert(score[i] == replay[i]);
    assert(!bp_score_i64(&cfg, NULL, score));
    assert(!bp_score_i64(&cfg, y, NULL));

    /* Largest representable positive/negative step for N=2. */
    cfg = (bp_config_t){2, INT32_MAX};
    assert(bp_validate(&cfg));
    assert(bp_delta(&cfg, 0, 0, &d) && d == INT32_MAX);
    assert(bp_delta(&cfg, 0, 1, &d) && d == -INT32_MAX);
    cfg.step = -INT32_MAX;
    assert(bp_validate(&cfg));
    assert(bp_delta(&cfg, 0, 0, &d) && d == -INT32_MAX);
    cfg.step = INT32_MIN;
    assert(!bp_validate(&cfg));
    cfg = (bp_config_t){3, INT32_MAX / 2};
    assert(bp_validate(&cfg));
    cfg.step = INT32_MAX / 2 + 1;
    assert(!bp_validate(&cfg));
    cfg = (bp_config_t){UINT16_MAX, 32769};
    assert(bp_validate(&cfg));
    assert(bp_delta(&cfg, 0, 0, &d) && d == INT32_C(2147483646));
    assert(bp_delta(&cfg, 0, 1, &d) && d == -32769);
    cfg.step = 32770;
    assert(!bp_validate(&cfg));

    /* Full count and int32 response range stay inside int64 arithmetic. */
    {
        static int32_t many_y[UINT16_MAX];
        static int64_t many_scores[UINT16_MAX];
        bp_config_t many = {UINT16_MAX, 1};
        int64_t total = INT32_MIN + (int64_t)(UINT16_MAX - 1u) * INT32_MAX;
        many_y[0] = INT32_MIN;
        for (uint32_t i = 1; i < UINT16_MAX; ++i) many_y[i] = INT32_MAX;
        assert(bp_score_i64(&many, many_y, many_scores));
        assert(many_scores[0] == (int64_t)UINT16_MAX * INT32_MIN - total);
    }

    /* Extreme response values remain safe in the wide decoder. */
    {
        const int32_t extreme[] = {INT32_MIN, INT32_MAX};
        int64_t wide[2];
        const bp_config_t pair = {2, 1};
        assert(bp_score_i64(&pair, extreme, wide));
        assert(wide[0] == -INT64_C(4294967295));
        assert(wide[1] == INT64_C(4294967295));
    }
    return 0;
}
