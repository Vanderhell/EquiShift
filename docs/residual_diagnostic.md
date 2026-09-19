# Sequence residual diagnostic

`cp_residual_*` is a bounded aggregate consistency check for one expected
ordered sequence of `N >= 2` balanced, zero-sum probes and one baseline
measurement. It is not a general fault detector.

## Definition

For baseline `b` and probe responses `y_i`, the emitted integer residual is

```text
r = sum_i (y_i - b) = sum_i y_i - N*b
```

For a stationary affine plant `f(x+d)=b+g·d`, if `sum_i d_i=0`, then `r=0`
exactly before measurement errors and quantization. Production accumulates the
differences directly in `int64_t`; no division, floating point, or 64-bit
multiply is required. With `N <= UINT16_MAX` and int32 inputs, the largest
possible magnitude is `UINT16_MAX * (INT32_MAX - INT32_MIN)`, which fits
int64_t.

## Sequence checks

The caller begins with the expected count, then supplies probe indices exactly
once in ascending order. Duplicate, out-of-order, and incomplete input return
structural status codes; no residual is written on failure. The state is
caller-owned, fixed-size, and must be initialized with `begin` before use.
This verifies API call order and count only. It cannot prove that the physical
actuator applied the intended probe or that an ADC sample belongs to that
probe.

## Deterministic threshold

If every probe reading and the baseline reading has an additive error bounded
by `B` integer units, then

```text
|r_error| <= 2*N*B
```

A threshold `T=2*N*B` has zero false positives under that stated bound; only
`|r| > T` violates it. For round-to-nearest ADC quantization with integer
scale `Q` counts per code, one may conservatively use
`B=ceil(Q/2)` in those same integer units. If the noise is statistical rather
than bounded, this is not a probabilistic confidence threshold. A true model
departure, drift, unbounded outlier, or baseline mismatch can also exceed the
threshold.

One corrupted probe by `e` changes `r` by `e`; several errors can cancel. An
aggregate residual cannot identify which sample or actuator caused it.

## Drift and model departures

For additive linear temporal drift `a*t` and baseline time `t_b`,

```text
r_drift = a * (sum_i t_i - N*t_b)
```

Thus scheduling the baseline at the mean probe timestamp cancels first-order
drift. Reversing only the probe-index ordering does not by itself help: the
aggregate sample times remain unchanged. Symmetric measurement times around
the baseline do help. Curved drift remains, as do actuator settling and
hysteresis effects.

For a twice differentiable plant, a local quadratic term contributes
`1/2 * sum_i d_i^T H d_i`; this scalar can be zero for some nonlinear plants.
Saturation and deadband can create either detectable residuals or masked
departures depending on the operating point and threshold. Residual magnitude
therefore signals aggregate inconsistency only, not its cause.
