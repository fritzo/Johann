#ifndef POMAGMA_DENSE_BIN_FUN_H
#define POMAGMA_DENSE_BIN_FUN_H

#include "util.hpp"
#include "dense_set.hpp"

namespace pomagma
{

// WARNING zero/null items are not allowed

// a tight binary function in 4x4 word blocks
class dense_bin_fun
{
    // data, in blocks
    const size_t m_item_dim;
    const size_t m_block_dim;
    const size_t m_word_dim;
    Block4x4 * const m_blocks;

    // dense sets for iteration
    Word * const m_Lx_lines;
    Word * const m_Rx_lines;
    mutable dense_set m_temp_set; // TODO FIXME this is not thread-safe
    mutable Word * m_temp_line; // TODO FIXME this is not thread-safe

    // block wrappers
    oid_t * _block (size_t i_, size_t j_)
    {
        return m_blocks[m_block_dim * j_ + i_];
    }
    const oid_t * _block (size_t i_, size_t j_) const
    {
        return m_blocks[m_block_dim * j_ + i_];
    }

    // set wrappers
public:
    Word * get_Lx_line (oid_t i) const { return m_Lx_lines + (i * m_word_dim); }
    Word * get_Rx_line (oid_t i) const { return m_Rx_lines + (i * m_word_dim); }
private:
    // TODO remove these
    dense_set & _get_Lx_set (oid_t i)
    {
        return m_temp_set.init(get_Lx_line(i));
    }
    dense_set & _get_Rx_set (oid_t i)
    {
        return m_temp_set.init(get_Rx_line(i));
    }
    const dense_set & _get_Lx_set (oid_t i) const
    {
        return m_temp_set.init(get_Lx_line(i));
    }
    const dense_set & _get_Rx_set (oid_t i) const
    {
        return m_temp_set.init(get_Rx_line(i));
    }

    // intersection wrappers
    Word * _get_RRx_line (oid_t i, oid_t j) const;
    Word * _get_LRx_line (oid_t i, oid_t j) const;
    Word * _get_LLx_line (oid_t i, oid_t j) const;

    // ctors & dtors
public:
    dense_bin_fun (size_t item_dim);
    ~dense_bin_fun ();
    void move_from (const dense_bin_fun & other); // for growing

    // function calling
private:
    inline oid_t & value (oid_t lhs, oid_t rhs);
public:
    inline oid_t value (oid_t lhs, oid_t rhs) const;
    oid_t get_value (oid_t lhs, oid_t rhs) const { return value(lhs, rhs); }

    // attributes
    size_t count_pairs () const; // slow!
    size_t item_dim () const { return m_item_dim; }
    void validate () const;

    // element operations
    // TODO add a replace method for merging
    // TODO add a return value for atomic operations
    void insert (oid_t lhs, oid_t rhs, oid_t val)
    {
        oid_t & old_val = value(lhs, rhs);
        POMAGMA_ASSERT2(old_val, "double insertion: " << lhs << "," << rhs);
        old_val = val;
        _get_Lx_set(lhs).insert(rhs);
        _get_Rx_set(rhs).insert(lhs);
        //return was_inserted; // TODO
    }
    void remove (oid_t lhs, oid_t rhs)
    {
        oid_t & old_val = value(lhs, rhs);
        POMAGMA_ASSERT2(old_val, "double removal: " << lhs << "," << rhs);
        old_val = 0;
        _get_Lx_set(lhs).remove(rhs);
        _get_Rx_set(rhs).remove(lhs);
        //return was_removed; // TODO
    }
    bool contains (oid_t lhs, oid_t rhs) const
    {
        return _get_Lx_set(lhs).contains(rhs);
    }

    // support operations
    void remove (
            const oid_t i,
            void remove_value(oid_t)); // rem
    void merge (
            const oid_t i,
            const oid_t j,
            void merge_values(oid_t, oid_t), // dep,rep
            void move_value(oid_t, oid_t, oid_t)); // moved,lhs,rhs

    // iteration
    class lr_iterator;
    enum { LHS_FIXED = false, RHS_FIXED = true };
    template<bool idx> class Iterator;
    class RRxx_Iter;
    class LRxx_Iter;
    class LLxx_Iter;
};

inline oid_t & dense_bin_fun::value (oid_t i, oid_t j)
{
    POMAGMA_ASSERT5(0 < i and i <= m_item_dim,
            "i=" << i << " out of bounds [1," << m_item_dim << "]");
    POMAGMA_ASSERT5(0 < j and j <= m_item_dim,
            "j=" << j<< " out of bounds [1," << m_item_dim << "]");

    oid_t * block = _block(i / ITEMS_PER_BLOCK, j / ITEMS_PER_BLOCK);
    return _block2value(block, i & BLOCK_POS_MASK, j & BLOCK_POS_MASK);
}

inline oid_t dense_bin_fun::value (oid_t i, oid_t j) const
{
    POMAGMA_ASSERT5(0 < i and i <= m_item_dim,
            "i=" << i << " out of bounds [1," << m_item_dim << "]");
    POMAGMA_ASSERT5(0 < j and j <= m_item_dim,
            "j=" << j <<" out of bounds [1," << m_item_dim << "]");

    const oid_t * block = _block(i / ITEMS_PER_BLOCK, j / ITEMS_PER_BLOCK);
    return _block2value(block, i & BLOCK_POS_MASK, j & BLOCK_POS_MASK);
}

//----------------------------------------------------------------------------
// Iteration over full table

class dense_bin_fun::lr_iterator : noncopyable
{
    const dense_bin_fun & m_fun;
    oid_t m_lhs;
    dense_set m_rhs_set;
    dense_set::iterator m_rhs_iter;

public:

    // construction
    lr_iterator (const dense_bin_fun & fun)
        : m_fun(fun),
          m_lhs(1),
          m_rhs_set(fun.item_dim(), fun.get_Lx_line(1)),
          m_rhs_iter(m_rhs_set)
    {
        _find_nonempty_rhs();
    }

    // traversal
private:
    void _find_nonempty_rhs ()
    {
        while (not m_rhs_iter.ok()) {
            ++m_lhs;
            if (m_lhs == m_fun.item_dim()) {
                m_lhs = 0;
                return;
            }
            m_rhs_set.init(m_fun.get_Lx_line(m_lhs));
            m_rhs_iter.begin();
        }
    }
public:
    void begin ()
    {
        m_lhs = 1;
        m_rhs_set.init(m_fun.get_Lx_line(1));
        m_rhs_iter.begin();
        _find_nonempty_rhs();
    }
    void next ()
    {
        POMAGMA_ASSERT_OK
        m_rhs_iter.next();
        _find_nonempty_rhs();
    }
    bool ok () const { return m_lhs; }

    // access
    oid_t lhs () const { POMAGMA_ASSERT_OK return m_lhs; }
    oid_t rhs () const { POMAGMA_ASSERT_OK return *m_rhs_iter; }
    oid_t value () const
    {
        POMAGMA_ASSERT_OK
        return m_fun.get_value(m_lhs, *m_rhs_iter);
    }
};

//----------------------------------------------------------------------------
// Iteration over a line

template<bool idx>
class dense_bin_fun::Iterator : noncopyable
{
    dense_set m_set;
    dense_set::iterator m_iter;
    const dense_bin_fun & m_fun;
    oid_t m_lhs;
    oid_t m_rhs;

public:

    // construction
    Iterator (const dense_bin_fun * fun)
        : m_set(fun->m_item_dim, NULL),
          m_iter(m_set, false),
          m_fun(*fun),
          m_lhs(0),
          m_rhs(0)
    {
    }
    Iterator (const dense_bin_fun * fun, oid_t fixed)
        : m_set(fun->m_item_dim,
                idx ? fun->get_Rx_line(fixed)
                    : fun->get_Lx_line(fixed)),
          m_iter(m_set, false),
          m_fun(*fun),
          m_lhs(fixed),
          m_rhs(fixed)
    {
        begin();
    }

    // traversal
private:
    void _set_pos () { if (idx) m_lhs = *m_iter; else m_rhs = *m_iter; }
public:
    bool ok () const { return m_iter.ok(); }
    void begin () { m_iter.begin(); if (ok()) _set_pos(); }
    void begin (oid_t fixed)
    {
        if (idx) { m_rhs=fixed; m_set.init(m_fun.get_Rx_line(fixed)); }
        else     { m_lhs=fixed; m_set.init(m_fun.get_Lx_line(fixed)); }
        begin();
    }
    void next () { m_iter.next(); if (ok()) _set_pos(); }

    // dereferencing
    oid_t lhs () const { POMAGMA_ASSERT_OK return m_lhs; }
    oid_t rhs () const { POMAGMA_ASSERT_OK return m_rhs; }
    oid_t value () const
    {
        POMAGMA_ASSERT_OK
        return m_fun.get_value(m_lhs, m_rhs);
    }
};

//----------------------------------------------------------------------------
// Intersection iteration over two lines

class dense_bin_fun::RRxx_Iter : noncopyable
{
    dense_set m_set;
    dense_set::iterator m_iter;
    const dense_bin_fun & m_fun;
    oid_t m_lhs;
    oid_t m_rhs1;
    oid_t m_rhs2;

public:

    // construction
    RRxx_Iter (const dense_bin_fun * fun)
        : m_set(fun->m_item_dim, NULL),
          m_iter(m_set, false),
          m_fun(*fun)
    {}

    // traversal
    void begin () { m_iter.begin(); if (ok()) m_lhs = *m_iter; }
    void begin (oid_t fixed1, oid_t fixed2)
    {
        m_set.init(m_fun._get_RRx_line(fixed1, fixed2));
        m_iter.begin();
        if (ok()) {
            m_rhs1 = fixed1;
            m_rhs2 = fixed2;
            m_lhs = *m_iter;
        }
    }
    bool ok () const { return m_iter.ok(); }
    void next () { m_iter.next(); if (ok()) m_lhs = *m_iter; }

    // dereferencing
    oid_t lhs () const { POMAGMA_ASSERT_OK return m_lhs; }
    oid_t value1 () const
    {
        POMAGMA_ASSERT_OK
        return m_fun.get_value(m_lhs, m_rhs1);
    }
    oid_t value2 () const
    {
        POMAGMA_ASSERT_OK
        return m_fun.get_value(m_lhs, m_rhs2);
    }
};

class dense_bin_fun::LRxx_Iter : noncopyable
{
    dense_set m_set;
    dense_set::iterator m_iter;
    const dense_bin_fun & m_fun;
    oid_t m_lhs1;
    oid_t m_rhs2;
    oid_t m_rhs1;

public:

    // construction
    LRxx_Iter (const dense_bin_fun * fun)
        : m_set(fun->m_item_dim, NULL),
          m_iter(m_set, false),
          m_fun(*fun)
    {}

    // traversal
    void begin () { m_iter.begin(); if (ok()) m_rhs1 = *m_iter; }
    void begin (oid_t fixed1, oid_t fixed2)
    {
        m_set.init(m_fun._get_LRx_line(fixed1, fixed2));
        m_iter.begin();
        if (ok()) {
            m_lhs1 = fixed1;
            m_rhs2 = fixed2;
            m_rhs1 = *m_iter;
        }
    }
    bool ok () const { return m_iter.ok(); }
    void next () { m_iter.next(); if (ok()) m_rhs1 = *m_iter; }

    // dereferencing
    oid_t rhs1 () const { POMAGMA_ASSERT_OK return m_rhs1; }
    oid_t lhs2 () const { POMAGMA_ASSERT_OK return m_rhs1; }
    oid_t value1 () const
    {
        POMAGMA_ASSERT_OK
        return m_fun.get_value(m_lhs1, m_rhs1);
    }
    oid_t value2 () const
    {
        POMAGMA_ASSERT_OK
        return m_fun.get_value(m_rhs1, m_rhs2);
    }
};

class dense_bin_fun::LLxx_Iter : noncopyable
{
    dense_set           m_set;
    dense_set::iterator m_iter;
    const dense_bin_fun & m_fun;
    oid_t m_lhs1;
    oid_t m_lhs2;
    oid_t m_rhs;

public:

    // construction
    LLxx_Iter (const dense_bin_fun * fun)
        : m_set(fun->m_item_dim, NULL),
          m_iter(m_set, false),
          m_fun(*fun)
    {
    }

    // traversal
    void begin () { m_iter.begin(); if (ok()) m_rhs = *m_iter; }
    void begin (oid_t fixed1, oid_t fixed2)
    {
        m_set.init(m_fun._get_LLx_line(fixed1, fixed2));
        m_iter.begin();
        if (ok()) {
            m_lhs1 = fixed1;
            m_lhs2 = fixed2;
            m_rhs = *m_iter;
        }
    }
    bool ok () const { return m_iter.ok(); }
    void next () { m_iter.next(); if (ok()) m_rhs = *m_iter; }

    // dereferencing
    oid_t rhs () const { POMAGMA_ASSERT_OK return m_rhs; }
    oid_t value1 () const
    {
        POMAGMA_ASSERT_OK
        return m_fun.get_value(m_lhs1, m_rhs);
    }
    oid_t value2 () const
    {
        POMAGMA_ASSERT_OK
        return m_fun.get_value(m_lhs2, m_rhs);
    }
};

} // namespace pomagma

#endif // POMAGMA_DENSE_BIN_FUN_H

