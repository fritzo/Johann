
#include "base_bin_rel.hpp"
#include "aligned_alloc.hpp"
#include <cstring>

namespace pomagma
{

base_bin_rel::base_bin_rel (size_t item_dim, bool symmetric)
    : m_item_dim(item_dim),
      m_word_dim(dense_set::word_count(m_item_dim)),
      m_round_item_dim(m_word_dim * BITS_PER_WORD - 1),
      m_round_word_dim(m_word_dim * m_round_item_dim),
      m_Lx_lines(pomagma::alloc_blocks<Word>(m_round_word_dim)),
      m_Rx_lines(symmetric ? m_Lx_lines
                           : pomagma::alloc_blocks<Word>(m_round_word_dim))
{
    POMAGMA_DEBUG("creating base_bin_rel with " << m_word_dim << " lines");
    POMAGMA_ASSERT(m_round_item_dim <= MAX_ITEM_DIM,
            "base_bin_rel is too large");

    // initialize to zeros
    bzero(m_Lx_lines, sizeof(Word) * m_round_word_dim);
    if (not symmetric) {
        bzero(m_Rx_lines, sizeof(Word) * m_round_word_dim);
    }
}

base_bin_rel::~base_bin_rel ()
{
    pomagma::free_blocks(m_Lx_lines);
    if (not _symmetric()) {
        pomagma::free_blocks(m_Rx_lines);
    }
}

// for growing
void base_bin_rel::move_from (const base_bin_rel & other)
{
    POMAGMA_DEBUG("Copying base_bin_rel");

    size_t minN = min(m_item_dim, other.m_item_dim);
    size_t minL = min(m_word_dim, other.m_word_dim);

    if (_symmetric()) {
        POMAGMA_ASSERT(other._symmetric(), "symmetry mismatch");
        for (size_t i = 1; i <= minN; ++i) {
            memcpy(Lx(i), other.Lx(i), sizeof(Word) * minL);
        }
    } else {
        for (size_t i = 1; i <= minN; ++i) {
            memcpy(Lx(i), other.Lx(i), sizeof(Word) * minL);
            memcpy(Rx(i), other.Rx(i), sizeof(Word) * minL);
        }
    }
}

void base_bin_rel::validate() const
{
    if (_symmetric()) {

        // check emptiness outside of support
        dense_set set(m_item_dim, NULL);
        dense_set round_set(m_round_item_dim, NULL);
        for (oid_t i = 0; i < m_round_item_dim; ++i) {
            if (1 <= i and i <= m_item_dim) {
                set.init(Lx(i));
                set.validate();
            } else {
                round_set.init(m_Lx_lines + m_word_dim * i);
                round_set.validate();
                POMAGMA_ASSERT(round_set.empty(),
                        "unsupported Lx(" << i << ") has " <<
                        round_set.count_items() << " items");
            }
        }

        // check for Lx/Rx agreement
        for (oid_t i = 1; i <= m_item_dim; ++i) {
        for (oid_t j = i; j <= m_item_dim; ++j) {
            POMAGMA_ASSERT(Lx(i, j) == Rx(i, j),
                    "Lx, Rx disagree at " << i << ',' << j);
        }}

    } else {

        // check emptiness outside of support
        dense_set set(m_item_dim, NULL);
        dense_set round_set(m_round_item_dim, NULL);
        for (oid_t i = 0; i < m_round_item_dim; ++i) {
            if (1 <= i and i <= m_item_dim) {
                set.init(Lx(i));
                set.validate();
                set.init(Rx(i));
                set.validate();
            } else {
                round_set.init(m_Lx_lines + m_word_dim * i);
                round_set.validate();
                POMAGMA_ASSERT(round_set.empty(),
                        "unsupported Lx(" << i << ") has " <<
                        round_set.count_items() << " items");
                round_set.init(m_Rx_lines + m_word_dim * i);
                round_set.validate();
                POMAGMA_ASSERT(round_set.empty(),
                        "unsupported Rx(" << i << ") has " <<
                        round_set.count_items() << " items");
            }
        }

        // check for Lx/Rx agreement
        for (oid_t i = 1; i <= m_item_dim; ++i) {
        for (oid_t j = 1; j <= m_item_dim; ++j) {
            POMAGMA_ASSERT(Lx(i, j) == Rx(i, j),
                    "Lx, Rx disagree at " << i << ',' << j);
        }}
    }
}

} // namespace pomagma
