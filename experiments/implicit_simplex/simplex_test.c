#include "simplex.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>

static void test_invariants(void)
{
    for (uint32_t nn = 0; nn <= 255; ++nn) {
        sx_dim_t n = (sx_dim_t)nn;
        uint64_t sum[256] = {0};
        assert((uint32_t)n + 1u == nn + 1u);
        for (sx_vertex_t i = 0; i <= n; ++i) {
            assert(sx_valid_vertex(n, i));
            assert(!sx_valid_vertex(n, (sx_vertex_t)(n + 1u)));
            for (sx_dim_t j = 0; j <= n; ++j) {
                sx_coord_t x = sx_coord(n, i, j);
                assert(x == (i == j ? (sx_coord_t)n : -1));
                sum[j] += (int64_t)x;
            }
            assert((uint64_t)sx_dot(n, i, i) == sx_norm2(n));
            assert(sx_distance2(n, i, i) == 0);
            for (sx_vertex_t j = (sx_vertex_t)(i + 1u); j <= n; ++j) {
                assert(sx_dot(n, i, j) == -(int64_t)(n + 1u));
                assert(sx_distance2(n, i, j) == 2u * (uint64_t)(n + 1u) * (n + 1u));
            }
        }
        for (sx_dim_t j = 0; j <= n; ++j) assert(sum[j] == 0);
    }
}

static void test_expansion(void)
{
    sx_dim_t n = 0;
    for (unsigned expected = 1; expected <= 65535; ++expected) {
        assert(n == expected - 1);
        if (expected == 65535) {
            assert(sx_expand(&n));
            break;
        }
        assert(sx_expand(&n));
    }
    assert(!sx_expand(&n));
}

static void benchmark(void)
{
    volatile sx_dim_t n = 0;
    const uint64_t iterations = 100000000;
    clock_t start = clock();
    for (uint64_t i = 0; i < iterations; ++i) {
        n = (sx_dim_t)(i & UINT16_MAX); /* models n -> n+1 without storage moves */
        (void)sx_coord((sx_dim_t)n, (sx_vertex_t)(i & UINT16_MAX), (sx_dim_t)(i & UINT16_MAX));
    }
    double seconds = (double)(clock() - start) / (double)CLOCKS_PER_SEC;
    printf("benchmark: %llu coordinate/implicit-state operations in %.3f s (final n=%u)\n",
           (unsigned long long)iterations, seconds, (unsigned)n);
}

int main(void)
{
    test_invariants();
    test_expansion();
    benchmark();
    puts("all tests passed");
    return 0;
}
