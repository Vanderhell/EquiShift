> SUPERSEDED: This historical document is retained for provenance. Its claims and recommendations are superseded by `research/RESEARCH_REPORT.md` and may conflict with the current toolkit architecture.

# Baseline Comparison for Balanced Probe Implementation

## Overview

This document compares the balanced probe implementation against several baseline methods for zero-sum perturbation in N-channel systems. The comparison focuses on practical embedded system constraints including measurement count, resource usage, noise robustness, and conditioning.

## Baseline Methods

### 1. Pair-transfer Zero-Sum Method
- **Operation**: Apply +q to one channel, -q to another, repeat for all pairs (N(N-1)/2 total measurements)
- **Memory**: O(1) persistent state 
- **Measurements**: N(N-1)/2
- **Channels updated**: O(NÂ˛) total updates

### 2. Coordinate Perturbation Methods
#### Forward Coordinate Perturbation  
- **Operation**: Apply +q to each channel in turn, record responses (N+1 measurements)
- **Memory**: O(1) persistent state
- **Measurements**: N+1

#### Central Coordinate Perturbation
- **Operation**: Apply +q and -q to each channel (2N measurements)  
- **Memory**: O(1) persistent state
- **Measurements**: 2N

### 3. Balanced Probe Implementation (Proposed)
- **Operation**: Apply +q to one channel, -q to others for each probe vector (N measurements total)  
- **Memory**: O(1) persistent state
- **Measurements**: N
- **Channels updated**: O(NÂ˛) total updates

## Critical Comparison: Maximum Amplitude Constraint

The key issue identified is the maximum amplitude scaling:
- **Pair-transfer method**: Â±q for each channel
- **Balanced probe**: +q to one channel, -q to (N-1) channels  
- For N=32: one channel receives +31q

**Impact Analysis**: 
This can be problematic for systems with:
- Hardware amplitude limitations
- Nonlinear behavior at high amplitudes  
- ADC saturation or quantization effects
- Thermal considerations

## Embedded System Metrics Comparison

| Metric | Pair-transfer | Coordinate (Forward) | Coordinate (Central) | Balanced Probe |
|--------|---------------|---------------------|---------------------|----------------|
| Measurements | N(N-1)/2 | N+1 | 2N | N |
| Channels updated | O(NÂ˛) | N | 2N | N |
| RAM usage | O(1) | O(1) | O(1) | O(1) |
| Persistent state | O(1) | O(1) | O(1) | O(1) |
| Worst-case amplitude | Â±q | Â±q | Â±q | +31q (N=32) |

## Key Questions to Answer

### 1. Linear System Response
For a system with response $y_i = b + g \cdot x_i$, where $x_i$ are the perturbations:

**Balanced Probe**:
- Mean response: $\bar{y} = b$
- Individual responses: $y_i - \bar{y} = Nq(x_i - \bar{x})$

**Central Coordinate**:
- Responses: $y_i - \bar{y} = q(x_i - \bar{x})$

### 2. Noise Robustness
- **Balanced Probe**: Equal variance contribution, no dominant channel
- **Central Coordinate**: Dominant channel effect with larger amplitude variations  
- **Pair-transfer**: Multiple channels at Â±q but less efficient 

### 3. Conditioning Analysis 
The matrix conditioned on perturbations is of key importance:

**Balanced Probe Matrix**: 
- Has full rank N-1 (zero-sum space)
- Uniform distribution of energy across measurements
- Better conditioning compared to coordinate methods

## Critical Experimental Testing Required

### 1. Hardware Validation
- Test with actual Cortex-M3/4 and ESP32 systems  
- Measure: RAM usage, flash footprint, assembly size
- Validate practical amplitude limits

### 2. Systematic Error Analysis 
- Linear responses 
- Nonlinear distortions (saturation)
- ADC quantization effects
- Noise characteristics
- Drift behavior over time

### 3. Real Performance Metrics
- Execution time with real hardware timers  
- Memory consumption during operation
- Robustness to component variations
- Conditioning number analysis

## Final Verdict

The balanced probe implementation provides superior mathematical properties and measurement efficiency but requires critical validation in practical embedded systems due to:
1. Maximum amplitude constraint for large N
2. Hardware practicality concerns 
3. Real-world system behavior vs. theoretical assumptions

**Current Status**: Experimental primitive - ready for aggressive benchmarking.
