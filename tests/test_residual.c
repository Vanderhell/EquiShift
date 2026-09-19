#include "residual.h"

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

static int64_t run_stream(const int32_t *y, uint16_t n, int32_t baseline)
{
    cp_residual_state_t state;
    int64_t result = INT64_MIN;
    uint16_t i;
    assert(cp_residual_begin(&state, n, baseline) == CP_RESIDUAL_OK);
    for (i = 0; i < n; ++i)
        assert(cp_residual_update(&state, i, y[i]) == CP_RESIDUAL_OK);
    assert(cp_residual_finish(&state, &result) == CP_RESIDUAL_OK);
    return result;
}

static void test_stream_structure(void)
{
    cp_residual_state_t s;
    int64_t r = 123;
    const int32_t y[] = {13, 9, 7, 11};
    uint16_t i;

    assert(cp_residual_begin(NULL, 4, 10) == CP_RESIDUAL_INVALID_ARGUMENT);
    assert(cp_residual_begin(&s, 0, 10) == CP_RESIDUAL_INVALID_ARGUMENT);
    assert(cp_residual_begin(&s, 1, 10) == CP_RESIDUAL_INVALID_ARGUMENT);

    assert(cp_residual_begin(&s, 4, 10) == CP_RESIDUAL_OK);
    assert(cp_residual_update(&s, 0, y[0]) == CP_RESIDUAL_OK);
    assert(cp_residual_update(&s, 0, y[0]) == CP_RESIDUAL_DUPLICATE_INDEX);
    assert(cp_residual_finish(&s, &r) == CP_RESIDUAL_DUPLICATE_INDEX);
    assert(r == 123);

    assert(cp_residual_begin(&s, 4, 10) == CP_RESIDUAL_OK);
    assert(cp_residual_update(&s, 1, y[1]) == CP_RESIDUAL_OUT_OF_ORDER);
    assert(cp_residual_finish(&s, &r) == CP_RESIDUAL_OUT_OF_ORDER);
    assert(r == 123);

    assert(cp_residual_begin(&s, 4, 10) == CP_RESIDUAL_OK);
    assert(cp_residual_update(&s, 0, y[0]) == CP_RESIDUAL_OK);
    assert(cp_residual_update(&s, 1, y[1]) == CP_RESIDUAL_OK);
    assert(cp_residual_finish(&s, &r) == CP_RESIDUAL_INCOMPLETE);
    assert(r == 123);
    assert(cp_residual_update(&s, 2, y[2]) == CP_RESIDUAL_BAD_STATE);
    assert(cp_residual_finish(&s, &r) == CP_RESIDUAL_INCOMPLETE);

    assert(cp_residual_begin(&s, 4, 10) == CP_RESIDUAL_OK);
    for (i = 0; i < 4; ++i)
        assert(cp_residual_update(&s, i, y[i]) == CP_RESIDUAL_OK);
    assert(cp_residual_finish(&s, &r) == CP_RESIDUAL_OK && r == 0);
    assert(cp_residual_finish(&s, &r) == CP_RESIDUAL_BAD_STATE);
}

static void test_ideal_and_offsets(void)
{
    /* N=4 balanced deviations from baseline 10 sum to zero. */
    const int32_t linear[] = {13, 9, 7, 11};
    const int32_t offset[] = {113, 109, 107, 111};
    assert(run_stream(linear, 4, 10) == 0);
    assert(run_stream(offset, 4, 110) == 0);
    assert(cp_sequence_residual(4, linear, 10, &(int64_t){99}));
    {
        int64_t r = 99;
        assert(cp_sequence_residual(4, linear, 10, &r) && r == 0);
    }
}

static void test_checked_integer_domain_and_replay(void)
{
    cp_residual_state_t s;
    int64_t r1, r2;
    const int32_t extremes[] = {INT32_MIN, INT32_MAX};
    assert(run_stream(extremes, 2, 0) == -1);
    assert(run_stream(extremes, 2, INT32_MIN) == INT64_C(4294967295));
    r1 = run_stream(extremes, 2, INT32_MAX);
    r2 = run_stream(extremes, 2, INT32_MAX);
    assert(r1 == r2);

    assert(cp_residual_begin(&s, UINT16_MAX, INT32_MIN) == CP_RESIDUAL_OK);
    for (uint32_t i = 0; i < UINT16_MAX; ++i)
        assert(cp_residual_update(&s, (uint16_t)i, INT32_MAX) == CP_RESIDUAL_OK);
    assert(cp_residual_finish(&s, &r1) == CP_RESIDUAL_OK);
    assert(r1 == (int64_t)UINT16_MAX * INT64_C(4294967295));

    assert(!cp_sequence_residual(1, extremes, 0, &r1));
    assert(!cp_sequence_residual(4, NULL, 0, &r1));
    assert(!cp_sequence_residual(4, extremes, 0, NULL));
}

static void test_disturbance_response(void)
{
    /* Drift y(t)=b+a*t. Probe times 0,1,3,4 around baseline time 2:
       sum(t_i)-N*t_baseline = 8-8 = 0, so first-order drift cancels. */
    const int32_t drift_centered[] = {100, 101, 103, 104};
    /* Natural probe times 0,1,2,3 and baseline at 0 yield residual 6. */
    const int32_t drift_forward[] = {100, 101, 102, 103};
    /* Reversing the probe order leaves the same aggregate when baseline stays
       at t=0; ordering alone does not cancel drift. */
    const int32_t drift_reverse[] = {103, 102, 101, 100};
    assert(run_stream(drift_centered, 4, 102) == 0);
    assert(run_stream(drift_forward, 4, 100) == 6);
    assert(run_stream(drift_reverse, 4, 100) == 6);

    /* Curved drift y(t)=100+(t-2)^2: symmetric first-order schedule still
       leaves the predicted second-order residual 10. */
    {
        const int32_t slow_curve[] = {104, 101, 101, 104};
        assert(run_stream(slow_curve, 4, 100) == 10);
    }
    /* Uniform additive offsets cancel when baseline is sampled under the
       same offset. A changing offset is temporal drift, not common offset. */
    assert(run_stream((int32_t[]){213,209,207,211}, 4, 210) == 0);
}

static void test_nonlinearity_and_fault_blind_spots(void)
{
    /* Two opposite transfer probes: z=+/-2. For f=b+g*z+h*z^2,
       r=2*h*z^2=8 when h=1. */
    const int32_t quadratic[] = {4, 4};
    assert(run_stream(quadratic, 2, 0) == 8);

    /* Cross-coupled plant f=x0*x1 around (9,9), probes (11,7),(7,11). */
    const int32_t cross_coupled[] = {77, 77};
    assert(run_stream(cross_coupled, 2, 81) == -8);

    /* Saturation at 10 per channel: baseline (9,9), probes (11,7),(7,11). */
    const int32_t saturated[] = {17, 17};
    assert(run_stream(saturated, 2, 18) == -2);

    /* Deadband response max(|x0-x1|-1,0): baseline zero, both probes 3. */
    const int32_t deadband[] = {3, 3};
    assert(run_stream(deadband, 2, 0) == 6);

    /* One bad sample appears as its error in r. Two opposite corruptions
       cancel exactly: aggregate residual cannot localize or guarantee detect. */
    assert(run_stream((int32_t[]){100, 100, 100, 109}, 4, 100) == 9);
    assert(run_stream((int32_t[]){100, 105, 95, 100}, 4, 100) == 0);

    /* With per-reading bounded error B=1, threshold T=2*N*B=4 for N=2.
       Quadratic/cross-coupled/deadband cases exceed T; mild saturation does
       not. This is model-inconsistency sensitivity, not fault attribution. */
    assert(run_stream(quadratic, 2, 0) > 4);
    assert(run_stream(cross_coupled, 2, 81) < -4);
    assert(run_stream(saturated, 2, 18) >= -4 &&
           run_stream(saturated, 2, 18) <= 4);
    assert(run_stream(deadband, 2, 0) > 4);
}

static void test_deterministic_threshold_bound(void)
{
    /* If every response and baseline has independently bounded additive
       error |e|<=B, then |r_error|<=2*N*B. Strictly exceeding this bound is
       sufficient evidence of violation of that particular bound. */
    int32_t y[4];
    int64_t r;
    unsigned cases = 0;
    for (int eb = -1; eb <= 1; ++eb) {
        for (int e0 = -1; e0 <= 1; ++e0) {
            for (int e1 = -1; e1 <= 1; ++e1) {
                for (int e2 = -1; e2 <= 1; ++e2) {
                    for (int e3 = -1; e3 <= 1; ++e3) {
                        y[0] = e0; y[1] = e1; y[2] = e2; y[3] = e3;
                        r = run_stream(y, 4, eb);
                        assert(r >= -8 && r <= 8);
                        ++cases;
                    }
                }
            }
        }
    }
    assert(cases == 243);
    assert(run_stream((int32_t[]){0, 0, 0, 9}, 4, 0) > 8);
    /* ADC quantization of each reading by at most one integer count has the
       same conservative threshold 8 for N=4; bounded errors caused no FP. */
}

int main(void)
{
    test_stream_structure();
    test_ideal_and_offsets();
    test_checked_integer_domain_and_replay();
    test_disturbance_response();
    test_nonlinearity_and_fault_blind_spots();
    test_deterministic_threshold_bound();
    return 0;
}
