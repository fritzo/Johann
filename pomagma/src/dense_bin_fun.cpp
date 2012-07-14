#include "dense_bin_fun.hpp"
#include "aligned_alloc.hpp"
#include <cstring>

namespace pomagma
{

dense_bin_fun::dense_bin_fun (size_t item_dim)
    : m_item_dim(item_dim),
      m_block_dim((m_item_dim + ITEMS_PER_BLOCK) / ITEMS_PER_BLOCK),
      m_word_dim(dense_set::word_count(m_item_dim)),
      m_blocks(pomagma::alloc_blocks<Block4x4>(m_block_dim * m_block_dim)),
      m_Lx_lines(pomagma::alloc_blocks<Word>((m_item_dim + 1) * m_word_dim)),
      m_Rx_lines(pomagma::alloc_blocks<Word>((m_item_dim + 1) * m_word_dim)),
      m_temp_set(m_item_dim, NULL),
      m_temp_line(pomagma::alloc_blocks<Word>(m_word_dim))
{
    POMAGMA_DEBUG("creating dense_bin_fun with "
            << (m_block_dim * m_block_dim) << " blocks");

    // FIXME allow larger
    POMAGMA_ASSERT(m_item_dim < (1<<15), "dense_bin_fun is too large");

    // initialize to zero
    bzero(m_blocks, m_block_dim * m_block_dim * sizeof(Block4x4));
    bzero(m_Lx_lines, (m_item_dim + 1) * m_word_dim * sizeof(Word));
    bzero(m_Rx_lines, (m_item_dim + 1) * m_word_dim * sizeof(Word));
}

dense_bin_fun::~dense_bin_fun ()
{
    pomagma::free_blocks(m_blocks);
    pomagma::free_blocks(m_Lx_lines);
    pomagma::free_blocks(m_Rx_lines);
    pomagma::free_blocks(m_temp_line);
}

// for growing
void dense_bin_fun::move_from (const dense_bin_fun & other)
{
    POMAGMA_DEBUG("Copying dense_bin_fun");

    // copy data
    size_t minM = min(m_block_dim, other.m_block_dim);
    for (size_t j_ = 0; j_ < minM; ++j_) {
        oid_t * destin = _block(0, j_);
        const oid_t * source = other._block(0, j_);
        memcpy(destin, source, sizeof(Block4x4) * minM);
    }

    // copy sets
    size_t minN = min(m_item_dim, other.m_item_dim);
    size_t minL = min(m_word_dim, other.m_word_dim);
    for (size_t i = 1; i <= minN; ++i) {
        memcpy(get_Lx_line(i), other.get_Lx_line(i), sizeof(Word) * minL);
        memcpy(get_Rx_line(i), other.get_Rx_line(i), sizeof(Word) * minL);
    }
}

//----------------------------------------------------------------------------
// Diagnostics

size_t dense_bin_fun::count_pairs () const
{
    size_t result = 0;
    for (size_t i = 1; i <= m_item_dim; ++i) {
        result += _get_Lx_set(i).count_items();
    }
    return result;
}

void dense_bin_fun::validate () const
{
    POMAGMA_DEBUG("Validating dense_bin_fun");

    POMAGMA_DEBUG("validating line-block consistency");
    for (size_t i_ = 0; i_ < m_block_dim; ++i_) {
    for (size_t j_ = 0; j_ < m_block_dim; ++j_) {
        const oid_t * block = _block(i_,j_);

        for (size_t _i = 0; _i < ITEMS_PER_BLOCK; ++_i) {
        for (size_t _j = 0; _j < ITEMS_PER_BLOCK; ++_j) {
            size_t i = i_ * ITEMS_PER_BLOCK+_i;
            size_t j = j_ * ITEMS_PER_BLOCK+_j;
            if (i == 0 or m_item_dim < i) continue;
            if (j == 0 or m_item_dim < j) continue;
            oid_t val = _block2value(block, _i, _j);

            if (val) {
                POMAGMA_ASSERT(contains(i, j),
                        "invalid: found unsupported value: "
                        << i << ',' << j);
            } else {
                POMAGMA_ASSERT(not contains(i, j),
                        "invalid: found supported null value: "
                        << i << ',' << j);
            }
        }}
    }}

    POMAGMA_DEBUG("validating left-right line consistency");
    for (size_t i = 1; i <= m_item_dim; ++i) {
        dense_set L_set(_get_Lx_set(i));

        for (size_t j = 1; j <= m_item_dim; ++j) {
            dense_set R_set(_get_Rx_set(j));

            if (L_set.contains(j) and not R_set.contains(i)) {
                POMAGMA_ERROR("invalid: L-set exceeds R-set: "
                        << i << "," << j);
            }
            if (R_set.contains(i) and not L_set.contains(j)) {
                POMAGMA_ERROR("invalid: R-set exceeds L-set: "
                        << i << "," << j);
            }
        }
    }
}

//----------------------------------------------------------------------------
// Operations

void dense_bin_fun::remove(
        const oid_t i,
        void remove_value(oid_t)) // rem
{
    POMAGMA_ASSERT4(0 < i and i <= m_item_dim,
            "item out of bounds: " << i);

    // (k,i)
    for (Iterator<RHS_FIXED> iter(this, i); iter.ok(); iter.next()) {
        oid_t k = iter.lhs();
        oid_t & dep = value(k, i);
        remove_value(dep);
        _get_Lx_set(k).remove(i);
        dep = 0;
    }
    _get_Rx_set(i).zero();

    // (i,k)
    for (Iterator<LHS_FIXED> iter(this, i); iter.ok(); iter.next()) {
        oid_t k = iter.rhs();
        oid_t & dep = value(i, k);
        remove_value(dep);
        _get_Rx_set(k).remove(i);
        dep = 0;
    }
    _get_Lx_set(i).zero();
}

void dense_bin_fun::merge(
        const oid_t i, // dep
        const oid_t j, // rep
        void merge_values(oid_t, oid_t), // dep,rep
        void move_value(oid_t, oid_t, oid_t)) // moved,lhs,rhs
{
    POMAGMA_ASSERT4(j != i,
            "in dense_bin_fun::merge, tried to merge with self");
    POMAGMA_ASSERT4(0 < i and i <= m_item_dim, "dep out of bounds: " << i);
    POMAGMA_ASSERT4(0 < j and j <= m_item_dim, "rep out of bounds: " << j);

    // Note: the spacial case (i,i) --> (i,j) --> (j,j) merges in two steps

    // (k,i) --> (k,j)
    for (Iterator<RHS_FIXED> iter(this,i); iter.ok(); iter.next()) {
        oid_t k = iter.lhs();
        oid_t & dep = value(k,i);
        oid_t & rep = value(k,j);
        _get_Lx_set(k).remove(i); // sets m_temp_set
        if (rep) {
            merge_values(dep, rep);
        } else {
            move_value(dep, k, j);
            m_temp_set.insert(j); // ie, _get_Lx_set(k).insert(j), as above
            rep = dep;
        }
        dep = 0;
    }
    dense_set Rx_rep = _get_Rx_set(j);
    dense_set Rx_dep = _get_Rx_set(i);
    Rx_rep.merge(Rx_dep);

    // (i,k) --> (j,k)
    for (Iterator<LHS_FIXED> iter(this,i); iter.ok(); iter.next()) {
        oid_t k = iter.rhs();
        oid_t & dep = value(i, k);
        oid_t & rep = value(j, k);
        _get_Rx_set(k).remove(i); // sets m_temp_set
        if (rep) {
            merge_values(dep,rep);
        } else {
            move_value(dep, j, k);
            m_temp_set.insert(j); // ie, _get_Rx_set(k).insert(j), as above
            rep = dep;
        }
        dep = 0;
    }
    dense_set Lx_rep = _get_Lx_set(j);
    dense_set Lx_dep = _get_Lx_set(i);
    Lx_rep.merge(Lx_dep);
}

// intersection iteration
Word * dense_bin_fun::_get_RRx_line (oid_t i, oid_t j) const
{
    Word * i_line = get_Rx_line(i);
    Word * j_line = get_Rx_line(j);
    for (size_t k_ = 0; k_ < m_word_dim; ++k_) {
        m_temp_line[k_] = i_line[k_] & j_line[k_];
    }
    return m_temp_line;
}
Word * dense_bin_fun::_get_LRx_line (oid_t i, oid_t j) const
{
    Word * i_line = get_Lx_line(i);
    Word * j_line = get_Rx_line(j);
    for (size_t k_ = 0; k_ < m_word_dim; ++k_) {
        m_temp_line[k_] = i_line[k_] & j_line[k_];
    }
    return m_temp_line;
}
Word * dense_bin_fun::_get_LLx_line (oid_t i, oid_t j) const
{
    Word * i_line = get_Lx_line(i);
    Word * j_line = get_Lx_line(j);
    for (size_t k_ = 0; k_ < m_word_dim; ++k_) {
        m_temp_line[k_] = i_line[k_] & j_line[k_];
    }
    return m_temp_line;
}

} // namespace pomagma
