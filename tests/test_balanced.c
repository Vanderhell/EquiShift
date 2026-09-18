#include "constrained_probe/balanced.h"
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stddef.h>

int main(void)
{
    bp_config_t cfg = {3, 2};
    int32_t d, old_change, new_change, responses[] = {11, 17, 23}, scores[3];
    uint16_t old_axis, new_axis;
    int64_t wide[3];
    bp_stream_t stream;
    bp_decision_t decision;
    int32_t x[] = {50,50,50}, lo[] = {0,0,0}, hi[] = {100,100,100}, safe;

    assert(!bp_validate(NULL));
    { bp_config_t bad={1,1}; assert(!bp_validate(&bad)); bad.count=2;bad.step=0;assert(!bp_validate(&bad)); }
    assert(bp_validate(&cfg));
    assert(bp_delta(&cfg, 0, 0, &d) && d == 4);
    assert(bp_delta(&cfg, 0, 1, &d) && d == -2);
    assert(!bp_delta(NULL,0,0,&d));assert(!bp_delta(&cfg,3,0,&d));
    assert(!bp_delta(&cfg,0,3,&d));assert(!bp_delta(&cfg,0,0,NULL));
    assert(bp_score(&cfg, responses, scores));
    assert(scores[0] == -18 && scores[1] == 0 && scores[2] == 18);
    assert(bp_score_i64(&cfg, responses, wide));
    for (int i=0; i<3; ++i) assert(wide[i] == scores[i]);
    { int32_t sum;assert(bp_response_sum(&cfg,responses,&sum)&&sum==51);
      assert(!bp_response_sum(NULL,responses,&sum));assert(!bp_response_sum(&cfg,NULL,&sum));
      assert(!bp_response_sum(&cfg,responses,NULL)); }
    assert(!bp_score(NULL,responses,scores));assert(!bp_score(&cfg,NULL,scores));
    assert(!bp_score(&cfg,responses,NULL));
    assert(bp_max_safe_step(3, 0, x, lo, hi, 100, &safe) && safe == 25);
    assert(bp_transition(3, 0, 1, 2, &old_axis, &old_change, &new_axis, &new_change));
    assert(old_axis == 0 && old_change == -6 && new_axis == 1 && new_change == 6);
    assert(bp_stream_begin(&stream, 3));
    assert(bp_stream_update(&stream, 4));
    assert(bp_stream_update(&stream, -1));
    assert(bp_stream_update(&stream, INT32_MIN));
    assert(bp_stream_finish(&stream, &decision));
    assert(decision.best_index == 0 && decision.worst_index == 2);
    return 0;
}
