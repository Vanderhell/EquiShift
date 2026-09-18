#include "constrained_probe/transfer.h"
#include "constrained_probe/transfer_score.h"
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

static int64_t sum_values(const int32_t *v, uint16_t n)
{
    int64_t s=0;for(uint16_t i=0;i<n;i++)s+=v[i];return s;
}

static void check_apply(const int32_t *initial, const int32_t *lo, const int32_t *hi,
                        uint16_t n, uint16_t src, uint16_t dst, int32_t delta)
{
    int32_t v[8],before[8];
    for(uint16_t i=0;i<n;i++)v[i]=before[i]=initial[i];
    int64_t total=sum_values(v,n);
    assert(cp_transfer_apply(n,src,dst,v,lo,hi,delta));
    assert(sum_values(v,n)==total);
    for(uint16_t i=0;i<n;i++)if(i!=src&&i!=dst)assert(v[i]==before[i]);
    assert(v[src]==(int64_t)before[src]-delta);
    assert(v[dst]==(int64_t)before[dst]+delta);
    if(delta==INT32_MIN) {
        assert(cp_transfer_apply(n,src,dst,v,lo,hi,INT32_MAX));
        assert(cp_transfer_apply(n,src,dst,v,lo,hi,1));
    } else assert(cp_transfer_apply(n,src,dst,v,lo,hi,-delta));
    for(uint16_t i=0;i<n;i++)assert(v[i]==before[i]);
}

static void test_safe_limits_and_extremes(void)
{
    int32_t v[]={50,50},lo[]={0,0},hi[]={100,100};int64_t low,high;
    assert(cp_transfer_safe_range(2,0,1,v,lo,hi,&low,&high));
    assert(low==-50&&high==50);
    check_apply(v,lo,hi,2,0,1,50);
    check_apply(v,lo,hi,2,0,1,-50);
    assert(!cp_transfer_apply(2,0,1,v,lo,hi,51));
    assert(!cp_transfer_apply(2,0,1,v,lo,hi,-51));
    assert(!cp_transfer_apply(2,0,0,v,lo,hi,1));
    assert(!cp_transfer_apply(2,0,2,v,lo,hi,1));
    assert(!cp_transfer_apply(1,0,0,v,lo,hi,1));
    assert(!cp_transfer_apply(2,0,1,v,lo,hi,0));
    assert(!cp_transfer_safe_range(2,2,0,v,lo,hi,&low,&high));

    {
        int32_t e[]={INT32_MIN,0},emin[]={INT32_MIN,INT32_MIN},emax[]={INT32_MAX,INT32_MAX};
        assert(cp_transfer_safe_range(2,0,1,e,emin,emax,&low,&high));
        assert(low==INT32_MIN&&high==0);
        check_apply(e,emin,emax,2,0,1,INT32_MIN);
    }
    {
        int32_t e[]={INT32_MAX,0},emin[]={INT32_MIN,INT32_MIN},emax[]={INT32_MAX,INT32_MAX};
        assert(cp_transfer_safe_range(2,0,1,e,emin,emax,&low,&high));
        assert(low==0&&high==INT32_MAX);
        check_apply(e,emin,emax,2,0,1,INT32_MAX);
    }
    {
        int32_t badlo[]={101,0};
        assert(!cp_transfer_safe_range(2,0,1,v,badlo,hi,&low,&high));
        int32_t outside[]={101,50};
        assert(!cp_transfer_safe_range(2,0,1,outside,lo,hi,&low,&high));
        assert(!cp_transfer_safe_range(2,0,1,v,lo,hi,NULL,&high));
    }
}

static void test_exhaustive_small_domain(void)
{
    int32_t lo[4]={-2,-2,-2,-2},hi[4]={2,2,2,2};
    for(int a=-2;a<=2;a++)for(int b=-2;b<=2;b++)for(int c=-2;c<=2;c++)for(int d=-2;d<=2;d++) {
        int32_t v[4]={a,b,c,d};
        for(uint16_t s=0;s<4;s++)for(uint16_t t=0;t<4;t++)if(s!=t) {
            int64_t low,high;
            assert(cp_transfer_safe_range(4,s,t,v,lo,hi,&low,&high));
            for(int32_t delta=-4;delta<=4;delta++) {
                int32_t out[4];for(int i=0;i<4;i++)out[i]=v[i];
                bool feasible=delta!=0&&delta>=low&&delta<=high;
                bool ok=cp_transfer_apply(4,s,t,out,lo,hi,delta);
                assert(ok==feasible);
                if(ok) {
                    assert(sum_values(out,4)==sum_values(v,4));
                    assert(out[s]==v[s]-delta&&out[t]==v[t]+delta);
                    for(uint16_t i=0;i<4;i++)if(i!=s&&i!=t)assert(out[i]==v[i]);
                    assert(cp_transfer_apply(4,s,t,out,lo,hi,-delta));
                    for(uint16_t i=0;i<4;i++)assert(out[i]==v[i]);
                }
            }
        }
    }
}

static const cp_transfer_operation_t plan[]={
    {0,1,2},{2,3,-1},{1,2,1}
};

static void test_sequence(void)
{
    int32_t values[4]={5,5,5,5},baseline[4],lo[4]={0,0,0,0},hi[4]={10,10,10,10};
    cp_transfer_sequence_t seq;
    const int64_t initial_sum=sum_values(values,4);
    assert(cp_transfer_sequence_begin(&seq,4,values,lo,hi,baseline,plan,3));
    assert(!cp_transfer_sequence_finish(&seq));
    assert(!cp_transfer_sequence_step(&seq,1)); /* out of order */
    assert(cp_transfer_sequence_step(&seq,0));
    assert(sum_values(values,4)==initial_sum);
    int32_t after_first[]={3,7,5,5};
    for(int i=0;i<4;i++)assert(values[i]==after_first[i]);
    assert(!cp_transfer_sequence_step(&seq,0)); /* duplicate is inert */
    for(int i=0;i<4;i++)assert(values[i]==after_first[i]);
    assert(cp_transfer_sequence_step(&seq,1));
    assert(sum_values(values,4)==initial_sum);
    assert(cp_transfer_sequence_step(&seq,2));
    assert(sum_values(values,4)==initial_sum);
    assert(!cp_transfer_sequence_step(&seq,3));
    assert(cp_transfer_sequence_finish(&seq));
    assert(!cp_transfer_sequence_finish(&seq));
    assert(cp_transfer_sequence_restore(&seq));
    for(int i=0;i<4;i++)assert(values[i]==5);

    /* Abort after a prefix, then restore repeatedly without drift. */
    for(int repeat=0;repeat<1000;repeat++) {
        assert(cp_transfer_sequence_begin(&seq,4,values,lo,hi,baseline,plan,3));
        assert(cp_transfer_sequence_step(&seq,0));
        assert(cp_transfer_sequence_abort(&seq));
        for(int i=0;i<4;i++)assert(values[i]==5);
    }
    assert(!cp_transfer_sequence_restore(NULL));
}

static void test_failed_step_restore_and_replay(void)
{
    int32_t values[3]={5,5,5},baseline[4],lo[4]={0,0,0,0},hi[4]={10,10,10,10};
    const cp_transfer_operation_t invalid_path[]={{0,1,20}};
    cp_transfer_sequence_t seq;
    assert(cp_transfer_sequence_begin(&seq,3,values,lo,hi,baseline,invalid_path,1));
    assert(!cp_transfer_sequence_step(&seq,0));
    assert(values[0]==5&&values[1]==5&&values[2]==5);
    assert(cp_transfer_sequence_restore(&seq));
    for(int i=0;i<3;i++)assert(values[i]==5);
    assert(!cp_transfer_sequence_finish(&seq));

    {
        const cp_transfer_operation_t prefix_then_fail[]={{0,1,2},{2,1,20}};
        assert(cp_transfer_sequence_begin(&seq,3,values,lo,hi,baseline,prefix_then_fail,2));
        assert(cp_transfer_sequence_step(&seq,0));
        assert(!cp_transfer_sequence_step(&seq,1));
        assert(values[0]==3&&values[1]==7&&values[2]==5);
        assert(cp_transfer_sequence_restore(&seq));
        assert(values[0]==5&&values[1]==5&&values[2]==5);
    }

    {
        int32_t first[4]={5,5,5,5},second[4]={5,5,5,5},b1[4],b2[4];
        cp_transfer_sequence_t s1,s2;
        assert(cp_transfer_sequence_begin(&s1,4,first,lo,hi,b1,plan,3));
        assert(cp_transfer_sequence_begin(&s2,4,second,lo,hi,b2,plan,3));
        for(uint16_t i=0;i<3;i++) {
            assert(cp_transfer_sequence_step(&s1,i));assert(cp_transfer_sequence_step(&s2,i));
            for(int j=0;j<4;j++)assert(first[j]==second[j]);
        }
        assert(cp_transfer_sequence_finish(&s1)&&cp_transfer_sequence_finish(&s2));
        assert(cp_transfer_sequence_restore(&s1)&&cp_transfer_sequence_restore(&s2));
        for(int j=0;j<4;j++)assert(first[j]==second[j]&&first[j]==5);
    }

    {
        cp_transfer_operation_t bad[]={{0,0,1}};
        assert(!cp_transfer_sequence_begin(&seq,3,values,lo,hi,baseline,plan,0));
        assert(!cp_transfer_sequence_begin(&seq,1,values,lo,hi,baseline,plan,1));
        assert(!cp_transfer_sequence_begin(&seq,3,values,lo,hi,baseline,bad,1));
        bad[0].source=3;bad[0].destination=1;bad[0].delta=1;
        assert(!cp_transfer_sequence_begin(&seq,3,values,lo,hi,baseline,bad,1));
        bad[0].source=0;
        bad[0].destination=1;bad[0].delta=0;
        assert(!cp_transfer_sequence_begin(&seq,3,values,lo,hi,baseline,bad,1));
        assert(!cp_transfer_sequence_begin(NULL,3,values,lo,hi,baseline,plan,1));
        assert(!cp_transfer_sequence_begin(&seq,3,values,lo,hi,values,plan,1));
    }

    {
        int32_t extreme[2]={INT32_MIN,0},base[2],emin[2]={INT32_MIN,INT32_MIN};
        int32_t emax[2]={INT32_MAX,INT32_MAX};
        const cp_transfer_operation_t edge[]={{0,1,INT32_MIN}};
        assert(cp_transfer_sequence_begin(&seq,2,extreme,emin,emax,base,edge,1));
        assert(cp_transfer_sequence_step(&seq,0));
        assert(extreme[0]==0&&extreme[1]==INT32_MIN);
        assert(cp_transfer_sequence_finish(&seq));
        assert(cp_transfer_sequence_restore(&seq));
        assert(extreme[0]==INT32_MIN&&extreme[1]==0);
    }
}

int main(void)
{
    test_safe_limits_and_extremes();
    test_exhaustive_small_domain();
    test_sequence();
    test_failed_step_restore_and_replay();
    return 0;
}
