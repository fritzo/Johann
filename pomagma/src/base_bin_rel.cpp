#include "base_bin_rel.hpp"
#include "aligned_alloc.hpp"
#include <cstring>

namespace pomagma
{

template<bool symmetric>
base_bin_rel_<symmetric>::base_bin_rel_ (const dense_set & support)
    : m_support(support, yes_copy_construct),
      m_round_item_dim(dense_set::round_item_dim(item_dim())),
      m_round_word_dim(dense_set::round_word_dim(item_dim())),
      m_data_size_words((1 + m_round_item_dim) * m_round_word_dim),
      m_Lx_lines(pomagma::alloc_blocks<Word>(m_data_size_words)),
      m_Rx_lines(symmetric ? m_Lx_lines
                           : pomagma::alloc_blocks<Word>(m_data_size_words))
{
    POMAGMA_DEBUG("creating base_bin_rel_ with "
            << m_data_size_words << " words");
    POMAGMA_ASSERT(m_round_item_dim <= MAX_ITEM_DIM,
            "base_bin_rel_ is too large");

    bzero(m_Lx_lines, sizeof(Word) * m_data_size_words);
    if (not symmetric) {
        bzero(m_Rx_lines, sizeof(Word) * m_data_size_words);
    }
}

template<bool symmetric>
base_bin_rel_<symmetric>::~base_bin_rel_ ()
{
    pomagma::free_blocks(m_Lx_lines);
    if (not symmetric) {
        pomagma::free_blocks(m_Rx_lines);
    }
}

// for growing
template<bool symmetric>
void base_bin_rel_<symmetric>::move_from (
        const base_bin_rel_<symmetric> & other)
{
    POMAGMA_DEBUG("Copying base_bin_rel_");

    size_t min_item_dim = min(item_dim(), other.item_dim());
    size_t min_word_dim = min(word_dim(), other.word_dim());

    if (symmetric) {
        for (size_t i = 1; i <= min_item_dim; ++i) {
            memcpy(Lx(i), other.Lx(i), sizeof(Word) * min_word_dim);
        }
    } else {
        for (size_t i = 1; i <= min_item_dim; ++i) {
            memcpy(Lx(i), other.Lx(i), sizeof(Word) * min_word_dim);
            memcpy(Rx(i), other.Rx(i), sizeof(Word) * min_word_dim);
        }
    }
}

template<bool symmetric>
void base_bin_rel_<symmetric>::validate() const
{
    support().validate();

    if (symmetric) {

        // check emptiness outside of support
        dense_set set(item_dim(), NULL);
        dense_set round_set(m_round_item_dim, NULL);
        for (oid_t i = 0; i < m_round_item_dim; ++i) {
            if (1 <= i and i <= item_dim() and m_support.contains(i)) {
                set.init(Lx(i));
                set.validate();
                POMAGMA_ASSERT(set <= support(), "Lx(i) exceeds support");
            } else {
                round_set.init(m_Lx_lines + m_round_word_dim * i);
                round_set.validate();
                POMAGMA_ASSERT(round_set.empty(),
                        "unsupported Lx(" << i << ") has " <<
                        round_set.count_items() << " items");
            }
        }

        // check for Lx/Rx agreement
        for (oid_t i = 1; i <= item_dim(); ++i) {
        for (oid_t j = i; j <= item_dim(); ++j) {
            POMAGMA_ASSERT(Lx(i, j) == Rx(i, j),
                    "Lx, Rx disagree at " << i << ',' << j);
        }}

    } else {

        // check emptiness outside of support
        dense_set set(item_dim(), NULL);
        dense_set round_set(m_round_item_dim, NULL);
        for (oid_t i = 0; i < m_round_item_dim; ++i) {
            if (1 <= i and i <= item_dim() and m_support.contains(i)) {
                set.init(Lx(i));
                set.validate();
                POMAGMA_ASSERT(set <= support(), "Lx(i) exceeds support");
                set.init(Rx(i));
                set.validate();
                POMAGMA_ASSERT(set <= support(), "Rx(i) exceeds support");
            } else {
                round_set.init(m_Lx_lines + m_round_word_dim * i);
                round_set.validate();
                POMAGMA_ASSERT(round_set.empty(),
                        "unsupported Lx(" << i << ") has " <<
                        round_set.count_items() << " items");
                round_set.init(m_Rx_lines + m_round_word_dim * i);
                round_set.validate();
                POMAGMA_ASSERT(round_set.empty(),
                        "unsupported Rx(" << i << ") has " <<
                        round_set.count_items() << " items");
            }
        }

        // check for Lx/Rx agreement
        for (dense_set::iterator i(m_support); i.ok(); i.next()) {
        for (dense_set::iterator j(m_support); j.ok(); j.next()) {
            POMAGMA_ASSERT(Lx(*i, *j) == Rx(*i, *j),
                    "Lx, Rx disagree at " << *i << ',' << *j);
        }}
    }
}

//----------------------------------------------------------------------------
// Explicit template instantiation

template base_bin_rel_<true>::base_bin_rel_ (const dense_set &);
template base_bin_rel_<true>::~base_bin_rel_ ();
template void base_bin_rel_<true>::validate () const;
template void base_bin_rel_<true>::move_from (const base_bin_rel_<true> &);

template base_bin_rel_<false>::base_bin_rel_ (const dense_set &);
template base_bin_rel_<false>::~base_bin_rel_ ();
template void base_bin_rel_<false>::validate () const;
template void base_bin_rel_<false>::move_from (const base_bin_rel_<false> &);

} // namespace pomagma
