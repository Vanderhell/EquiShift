# Stable transfer API contract

The API declared by `include/constrained_probe/transfer.h` is stable. The optional legacy response decoder is declared separately in `transfer_score.h` and is not part of the hot runtime path.

## Invariants

For distinct source `A` and destination `B`, a signed transfer `d` applies:

```
A' = A - d
B' = B + d
```

The mathematical sum is unchanged. Every other channel is bit-identical. A successful `cp_transfer_apply()` writes exactly those two array elements. It computes and validates both results before either write, so a failed call writes nothing.

`cp_transfer_safe_range()` returns the complete feasible interval `[minimum_delta, maximum_delta]`, intersected with the representable `int32_t` delta domain. The current values for both selected channels must already satisfy their respective bounds. A range containing only zero means no nonzero transfer is currently possible.

## Valid input domain

* `count >= 2`; source and destination are in `[0,count)` and distinct.
* Every used pointer is non-null. Bounds are inclusive `int32_t` values with `minimum[i] <= maximum[i]`.
* Current source/destination values must lie within their bounds. Sequence begin checks all channels.
* Applied delta is nonzero. The full `int32_t` domain, including `INT32_MIN`, is supported by the runtime apply path. No negation of the delta is performed.
* Sequence operation count is 1 through `UINT16_MAX`; each operation has valid distinct channels and a nonzero delta. Runtime headroom is checked at each step, so a later infeasible operation fails without applying that operation.

The older `cp_transfer_validate()`/`cp_transfer_delta()` pair-vector helper emits both `step` and `-step` as int32 values, so it rejects `INT32_MIN`. This does not constrain `cp_transfer_apply()` or sequence operations, which can apply `INT32_MIN` safely.

## Failure semantics

All boolean functions return false for invalid pointers, dimensions, indices, bounds, or infeasible requested transfers. Safe-range outputs are written only on success. `cp_transfer_apply()` performs no writes on failure. Sequence begin snapshots only after validating the supplied state and operation descriptors. An out-of-order or duplicate sequence index fails without changing state or channel values. If the next planned operation is infeasible, sequence status becomes `FAILED`, but that operation changes no channel.

## Sequence semantics and restore

The caller allocates the sequence object, value vector, baseline vector, limits, and immutable operation list. `begin` copies the exact initial vector into `baseline`. Call `step(sequence, expected_index)` once for each planned operation, in increasing index order. `finish` succeeds only after all declared steps and marks the sequence complete; it does not restore values. Call `restore` after completion to copy every baseline element back. `abort` restores immediately after any active prefix, failure, or completion. Restore can also be called after a failed step. Restoration is a baseline copy, not inverse arithmetic, so it cannot accumulate rounding or integer drift.

The sequence struct is caller-allocated opaque storage. Its reserved fields must not be read or modified. The caller tracks its expected operation index. Calls on one sequence must be serialized.

## Ownership and hardware responsibilities

All arrays must be disjoint, caller-owned, and remain valid for the sequence lifetime. The caller must not modify the baseline, bounds, or operation list while a sequence is active. There is no heap use, global mutable state, or hidden allocation. A successful transfer changes two in-memory channel values; the caller is responsible for issuing the corresponding physical actuator writes and for applying the N writes produced by restore.

The two C assignments are not an atomic physical multi-channel update. A device requiring synchronized actuator changes must provide a hardware latch/commit layer. Early software interruption is recoverable while the sequence object and baseline are still available. Power-loss recovery requires caller-managed persistence and pointer rebinding; this API cannot recover volatile state after reset by itself.

## Optional score helper

`cp_transfer_score()` remains available through `transfer_score.h` for compatibility. It is outside the critical runtime translation unit because its int64 score scaling can require a compiler runtime multiply helper on Cortex-M0. Applications that do not need response decoding need not link `src/transfer_score.c`.
