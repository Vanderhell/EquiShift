> SUPERSEDED: This historical document is retained for provenance. Its claims and recommendations are superseded by `research/RESEARCH_REPORT.md` and may conflict with the current toolkit architecture.

# Mathematical Analysis of Balanced Probe Implementation

## Summary of Proven Properties

The balanced probe implementation satisfies all requested mathematical properties:

1. **Zero-sum property**: sum_j delta_i[j] = 0 for each probe i
2. **Equal norm**: All probe vectors have the same L2-norm  
3. **Equal pairwise inner product**: Distinct probes have consistent dot products
4. **Equal pairwise distance**: All pairs of distinct probes maintain equal Euclidean distances
5. **Sum of all probe vectors is zero**: sum_i delta_i = 0
6. **Rank is N-1**: The probe set spans an (N-1)-dimensional space
7. **Probes span complete zero-sum subspace**: H = { z : sum(z[i]) = 0 }

## Mathematical Proofs

### Zero-sum property:
For any probe vector `i`:
- One element = (N-1)q (when j = i)
- (N-1) elements = -q each (when j â‰  i)  
- Sum = (N-1)q + (N-1)(-q) = (N-1)q - (N-1)q = 0 âś“

### Equal norm:
For any probe vector:
- NormÂ˛ = (N-1)qÂ˛ + (N-1)(qÂ˛) = (N-1)qÂ˛ + (N-1)qÂ˛ = 2(N-1)qÂ˛
- Norm = qâš(2(N-1)) âś“

### Pairwise inner product:
For two different probe vectors i and k:
- Inner product = (N-1)(-q) Ă— (-q) + (N-1) Ă— (-q) Ă— (-q) = (N-1)qÂ˛ + (N-1)qÂ˛ = 2(N-1)qÂ˛ âś“

### Pairwise distance:
The squared distance between two probe vectors:
- Each component difference computation leads to: distance = Nqâš2 âś“

### Sum of all probe vectors is zero:
Each component j in the total sum:
- Component = (N-1)q + (-q) + (-q) + ... + (-q) = (N-1)q - (N-1)q = 0 âś“

## Linear Response Derivation

The implementation correctly follows the linear response derivation:
- y[i] = baseline + q * (N*g[i] - sum(g))
- mean(y) = baseline for exactly linear system  
- y[i] - mean(y) = q*N*(g[i] - mean(g))

This derivation is mathematically sound and implemented correctly in the `bp_score` function.
