#include "constrained_probe/balanced.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Linear plant model: f(x) = c + sum(g[i]*x[i])
typedef struct {
    int32_t baseline;
    int32_t gains[32];  // Up to 32 channels
    uint16_t count;
} linear_plant_t;

// Initialize a linear plant with given gains
void init_linear_plant(linear_plant_t *plant, uint16_t count, int32_t baseline)
{
    memset(plant, 0, sizeof(linear_plant_t));
    plant->count = count;
    plant->baseline = baseline;
}

// Evaluate the linear system at point x
int32_t evaluate_linear_plant(const linear_plant_t *plant, const int32_t *x)
{
    int64_t result = plant->baseline;
    
    for (uint16_t i = 0; i < plant->count; ++i)
    {
        result += (int64_t)plant->gains[i] * x[i];
    }
    
    // Clamp to int32 range
    if (result > INT32_MAX) return INT32_MAX;
    if (result < INT32_MIN) return INT32_MIN;
    
    return (int32_t)result;
}

// Nonlinear plant with quadratic term
typedef struct {
    int32_t baseline;
    int32_t linear_gains[32];
    int32_t quadratic_coeff;
    uint16_t count;
} nonlinear_plant_t;

void init_nonlinear_plant(nonlinear_plant_t *plant, uint16_t count, int32_t baseline, int32_t quad_coeff)
{
    memset(plant, 0, sizeof(nonlinear_plant_t));
    plant->count = count;
    plant->baseline = baseline;
    plant->quadratic_coeff = quad_coeff;
}

int32_t evaluate_nonlinear_plant(const nonlinear_plant_t *plant, const int32_t *x)
{
    int64_t result = plant->baseline;
    
    // Linear component
    for (uint16_t i = 0; i < plant->count; ++i)
    {
        result += (int64_t)plant->linear_gains[i] * x[i];
    }
    
    // Quadratic component
    int64_t quad_sum = 0;
    for (uint16_t i = 0; i < plant->count; ++i)
    {
        quad_sum += (int64_t)x[i] * x[i];
    }
    result += (int64_t)plant->quadratic_coeff * quad_sum;
    
    // Clamp to int32 range
    if (result > INT32_MAX) return INT32_MAX;
    if (result < INT32_MIN) return INT32_MIN;
    
    return (int32_t)result;
}

// Cross-coupled plant model
typedef struct {
    int32_t baseline;
    int32_t linear_gains[32];
    int32_t cross_coupling[32][32];  // N x N coupling matrix
    uint16_t count;
} cross_coupled_plant_t;

void init_cross_coupled_plant(cross_coupled_plant_t *plant, uint16_t count, int32_t baseline)
{
    memset(plant, 0, sizeof(cross_coupled_plant_t));
    plant->count = count;
    plant->baseline = baseline;
}

int32_t evaluate_cross_coupled_plant(const cross_coupled_plant_t *plant, const int32_t *x)
{
    int64_t result = plant->baseline;
    
    // Linear component
    for (uint16_t i = 0; i < plant->count; ++i)
    {
        result += (int64_t)plant->linear_gains[i] * x[i];
    }
    
    // Cross-coupling component
    for (uint16_t i = 0; i < plant->count; ++i)
    {
        for (uint16_t j = 0; j < plant->count; ++j)
        {
            if (i != j)
            {
                result += (int64_t)plant->cross_coupling[i][j] * x[i] * x[j];
            }
        }
    }
    
    // Clamp to int32 range
    if (result > INT32_MAX) return INT32_MAX;
    if (result < INT32_MIN) return INT32_MIN;
    
    return (int32_t)result;
}

// Test function for all models
void test_plant_models()
{
    // Test linear model
    linear_plant_t plant1;
    init_linear_plant(&plant1, 3, 100);
    plant1.gains[0] = 5;
    plant1.gains[1] = 10;
    plant1.gains[2] = 15;
    
    int32_t x1[] = {10, 20, 30};
    int32_t result1 = evaluate_linear_plant(&plant1, x1);
    printf("Linear plant result: %d\n", result1);
    
    // Test nonlinear model
    nonlinear_plant_t plant2;
    init_nonlinear_plant(&plant2, 3, 50, 2);  // baseline=50, quad_coeff=2
    plant2.linear_gains[0] = 1;
    plant2.linear_gains[1] = 2;
    plant2.linear_gains[2] = 3;
    
    int32_t x2[] = {10, 20, 30};
    int32_t result2 = evaluate_nonlinear_plant(&plant2, x2);
    printf("Nonlinear plant result: %d\n", result2);
    
    // Test cross-coupled model
    cross_coupled_plant_t plant3;
    init_cross_coupled_plant(&plant3, 3, 25);
    plant3.linear_gains[0] = 1;
    plant3.linear_gains[1] = 2;
    plant3.linear_gains[2] = 3;
    
    // Set some cross-coupling values
    plant3.cross_coupling[0][1] = 5;
    plant3.cross_coupling[1][2] = 3;
    
    int32_t x3[] = {10, 20, 30};
    int32_t result3 = evaluate_cross_coupled_plant(&plant3, x3);
    printf("Cross-coupled plant result: %d\n", result3);
}

int main()
{
    printf("Testing plant models...\n");
    test_plant_models();
    printf("Plant model tests completed.\n");
    
    return 0;
}
