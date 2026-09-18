#include "constrained_probe/balanced.h"
#include "constrained_probe/transfer.h"
#include "constrained_probe/transfer_score.h"
#include "constrained_probe/residual.h"
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stddef.h>

int main(void)
{
    bp_config_t cfg={2,INT32_MAX};int32_t d;
    assert(bp_validate(&cfg));assert(bp_delta(&cfg,0,0,&d)&&d==INT32_MAX);
    assert(bp_delta(&cfg,0,1,&d)&&d==-INT32_MAX);
    cfg.count=3;assert(!bp_validate(&cfg));
    cfg.step=INT32_MAX/2+1;assert(!bp_validate(&cfg));
    cfg.step=INT32_MAX/2;assert(bp_validate(&cfg));
    cfg.count=2;cfg.step=INT32_MIN;assert(!bp_validate(&cfg));
    cfg.step=0;assert(!bp_validate(&cfg));
    cfg.count=1;cfg.step=1;assert(!bp_validate(&cfg));
    {
        int32_t y[]={INT32_MAX,INT32_MIN,INT32_MAX};int64_t scores[3];int32_t narrow[3];
        bp_config_t c={3,1};assert(bp_score_i64(&c,y,scores));assert(!bp_score(&c,y,narrow));
    }
    assert(cp_transfer_validate(2,0,INT32_MAX));
    assert(!cp_transfer_validate(2,0,INT32_MIN));
    assert(!cp_transfer_validate(1,0,1));
    assert(!cp_transfer_delta(3,0,1,1,1,NULL));
    {
        static int32_t y[UINT16_MAX];int64_t r;
        for(uint32_t i=0;i<UINT16_MAX;i++)y[i]=INT32_MAX;
        assert(cp_sequence_residual(UINT16_MAX,y,INT32_MIN,&r));
        assert(r==(int64_t)UINT16_MAX*((int64_t)INT32_MAX-INT32_MIN));
    }
    return 0;
}
