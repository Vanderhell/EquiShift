#include "balanced.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>

static void check(uint16_t n, int32_t q)
{
    bp_config_t cfg={n,q}; int32_t d[32][32];
    assert(bp_validate(&cfg));
    for(uint16_t i=0;i<n;i++) for(uint16_t j=0;j<n;j++) assert(bp_delta(&cfg,i,j,&d[i][j]));
    for(uint16_t i=0;i<n;i++) {
        int64_t sum=0,norm=0;
        for(uint16_t j=0;j<n;j++){sum+=d[i][j];norm+=(int64_t)d[i][j]*d[i][j];}
        assert(sum==0 && norm==(int64_t)n*(n-1)*q*q);
        for(uint16_t k=i+1;k<n;k++) {
            int64_t dot=0,dist=0;
            for(uint16_t j=0;j<n;j++) {
                dot+=(int64_t)d[i][j]*d[k][j];
                dist+=(int64_t)(d[i][j]-d[k][j])*(d[i][j]-d[k][j]);
            }
            assert(dot==-(int64_t)n*q*q && dist==2*(int64_t)n*n*q*q);
        }
    }
    for(uint16_t j=0;j<n;j++){int64_t s=0;for(uint16_t i=0;i<n;i++)s+=d[i][j];assert(s==0);}
    /* Numerical rank in test code only. */
    {
        double a[32][32]; uint16_t row=0;
        for(uint16_t i=0;i<n;i++)for(uint16_t j=0;j<n;j++)a[i][j]=d[i][j];
        for(uint16_t col=0;col<n;col++) {
            uint16_t p=row;
            for(uint16_t i=row;i<n;i++)if(fabs(a[i][col])>fabs(a[p][col]))p=i;
            if(fabs(a[p][col])<1e-9)continue;
            for(uint16_t j=col;j<n;j++){double t=a[row][j];a[row][j]=a[p][j];a[p][j]=t;}
            for(uint16_t i=row+1;i<n;i++){double f=a[i][col]/a[row][col];for(uint16_t j=col;j<n;j++)a[i][j]-=f*a[row][j];}
            row++;
        }
        assert(row==n-1);
    }
}

int main(void)
{
    for(uint16_t n=2;n<=32;n++){check(n,1);check(n,-1);}
    {
        const uint16_t n=7; const int32_t q=3; bp_config_t cfg={n,q};
        const int32_t z[7]={-9,-3,0,3,6,9,-6};
        for(uint16_t j=0;j<n;j++) {
            double reconstructed=0;
            for(uint16_t i=0;i<n;i++){int32_t d;assert(bp_delta(&cfg,i,j,&d));reconstructed+=(double)z[i]/(n*q)*d;}
            assert(fabs(reconstructed-z[j])<1e-9);
        }
    }
    return 0;
}
