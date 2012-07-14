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
        oid_t lhs;
        oid_t rhs;
        Pos (oid_t l = 0, oid_t r = 0) : lhs(l), rhs(r) {}
        bool operator == (const Pos & p) const
        {
            return lhs == p.lhs and rhs == p.rhs;
        }
        bool operator != (const Pos & p) const
        {
            return lhs != p.lhs or rhs != p.rhs;
        }
    };

    // data
    // TODO switch usigned -> size_t
    const size_t m_item_dim; // number of items per slice
    const size_t m_word_dim;
    const size_t m_round_item_dim; // = m_word_dim * BITS_PER_WORD
    const size_t m_round_word_dim; // = m_round_item_dim * BITS_PER_WORD
    dense_set m_support;
    Word * m_Lx_lines;
    Word * m_Rx_lines;
    mutable dense_set m_temp_set; // TODO FIXME this is not thread-safe
    mutable Word * m_temp_line; // TODO FIXME this is not thread-safe

    // bit wrappers
    inline bool_ref _bit_Lx (oid_t i, oid_t j);
    inline bool_ref _bit_Rx (oid_t i, oid_t j);
    inline bool _bit_Lx (oid_t i, oid_t j) const;
    inline bool _bit_Rx (oid_t i, oid_t j) const;

    // set wrappers
public:
    Word * get_Lx_line (oid_t i) const { return m_Lx_lines + i * m_word_dim; }
    Word * get_Rx_line (oid_t i) const { return m_Rx_lines + i * m_word_dim; }
private:
    dense_set & _get_Lx_set (oid_t i) { return m_temp_set.init(get_Lx_line(i)); }
    dense_set & _get_Rx_set (oid_t i) { return m_temp_set.init(get_Rx_line(i)); }
    const dense_set & _get_Lx_set (oid_t i) const
    {
        return m_temp_set.init(get_Lx_line(i));
    }
    const dense_set & _get_Rx_set (oid_t i) const
    {
        return m_temp_set.init(get_Rx_line(i));
    }

    // ctors & dtors
public:
    dense_bin_rel (size_t item_dim, bool is_full = false);
    ~dense_bin_rel ();
    void move_from (const dense_bin_rel & other, const oid_t* new2old=NULL);

    // attributes
    const dense_set & support () const { return m_support; }
    size_t count_pairs () const; // supa-slow, try not to use
    size_t count_items_support () const { return m_support.count_items(); }
    void validate () const;
    void validate_disjoint (const dense_bin_rel& other) const;
    void print_table (size_t n = 0) const;

    // element operations
    bool contains_Lx (oid_t i, oid_t j) const { return _bit_Lx(i, j); }
    bool contains_Rx (oid_t i, oid_t j) const { return _bit_Rx(i, j); }
    bool contains (oid_t i, oid_t j) const { return contains_Lx(i, j); }
    bool contains (const Pos & p) const { return contains_Lx(p); }
    bool contains_Lx (const Pos & p) const
    {
        return contains_Lx(p.lhs, p.rhs);
    }
    bool contains_Rx (const Pos & p) const
    {
        return contains_Rx(p.lhs, p.rhs);
    }
private:
    // one-sided versions
    void insert_Lx (oid_t i, oid_t j) { _bit_Lx(i,j).one(); }
    void insert_Rx (oid_t i, oid_t j) { _bit_Rx(i,j).one(); }
    void remove_Lx (oid_t i, oid_t j) { _bit_Lx(i,j).zero(); }
    void remove_Rx (oid_t i, oid_t j) { _bit_Rx(i,j).zero(); }
    void remove_Lx (const dense_set & is, oid_t i);
    void remove_Rx (oid_t i, const dense_set& js);
public:
    // two-sided versions
    void insert (oid_t i, oid_t j) { insert_Lx(i,j); insert_Rx(i,j); }
    void remove (oid_t i, oid_t j) { remove_Lx(i,j); remove_Rx(i,j); }
    // these return whether there was a change
    inline bool ensure_inserted_Lx (oid_t i, oid_t j);
    inline bool ensure_inserted_Rx (oid_t i, oid_t j);
    bool ensure_inserted (oid_t i, oid_t j) { return ensure_inserted_Lx(i,j); }
    void ensure_inserted (
            oid_t i,
            const dense_set & js,
            void (*change)(oid_t, oid_t));
    void ensure_inserted (
            const dense_set & is,
            oid_t j,
            void (*change)(oid_t, oid_t));

    // support operations
    bool supports (oid_t i) const { return m_support.contains(i); }
    bool supports (oid_t i, oid_t j) const
    {
        return supports(i) and supports(j);
    }
    void insert   (oid_t i) { m_support.insert(i); }
    void remove   (oid_t i);
    void merge    (oid_t dep, oid_t rep, void (*move_to)(oid_t, oid_t));

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
              m_rhs_set(rel->m_item_dim, NULL),
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
            if (m_rhs.ok()) {
                _update_rhs();
            } else {
                m_lhs.next();
                _find_rhs();
            }
        }
        bool ok () const { return m_lhs.ok(); }

        // dereferencing
    private:
        void _deref_assert () const
        {
            POMAGMA_ASSERT5(ok(), "dereferenced done br::iterator");
        }
    public:
        const Pos & operator *  () const { _deref_assert(); return m_pos; }
        const Pos * operator -> () const { _deref_assert(); return &m_pos; }

        // access
        oid_t lhs () const { return m_pos.lhs; }
        oid_t rhs () const { return m_pos.rhs; }
    };

    //------------------------------------------------------------------------
    // Iteration over a line, LR or RL

    enum Direction { LHS_FIXED=true, RHS_FIXED=false };
    template<bool dir> // REQUIRES Direction dir and Complement comp
    class Iterator : noncopyable
    {
    protected:
        dense_set m_temp_set;
        dense_set::iterator m_moving;
        oid_t m_fixed;
        Pos m_pos;
        const dense_bin_rel & m_rel;

    public:

        // construction
        Iterator (oid_t fixed, const dense_bin_rel * rel)
            : m_temp_set(rel->m_item_dim, dir ? rel->get_Lx_line(fixed)
                                : rel->get_Rx_line(fixed)),
              m_moving(m_temp_set, false),
              m_fixed(fixed),
              m_rel(*rel)
        {
            POMAGMA_ASSERT2(m_rel.supports(fixed),
                    "br::Iterator's fixed pos is unsupported");
            begin();
        }
        Iterator (const dense_bin_rel * rel)
            : m_temp_set(rel->m_item_dim, NULL),
              m_moving(m_temp_set, false),
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
            if (m_moving.ok()) { _fix(); _move(); }
        }
        void begin (oid_t fixed)
        {   POMAGMA_ASSERT2(m_rel.supports(fixed),
                    "br::Iterator's fixed pos is unsupported");
            m_fixed = fixed;
            m_temp_set.init(dir ? m_rel.get_Lx_line(fixed)
                                : m_rel.get_Rx_line(fixed));
            begin();
        }
        void next ()
        {
            m_moving.next();
            if (m_moving.ok()) { _move(); }
        }
        bool ok () const { return m_moving.ok(); }

        // dereferencing
    private:
        void _deref_assert () const
        {
            POMAGMA_ASSERT5(ok(), "dereferenced done dense_bin_rel'n::iter");
        }
    public:
        const Pos & operator *  () const { _deref_assert(); return m_pos; }
        const Pos * operator -> () const { _deref_assert(); return &m_pos; }

        // access
        oid_t fixed () const { return m_fixed; }
        oid_t moving () const { return * m_moving; }
        oid_t lhs () const { return m_pos.lhs; }
        oid_t rhs () const { return m_pos.rhs; }
    };
};

// bit wrappers
inline bool_ref dense_bin_rel::_bit_Lx (oid_t i, oid_t j)
{
    POMAGMA_ASSERT5(supports(i, j),
            "_bit_Lx called on unsupported pair " << i << ',' << j);
    return _get_Lx_set(i)(j);
}
inline bool_ref dense_bin_rel::_bit_Rx (oid_t i, oid_t j)
{
    POMAGMA_ASSERT5(supports(i, j),
            "_bit_Rx called on unsupported pair " << i << ',' << j);
    return _get_Rx_set(j)(i);
}
inline bool dense_bin_rel::_bit_Lx (oid_t i, oid_t j) const
{
    POMAGMA_ASSERT5(supports(i,j),
            "_bit_Lx called on unsupported pair " << i << ',' << j);
    return _get_Lx_set(i)(j);
}
inline bool dense_bin_rel::_bit_Rx (oid_t i, oid_t j) const
{
    POMAGMA_ASSERT5(supports(i,j),
            "_bit_Rx called on unsupported pair " << i << ',' << j);
    return _get_Rx_set(j)(i);
}

//----------------------------------------------------------------------------
// Operations

// returns whether there was a change
inline bool dense_bin_rel::ensure_inserted_Lx (oid_t i, oid_t j)
{
    bool_ref contained = _bit_Lx(i,j);
    if (contained) return false;
    contained.one();
    insert_Rx(i,j);
    return true;
}

// returns whether there was a change
inline bool dense_bin_rel::ensure_inserted_Rx (oid_t i, oid_t j)
{
    bool_ref contained = _bit_Rx(i,j);
    if (contained) return false;
    contained.one();
    insert_Lx(i,j);
    return true;
}

} // namespace pomagma

#endif // POMAGMA_DENSE_BIN_REL_H
