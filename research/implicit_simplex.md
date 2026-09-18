# Implicit balanced simplex: experimental note

For `N >= 2` and nonzero integer `q`, define `D=q(NI-J)`. Row i contains `(N-1)q` at i and `-q` elsewhere. Since `P=I-J/N` projects onto `H={z:sum(z)=0}`, `D=qNP`; its restriction to H is `qN I`. The frame has rank `N-1`, equal nonzero singular values `|q|N`, and condition number one on H.

For a local linear response, centered probe measurements recover the gradient projected onto H. They do not recover common-mode sensitivity. Under equal peak excursion, the required step is `|q|=A/(N-1)`, so response scaling does not grow linearly with N.

Research did not establish balanced probing as the default. At N=8 in the deterministic harness, its uniform-radius probes had no bias under the tested radial quadratic term, but the reference-transfer design used fewer actuator writes (16 changed writes versus 30), and settled faster in the modeled sequence (136 versus 153 ticks). Keep this family experimental and choose it only when the symmetry or equal-radius response behavior fits the plant.

The checked implementation is in `include/constrained_probe/balanced.h` and `src/balanced.c`; it is excluded from the primary API discussion in the README. See [`RESEARCH_REPORT.md`](RESEARCH_REPORT.md) for canonical equations and experiment details.
