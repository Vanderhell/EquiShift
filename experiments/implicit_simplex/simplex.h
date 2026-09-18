#ifndef SIMPLEX_H
#define SIMPLEX_H

#include <stdbool.h>
#include <stdint.h>

typedef uint16_t sx_dim_t;
typedef uint16_t sx_vertex_t;
typedef int32_t sx_coord_t;

static inline bool sx_valid_vertex(sx_dim_t n, sx_vertex_t vertex)
{
    return vertex <= n;
}

static inline sx_coord_t sx_coord(sx_dim_t n, sx_vertex_t vertex, sx_dim_t axis)
{
    return vertex == axis ? (sx_coord_t)n : (sx_coord_t)-1;
}

static inline uint64_t sx_norm2(sx_dim_t n)
{
    return (uint64_t)n * ((uint64_t)n + 1u);
}

static inline int64_t sx_dot(sx_dim_t n, sx_vertex_t a, sx_vertex_t b)
{
    return a == b ? (int64_t)sx_norm2(n) : -(int64_t)(n + 1u);
}

static inline uint64_t sx_distance2(sx_dim_t n, sx_vertex_t a, sx_vertex_t b)
{
    return a == b ? 0u : 2u * (uint64_t)(n + 1u) * (uint64_t)(n + 1u);
}

/* Expansion is deliberately just a state update. */
static inline bool sx_expand(sx_dim_t *n)
{
    if (*n == UINT16_MAX) return false;
    ++*n;
    return true;
}

#endif
