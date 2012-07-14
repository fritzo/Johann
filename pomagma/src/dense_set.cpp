
#include "dense_set.hpp"
#include "aligned_alloc.hpp"
#include <cstring>

namespace pomagma
{

dense_set::dense_set (size_t num_items)
    : N(num_items),
      M((N+LINE_STRIDE)/LINE_STRIDE),
      m_lines(pomagma::alloc_blocks<Line>(M)),
      m_alias(false)
{
    POMAGMA_DEBUG("creating dense_set with "
        << M << " lines");
    POMAGMA_ASSERT(N < (1<<26), "dense_set is too large");
    POMAGMA_ASSERT(m_lines, "failed to allocate lines");

    // initialize to zeros
    bzero(m_lines, sizeof(Line) * M);
}

dense_set::~dense_set ()
{
  if (not m_alias) pomagma::free_blocks(m_lines);
}

void dense_set::move_from (const dense_set & other, const oid_t * new2old)
{
    POMAGMA_DEBUG("Copying dense_set");

    size_t minM = min(M, other.M);
    if (new2old == NULL) {
        // just copy
        memcpy(m_lines, other.m_lines, sizeof(Line) * minM);
    } else {
        // copy & reorder
        bzero(m_lines, sizeof(Line) * M);
        for (size_t n = 1; n <= N; ++n) {
            if (other.contains(new2old[n])) insert(n);
        }
    }
}

//----------------------------------------------------------------------------
// Diagnostics

// not fast
bool dense_set::empty () const
{
    for (size_t m = 0; m < M; ++m) {
        if (m_lines[m]) return false;
    }
    return true;
}

// supa-slow, try not to use
size_t dense_set::size () const
{
    unsigned result = 0;
    for (size_t m = 0; m < M; ++m) {
        // WARNING: only unsigned's work with >>
        for (Line line = m_lines[m]; line; line>>=1) {
            result += line & 1;
        }
    }
    return result;
}

void dense_set::validate () const
{
    // make sure extra bits aren't used
    POMAGMA_ASSERT(not (m_lines[0] & 1), "dense set contains null item");
    size_t end = (N + 1) % LINE_STRIDE; // bit count in partially-filled block
    if (end == 0) return;
    POMAGMA_ASSERT(not (m_lines[M - 1] >> end),
            "dense set's end bits are used: " << m_lines[M - 1]);
}


// insertion
void dense_set::insert_all ()
{
    // slow version
    // for (size_t i = 1; i <= N; ++i) { insert(i); }

    // fast version
    const Line full = 0xFFFFFFFF;
    for (size_t m = 0; m < M; ++m) m_lines[m] = full;
    size_t end = (N + 1) % LINE_STRIDE; // bit count in partially-filled block
    if (end) m_lines[M - 1] = full >> (LINE_STRIDE - end);
    m_lines[0] ^= 1; // remove zero element
}

//----------------------------------------------------------------------------
// Entire operations

void dense_set::zero ()
{
    bzero(m_lines, sizeof(Line) * M);
}

bool dense_set::operator== (const dense_set & other) const
{
    POMAGMA_ASSERT1(capacity() == other.capacity(),
            "tried to == compare dense_sets of different capacity");

    for (size_t m = 0; m < M; ++m) {
        if (m_lines[m] != other.m_lines[m]) return false;
    }
    return true;
}

bool dense_set::operator<= (const dense_set & other) const
{
    POMAGMA_ASSERT1(capacity() == other.capacity(),
            "tried to <= compare dense_sets of different capacity");

    for (size_t m = 0; m < M; ++m) {
        if (m_lines[m] & ~other.m_lines[m]) return false;
    }
    return true;
}

bool dense_set::disjoint (const dense_set & other) const
{
    POMAGMA_ASSERT1(capacity() == other.capacity(),
            "tried to disjoint-compare dense_sets of different capacity");

    for (size_t m = 0; m < M; ++m) {
        if (m_lines[m] & other.m_lines[m]) return false;
    }
    return true;
}

// inplace union
void dense_set::operator += (const dense_set & other)
{
    const Line * restrict s = other.m_lines;
    Line * restrict t = m_lines;

    for (size_t m = 0; m < M; ++m) {
        t[m] |= s[m];
    }
}

// inplace intersection
void dense_set::operator *= (const dense_set & other)
{
    const Line * restrict s = other.m_lines;
    Line * restrict t = m_lines;

    for (size_t m = 0; m < M; ++m) {
        t[m] &= s[m];
    }
}

void dense_set::set_union (const dense_set & lhs, const dense_set & rhs)
{
    const Line * restrict s = lhs.m_lines;
    const Line * restrict t = rhs.m_lines;
    Line * restrict u = m_lines;

    for (size_t m = 0; m < M; ++m) {
        u[m] = s[m] | t[m];
    }
}

void dense_set::set_diff (const dense_set & lhs, const dense_set & rhs)
{
    const Line * restrict s = lhs.m_lines;
    const Line * restrict t = rhs.m_lines;
    Line * restrict u = m_lines;

    for (size_t m = 0; m < M; ++m) {
        u[m] = s[m] & ~t[m];
    }
}

void dense_set::set_insn (const dense_set & lhs, const dense_set & rhs)
{
    const Line * restrict s = lhs.m_lines;
    const Line * restrict t = rhs.m_lines;
    Line * restrict u = m_lines;

    for (size_t m = 0; m < M; ++m) {
        u[m] = s[m] & t[m];
    }
}

void dense_set::set_nor (const dense_set & lhs, const dense_set & rhs)
{
    const Line * restrict s = lhs.m_lines;
    const Line * restrict t = rhs.m_lines;
    Line * restrict u = m_lines;

    for (size_t m = 0; m < M; ++m) {
        u[m] = ~ (s[m] | t[m]);
    }
}

// this += dep; dep = 0;
void dense_set::merge (dense_set & dep)
{
    POMAGMA_ASSERT4(N == dep.N, "dep has wrong size in rep.merge(dep)");

    Line * restrict d = dep.m_lines;
    Line * restrict r = m_lines;

    for (size_t m = 0; m < M; ++m) {
        r[m] |= d[m];
        d[m] = 0;
    }
}

// diff = dep - this; this += dep; dep = 0; return diff not empty;
bool dense_set::merge (dense_set & dep, dense_set & diff)
{
    POMAGMA_ASSERT4(N == dep.N, "dep has wrong size in rep.merge(dep,diff)");
    POMAGMA_ASSERT4(N == diff.N, "diff has wrong size in rep.merge(dep,diff)");

    Line * restrict d = dep.m_lines;
    Line * restrict r = m_lines;
    Line * restrict c = diff.m_lines;

    Line changed = 0;
    for (size_t m = 0; m < M; ++m) {
        changed |= (c[m] = d[m] & ~r[m]);
        r[m] |= d[m];
        d[m] = 0;
    }

    return changed;
}

// diff = src - this; this += src; return diff not empty;
bool dense_set::ensure (const dense_set & src, dense_set & diff)
{
    POMAGMA_ASSERT4(N == src.N, "src has wrong size in rep.ensure(src,diff)");
    POMAGMA_ASSERT4(N == diff.N, "diff has wrong size in rep.ensure(src,diff)");

    const Line * restrict d = src.m_lines;
    Line * restrict r = m_lines;
    Line * restrict c = diff.m_lines;

    Line changed = 0;
    for (size_t m = 0; m < M; ++m) {
        changed |= (c[m] = d[m] & ~r[m]);
        r[m] |= d[m];
    }

    return changed;
}

//----------------------------------------------------------------------------
// Iteration

void dense_set::iterator::_next_block ()
{
    // traverse to next nonempty block
    const Line * lines = m_set.m_lines;
    do { if (++m_quot == m_set.M) { m_i = 0; return; }
    } while (!lines[m_quot]);

    // traverse to first nonempty bit in a nonempty block
    Line line = lines[m_quot];
    for (m_rem = 0, m_mask = 1; !(m_mask & line); ++m_rem, m_mask <<= 1) {
        POMAGMA_ASSERT4(m_rem != LINE_STRIDE,
                "dense_set::_next_block found no bits");
    }
    m_i = m_rem + LINE_STRIDE * m_quot;
    POMAGMA_ASSERT5(0 < m_i and m_i <= m_set.N,
            "dense_set::iterator::_next_block landed on invalid pos " << m_i);
    POMAGMA_ASSERT5(m_set.contains(m_i),
            "dense_set::iterator::_next_block landed on empty pos " << m_i);
}

// PROFILE this is one of the slowest methods
void dense_set::iterator::next ()
{
    POMAGMA_ASSERT5(ok(), "tried to increment a finished dense_set::iterator");
    Line line = m_set.m_lines[m_quot];
    do {
        ++m_rem;
        //if (m_rem < LINE_STRIDE) m_mask <<=1; // slow version
        if (m_rem & LINE_MASK) m_mask <<= 1;    // fast version
        else { _next_block(); return; }
    } while (!(m_mask & line));
    m_i = m_rem + LINE_STRIDE * m_quot;
    POMAGMA_ASSERT5(0 < m_i and m_i <= m_set.N,
            "dense_set::iterator::next landed on invalid pos " << m_i);
    POMAGMA_ASSERT5(m_set.contains(m_i),
            "dense_set::iterator::next landed on empty pos " << m_i);
}

} // namespace pomagma

