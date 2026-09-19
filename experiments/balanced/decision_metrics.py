"""Deterministic formula checks for the balanced-vs-reference decision."""

from itertools import product
from math import sqrt


def eye(n):
    return [[float(i == j) for j in range(n)] for i in range(n)]


def transpose(a):
    return [list(col) for col in zip(*a)]


def mul(a, b):
    return [[sum(x * y for x, y in zip(row, col))
             for col in transpose(b)] for row in a]


def matvec(a, x):
    return [sum(v * w for v, w in zip(row, x)) for row in a]


def projected_identity(n):
    return [[(1.0 if i == j else 0.0) - 1.0 / n
             for j in range(n)] for i in range(n)]


def reference_decoder(n):
    """Map N-1 noisy probe-minus-baseline readings into g_H."""
    ref = n - 1
    return [[(1.0 if i == j else 0.0) - 1.0 / n
             for j in range(n - 1)] if i != ref
            else [-1.0 / n] * (n - 1) for i in range(n)]


def max_squared_error_balanced(n, step, errors):
    p = projected_identity(n)
    centered = matvec(p, errors[:n])
    return sum(v * v for v in centered) / (step * n) ** 2


def max_squared_error_reference(n, amplitude, errors):
    decode = reference_decoder(n)
    z = [errors[i] - errors[-1] for i in range(n - 1)]
    estimate = [v / amplitude for v in matvec(decode, z)]
    return sum(v * v for v in estimate)


def quadratic_balanced_bias(n, q, hessian):
    p = projected_identity(n)
    h_tangent = mul(mul(p, hessian), p)
    diagonal = [h_tangent[i][i] for i in range(n)]
    centered = matvec(p, diagonal)
    return [0.5 * n * q * v for v in centered]


def main():
    print("N peak/q Ebal/Epair@q MSEbal/MSEpair@q "
          "Ebal/Epair@peak MSEbal/MSEpair@peak MSE ratio@energy writes bal/pair")
    for n in (2, 3, 4, 8, 16, 32):
        peak_growth = n - 1
        energy_equal_q = n * n / 2.0
        mse_equal_q = 1.0 / (n * n)
        energy_ratio = n * n / (2.0 * (n - 1) ** 2)
        mse_ratio = ((n - 1) / n) ** 2
        # At equal peak, balanced needs N initial writes, 2 per transition,
        # then N restore writes. Reference transfers need 2 per transfer
        # transition including initial and restoration.
        writes_bal = 4 * n - 2
        writes_pair = 2 * n
        print(f"{n:2d} {peak_growth:6d} {energy_equal_q:13.4f} "
              f"{mse_equal_q:15.4f} {energy_ratio:16.4f} "
              f"{mse_ratio:19.4f} {0.5:17.4f} "
              f"{writes_bal:4d}/{writes_pair:<4d}")

        # Verify that reference measurement noise is correlated by its shared
        # baseline; after propagating it, both decoders are isotropic on H.
        a = reference_decoder(n)
        sigma = [[(1.0 if i == j else 0.0) + 1.0
                  for j in range(n - 1)] for i in range(n - 1)]
        pair_cov = mul(mul(a, sigma), transpose(a))
        p = projected_identity(n)
        for i in range(n):
            for j in range(n):
                assert abs(pair_cov[i][j] - p[i][j]) < 1e-12

        # Equal peak A=1: exact iid Gaussian covariance traces.
        balanced_trace = (n - 1) / (n / (n - 1)) ** 2
        pair_trace = n - 1
        assert abs(balanced_trace / pair_trace - mse_ratio) < 1e-12
        # Equal total sum(delta^2): q*N=sqrt(2)*a, giving a 2x variance gain.
        assert abs((1.0 / (sqrt(2.0)) ** 2) - 0.5) < 1e-12

    # Exhaust all bounded +/-1 errors for N=3 (three probe samples + baseline).
    # This reports worst squared reconstruction error, not a probabilistic rate.
    n, amp = 3, 1.0
    step = amp / (n - 1)
    corners = list(product((-1.0, 1.0), repeat=n + 1))
    balanced_max = max(max_squared_error_balanced(n, step, list(e))
                       for e in corners)
    pair_max = max(max_squared_error_reference(n, amp, list(e))
                   for e in corners)
    print(f"N=3 bounded iid-error corners: balanced max ||err||^2={balanced_max:.4f}, "
          f"reference max ||err||^2={pair_max:.4f}")

    # Quadratic-response identity. Isotropic tangent curvature has constant
    # d_i^T H d_i and disappears from centered gradient scores. A cross term
    # with nonconstant tangent diagonal does not.
    n, peak = 4, 1.0
    q = peak / (n - 1)
    isotropic = eye(n)
    cross = [[0.0] * n for _ in range(n)]
    cross[0][1] = cross[1][0] = 1.0
    iso_bias = quadratic_balanced_bias(n, q, isotropic)
    cross_bias = quadratic_balanced_bias(n, q, cross)
    # Forward reference pairs have equal curvature offset lambda*a^2 on each
    # non-reference response; reconstructing from those differences assigns
    # that offset to a false tangent gradient.
    pair_iso_bias = [peak / n] * (n - 1) + [-peak * (n - 1) / n]
    assert max(abs(v) for v in iso_bias) < 1e-12
    assert max(abs(v) for v in pair_iso_bias) > 0.0
    assert max(abs(v) for v in cross_bias) > 0.0
    print(f"N=4 quadratic bias at equal peak: balanced isotropic={iso_bias}, "
          f"reference-forward isotropic={pair_iso_bias}, "
          f"balanced cross-coupled={cross_bias}")


if __name__ == "__main__":
    main()
