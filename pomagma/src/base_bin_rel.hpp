#ifndef POMAGMA_BASE_BIN_REL_HPP
#define POMAGMA_BASE_BIN_REL_HPP

#include "util.hpp"
#include "dense_set.hpp"

namespace pomagma
{

// WARNING zero/null items are not allowed

template<bool symmetric>
class base_bin_rel_ : noncopyable
{
    // TODO switch m_support to Carrier & m_carrier
    const dense_set m_support; // aliased
    const size_t m_round_item_dim;
    const size_t m_round_word_dim;
    const size_t m_data_size_words;
    Word * const m_Lx_lines;
    Word * const m_Rx_lines;

public:

    base_bin_rel_ (const dense_set & support);
    ~base_bin_rel_ ();
    void move_from (const base_bin_rel_<symmetric> & other); // for growing
    void validate () const;

    const dense_set & support () const { return m_support; }
    size_t item_dim () const { return m_support.item_dim(); }
    size_t word_dim () const { return m_support.word_dim(); }
    size_t round_item_dim () const { return m_round_item_dim; }
    size_t round_word_dim () const { return m_round_word_dim; }
    size_t data_size_words () const { return m_data_size_words; }

    // full table
    Word * Lx () const { return m_Lx_lines; }
    Word * Rx () const { return m_Rx_lines; }

    // single line
    Word * Lx (oid_t lhs) const
    {
        POMAGMA_ASSERT_RANGE_(5, lhs, item_dim());
        return m_Lx_lines + (lhs * m_round_word_dim);
    }
    Word * Rx (oid_t rhs) const
    {
        POMAGMA_ASSERT_RANGE_(5, rhs, item_dim());
        return m_Rx_lines + (rhs * m_round_word_dim);
    }

    // single element
    bool Lx (oid_t lhs, oid_t rhs) const
    {
        POMAGMA_ASSERT_RANGE_(5, rhs, item_dim());
        return bool_ref::index(Lx(lhs), rhs);
    }
    bool Rx (oid_t lhs, oid_t rhs) const
    {
        POMAGMA_ASSERT_RANGE_(5, lhs, item_dim());
        return bool_ref::index(Rx(rhs), lhs);
    }
    bool_ref Lx (oid_t lhs, oid_t rhs)
    {
        POMAGMA_ASSERT_RANGE_(5, rhs, item_dim());
        return bool_ref::index(Lx(lhs), rhs);
    }
    bool_ref Rx (oid_t lhs, oid_t rhs)
    {
        POMAGMA_ASSERT_RANGE_(5, lhs, item_dim());
        return bool_ref::index(Rx(rhs), lhs);
    }

    // set wrappers
    dense_set Lx_set (oid_t lhs) const
    {
        return dense_set(item_dim(), Lx(lhs));
    }
    dense_set Rx_set (oid_t rhs) const
    {
        return dense_set(item_dim(), Rx(rhs));
    }
};

typedef base_bin_rel_<false> base_bin_rel;
typedef base_bin_rel_<true> base_sym_rel;

} // namespace pomagma

#endif // POMAGMA_BASE_BIN_REL_HPP
