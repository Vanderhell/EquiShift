#include "balanced.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>

// Simple benchmark for basic operations
static void benchmark_delta()
{
    bp_config_t cfg = {10, 100};
    int32_t delta;
    clock_t start, end;
    volatile uint64_t sum = 0;
    
    start = clock();
    for (int i = 0; i < 1000000; i++)
    {
        bp_delta(&cfg, i % cfg.count, i % cfg.count, &delta);
        sum += delta;
    }
    end = clock();
    
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Delta benchmark: %.6f seconds for 1,000,000 operations\n", time_taken);
}

static void benchmark_score()
{
    bp_config_t cfg = {10, 100};
    int32_t responses[10] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    int64_t scores[10];
    clock_t start, end;
    volatile uint64_t sum = 0;
    
    start = clock();
    for (int i = 0; i < 1000000; i++)
    {
        bp_score_i64(&cfg, responses, scores);
        sum += scores[0];
    }
    end = clock();
    
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Score benchmark: %.6f seconds for 1,000,000 operations\n", time_taken);
}

int main()
{
    printf("Running benchmark tests...\n");
    
    benchmark_delta();
    benchmark_score();
    printf("Benchmark completed.\n");
    return 0;
}
