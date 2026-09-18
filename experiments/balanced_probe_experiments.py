"""Deterministic shared harness for balanced/reference/zero-sum designs.

Run with Python 3; this is offline analysis code, not production firmware.
All designs share peak=1, N response samples, the same plant and seeded noise.
"""
import math
import random

N = 8
TRIALS = 400
PEAK = 1.0
MEASUREMENT_SD = 0.04
SEED = 0x51A7
TRUE_G = [0.8, -0.35, 0.25, 0.1, -0.6, 0.45, -0.15, -0.5]
assert abs(sum(TRUE_G)) < 1e-12


def dot(a, b):
    return sum(x*y for x, y in zip(a, b))


def designs():
    simplex = []
    q = PEAK/(N-1)
    for hot in range(N):
        simplex.append([q*(N-1) if j == hot else -q for j in range(N)])
    ref = reference_design(0)
    helmert = [[0.0]*N]
    for k in range(1, N):
        v = [0.0]*N
        for j in range(k):
            v[j] = PEAK/k
        v[k] = -PEAK
        helmert.append(v)
    return {"simplex": simplex, "reference": ref, "N-1 basis": helmert}


def reference_design(r):
    probes = [[0.0]*N]
    for i in range(N):
        if i == r:
            continue
        v = [0.0]*N
        v[i], v[r] = PEAK, -PEAK
        probes.append(v)
    return probes


def solve(a, b):
    """Small deterministic Gauss-Jordan solve with partial pivoting."""
    a = [row[:] + [rhs] for row, rhs in zip(a, b)]
    n = len(b)
    for c in range(n):
        p = max(range(c, n), key=lambda r: abs(a[r][c]))
        if abs(a[p][c]) < 1e-12:
            raise ArithmeticError("singular experiment design")
        a[c], a[p] = a[p], a[c]
        z = a[c][c]
        a[c] = [x/z for x in a[c]]
        for r in range(n):
            if r != c:
                z = a[r][c]
                a[r] = [x-z*y for x, y in zip(a[r], a[c])]
    return [a[i][-1] for i in range(n)]


def decode(probes, responses):
    # Fit intercept and N-1 coordinates, setting g[N-1] = -sum(g[:N-1]).
    x = [[1.0] + [v[j]-v[-1] for j in range(N-1)] for v in probes]
    beta = solve(x, responses)
    g = beta[1:] + [-sum(beta[1:])]
    return g


def rank_best(g):
    return max(range(N), key=lambda i: g[i])


def actuator_delta(command, fault, t, rng, fault_index=0):
    d = command[:]
    j = fault_index
    if fault == "noisy actuator":
        d[j] += rng.gauss(0, 0.12)
    elif fault == "gain error":
        d[j] *= 0.5
    elif fault == "channel drift":
        d[j] += 0.025*t
    elif fault == "saturation":
        d[j] = max(-0.35, min(0.35, d[j]))
    elif fault == "stuck channel":
        d[j] = 0.0
    return [max(-PEAK, min(PEAK, x)) for x in d]


def run_trial(probes, fault, trial, nonlinear=0.0, temporal_drift=0.0, fault_index=0):
    rng = random.Random(SEED + trial)
    noise = [rng.gauss(0, MEASUREMENT_SD) for _ in probes]
    actuator_rng = random.Random(SEED*3 + trial)
    ys = []
    for t, command in enumerate(probes):
        actual = actuator_delta(command, fault, t, actuator_rng, fault_index)
        y = dot(TRUE_G, actual) + nonlinear*dot(actual, actual)
        y += temporal_drift*t + noise[t]
        ys.append(y)
    estimate = decode(probes, ys)
    rmse = math.sqrt(sum((x-y)**2 for x, y in zip(estimate, TRUE_G))/N)
    loc = max(range(N), key=lambda i: abs(estimate[i]-TRUE_G[i]))
    return rmse, rank_best(estimate) == rank_best(TRUE_G), loc == 0


def transition_cost(probes):
    # Full builder rewrites N targets at each sample and restoration. Diff-only
    # writes just changed setpoints. Settling is shared first-order actuator lag.
    seq = [[0.0]*N] + [p[:] for p in probes] + [[0.0]*N]
    full_writes = 0
    diff_writes = 0
    settle_ticks = 0
    alpha, tol = 0.25, 0.01
    for old, new in zip(seq, seq[1:]):
        changed = sum(a != b for a, b in zip(old, new))
        if changed:
            full_writes += N
        diff_writes += changed
        jump = max(abs(a-b) for a, b in zip(old, new))
        if jump > tol:
            settle_ticks += math.ceil(math.log(tol/jump)/math.log(1-alpha))
    return full_writes, diff_writes, settle_ticks


def residual_trials(probes, mode, trials=TRIALS):
    """Baseline plus N balanced samples: r=sum(y_i)-N*y0."""
    vals = []
    for trial in range(trials):
        rng = random.Random(SEED + 10000 + trial)
        base = rng.gauss(0, MEASUREMENT_SD)
        ys = []
        for t, command in enumerate(probes):
            actual = actuator_delta(command, mode, t, random.Random(SEED+trial*17+t))
            y = dot(TRUE_G, actual)
            if mode == "quadratic":
                y += 0.3*dot(actual, actual)
            elif mode == "bad measurement" and t == 3:
                y += 0.8
            elif mode == "temporal drift":
                y += 0.08*t
            y += rng.gauss(0, MEASUREMENT_SD)
            ys.append(y)
        vals.append(sum(ys)-N*base)
    return vals


def pct(values, p):
    s = sorted(abs(x) for x in values)
    return s[min(len(s)-1, int(p*(len(s)-1)))]


def main():
    ds = designs()
    print(f"N={N}; peak={PEAK}; N responses/design; trials={TRIALS}; seed={SEED}")
    print("METHODS: method             writes(full/diff) settle_ticks  fault:RMSE/rank/localize")
    scenarios = ["none", "weak quadratic", "measurement drift", "noisy actuator",
                 "gain error", "channel drift", "saturation", "stuck channel"]
    for name, probes in ds.items():
        assert len(probes) == N and max(abs(x) for p in probes for x in p) <= PEAK
        full, diff, ticks = transition_cost(probes)
        print(f"METHOD: {name:16s} {full:3d}/{diff:3d} {ticks:4d}")
        for fault in scenarios:
            out = [run_trial(probes, fault, k,
                             nonlinear=0.12 if fault == "weak quadratic" else 0.0,
                             temporal_drift=0.025 if fault == "measurement drift" else 0.0)
                   for k in range(TRIALS)]
            rmse = sum(x[0] for x in out)/TRIALS
            rank = sum(x[1] for x in out)/TRIALS
            loc = sum(x[2] for x in out)/TRIALS
            print(f"  {fault:16s} rmse={rmse:.4f} rank={rank:.3f} loc0={loc:.3f}")
    print("REFERENCE SWEEP: actuator fault placed on chosen reference; per-channel simplex mean")
    for fault in ["noisy actuator", "gain error", "channel drift", "saturation", "stuck channel"]:
        ref_errors, simp_errors = [], []
        ref_ranks, simp_ranks = [], []
        for channel in range(N):
            ref = reference_design(channel)
            for k in range(120):
                rr = run_trial(ref, fault, k, fault_index=channel)
                sr = run_trial(ds["simplex"], fault, k, fault_index=channel)
                ref_errors.append(rr[0]); ref_ranks.append(rr[1])
                simp_errors.append(sr[0]); simp_ranks.append(sr[1])
        print(f"  {fault:16s} ref rmse={sum(ref_errors)/len(ref_errors):.4f} rank={sum(ref_ranks)/len(ref_ranks):.3f}; "
              f"simplex rmse={sum(simp_errors)/len(simp_errors):.4f} rank={sum(simp_ranks)/len(simp_ranks):.3f}")
    print("RESIDUAL: r=sum(balanced probe outputs)-N*baseline; baseline+N probes")
    rp = ds["simplex"]
    normal = residual_trials(rp, "none")
    threshold = pct(normal, 0.99)
    print(f"  normal |r| p99 threshold={threshold:.4f}; nominal false-positive={sum(abs(x)>threshold for x in normal)/len(normal):.3f}")
    for mode in ["quadratic", "temporal drift", "bad measurement", "gain error", "saturation", "stuck channel"]:
        vals = residual_trials(rp, mode)
        detection = sum(abs(x)>threshold for x in vals)/len(vals)
        print(f"  {mode:16s} |r| median={pct(vals,0.50):.4f} detect={detection:.3f}")


if __name__ == "__main__":
    main()
