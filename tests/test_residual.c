#include "constrained_probe/residual.h"
#include <assert.h>
#include <stdint.h>

int main(void)
{
    int32_t linear[]={13,9,7,11};int64_t r;
    assert(cp_sequence_residual(4,linear,10,&r)&&r==0);
    linear[2]+=5;assert(cp_sequence_residual(4,linear,10,&r)&&r==5);
    assert(!cp_sequence_residual(1,linear,10,&r));
    assert(!cp_sequence_residual(4,0,10,&r));
    assert(!cp_sequence_residual(4,linear,10,0));
    return 0;
}
