#ifndef POMAGMA_DENSE_SYM_FUN_H
#define POMAGMA_DENSE_SYM_FUN_H

#include "util.hpp"
#include "dense_set.hpp"

namespace pomagma
{

// WARNING zero/null items are not allowed

inline size_t unordered_pair_count (size_t i) { return (i * (i + 1)) / 2; }

// a tight binary function in 4x4 word blocks
class dense_sym_fun
{
    // data, in blocks
    const size_t m_item_dim;
    const size_t m_block_dim;
    const size_t m_word_dim;
    Block4x4 * const m_blocks;

    // dense sets for iteration
    Word * const m_Lx_lines;
    mutable dense_set m_temp_set; // TODO FIXME this is not thread-safe
    mutable Word * m_temp_line; // TODO FIXME this is not thread-safe

    // block wrappers
    template<class T>
    static void sort (T & i, T & j) { if (j < i) { T k = j; j = i; i = k; }  }
    oid_t * _block (int i_, int j_)
    {
        return m_blocks[unordered_pair_count(j_) + i_];
    }
    const oid_t * _block (int i_, int j_) const
    {
        return m_blocks[unordered_pair_count(j_) + i_];
    }
    static oid_t & _block2value (oid_t * block, oid_t i, oid_t j)
    {
        return block[(j << 2) | i];
    }
    static oid_t _block2value (const oid_t * block, oid_t i, oid_t j)
    {
        return block[(j << 2) | i];
    }

    // set wrappers
public:
    Word * get_Lx_line (oid_t i) const { return m_Lx_lines + (i * m_word_dim); }
private:
    dense_set & _get_Lx_set (oid_t i) { return m_temp_set.init(get_Lx_line(i)); }
    const dense_set & _get_Lx_set (oid_t i) const
    {
        return m_temp_set.init(get_Lx_line(i));
    }

    // intersection wrappers
    Word * _get_LLx_line (oid_t i, oid_t j) const;

    // ctors & dtors
public:
    dense_sym_fun (size_t item_dim);
    ~dense_sym_fun ();
    void move_from (const dense_sym_fun & other); // for growing

    // function calling
private:
    inline oid_t & value (oid_t lhs, oid_t rhs);
public:
    inline oid_t value (oid_t lhs, oid_t rhs) const;
    oid_t get_value (oid_t lhs, oid_t rhs) const { return value(lhs, rhs); }

    // attributes
    size_t item_dim () const { return m_item_dim; }
    unsigned count_pairs () const; // slow!
    void validate () const;

    // element operations
    void insert (oid_t lhs, oid_t rhs, oid_t val)
    {
        value(lhs,rhs) = val;
        _get_Lx_set(lhs).insert(rhs); if (lhs == rhs) return;
        _get_Lx_set(rhs).insert(lhs);
    }
    void remove (oid_t lhs, oid_t rhs)
    {
        value(lhs,rhs) = 0;
        _get_Lx_set(lhs).remove(rhs); if (lhs == rhs) return;
        _get_Lx_set(rhs).remove(lhs);
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
            void merge_values(oid_t, oid_t),     // dep, rep
            void move_value(oid_t, oid_t, oid_t)); // moved, lhs, rhs

    //------------------------------------------------------------------------
    // Iteration over a line

    class Iterator : noncopyable
    {
        dense_set m_set;
        dense_set::iterator m_iter;
        const dense_sym_fun * m_fun;
        oid_t m_fixed;
        oid_t m_moving;

    public:

        // construction
        Iterator (const dense_sym_fun * fun)
            : m_set(fun->m_item_dim, NULL),
              m_iter(m_set, false),
              m_fun(fun),
              m_fixed(0),
              m_moving(0)
        {}
        Iterator (const dense_sym_fun * fun, oid_t fixed)
            : m_set(fun->m_item_dim, fun->get_Lx_line(fixed)),
              m_iter(m_set, false),
              m_fun(fun),
              m_fixed(fixed),
              m_moving(0)
        {
            begin();
        }

        // traversal
    private:
        void _set_pos () { m_moving = *m_iter; }
    public:
        bool ok () const { return m_iter.ok(); }
        void begin () { m_iter.begin(); if (ok()) _set_pos(); }
        void begin (oid_t fixed)
        {
            m_fixed=fixed;
            m_set.init(m_fun->get_Lx_line(fixed));
            begin();
        }
        void next () { m_iter.next(); if (ok()) _set_pos(); }

        // dereferencing
    private:
        void _deref_assert () const
        {
            POMAGMA_ASSERT5(ok(), "dereferenced done dense_set::iter");
        }
    public:
        oid_t fixed () const { _deref_assert(); return m_fixed; }
        oid_t moving () const { _deref_assert(); return m_moving; }
        oid_t value () const
        {
            _deref_assert();
            return m_fun->get_value(m_fixed,m_moving);
        }
    };

    //------------------------------------------------------------------------
    // Intersection iteration over 2 lines

    class LLxx_Iter : noncopyable
    {
        dense_set m_set;
        dense_set::iterator m_iter;
        const dense_sym_fun * m_fun;
        oid_t m_fixed1;
        oid_t m_fixed2;
        oid_t m_moving;

    public:

        // construction
        LLxx_Iter (const dense_sym_fun* fun)
            : m_set(fun->m_item_dim, NULL),
              m_iter(m_set, false),
              m_fun(fun)
        {}

        // traversal
        void begin () { m_iter.begin(); if (ok()) m_moving = *m_iter; }
        void begin (oid_t fixed1, oid_t fixed2)
        {
            m_set.init(m_fun->_get_LLx_line(fixed1, fixed2));
            m_iter.begin();
            if (ok()) {
                m_fixed1 = fixed1;
                m_fixed2 = fixed2;
                m_moving = *m_iter;
            }
        }
        bool ok () const { return m_iter.ok(); }
        void next () { m_iter.next(); if (ok()) m_moving = *m_iter; }

        // dereferencing
        oid_t fixed1 () const { return m_fixed1; }
        oid_t fixed2 () const { return m_fixed2; }
        oid_t moving () const { return m_moving; }
        oid_t value1 () const { return m_fun->get_value(m_fixed1, m_moving); }
        oid_t value2 () const { return m_fun->get_value(m_fixed2, m_moving); }
    };
};

//----------------------------------------------------------------------------
// Function calling

inline oid_t & dense_sym_fun::value (oid_t i, oid_t j)
{
    sort(i, j);
    POMAGMA_ASSERT5(0 < i and i <= m_item_dim,
            "i=" << i << " out of bounds [1," << m_item_dim << "]");
    POMAGMA_ASSERT5(i < j and j <= m_item_dim,
            "j=" << j << " out of bounds [" << i << "," << m_item_dim << "]");

    oid_t * block = _block(i >> 2, j >> 2);
    return _block2value(block, i & 3, j & 3);
}

inline oid_t dense_sym_fun::value (oid_t i, oid_t j) const
{
    sort(i, j);
    POMAGMA_ASSERT5(0 < i and i <= m_item_dim,
            "i=" << i << " out of bounds [1," << m_item_dim << "]");
    POMAGMA_ASSERT5(i < j and j <= m_item_dim,
            "j=" << j << " out of bounds [" << i << "," << m_item_dim << "]");

    const oid_t * block = _block(i >> 2, j >> 2);
    return _block2value(block, i & 3, j & 3);
}

} // namespace pomagma

#endif // POMAGMA_DENSE_SYM_FUN_H
