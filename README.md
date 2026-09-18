# Constrained Probe Toolkit

A small deterministic C11 toolkit for constraint-preserving perturbation and measurement analysis. The modules are intentionally narrow and use caller-owned buffers, integer arithmetic, and checked inputs.

## Modules

**PRIMARY — constrained pair transfer**

Move one channel by `+a` and another by `-a`. This preserves the total actuator command. `transfer.h` provides checked limits, two-channel application, and a caller-owned sequence state machine. The stable contract is in [`docs/transfer_api.md`](docs/transfer_api.md). The optional response-score helper is isolated in `transfer_score.h`.

**SPECIALIZED — sequence consistency residual**

For a complete balanced probe sequence and an independently measured baseline, compute `sum(probe responses) - N * baseline`. This is useful for selected model-consistency checks; it does not localize arbitrary faults.

**EXPERIMENTAL — balanced simplex probing**

The implicit N-probe balanced frame, checked decoder, headroom helper, and transition helper remain available through `balanced.h` for controlled experiments. They are not the default practical design.

## Build and test

```sh
make test
make sanitize
make benchmark
make example
make experimental-test
```

The deterministic comparison harness is `experiments/balanced_probe_experiments.py`. The mathematical record and limits are in [`research/RESEARCH_REPORT.md`](research/RESEARCH_REPORT.md). Old exploratory conclusions are retained under `research/historical/` with supersession notices.
