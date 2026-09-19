#include "constrained_probe/transfer.h"
#include "transfer_score.h"
#include "residual.h"
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stddef.h>

int main(void)
{
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
