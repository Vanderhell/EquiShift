#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define N 8u
#define REPS 100000u

typedef struct {
    uint16_t best_index;
    int32_t best;
    int32_t second;
    int32_t margin;
} decision_t;

static volatile uint32_t rng_state = 0x6d2b79f5u;
static volatile uint32_t sink;

static uint32_t next_u32(void)
{
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}

static int32_t noise(int32_t amplitude)
{
    return (int32_t)(next_u32() % (uint32_t)(2 * amplitude + 1)) - amplitude;
}

/* Main embedded baseline: one pass, O(1) state, no multiplication/division. */
__attribute__((noinline)) static decision_t best_second(const int32_t *x, uint16_t count)
{
    decision_t d = {0, INT32_MIN, INT32_MIN, 0};
    for (uint16_t i = 0; i < count; ++i) {
        if (x[i] > d.best) {
            d.second = d.best;
            d.best = x[i];
            d.best_index = i;
        } else if (x[i] > d.second) {
            d.second = x[i];
        }
    }
    d.margin = d.best - d.second;
    return d;
}

__attribute__((noinline)) static uint16_t best_only(const int32_t *x, uint16_t count)
{
    int32_t best = INT32_MIN;
    uint16_t index = 0;
    for (uint16_t i = 0; i < count; ++i) {
        if (x[i] > best) { best = x[i]; index = i; }
    }
    return index;
}

/* Same information, with the simplex boundary metric in integer-scaled form. */
__attribute__((noinline)) static decision_t simplex_confidence(
    const int32_t *x, uint16_t count, uint64_t *boundary_metric2)
{
    decision_t d = best_second(x, count);
    /* distance_to_boundary = margin/sqrt(2); compare its square without sqrt. */
    *boundary_metric2 = (uint64_t)(uint32_t)d.margin * (uint64_t)(uint32_t)d.margin;
    return d;
}

static void make_input(int32_t *x, unsigned mode, int32_t amplitude)
{
    for (unsigned i = 0; i < N; ++i) x[i] = 0;
    x[3] = 1000;
    if (mode == 0) { /* uniform independent noise */
        for (unsigned i = 0; i < N; ++i) x[i] += noise(amplitude);
    } else if (mode == 1) { /* sum of uniforms: Gaussian-like, no distribution library */
        for (unsigned i = 0; i < N; ++i) {
            int32_t total = 0;
            for (unsigned k = 0; k < 6; ++k) total += noise(amplitude);
            x[i] += total;
        }
    } else if (mode == 2) { /* one competing component receives a spike */
        unsigned competitor = next_u32() % (N - 1u);
        if (competitor >= 3u) ++competitor;
        x[competitor] += amplitude;
    } else { /* several components are perturbed */
        for (unsigned i = 0; i < N; ++i) x[i] += noise(amplitude);
    }
}

static void noise_experiment(void)
{
    const int32_t amplitudes[] = {25, 100, 250, 500, 750, 1000, 1250};
    int32_t x[N];
    puts("noise_mode,amplitude,error_rate,reject_1000,reject_250,mean_margin,margin_error_corr");
    for (unsigned mode = 0; mode < 4; ++mode) {
        for (unsigned a = 0; a < sizeof(amplitudes)/sizeof(amplitudes[0]); ++a) {
            uint32_t errors = 0, reject1000 = 0, reject250 = 0;
            uint64_t margin_sum = 0, margin_sq_sum = 0, error_margin_sum = 0;
            for (unsigned r = 0; r < 10000; ++r) {
                make_input(x, mode, amplitudes[a]);
                decision_t d = best_second(x, N);
                errors += d.best_index != 3;
                reject1000 += d.margin < 1000;
                reject250 += d.margin < 250;
                margin_sum += (uint32_t)d.margin;
                margin_sq_sum += (uint64_t)(uint32_t)d.margin * (uint32_t)d.margin;
                error_margin_sum += d.best_index != 3 ? (uint32_t)d.margin : 0u;
            }
            double mean_m = margin_sum / 10000.0;
            double mean_e = errors / 10000.0;
            double var_m = margin_sq_sum / 10000.0 - mean_m * mean_m;
            double corr = var_m > 0.0 && mean_e > 0.0 && mean_e < 1.0
                        ? (error_margin_sum / 10000.0 - mean_m * mean_e) /
                          sqrt(var_m * mean_e * (1.0 - mean_e)) : 0.0;
            printf("%u,%ld,%.4f,%.4f,%.4f,%.1f,%.4f\n", mode, (long)amplitudes[a],
                   errors / 10000.0, reject1000 / 10000.0,
                   reject250 / 10000.0, mean_m, corr);
        }
    }
}

static double bench(unsigned variant)
{
    int32_t x[N];
    clock_t begin = clock();
    for (unsigned r = 0; r < REPS; ++r) {
        make_input(x, 0, 250);
        if (variant == 0) {
            sink += best_only(x, N);
        } else if (variant == 1) {
            decision_t d = best_second(x, N);
            sink += d.best_index + (uint32_t)d.margin;
        } else if (variant == 2) {
            uint64_t metric = 0;
            decision_t d = simplex_confidence(x, N, &metric);
            sink += d.best_index + (uint32_t)metric;
        } else {
            decision_t d = best_second(x, N);
            sink += d.margin >= 250 ? d.best_index : 255u;
        }
    }
    return (double)(clock() - begin) * 1e9 / (double)CLOCKS_PER_SEC / REPS;
}

int main(void)
{
    printf("best_only_ns=%.1f best_second_ns=%.1f simplex_metric_ns=%.1f reject_ns=%.1f sink=%u\n",
           bench(0), bench(1), bench(2), bench(3), (unsigned)sink);
    noise_experiment();
    return 0;
}
