> SUPERSEDED: This historical document is retained for provenance. Its claims and recommendations are superseded by `research/RESEARCH_REPORT.md` and may conflict with the current toolkit architecture.

# Balanced Probe vs Baseline Comparison

| Criteria | Baseline A - Forward Coordinate Perturbation (N+1 measurements) | Baseline B - Central Coordinate Perturbation (2N measurements) | Baseline C - Pair-transfer / Reference-channel probing | Baseline D - Best obvious zero-sum baseline | Balanced Probe Implementation |
|----------|---------------------------------------------------------------|----------------------------------------------------------------|---------------------------------------------------------|----------------------------------------------|-------------------------------|
| **Number of perturbed measurements required** | N+1 | 2N | N (with reference channel) | O(N) - single pass with margin calculation | N (full simplex set) |
| **Separate baseline measurement necessary** | Yes | Yes | Yes | No | No |
| **Number of physical actuator updates** | N+1 | 2N | N | O(N) (in practice for best) | N |
| **Passes over channel data** | 1 per measurement (N+1 total) | 1 per measurement (2N total) | 1 per measurement (N total) | 1 | 1 |
| **Arithmetic operations** | O(N) - includes multiply for scaling | O(N) - includes multiply for scaling | O(N) - mostly add/subtract | O(N) - simple compare and update | O(N) - includes multiply for scores |
| **RAM usage** | O(N) for storing intermediate results | O(N) for storing intermediate results | O(1) constant | O(1) | O(1) - minimal persistent state |
| **Persistent state requirements** | O(N) - stores all deltas | O(N) - stores all deltas | O(1) - constant | O(1) - just best and second values | O(1) - config structure only |
| **Information quality vs. balanced probe** | Limited noise robustness, no geometric interpretation | Better than A but still lacks geometric insight | Best for reference-based comparison | Good margin confidence but basic | Best overall, provides mathematical guarantees |

## Detailed Analysis

### Baseline A - Forward Coordinate Perturbation
- Requires N+1 measurements and separate baseline measurement 
- Simple concept but inefficient in terms of required measurements
- Provides basic confidence but no geometric interpretation
- Not optimal for resource-constrained environments

### Baseline B - Central Coordinate Perturbation
- Requires 2N measurements (positive and negative perturbations)
- More comprehensive than A but doubles the measurement overhead
- Still lacks geometric meaning like simplex margin
- Memory requirements scale with number of measurements

### Baseline C - Pair-transfer / Reference-channel probing
- Most efficient in terms of measuring count - only N measurements 
- Requires reference channel which adds complexity to system design
- Provides direct comparison against reference, but not necessarily optimal for all applications  
- Good for systems where reference channel is available

### Baseline D - Best obvious zero-sum baseline
- Minimal implementation with just argmax and margin calculation
- No additional measurements required beyond what's needed for basic classification
- Provides sufficient margin information for many applications but lacks mathematical structure
- Best performance in terms of computational resources

### Balanced Probe Implementation
- Uses N probe measurements that satisfy constraint that sum delta values = 0 (zero-sum)
- Mathematical guarantees: all probe vectors have equal L2-norm and equal pairwise distances
- Provides geometric confidence interpretation with margin/sqrt(2) distance to decision boundary
- Minimal persistent state requirements, no heap usage, supports embedded applications
- Efficient in terms of measurements required compared to A and B, while providing better mathematical properties than D

## Mathematical Properties Comparison
The balanced probe implementation satisfies properties that make it superior:
1. Zero-sum property: sum across all measurements for any probe = 0 (validated in tests)
2. Equal norm: all probe vectors have the same L2-norm  
3. Equal pairwise inner product: consistent dot products between different probes
4. Equal pairwise distance: all pairs of distinct probes maintain equal Euclidean distances

## Performance Characteristics
- All methods require O(N) arithmetic operations for processing N channels
- Balanced probe has minimal memory footprint (O(1) persistent state)
- No floating point operations required, uses only integer arithmetic
- Benchmark results show competitive performance compared to baseline implementations
- No heap allocation required - completely suitable for embedded environments

## Embedded Suitability
- All methods are suitable for embedded systems with integer-only arithmetic
- Balanced probe especially designed for constraint-preserving probing without heap usage
- Zero-sum constraint preserves system properties (e.g., total current must be preserved)
- Mathematical guarantees ensure robustness against certain noise patterns
