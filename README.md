# EquiShift

[![CI](https://github.com/Vanderhell/EquiShift/actions/workflows/ci.yml/badge.svg)](https://github.com/Vanderhell/EquiShift/actions/workflows/ci.yml)

*Move values. Preserve constraints. Stay deterministic.*

Version **0.1.0**. Deterministic C11 primitives for safe constraint-preserving
transfers on embedded systems. The stable public include tree contains only
`constrained_probe/transfer.h`.

License: Apache-2.0 — Vanderhell

## Modules

**Stable: transfer** — bounded two-channel updates that preserve the exact
integer sum, plus a caller-owned sequence with exact restore. The contract is
in [`docs/transfer_api.md`](docs/transfer_api.md).

**Internal: residual** — specialized ordered-sequence integrity checks and an
aggregate balanced-probe consistency residual. It cannot identify a fault's
cause. Header and implementation are under `src/internal` and `src`.

**Internal: balanced** — specialized reference-free N-probe frame generation
and projected-score decoding. Its peak component grows as `(N-1)|q|`; it is
not the default transfer method.

The implicit-simplex implementation and comparison programs remain under
`experiments/`; the canonical mathematical conclusions are in
[`research/RESEARCH_REPORT.md`](research/RESEARCH_REPORT.md). Historical reports
are archived under `research/historical/`.

## Embedded properties

The stable transfer path uses caller-owned memory, no heap or mutable globals,
checked int32 arithmetic, and exactly two channel-array writes on success.
Sequence restoration copies the saved baseline exactly. Physical actuator
atomicity and power-loss persistence remain the caller's responsibility. The
sequence object layout is not a cross-release ABI promise.

## Why this matters in embedded systems

Many embedded systems must redistribute a fixed budget without changing its
total. For four channels, the state might be:

```text
[25, 25, 25, 25]  total = 100
```

If channel B needs to increase by 5, simply doing `B += 5` produces a total of
105 and breaks the system invariant. EquiShift performs a compensating
two-channel transfer instead:

```text
A -= 5
B += 5

[20, 30, 25, 25]  total = 100
```

This makes it possible to safely perturb a system while preserving its global
constraint—for example when splitting power between outputs, driving PWM or
other actuators, enforcing current or power budgets, coordinating valves and
multi-channel control, calibrating channels, running diagnostic probes, or
performing online system identification.

Before writing, EquiShift checks headroom and configured limits. If the transfer
cannot be completed, it leaves the state unchanged. A successful transfer
writes only two channels, uses no heap, and a sequence can be restored exactly
from its saved baseline, avoiding cumulative drift during repeated probing or
calibration.

## Build and verification

```sh
make                 # stable library plus host tests
make test            # optimized host tests
make sanitize        # ASan + UBSan host tests
make cross-build     # Cortex-M0, Cortex-M4, RV32IMC objects (Clang)
```

Experimental targets are opt-in:

```sh
make experimental-test
make experimental-benchmark
make experimental-balanced-benchmark
make experimental-balanced-example
make balanced-evaluation
```

The target objects have been cross-compiled for Cortex-M0/M4 and RV32IMC
(compatible with ESP32-C3's ISA). No physical MCU cycle measurements are
claimed. Host tests use C11 with GCC/Clang-compatible warning flags; public
headers are C++ compatible. Build from source for each release: v0.1.0 does
not promise binary ABI compatibility across releases.
