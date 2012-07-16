
#include "dense_set.hpp"
#include "aligned_alloc.hpp"
#include <cstring>

namespace pomagma
{

dense_set::dense_set (size_t num_items)
    : m_item_dim(num_items),
      m_word_dim(word_count(m_item_dim)),
      m_line(pomagma::alloc_blocks<Word>(m_word_dim)),
      m_alias(false)
{
    POMAGMA_DEBUG("creating dense_set with " << m_word_dim << " lines");
    POMAGMA_ASSERT(m_item_dim < (1<<26), "dense_set is too large");

    // initialize to zeros
    bzero(m_line, sizeof(Word) * m_word_dim);
}

dense_set::~dense_set ()
{
  if (not m_alias) pomagma::free_blocks(m_line);
}

void dense_set::move_from (const dense_set & other, const oid_t * new2old)
{
    POMAGMA_DEBUG("Copying dense_set");

    size_t minM = min(m_word_dim, other.m_word_dim);
    if (new2old == NULL) {
        // just copy
        memcpy(m_line, other.m_line, sizeof(Word) * minM);
    } else {
        // copy & reorder
        bzero(m_line, sizeof(Word) * m_word_dim);
        for (size_t n = 1; n <= m_item_dim; ++n) {
            if (other.contains(new2old[n])) insert(n);
        }
    }
}

//----------------------------------------------------------------------------
// Diagnostics

// not fast
bool dense_set::empty () const
{
    for (size_t m = 0; m < m_word_dim; ++m) {
        if (m_line[m]) return false;
    }
    return true;
}

// supa-slow, try not to use
size_t dense_set::count_items () const
{
    unsigned result = 0;
    for (size_t m = 0; m < m_word_dim; ++m) {
        // WARNING only unsigned's work with >>
        static_assert(Word(1) >> 1 == 0, "bitshifting Word fails");
        for (Word word = m_line[m]; word; word>>=1) {
            result += word & 1;
        }
    }
    return result;
}

void dense_set::validate () const
{
    // make sure padding bits are zero
    POMAGMA_ASSERT(not (m_line[0] & 1), "dense set contains null item");
    size_t end = (m_item_dim + 1) % BITS_PER_WORD; // bit count in partially-filled block
    if (end == 0) return;
    POMAGMA_ASSERT(not (m_line[m_word_dim - 1] >> end),
            "dense set's end bits are used: " << m_line[m_word_dim - 1]);
}


// insertion
void dense_set::insert_all ()
{
    // slow version
    // for (size_t i = 1; i <= m_item_dim; ++i) { insert(i); }

    // fast version
    const Word full = 0xFFFFFFFF;
    for (size_t m = 0; m < m_word_dim; ++m) m_line[m] = full;
    size_t end = (m_item_dim + 1) % BITS_PER_WORD; // bit count in partially-filled block
    if (end) m_line[m_word_dim - 1] = full >> (BITS_PER_WORD - end);
    m_line[0] ^= 1; // remove zero element
}

//----------------------------------------------------------------------------
// Entire operations

void dense_set::zero ()
{
    bzero(m_line, sizeof(Word) * m_word_dim);
}

bool dense_set::operator== (const dense_set & other) const
{
    POMAGMA_ASSERT1(item_dim() == other.item_dim(), "item_dim mismatch");

    for (size_t m = 0; m < m_word_dim; ++m) {
        if (m_line[m] != other.m_line[m]) return false;
    }
    return true;
}

bool dense_set::operator<= (const dense_set & other) const
{
    POMAGMA_ASSERT1(item_dim() == other.item_dim(), "item_dim mismatch");

    for (size_t m = 0; m < m_word_dim; ++m) {
        if (m_line[m] & ~other.m_line[m]) return false;
    }
    return true;
}

bool dense_set::disjoint (const dense_set & other) const
{
    POMAGMA_ASSERT1(item_dim() == other.item_dim(), "item_dim mismatch");

    for (size_t m = 0; m < m_word_dim; ++m) {
        if (m_line[m] & other.m_line[m]) return false;
    }
    return true;
}

// inplace union
void dense_set::operator += (const dense_set & other)
{
    POMAGMA_ASSERT1(item_dim() == other.item_dim(), "item_dim mismatch");

    const Word * restrict s = other.m_line;
    Word * restrict t = m_line;

    for (size_t m = 0; m < m_word_dim; ++m) {
        t[m] |= s[m];
    }
}

// inplace intersection
void dense_set::operator *= (const dense_set & other)
{
    POMAGMA_ASSERT1(item_dim() == other.item_dim(), "item_dim mismatch");

    const Word * restrict s = other.m_line;
    Word * restrict t = m_line;

    for (size_t m = 0; m < m_word_dim; ++m) {
        t[m] &= s[m];
    }
}

void dense_set::set_union (const dense_set & lhs, const dense_set & rhs)
{
    POMAGMA_ASSERT1(item_dim() == lhs.item_dim(), "lhs.item_dim mismatch");
    POMAGMA_ASSERT1(item_dim() == rhs.item_dim(), "rhs.item_dim mismatch");

    const Word * restrict s = lhs.m_line;
    const Word * restrict t = rhs.m_line;
    Word * restrict u = m_line;

    for (size_t m = 0; m < m_word_dim; ++m) {
        u[m] = s[m] | t[m];
    }
}

void dense_set::set_diff (const dense_set & lhs, const dense_set & rhs)
{
    POMAGMA_ASSERT1(item_dim() == lhs.item_dim(), "lhs.item_dim mismatch");
    POMAGMA_ASSERT1(item_dim() == rhs.item_dim(), "rhs.item_dim mismatch");

    const Word * restrict s = lhs.m_line;
    const Word * restrict t = rhs.m_line;
    Word * restrict u = m_line;

    for (size_t m = 0; m < m_word_dim; ++m) {
        u[m] = s[m] & ~t[m];
    }
}

void dense_set::set_insn (const dense_set & lhs, const dense_set & rhs)
{
    POMAGMA_ASSERT1(item_dim() == lhs.item_dim(), "lhs.item_dim mismatch");
    POMAGMA_ASSERT1(item_dim() == rhs.item_dim(), "rhs.item_dim mismatch");

    const Word * restrict s = lhs.m_line;
    const Word * restrict t = rhs.m_line;
    Word * restrict u = m_line;

    for (size_t m = 0; m < m_word_dim; ++m) {
        u[m] = s[m] & t[m];
    }
}

void dense_set::set_nor (const dense_set & lhs, const dense_set & rhs)
{
    POMAGMA_ASSERT1(item_dim() == lhs.item_dim(), "lhs.item_dim mismatch");
    POMAGMA_ASSERT1(item_dim() == rhs.item_dim(), "rhs.item_dim mismatch");

    const Word * restrict s = lhs.m_line;
    const Word * restrict t = rhs.m_line;
    Word * restrict u = m_line;

    for (size_t m = 0; m < m_word_dim; ++m) {
        u[m] = ~ (s[m] | t[m]);
    }
}

// this += dep; dep = 0;
void dense_set::merge (dense_set & dep)
{
    POMAGMA_ASSERT4(m_item_dim == dep.m_item_dim, "dep has wrong size");

    Word * restrict d = dep.m_line;
    Word * restrict r = m_line;

    for (size_t m = 0; m < m_word_dim; ++m) {
        r[m] |= d[m];
        d[m] = 0;
    }
}

// diff = dep - this; this += dep; dep = 0; return diff not empty;
bool dense_set::merge (dense_set & dep, dense_set & diff)
{
    POMAGMA_ASSERT4(m_item_dim == dep.m_item_dim, "dep has wrong size");
    POMAGMA_ASSERT4(m_item_dim == diff.m_item_dim, "diff has wrong size");

    Word * restrict d = dep.m_line;
    Word * restrict r = m_line;
    Word * restrict c = diff.m_line;

    Word changed = 0;
    for (size_t m = 0; m < m_word_dim; ++m) {
        changed |= (c[m] = d[m] & ~r[m]);
        r[m] |= d[m];
        d[m] = 0;
    }

    return changed;
}

// diff = src - this; this += src; return diff not empty;
bool dense_set::ensure (const dense_set & src, dense_set & diff)
{
    POMAGMA_ASSERT4(m_item_dim == src.m_item_dim, "src has wrong size");
    POMAGMA_ASSERT4(m_item_dim == diff.m_item_dim, "diff has wrong size");

    const Word * restrict d = src.m_line;
    Word * restrict r = m_line;
    Word * restrict c = diff.m_line;

    Word changed = 0;
    for (size_t m = 0; m < m_word_dim; ++m) {
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
    const Word * lines = m_set.m_line;
    do { if (++m_quot == m_set.m_word_dim) { m_i = 0; return; }
    } while (!lines[m_quot]);

    // traverse to first nonempty bit in a nonempty block
    Word word = lines[m_quot];
    for (m_rem = 0, m_mask = 1; !(m_mask & word); ++m_rem, m_mask <<= 1) {
        POMAGMA_ASSERT4(m_rem != BITS_PER_WORD, "found no bits");
    }
    m_i = m_rem + BITS_PER_WORD * m_quot;
    POMAGMA_ASSERT5(m_set.contains(m_i), "landed on empty pos " << m_i);
}

// PROFILE this is one of the slowest methods
void dense_set::iterator::next ()
{
    POMAGMA_ASSERT_OK
    Word word = m_set.m_line[m_quot];
    do {
        ++m_rem;
        //if (m_rem < BITS_PER_WORD) m_mask <<=1; // slow version
        if (m_rem & WORD_POS_MASK) m_mask <<= 1;    // fast version
        else { _next_block(); return; }
    } while (!(m_mask & word));
    m_i = m_rem + BITS_PER_WORD * m_quot;
    POMAGMA_ASSERT5(m_set.contains(m_i), "landed on empty pos " << m_i);
}

} // namespace pomagma

