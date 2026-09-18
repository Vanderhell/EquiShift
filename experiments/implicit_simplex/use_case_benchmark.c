#include "simplex.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define MAX_N 255
#define REPS 20000u

static int32_t explicit_vertices[MAX_N + 1][MAX_N + 1];
static int32_t input_vector[MAX_N + 1];
static volatile uint32_t rng_state = 0x13579bdfu;
static volatile int32_t benchmark_sink;

static uint32_t next_random(void)
{
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}

static void fill_input(uint16_t n)
{
    for (uint16_t i = 0; i <= n; ++i)
        input_vector[i] = (int32_t)(next_random() % 2001u) - 1000;
}

static void build_explicit(uint16_t n)
{
    for (uint16_t i = 0; i <= n; ++i)
        for (uint16_t j = 0; j <= n; ++j)
            explicit_vertices[i][j] = sx_coord(n, i, j);
}

__attribute__((noinline)) static uint16_t classify_explicit(uint16_t n, const int32_t *x)
{
    int64_t best_score = INT64_MIN;
    uint16_t best = 0;
    for (uint16_t i = 0; i <= n; ++i) {
        int64_t score = 0;
        for (uint16_t j = 0; j <= n; ++j)
            score += (int64_t)x[j] * explicit_vertices[i][j];
        if (score > best_score) { best_score = score; best = i; }
    }
    return best;
}

__attribute__((noinline)) static uint16_t classify_implicit(uint16_t n, const int32_t *x)
{
    int64_t best_score = INT64_MIN;
    uint16_t best = 0;
    for (uint16_t i = 0; i <= n; ++i) {
        int64_t score = 0;
        for (uint16_t j = 0; j <= n; ++j)
            score += (int64_t)x[j] * sx_coord(n, i, j);
        if (score > best_score) { best_score = score; best = i; }
    }
    return best;
}

__attribute__((noinline)) static uint16_t classify_reduced(uint16_t n, const int32_t *x)
{
    int32_t best_value = INT32_MIN;
    uint16_t best = 0;
    for (uint16_t i = 0; i <= n; ++i) {
        if (x[i] > best_value) { best_value = x[i]; best = i; }
    }
    return best;
}

__attribute__((noinline)) static uint16_t classify_reduced_with_margin(
    uint16_t n, const int32_t *x, int32_t *margin)
{
    int32_t best = INT32_MIN, second = INT32_MIN;
    uint16_t index = 0;
    for (uint16_t i = 0; i <= n; ++i) {
        if (x[i] > best) { second = best; best = x[i]; index = i; }
        else if (x[i] > second) second = x[i];
    }
    *margin = best - second;
    return index;
}

static double measure(uint16_t n, int variant)
{
    clock_t start = clock();
    for (unsigned r = 0; r < REPS; ++r) {
        fill_input(n);
        uint16_t result = variant == 0 ? classify_explicit(n, input_vector) :
                          variant == 1 ? classify_implicit(n, input_vector) :
                                         classify_reduced(n, input_vector);
        benchmark_sink += result;
    }
    return (double)(clock() - start) / (double)CLOCKS_PER_SEC * 1e9 / REPS;
}

int main(void)
{
    static const uint16_t dimensions[] = {2, 3, 4, 7, 15, 31, 63, 127, 255};
    puts("N,explicit_ns,implicit_generic_ns,reduced_ns,explicit_bytes,model_bytes");
    for (size_t k = 0; k < sizeof(dimensions)/sizeof(dimensions[0]); ++k) {
        uint16_t n = dimensions[k];
        build_explicit(n);
        for (unsigned check = 0; check < 100; ++check) {
            fill_input(n);
            uint16_t a_check = classify_explicit(n, input_vector);
            uint16_t b_check = classify_implicit(n, input_vector);
            uint16_t c_check = classify_reduced(n, input_vector);
            if (a_check != b_check || b_check != c_check) {
                fprintf(stderr, "classification mismatch at N=%u\n", (unsigned)(n + 1));
                return 1;
            }
        }
        double a = measure(n, 0);
        double b = measure(n, 1);
        double c = measure(n, 2);
        int32_t margin = 0;
        fill_input(n);
        uint16_t best = classify_reduced_with_margin(n, input_vector, &margin);
        printf("%u,%.1f,%.1f,%.1f,%llu,%zu (best=%u margin=%ld)\n",
               (unsigned)(n + 1), a, b, c,
               (unsigned long long)(n + 1u) * (n + 1u) * sizeof(int32_t),
               sizeof(uint16_t) * 2u, (unsigned)best, (long)margin);
    }
    printf("sink=%u\n", (unsigned)benchmark_sink);
    return 0;
}
