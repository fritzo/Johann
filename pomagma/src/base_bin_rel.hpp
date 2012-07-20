#ifndef POMAGMA_BASE_BIN_REL_HPP
#define POMAGMA_BASE_BIN_REL_HPP

#include "util.hpp"
#include "dense_set.hpp"

namespace pomagma
{

// WARNING zero/null items are not allowed

class base_bin_rel : noncopyable
{
    const size_t m_item_dim;
    const size_t m_word_dim;
    const size_t m_round_item_dim;
    const size_t m_round_word_dim;
    Word * const m_Lx_lines;
    Word * const m_Rx_lines;
    //const dense_set & m_support; // TODO add for validation

    bool _symmetric () const { return m_Lx_lines == m_Rx_lines; }

public:

    base_bin_rel (size_t item_dim, bool symmetric);
    ~base_bin_rel ();
    void move_from (const base_bin_rel & other); // for growing

    // full table
    Word * Lx () const { return m_Lx_lines; }
    Word * Rx () const { return m_Rx_lines; }

    // single line
    Word * Lx (oid_t lhs) const
    {
        POMAGMA_ASSERT_RANGE_(5, lhs, m_item_dim);
        return m_Lx_lines + (lhs * m_word_dim);
    }
    Word * Rx (oid_t rhs) const
    {
        POMAGMA_ASSERT_RANGE_(5, rhs, m_item_dim);
        return m_Rx_lines + (rhs * m_word_dim);
    }

    // single element
    bool Lx (oid_t lhs, oid_t rhs) const
    {
        POMAGMA_ASSERT_RANGE_(5, rhs, m_item_dim);
        return bool_ref::index(Lx(lhs), rhs);
    }
    bool Rx (oid_t lhs, oid_t rhs) const
    {
        POMAGMA_ASSERT_RANGE_(5, lhs, m_item_dim);
        return bool_ref::index(Rx(rhs), lhs);
    }
    bool_ref Lx (oid_t lhs, oid_t rhs)
    {
        POMAGMA_ASSERT_RANGE_(5, rhs, m_item_dim);
        return bool_ref::index(Lx(lhs), rhs);
    }
    bool_ref Rx (oid_t lhs, oid_t rhs)
    {
        POMAGMA_ASSERT_RANGE_(5, lhs, m_item_dim);
        return bool_ref::index(Rx(rhs), lhs);
    }

    // access
    void validate () const;
};

} // namespace pomagma

#endif // POMAGMA_BASE_BIN_REL_HPP
