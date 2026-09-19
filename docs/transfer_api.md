# Stable transfer API contract

For release v0.1.0, the stable public surface is exactly the declarations in
`include/constrained_probe/transfer.h`. The optional response-score helper is
internal at `src/internal/transfer_score.h` and is not linked into the default
library.

## Transfer behavior

For distinct source `A`, destination `B`, and signed nonzero delta `d`:

```text
A' = A - d
B' = B + d
```

The mathematical sum is unchanged; other channels are unchanged. A successful
`cp_transfer_apply` validates both results before writing and writes exactly
the two selected `int32_t` elements. Failure writes no channel elements.
`cp_transfer_safe_range` returns the complete feasible delta interval
intersected with int32 range. Both selected current values and bounds must be
valid. Its two output pointers must differ. A range containing only zero means
no nonzero transfer is feasible.

## Valid inputs and arithmetic

* Count is `2..UINT16_MAX`; channel indices are less than count; source and
  destination differ. Vectors contain at least `count` elements.
* Required pointers are non-null. Bounds are inclusive int32 values with
  `minimum[i] <= maximum[i]` for each used channel.
* `cp_transfer_apply` accepts any nonzero int32 delta, including
  `INT32_MIN`; it uses widened checks and does not negate the delta.
* `cp_transfer_validate` and `cp_transfer_delta` describe a signed pair
  vector containing both `step` and `-step`. They reject `INT32_MIN` because
  its negation is not representable.
* Sequence operation count is `1..UINT16_MAX`. Each operation has valid,
  distinct channels and nonzero delta. Headroom is rechecked when executed.
* All result arithmetic is checked before narrowing or mutation. Invalid
  pointers, dimensions, indices, limits, or infeasible operations return
  `false`; output parameters are written only on success.

Direct `cp_transfer_apply` callers must keep `values`, `minimum`, and
`maximum` arrays non-overlapping. `cp_transfer_safe_range` output pointers must
refer to distinct, non-overlapping int64 objects. These overlap requirements
are caller preconditions, not generally detected errors. A sequence requires
its state, values, baseline, minimum, maximum, and operation arrays all be
pairwise disjoint.

## Sequence and restore

The caller allocates every object and array; there is no heap, mutable global,
or hidden allocation. Call `cp_transfer_sequence_begin` before any other
sequence operation. It validates the current vector and descriptors, then
copies the exact initial values to the caller-owned baseline. The baseline,
limits, operations, and arrays must remain valid and unchanged for the
sequence lifetime. Calls on one sequence must be serialized.

`step` accepts only the next operation index. Duplicate or out-of-order calls
fail without changing the sequence state or channel values. A planned transfer
that fails headroom marks the sequence failed and applies no part of that
operation. `finish` succeeds only when every planned operation has run and does
not restore the vector. `restore` copies the saved baseline exactly after a
partial prefix, completion, or failed operation; `abort` restores immediately
from active, failed, or completed state. Restore is a copy, not inverse
arithmetic, so it adds no arithmetic drift.

The sequence struct is caller-allocated opaque storage. Do not read or modify
its reserved fields. The v0.1.0 release makes no binary ABI or cross-release
struct-layout promise; compile clients against the matching header and rebuild
when upgrading.

## Hardware responsibilities and limits

The C updates are not an atomic physical multi-channel actuator operation.
The caller must perform the physical writes and provide a hardware latch or
commit mechanism if simultaneous changes are required. Software restoration
works only while state and baseline memory remain available. Power-loss
recovery needs caller-managed persistence and pointer rebinding; the library
does not provide it.

The optional internal `cp_transfer_score` helper is kept for existing in-tree
users. Its int64 scaling may need a compiler runtime multiply helper on
Cortex-M0, so it is isolated from the default library and hot transfer path.
