#ifndef POMAGMA_DENSE_BIN_REL_H
#define POMAGMA_DENSE_BIN_REL_H

#include "util.hpp"
#include "dense_set.hpp"

namespace pomagma
{

// WARNING zero/null items are not allowed

// a pair of dense sets of dense sets, one col-row, one row-col
class dense_bin_rel
{
    struct Pos
    {
        int lhs, rhs;
        Pos (int l = 0, int r = 0) : lhs(l), rhs(r) {}
        bool operator == (const Pos& p) const
        {
            return lhs == p.lhs and rhs == p.rhs;
        }
        bool operator != (const Pos& p) const
        {
            return lhs != p.lhs or rhs != p.rhs;
        }
    };

    // data
    const unsigned N, M;        // number of items,Lines per slice
    const unsigned N_up;        // N rounded up, = M * LINE_STRIDE
    const unsigned NUM_LINES;   // number of Lines in each orientation
    dense_set m_support;
    Line * m_Lx_lines, * m_Rx_lines;
    mutable dense_set m_set;    // this is a temporary
    mutable Line* m_temp_line;  // this is a temporary

    // bit wrappers
    inline bool_ref _bit_Lx (int i, int j);
    inline bool_ref _bit_Rx (int i, int j);
    inline bool     _bit_Lx (int i, int j) const;
    inline bool     _bit_Rx (int i, int j) const;

    // set wrappers
public:
    Line* get_Lx_line (int i) const { return m_Lx_lines + i * M; }
    Line* get_Rx_line (int i) const { return m_Rx_lines + i * M; }
private:
    dense_set& _get_Lx_set (int i) { return m_set.init(get_Lx_line(i)); }
    dense_set& _get_Rx_set (int i) { return m_set.init(get_Rx_line(i)); }
    const dense_set & _get_Lx_set (int i) const
    {
        return m_set.init(get_Lx_line(i));
    }
    const dense_set & _get_Rx_set (int i) const
    {
        return m_set.init(get_Rx_line(i));
    }

    // ctors & dtors
public:
    dense_bin_rel (int num_items, bool is_full = false);
    ~dense_bin_rel ();
    void move_from (const dense_bin_rel & other, const oid_t* new2old=NULL);

    // attributes
    unsigned size     () const; // supa-slow, try not to use
    unsigned sup_size () const { return m_support.size(); }
    unsigned capacity () const { return N * N; }
    unsigned sup_capacity () const { return N; }
    void validate () const;
    void validate_disjoint (const dense_bin_rel& other) const;
    void print_table (unsigned n=0) const;

    // element operations
    bool contains_Lx (int i, int j) const { return _bit_Lx(i,j); }
    bool contains_Rx (int i, int j) const { return _bit_Rx(i,j); }
    bool contains_Lx (const Pos & p) const { return contains_Lx(p.lhs, p.rhs); }
    bool contains_Rx (const Pos & p) const { return contains_Rx(p.lhs, p.rhs); }
    bool contains (int i, int j) const { return contains_Lx(i,j); }
    bool contains (const Pos & p) const { return contains_Lx(p); }
private:
    // one-sided versions
    void insert_Lx (int i, int j) { _bit_Lx(i,j).one(); }
    void insert_Rx (int i, int j) { _bit_Rx(i,j).one(); }
    void remove_Lx (int i, int j) { _bit_Lx(i,j).zero(); }
    void remove_Rx (int i, int j) { _bit_Rx(i,j).zero(); }
    void remove_Lx (const dense_set & is, int i);
    void remove_Rx (int i, const dense_set& js);
public:
    // two-sided versions
    void insert (int i, int j) { insert_Lx(i,j); insert_Rx(i,j); }
    void remove (int i, int j) { remove_Lx(i,j); remove_Rx(i,j); }
    // these return whether there was a change
    inline bool ensure_inserted_Lx (int i, int j);
    inline bool ensure_inserted_Rx (int i, int j);
    bool ensure_inserted (int i, int j) { return ensure_inserted_Lx(i,j); }
    void ensure_inserted (int i, const dense_set & js, void (*change)(int,int));
    void ensure_inserted (const dense_set & is, int j, void (*change)(int,int));

    // support operations
    bool supports (int i) const { return m_support.contains(i); }
    bool supports (int i, int j) const { return supports(i) and supports(j); }
    void insert   (int i) { m_support.insert(i); }
    void remove   (int i);
    void merge    (int dep, int rep, void (*move_to)(int,int));

    // saving/loading of block data
    oid_t data_size () const;
    void write_to_file (FILE* file);
    void read_from_file (FILE* file);

    //------------------------------------------------------------------------
    // Iteration, always LR

    class iterator : noncopyable
    {
        dense_set::iterator m_lhs;
        dense_set::iterator m_rhs;
        dense_set m_rhs_set;
        const dense_bin_rel & m_rel;
        Pos m_pos;

    public:

        // construction
        iterator (const dense_bin_rel * rel)
            : m_lhs(rel->m_support, false),
              m_rhs(m_rhs_set, false),
              m_rhs_set(rel->N, NULL),
              m_rel(*rel)
        {
            begin();
        }

        // traversal
    private:
        void _update_lhs () { m_pos.lhs = *m_lhs; }
        void _update_rhs () { m_pos.rhs = *m_rhs; }
        void _finish () { m_pos.lhs = m_pos.rhs = 0; }
        void _find_rhs (); // finds first rhs, possibly incrementing lhs
    public:
        void begin () { m_lhs.begin(); _find_rhs(); }
        void next ()
        {
            m_rhs.next();
            if (m_rhs) {
                _update_rhs();
            } else {
                m_lhs.next();
                _find_rhs();
            }
        }
        operator bool () const { return m_lhs; }
        bool done () const { return m_lhs.done(); }

        // dereferencing
    private:
        void _deref_assert () const
        {
            POMAGMA_ASSERT5(not done(), "dereferenced done br::iterator");
        }
    public:
        const Pos & operator *  () const { _deref_assert(); return m_pos; }
        const Pos * operator -> () const { _deref_assert(); return &m_pos; }

        // access
        int lhs () const { return m_pos.lhs; }
        int rhs () const { return m_pos.rhs; }
    };

    //------------------------------------------------------------------------
    // Iteration over a line, LR or RL

    enum Direction { LHS_FIXED=true, RHS_FIXED=false };
    template<int dir> // REQUIRES Direction dir and Complement comp
    class Iterator : noncopyable
    {
    protected:
        dense_set            m_set;
        dense_set::iterator  m_moving;
        int                  m_fixed;
        Pos                  m_pos;
        const dense_bin_rel& m_rel;

    public:

        // construction
        Iterator (int fixed, const dense_bin_rel * rel)
            : m_set(rel->N, dir ? rel->get_Lx_line(fixed)
                                : rel->get_Rx_line(fixed)),
              m_moving(m_set, false),
              m_fixed(fixed),
              m_rel(*rel)
        {
            POMAGMA_ASSERT2(m_rel.supports(fixed),
                    "br::Iterator's fixed pos is unsupported");
            begin();
        }
        Iterator (const dense_bin_rel * rel)
            : m_set(rel->N, NULL),
              m_moving(m_set, false),
              m_fixed(0),
              m_rel(*rel)
        {}

        // traversal
    private:
        void _fix  () { (dir ? m_pos.lhs : m_pos.rhs) = m_fixed; }
        void _move () { (dir ? m_pos.rhs : m_pos.lhs) = *m_moving; }
    public:
        void begin ()
        {
            POMAGMA_ASSERT(m_fixed, "tried to begin() a null br::Iterator");
            m_moving.begin();
            if (m_moving) { _fix(); _move(); }
        }
        void begin (int fixed)
        {   POMAGMA_ASSERT2(m_rel.supports(fixed),
                    "br::Iterator's fixed pos is unsupported");
            m_fixed = fixed;
            m_set.init(dir ? m_rel.get_Lx_line(fixed)
                           : m_rel.get_Rx_line(fixed));
            begin();
        }
        void next ()
        {
            m_moving.next();
            if (m_moving) { _move(); }
        }
        operator bool () const { return m_moving; }
        bool done  () const { return m_moving.done(); }

        // dereferencing
    private:
        void _deref_assert () const
        {
            POMAGMA_ASSERT5(not done(), "dereferenced done dense_bin_rel'n::iter");
        }
    public:
        const Pos & operator *  () const { _deref_assert(); return m_pos; }
        const Pos * operator -> () const { _deref_assert(); return &m_pos; }

        // access
        int fixed () const { return m_fixed; }
        int moving () const { return *m_moving; }
        int lhs () const { return m_pos.lhs; }
        int rhs () const { return m_pos.rhs; }
    };
};

// bit wrappers
inline bool_ref dense_bin_rel::_bit_Lx (int i, int j)
{
    POMAGMA_ASSERT5(supports(i,j), "_bit_Lx called on unsupported pair "<<i<<','<<j);
    return _get_Lx_set(i)(j);
}
inline bool_ref dense_bin_rel::_bit_Rx (int i, int j)
{
    POMAGMA_ASSERT5(supports(i,j), "_bit_Rx called on unsupported pair "<<i<<','<<j);
    return _get_Rx_set(j)(i);
}
inline bool dense_bin_rel::_bit_Lx (int i, int j) const
{
    POMAGMA_ASSERT5(supports(i,j), "_bit_Lx called on unsupported pair "<<i<<','<<j);
    return _get_Lx_set(i)(j);
}
inline bool dense_bin_rel::_bit_Rx (int i, int j) const
{
    POMAGMA_ASSERT5(supports(i,j), "_bit_Rx called on unsupported pair "<<i<<','<<j);
    return _get_Rx_set(j)(i);
}

//----------------------------------------------------------------------------
// Operations

// returns whether there was a change
inline bool dense_bin_rel::ensure_inserted_Lx (int i, int j)
{
    bool_ref contained = _bit_Lx(i,j);
    if (contained) return false;
    contained.one();
    insert_Rx(i,j);
    return true;
}

// returns whether there was a change
inline bool dense_bin_rel::ensure_inserted_Rx (int i, int j)
{
    bool_ref contained = _bit_Rx(i,j);
    if (contained) return false;
    contained.one();
    insert_Lx(i,j);
    return true;
}

}

#endif

