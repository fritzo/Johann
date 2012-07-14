#ifndef POMAGMA_DENSE_SET_H
#define POMAGMA_DENSE_SET_H

#include "util.hpp"
#include "aligned_alloc.hpp"

namespace pomagma
{

// WARNING zero/null items are not allowed

typedef uint32_t Line; // TODO switch to uint64_t
const size_t BITS_PER_LINE = 8 * sizeof(Line);
const size_t LINE_POS_MASK = BITS_PER_LINE - 1;

// proxy class for single bit
class bool_ref
{
    Line & m_line;
    const Line m_mask;
public:
    bool_ref (Line & line, size_t _i) : m_line(line), m_mask(1 << _i) {}

    operator bool () const { return m_line & m_mask; } // ATOMIC
    void operator |= (bool b) { m_line |= b * m_mask; } // ATOMIC
    void operator &= (bool b) { m_line &= ~(!b * m_mask); } // ATOMIC
    void zero () { m_line &= ~m_mask; } // ATOMIC
    void one () { m_line |= m_mask; } // ATOMIC
    void invert () { m_line ^= m_mask; } // ATOMIC
};

// basically a bitfield
class dense_set
{
    // data, in lines
    const size_t N; // number of items
    const size_t M; // number of lines
    Line * m_lines;
    const bool m_alias;

    // bit wrappers
    inline bool_ref _bit (size_t i);
    inline bool _bit (size_t i) const;

public:

    // line wrappers
    static size_t line_count (size_t item_count)
    {
        // position 0 is unused, so we count from item 1
        return (item_count + BITS_PER_LINE) / BITS_PER_LINE;
    }

    // ctors & dtors
    dense_set (size_t item_count);
    dense_set (size_t item_count, Line * lines)
        : N(item_count),
          M(line_count(item_count)),
          m_lines(lines),
          m_alias(true)
    {}
    //dense_set (size_t item_count, AlignedBuffer<Line> & buffer)
    //    : N(item_count),
    //      M(line_count(item_count)),
    //      m_lines(buffer(M)),
    //      m_alias(true)
    //{}
    ~dense_set ();
    void move_from (const dense_set & other, const oid_t * new2old = NULL);
    dense_set & init (Line * lines)
    {
        POMAGMA_ASSERT4(m_alias,
                "tried to set lines on non-alias dense set");
        m_lines = lines;
        return * this;
    }

    // attributes
    bool empty () const; // not fast
    size_t count_items () const; // supa-slow, try not to use
    size_t capacity () const { return N; }
    size_t line_count () const { return M; }
    unsigned data_size () const { return sizeof(Line) * M; }
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

    //------------------------------------------------------------------------
    // Iteration
    class iterator : noncopyable
    {
        size_t m_i;
        size_t m_rem;
        size_t m_quot;
        Line m_mask;
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
    private:
        void _deref_assert () const
        {
            POMAGMA_ASSERT5(ok(), "dereferenced done dense_set::iter");
        }
    public:
        size_t         operator *  () const { _deref_assert(); return m_i; }
        const size_t * operator -> () const { _deref_assert(); return & m_i; }

        // access
        size_t rem  () const { return m_rem; }
        size_t quot () const { return m_quot; }
        size_t mask () const { return m_mask; }
        const dense_set & set () const { return m_set; };
    };
};

inline bool_ref dense_set::_bit (size_t i)
{
    POMAGMA_ASSERT5(0 < i and i <= N,
            "dense_set[i] index out of range: " << i);
    auto I = div(i, BITS_PER_LINE); // either div_t or ldiv_t
    return bool_ref(m_lines[I.quot], I.rem);
}
inline bool dense_set::_bit (size_t i) const
{
    POMAGMA_ASSERT5(0 < i and i <= N,
            "const dense_set[i] index out of range: " << i);
    auto I = div(i, BITS_PER_LINE); // either div_t or ldiv_t
    return m_lines[I.quot] & (1 << I.rem);
}

inline void dense_set::insert (size_t i)
{
    POMAGMA_ASSERT5(0 < i and i <= N,
            "dense_set::insert item out of range: " << i);
    POMAGMA_ASSERT4(not contains(i),
            "tried to insert item " << i << " in dense_set twice");
    _bit(i).one();
}
inline void dense_set::remove (size_t i)
{
    POMAGMA_ASSERT5(0 < i and i <= N,
            "dense_set::remove item out of range: " << i);
    POMAGMA_ASSERT4(contains(i),
            "tried to remove item " << i << " from dense_set twice");
    _bit(i).zero();
}
inline void dense_set::merge (size_t i, size_t j __attribute__((unused)))
{
    POMAGMA_ASSERT5(0 < i and i <= N,
            "dense_set.merge(i,j) index i="<<i<<" out of range");
    POMAGMA_ASSERT5(0 < j and j <= N,
            "dense_set.merge(i,j) index j="<<j<<" out of range");
    POMAGMA_ASSERT5(i != j,
            "dense_set tried to merge item " << i << " into itself");
    POMAGMA_ASSERT5(i > j,
            "dense_set tried to merge in wrong order: " << i << '>' << j);
    POMAGMA_ASSERT4(contains(i) and contains(j),
            "dense_set tried to merge uninserted items: " << i << "," << j);
    _bit(i).zero();
}

inline void dense_set::iterator::begin ()
{
    POMAGMA_ASSERT4(m_set.m_lines, "tried to begin a null dense_set::iterator");
    m_quot = 0;
    --m_quot;
    _next_block();
    POMAGMA_ASSERT5(not ok() or m_set.contains(m_i),
            "dense_set::iterator::begin landed on empty pos " << m_i);
}

} // namespace pomagma

#endif // POMAGMA_DENSE_SET_H
