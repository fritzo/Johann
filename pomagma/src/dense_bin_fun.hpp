#ifndef POMAGMA_DENSE_BIN_FUN_H
#define POMAGMA_DENSE_BIN_FUN_H

#include "util.hpp"
#include "dense_set.hpp"

namespace pomagma
{

// WARNING zero/null items are not allowed

enum { ARG_STRIDE = 4 };

typedef int Block4x4W[ARG_STRIDE * ARG_STRIDE];

// a tight binary function in 4x4 word blocks
class dense_bin_fun
{
    // data, in blocks
    // TODO switch usigned -> size_t
    const unsigned N, M; // item,block dimension
    Block4x4W * const m_blocks;

    // dense sets for iteration
    const size_t m_line_count;
    Line * const m_Lx_lines;
    Line * const m_Rx_lines;
    mutable dense_set m_temp_set; // TODO FIXME this is not thread-safe
    mutable Line * m_temp_line; // TODO FIXME this is not thread-safe

    // block wrappers
    int * _block (int i_, int j_) { return m_blocks[M * j_ + i_]; }
    const int * _block (int i_, int j_) const { return m_blocks[M * j_ + i_]; }
    static int & _block2value (int * block, int i, int j)
    {
        return block[(j << 2) | i];
    }
    static int _block2value (const int* block, int i, int j)
    {
        return block[(j << 2) | i];
    }

    // set wrappers
public:
    Line * get_Lx_line (int i) const { return m_Lx_lines + (i * m_line_count); }
    Line * get_Rx_line (int i) const { return m_Rx_lines + (i * m_line_count); }
private:
    dense_set & _get_Lx_set (int i) { return m_temp_set.init(get_Lx_line(i)); }
    dense_set & _get_Rx_set (int i) { return m_temp_set.init(get_Rx_line(i)); }
    const dense_set & _get_Lx_set (int i) const
    {
        return m_temp_set.init(get_Lx_line(i));
    }
    const dense_set & _get_Rx_set (int i) const
    {
        return m_temp_set.init(get_Rx_line(i));
    }

    // intersection wrappers
    Line* _get_RRx_line (int i, int j) const;
    Line* _get_LRx_line (int i, int j) const;
    Line* _get_LLx_line (int i, int j) const;

    // ctors & dtors
public:
    dense_bin_fun (int num_items);
    ~dense_bin_fun ();
    void move_from (const dense_bin_fun & other); // for growing

    // function calling
private:
    inline int & value (int lhs, int rhs);
public:
    inline int value (int lhs, int rhs) const;
    int  get_value (int lhs, int rhs) const { return value(lhs, rhs); }

    // attributes
    unsigned count_pairs () const; // slow!
    unsigned sup_capacity () const { return N; }
    void validate () const;

    // element operations
    void insert (int lhs, int rhs, int val)
    {
        value(lhs, rhs) = val;
        _get_Lx_set(lhs).insert(rhs);
        _get_Rx_set(rhs).insert(lhs);
    }
    void remove (int lhs, int rhs)
    {
        value(lhs, rhs) = 0;
        _get_Lx_set(lhs).remove(rhs);
        _get_Rx_set(rhs).remove(lhs);
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
            void merge_values(int,int),    // dep,rep
            void move_value(int,int,int)); // moved,lhs,rhs

    //------------------------------------------------------------------------
    // Iteration over a line

    enum { LHS_FIXED = false, RHS_FIXED = true };
    template<int idx> class Iterator : noncopyable
    {
        dense_set m_set;
        dense_set::iterator m_iter;
        const dense_bin_fun * m_fun;
        int m_lhs;
        int m_rhs;

    public:

        // construction
        Iterator (const dense_bin_fun * fun)
            : m_set(fun->N, NULL),
              m_iter(m_set, false),
              m_fun(fun),
              m_lhs(0),
              m_rhs(0)
        {}

        Iterator (const dense_bin_fun * fun, int fixed)
            : m_set(fun->N, idx ? fun->get_Rx_line(fixed)
                                : fun->get_Lx_line(fixed)),
              m_iter(m_set, false),
              m_fun(fun),
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
        void begin (int fixed)
        {
            if (idx) { m_rhs=fixed; m_set.init(m_fun->get_Rx_line(fixed)); }
            else     { m_lhs=fixed; m_set.init(m_fun->get_Lx_line(fixed)); }
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
        int lhs () const { _deref_assert(); return m_lhs; }
        int rhs () const { _deref_assert(); return m_rhs; }
        int value () const
        {
            _deref_assert();
            return m_fun->get_value(m_lhs, m_rhs);
        }
    };

    //------------------------------------------------------------------------
    // Intersection iteration over 2 lines

    class RRxx_Iter : noncopyable
    {
        dense_set m_set;
        dense_set::iterator m_iter;
        const dense_bin_fun * m_fun;
        int m_lhs;
        int m_rhs1;
        int m_rhs2;

    public:

        // construction
        RRxx_Iter (const dense_bin_fun * fun)
            : m_set(fun->N, NULL),
              m_iter(m_set, false),
              m_fun(fun)
        {}

        // traversal
        void begin () { m_iter.begin(); if (ok()) m_lhs = *m_iter; }
        void begin (int fixed1, int fixed2)
        {
            m_set.init(m_fun->_get_RRx_line(fixed1, fixed2));
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
        int lhs () const { return m_lhs; }
        int value1 () const { return m_fun->get_value(m_lhs, m_rhs1); }
        int value2 () const { return m_fun->get_value(m_lhs, m_rhs2); }
    };

    class LRxx_Iter : noncopyable
    {
        dense_set m_set;
        dense_set::iterator m_iter;
        const dense_bin_fun * m_fun;
        int m_lhs1;
        int m_rhs2;
        int m_rhs1;

    public:

        // construction
        LRxx_Iter (const dense_bin_fun * fun)
            : m_set(fun->N, NULL),
              m_iter(m_set, false),
              m_fun(fun)
        {}

        // traversal
        void begin () { m_iter.begin(); if (ok()) m_rhs1 = *m_iter; }
        void begin (int fixed1, int fixed2)
        {
            m_set.init(m_fun->_get_LRx_line(fixed1, fixed2));
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
        int rhs1 () const { return m_rhs1; }
        int lhs2 () const { return m_rhs1; }
        int value1 () const { return m_fun->get_value(m_lhs1, m_rhs1); }
        int value2 () const { return m_fun->get_value(m_rhs1, m_rhs2); }
    };

    class LLxx_Iter : noncopyable
    {
        dense_set           m_set;
        dense_set::iterator m_iter;
        const dense_bin_fun * m_fun;
        int m_lhs1;
        int m_lhs2;
        int m_rhs;

    public:

        // construction
        LLxx_Iter (const dense_bin_fun * fun)
            : m_set(fun->N, NULL),
              m_iter(m_set, false),
              m_fun(fun)
        {}

        // traversal
        void begin () { m_iter.begin(); if (ok()) m_rhs = *m_iter; }
        void begin (int fixed1, int fixed2)
        {
            m_set.init(m_fun->_get_LLx_line(fixed1, fixed2));
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
        int rhs () const { return m_rhs; }
        int value1 () const { return m_fun->get_value(m_lhs1, m_rhs); }
        int value2 () const { return m_fun->get_value(m_lhs2, m_rhs); }
    };
};

//----------------------------------------------------------------------------
// Function calling

inline int & dense_bin_fun::value (int i, int j)
{
    POMAGMA_ASSERT5(0 <= i and i <= int(N),
            "i=" << i << " out of bounds [1," << N << "]");
    POMAGMA_ASSERT5(0 <= j and j <= int(N),
            "j=" << j<< " out of bounds [1," << N << "]");

    int * block = _block(i >> 2, j >> 2);
    return _block2value(block, i & 3, j & 3);
}

inline int dense_bin_fun::value (int i, int j) const
{
    POMAGMA_ASSERT5(0 <= i and i <= int(N),
            "i=" << i << " out of bounds [1," << N << "]");
    POMAGMA_ASSERT5(0 <= j and j <= int(N),
            "j=" << j <<" out of bounds [1," << N << "]");

    const int * block = _block(i >> 2, j >> 2);
    return _block2value(block, i & 3, j & 3);
}

} // namespace pomagma

#endif // POMAGMA_DENSE_BIN_FUN_H

