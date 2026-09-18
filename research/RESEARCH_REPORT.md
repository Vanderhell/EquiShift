# Constrained Probe Toolkit: research record

## Project outcome

The surviving practical default is a constrained pair transfer: perturb channel i by `+a` and reference r by `-a`. It preserves the sum constraint and needs one baseline plus `N-1` transfers to recover all relative sensitivities. The sequence consistency residual is specialized and depends on an independent baseline. Balanced simplex probing remains experimental.

## Canonical balanced-frame mathematics

For `N >= 2` and nonzero q, let `D=q(NI-J)` and `P=I-J/N`. Then `D=qNP`; on `H={z:sum(z)=0}`, it acts as `qN I`. Its rank is `N-1`; all nonzero singular values are `|q|N`; its condition number on H is one; `D^+=P/(qN)`.

The canonical identities are:

```
sum_j delta_i[j] = 0
||delta_i||^2 = N(N-1)q^2
delta_i dot delta_k = -Nq^2                  (i != k)
||delta_i-delta_k||^2 = 2N^2q^2
sum_i delta_i = 0
rank(D) = N-1
```

For `y_i=b+g dot delta_i`, centering recovers `qN(g_i-mean(g))`; the integer score `N*y_i-sum(y)` is `qN^2*g_H[i]`. Common-mode sensitivity is not observable under the sum constraint. At fixed peak A, `|q|=A/(N-1)` and response contrast is `AN/(N-1)`, bounded near A; the apparent N amplification at fixed q spends a growing peak and energy budget.

## Methods compared

`experiments/balanced_probe_experiments.py` is one deterministic harness using the same linear plant, operating point, symmetric per-channel peak limit, response noise sequence, measurement drift sequence, and N response budget for:

* N balanced simplex probes;
* baseline plus N-1 reference transfers;
* baseline plus N-1 independent Helmert zero-sum basis probes.

All patterns obey peak excursion one. The selected operating gradient is fixed and zero-sum. The harness uses a fixed seed and 400 trials for method comparisons; response noise standard deviation is 0.04. Reconstruction RMSE and ranking success are computed against the known projected gradient. Fault localization is scored offline by asking whether the channel with maximum reconstruction error is the injected channel; this uses ground truth and is not itself a deployed detector.

### N=8 comparison evidence

The sequence begins at baseline and returns to baseline. Rebuilding every nontrivial full vector costs 72 writes for simplex and 64 for both baseline-based methods. Writing only changed setpoints costs 30 for simplex, 16 for reference transfers, and 43 for the chosen Helmert ordering. The two-channel simplex identity is exact and reduces writes versus dense vector application, but reference transfers need fewer writes. With first-order actuator lag `alpha=0.25` and a 0.01 settling tolerance, modeled accumulated settling was 153 ticks for simplex, 136 for reference transfer, and 138 for the Helmert basis. Differential writes do not reduce settling in this model; target changes determine it.

Nominal gradient RMSE/ranking success were: simplex `0.0316 / 1.000`, reference transfer `0.0361 / 1.000`, Helmert basis `0.0432 / 1.000`. Under a weak radial quadratic term, simplex RMSE stayed `0.0316` because all its probes have equal norm; reference transfer rose to `0.0863`, and Helmert to `0.1343`. This is a specific equal-radius benefit, not general nonlinear robustness.

With an injected channel-0 actuator disturbance, reference-transfer vs simplex RMSE/ranking were: extra actuator noise `0.0678/0.995` vs `0.0805/1.000`; gain loss `0.1364/0.203` vs `0.1354/0.170`; actuator drift `0.0583/1.000` vs `0.0510/1.000`; saturation `0.1751/0.000` vs `0.1532/0.013`; stuck channel `0.2665/0.000` vs `0.2660/0.000`. Across all reference choices with the fault on the selected reference, aggregate RMSE and ranking were close to simplex's channel-averaged performance; no consistent reference-free robustness advantage survived.

These are deterministic software experiments, not hardware data. The actuator model enforces the same peak limits, but it does not characterize a particular bus or physical device.

## Sequence residual

With an independent baseline `y0`, define `r=sum_i y_i-N*y0`. For the balanced probes, `sum_i delta_i=0`, so r is exactly zero for a linear plant. For `f(x+delta)=f(x)+g^Tdelta+lambda||delta||^2`, `r=lambda*sum_i||delta_i||^2=lambda*N^2/(N-1)*A^2` at equal peak A. A sample-time drift also contributes according to the probe order. Static channel gain loss or a stuck actuator still gives zero linear residual because each channel's commanded perturbations sum to zero.

At N=8, the harness used one baseline plus eight balanced responses, response noise SD 0.04, and a 99th-percentile nominal absolute-residual threshold of 0.9026 (1% observed nominal false positives). It detected all trials for the tested quadratic coefficient 0.3 and linear temporal drift 0.08 per sample. It detected 39.8% of a single +0.8 bad measurement, 11.3% of tested saturation, and 1.3% each for static gain loss and stuck channel. The residual is therefore a specialized large model-departure/drift indicator, not a general fault localizer. It costs an independent baseline response; without it, subtracting the probe mean makes the aggregate residual identically zero.

The exposed API is `cp_sequence_residual()` in `residual.h`; it does not choose thresholds or assert a fault.

## Public modules

* **PRIMARY — transfer:** `transfer.h` validates/generates a zero-sum pair transfer and decodes projected relative response scores. The integer score is scale-preserving; callers decide whether and how to normalize it.
* **SPECIALIZED — residual:** `residual.h` computes the aggregate sequence residual in int64.
* **EXPERIMENTAL — balanced:** `balanced.h` retains the prior checked balanced generator, decoder, headroom helper, transition helper, and streaming decision API without behavior changes. It is not the default.

All APIs use deterministic caller-owned storage and reject invalid dimensions/pointers. Arithmetic that spans int32 inputs is widened before operations. `balanced.h` preserves the `bp_*` symbols from the prior implementation.

## Verification and limits

The reorganized tests cover transfer behavior, residual behavior, balanced behavior, arithmetic boundaries, and balanced matrix invariants/rank/span. Optimized host tests pass. ASan+UBSan are available through `make sanitize` on a host toolchain that provides those runtimes; the project environment previously passed the existing balanced tests under WSL GCC sanitizers. No MCU flash, stack, or cycle results are claimed.

There is no production target-specific bus scheduler or actuator I/O in this toolkit. The measured write and settling counts are experiment-model outputs, not timing or hardware guarantees. Reference transfer remains the simpler default; the balanced family is appropriate only when its equal-radius behavior or symmetric frame is useful for a particular plant.
