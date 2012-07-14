#ifndef POMAGMA_DENSE_SYM_FUN_H
#define POMAGMA_DENSE_SYM_FUN_H

#include "util.hpp"
#include "dense_set.hpp"

namespace pomagma
{

// WARNING zero/null items are not allowed

enum { DSF_STRIDE = 4 };

typedef int Block4x4W[DSF_STRIDE * DSF_STRIDE];

inline size_t unordered_pair_count (size_t i) { return (i * (i + 1)) / 2; }

// a tight binary function in 4x4 word blocks
class dense_sym_fun
{
    // data, in blocks
    // TODO switch usigned -> size_t
    const unsigned m_item_dim;
    const unsigned m_block_dim;
    Block4x4W * const m_blocks;

    // dense sets for iteration
    const size_t m_line_count;
    Line * const m_Lx_lines;
    mutable dense_set m_temp_set; // TODO FIXME this is not thread-safe
    mutable Line * m_temp_line; // TODO FIXME this is not thread-safe

    // block wrappers
    template<class T>
    static void sort (T & i, T & j) { if (j < i) { T k = j; j = i; i = k; }  }
    int * _block (int i_, int j_)
    {
        return m_blocks[unordered_pair_count(j_) + i_];
    }
    const int * _block (int i_, int j_) const
    {
        return m_blocks[unordered_pair_count(j_) + i_];
    }
    static int & _block2value (int * block, int i, int j)
    {
        return block[(j << 2) | i];
    }
    static int _block2value (const int * block, int i, int j)
    {
        return block[(j << 2) | i];
    }

    // set wrappers
public:
    Line * get_Lx_line (int i) const { return m_Lx_lines + (i * m_line_count); }
private:
    dense_set & _get_Lx_set (int i) { return m_temp_set.init(get_Lx_line(i)); }
    const dense_set & _get_Lx_set (int i) const
    {
        return m_temp_set.init(get_Lx_line(i));
    }

    // intersection wrappers
    Line * _get_LLx_line (int i, int j) const;

    // ctors & dtors
public:
    dense_sym_fun (int num_items);
    ~dense_sym_fun ();
    void move_from (const dense_sym_fun & other); // for growing

    // function calling
private:
    inline int & value (int lhs, int rhs);
public:
    inline int value (int lhs, int rhs) const;
    int get_value (int lhs, int rhs) const { return value(lhs, rhs); }

    // attributes
    size_t item_capacity () const { return m_item_dim; }
    unsigned count_pairs () const; // slow!
    void validate () const;

    // element operations
    void insert (int lhs, int rhs, int val)
    {
        value(lhs,rhs) = val;
        _get_Lx_set(lhs).insert(rhs); if (lhs == rhs) return;
        _get_Lx_set(rhs).insert(lhs);
    }
    void remove (int lhs, int rhs)
    {
        value(lhs,rhs) = 0;
        _get_Lx_set(lhs).remove(rhs); if (lhs == rhs) return;
        _get_Lx_set(rhs).remove(lhs);
    }
    bool contains (int lhs, int rhs) const
    {
        return _get_Lx_set(lhs).contains(rhs);
    }

    // support operations
    void remove (
            const int i,
            void remove_value(int)); // rem
    void merge (
            const int i,
            const int j,
            void merge_values(int, int),     // dep, rep
            void move_value(int, int, int)); // moved, lhs, rhs

    //------------------------------------------------------------------------
    // Iteration over a line

    class Iterator : noncopyable
    {
        dense_set           m_set;
        dense_set::iterator m_iter;
        const dense_sym_fun * m_fun;
        int m_fixed, m_moving;

    public:

        // construction
        Iterator (const dense_sym_fun * fun)
            : m_set(fun->m_item_dim, NULL),
              m_iter(m_set, false),
              m_fun(fun),
              m_fixed(0),
              m_moving(0)
        {}
        Iterator (const dense_sym_fun * fun, int fixed)
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
        void begin (int fixed)
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
        int fixed  () const { _deref_assert(); return m_fixed; }
        int moving () const { _deref_assert(); return m_moving; }
        int value  () const
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
        int m_fixed1;
        int m_fixed2;
        int m_moving;

    public:

        // construction
        LLxx_Iter (const dense_sym_fun* fun)
            : m_set(fun->m_item_dim, NULL),
              m_iter(m_set, false),
              m_fun(fun)
        {}

        // traversal
        void begin () { m_iter.begin(); if (ok()) m_moving = *m_iter; }
        void begin (int fixed1, int fixed2)
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
        int fixed1 () const { return m_fixed1; }
        int fixed2 () const { return m_fixed2; }
        int moving () const { return m_moving; }
        int value1 () const { return m_fun->get_value(m_fixed1, m_moving); }
        int value2 () const { return m_fun->get_value(m_fixed2, m_moving); }
    };
};

//----------------------------------------------------------------------------
// Function calling

inline int & dense_sym_fun::value (int i, int j)
{
    sort(i, j);
    POMAGMA_ASSERT5(0 <= i and i <= int(m_item_dim),
            "i=" << i << " out of bounds [1," << m_item_dim << "]");
    POMAGMA_ASSERT5(i <= j and j <= int(m_item_dim),
            "j=" << j << " out of bounds [" << i << "," << m_item_dim << "]");

    int * block = _block(i >> 2, j >> 2);
    return _block2value(block, i & 3, j & 3);
}

inline int dense_sym_fun::value (int i, int j) const
{
    sort(i, j);
    POMAGMA_ASSERT5(0 <= i and i <= int(m_item_dim),
            "i=" << i << " out of bounds [1," << m_item_dim << "]");
    POMAGMA_ASSERT5(i <= j and j <= int(m_item_dim),
            "j=" << j << " out of bounds [" << i << "," << m_item_dim << "]");

    const int * block = _block(i >> 2, j >> 2);
    return _block2value(block, i & 3, j & 3);
}

} // namespace pomagma

#endif // POMAGMA_DENSE_SYM_FUN_H
