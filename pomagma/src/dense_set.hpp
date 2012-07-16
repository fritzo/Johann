#ifndef POMAGMA_DENSE_SET_H
#define POMAGMA_DENSE_SET_H

#include "util.hpp"
//#include "aligned_alloc.hpp"

namespace pomagma
{

// WARNING zero/null items are not allowed

//----------------------------------------------------------------------------
// Bool reference

class bool_ref
{
    Word & m_word;
    const Word m_mask;

public:

    bool_ref (Word & word, size_t _i)
        : m_word(word),
          m_mask(1 << _i)
    {
        POMAGMA_ASSERT6(_i < BITS_PER_WORD, "out of range: " << _i);
    }
    static bool_ref index (Word * line, size_t i)
    {
        auto I = div(i, BITS_PER_WORD); // either div_t or ldiv_t
        return bool_ref(line[I.quot], I.rem);
    }

    operator bool () const { return m_word & m_mask; } // ATOMIC
    void operator |= (bool b) { m_word |= b * m_mask; } // ATOMIC
    void operator &= (bool b) { m_word &= ~(!b * m_mask); } // ATOMIC
    void zero () { m_word &= ~m_mask; } // ATOMIC
    void one () { m_word |= m_mask; } // ATOMIC
    void invert () { m_word ^= m_mask; } // ATOMIC
};

//----------------------------------------------------------------------------
// Dense set - basically a bitfield

class dense_set
{
    // data
    const size_t m_item_dim;
    const size_t m_word_dim;
    Word * m_line;
    const bool m_alias;

    // bit wrappers
    inline bool_ref _bit (size_t i);
    inline bool _bit (size_t i) const;

public:

    // position 0 is unused, so we count from item 1
    static size_t word_count (size_t item_dim)
    {
        return (item_dim + BITS_PER_WORD) / BITS_PER_WORD;
    }

    // ctors & dtors
    dense_set (size_t item_dim);
    dense_set (size_t item_dim, Word * line)
        : m_item_dim(item_dim),
          m_word_dim(word_count(item_dim)),
          m_line(line),
          m_alias(true)
    {
    }
    //dense_set (size_t item_dim, AlignedBuffer<Word> & buffer)
    //    : m_item_dim(item_dim),
    //      m_word_dim(word_count(item_dim)),
    //      m_line(buffer(m_word_dim)),
    //      m_alias(true)
    //{
    //}
    ~dense_set ();
    void move_from (const dense_set & other, const oid_t * new2old = NULL);
    dense_set & init (Word * line)
    {
        POMAGMA_ASSERT4(m_alias, "tried to init() non-alias dense set");
        m_line = line;
        return * this;
    }

    // attributes
    bool empty () const; // not fast
    size_t count_items () const; // supa-slow, try not to use
    size_t item_dim () const { return m_item_dim; }
    size_t word_dim () const { return m_word_dim; }
    unsigned data_size_bytes () const { return sizeof(Word) * m_word_dim; }
    void validate () const;

    // element operations
    bool_ref operator() (size_t i) { return _bit(i); }
    bool operator() (size_t i) const { return _bit(i); }
    bool contains (size_t i) const { return _bit(i); }
    inline void insert (size_t i);
    inline void remove (size_t i);
    inline void merge  (size_t i, size_t j);
    void insert_all ();

    // entire operations
    void zero ();
    bool operator == (const dense_set & other) const;
    bool operator <= (const dense_set & other) const;
    bool disjoint    (const dense_set & other) const;
    void operator += (const dense_set & other);
    void operator *= (const dense_set & other);
    void set_union   (const dense_set & lhs, const dense_set & rhs);
    void set_diff    (const dense_set & lhs, const dense_set & rhs);
    void set_insn    (const dense_set & lhs, const dense_set & rhs);
    void set_nor     (const dense_set & lhs, const dense_set & rhs);
    void merge       (dense_set & dep);
    bool merge       (dense_set & dep, dense_set & diff);
    bool ensure      (const dense_set & dep, dense_set & diff);
    // returns true if anything in rep changes

    // TODO move iterator out of class
    //------------------------------------------------------------------------
    // Iteration
    class iterator : noncopyable
    {
        size_t m_i;
        size_t m_rem;
        size_t m_quot;
        Word m_mask;
        const dense_set & m_set;

    public:

        // construction
        iterator (const dense_set & set, bool b = true)
            : m_set(set)
        {
            if (b) { begin(); }
        }

        // traversal
    private:
        void _next_block ();
    public:
        inline void begin ();
        void next ();
        bool ok () const { return m_i; }

        // dereferencing
        size_t         operator *  () const { POMAGMA_ASSERT_OK return m_i; }
        const size_t * operator -> () const { POMAGMA_ASSERT_OK return & m_i; }

        // access
        // TODO remove these
        size_t rem () const { return m_rem; }
        size_t quot () const { return m_quot; }
        size_t mask () const { return m_mask; }
        const dense_set & set () const { return m_set; };
    };
};

inline bool_ref dense_set::_bit (size_t i)
{
    POMAGMA_ASSERT5(0 < i and i <= m_item_dim, "out of range: " << i);
    return bool_ref::index(m_line, i);
}
inline bool dense_set::_bit (size_t i) const
{
    POMAGMA_ASSERT5(0 < i and i <= m_item_dim, "out of range: " << i);
    return bool_ref::index(m_line, i);
}

inline void dense_set::insert (size_t i)
{
    POMAGMA_ASSERT4(not contains(i), "double insertion " << i);
    _bit(i).one();
}
inline void dense_set::remove (size_t i)
{
    POMAGMA_ASSERT4(contains(i), "double removal: " << i);
    _bit(i).zero();
}
inline void dense_set::merge (size_t i, size_t j __attribute__((unused)))
{
    POMAGMA_ASSERT5(0 < i and i <= m_item_dim, "rep out of range: " << i);
    POMAGMA_ASSERT5(0 < j and j <= m_item_dim, "dep out of range: " << j);
    POMAGMA_ASSERT5(i != j, "merge with self: " << i);
    POMAGMA_ASSERT5(i > j, "merge wrong order: " << i << " > " << j);
    POMAGMA_ASSERT4(contains(i), "merge rep not contained: " << i);
    POMAGMA_ASSERT4(contains(j), "merge dep not contained: " << j);
    _bit(i).zero();
}

inline void dense_set::iterator::begin ()
{
    POMAGMA_ASSERT4(m_set.m_line, "begin with null line");
    m_quot = 0;
    --m_quot;
    _next_block();
    POMAGMA_ASSERT5(not ok() or m_set.contains(m_i),
            "begin on empty pos: " << m_i);
}

} // namespace pomagma

#endif // POMAGMA_DENSE_SET_H
