#ifndef NONSTD_DENSE_SET_H
#define NONSTD_DENSE_SET_H

#include "definitions.h"
#include <utility> //for pair

namespace nonstd
{

using Logging::logger;

//Note: zero/null items are not allowed

typedef uint32_t Line;
enum { LINE_STRIDE = 32 };
const Line LINE_MASK = 0x1F;

class bool_ref
{//proxy class for single bit
    Line* const m_line;
    const Line m_mask;
    void _deref_assert () { Assert4(m_line != NULL, "null bit_ref accessed"); }
public:
    bool_ref (Line* line, int _i) : m_line(line), m_mask(1 << _i) {}
    bool_ref () : m_line(NULL), m_mask(0) {} //for containers
    operator bool () { _deref_assert(); return (*m_line) & m_mask; }
    bool_ref& operator = (bool b)
    {
        _deref_assert();
        (*m_line) |= b * m_mask;
        (*m_line) &= ~(b * ~m_mask);
        return *this;
    }
    void zero   () { _deref_assert(); (*m_line) &= ~m_mask; }
    void one    () { _deref_assert(); (*m_line) |= m_mask; }
    void invert () { _deref_assert(); (*m_line) ^= m_mask; }

    bool_ref& operator |= (bool b)
    { _deref_assert(); (*m_line) |= b * m_mask; return *this; }
    bool_ref& operator &= (bool b)
    { _deref_assert(); (*m_line) &= ~(!b * m_mask); return *this; }
};

class dense_set
{//basically a bitfield
    typedef dense_set MyType;

    //data, in lines
    const int N,M; //number of items,lines
    Line* m_lines;
    const bool m_borrowing;

    //bit wrappers
    inline bool_ref _bit (int i);
    inline bool     _bit (int i) const;

    //line wrappers
public:
    const Int _lines () const { return M; }
    const Line  _line (int i_) const { return m_lines[i_]; }
          Line& _line (int i_)       { return m_lines[i_]; }
    Line* data () { return m_lines; }

    //ctors & dtors
    dense_set (int num_items);
    dense_set (int num_items, Line* lines)
        : N(num_items),
          M((N+LINE_STRIDE)/LINE_STRIDE),
          m_lines(lines),
          m_borrowing(true)
    {}
    ~dense_set () { if (not m_borrowing) delete[] m_lines; }
    void move_from (const dense_set& other, const Int* new2old=NULL);
    dense_set& init (Line* lines)
    {
        Assert4(m_borrowing, "tried to set lines on non-borrowing dense set");
        m_lines = lines;
        return *this;
    }

    //attributes
    bool empty () const; //not fast
    Int size     () const; //supa-slow, try not to use
    Int capacity  () const { return N; }
    Int num_lines () const { return M; }
    unsigned data_size () const { return sizeof(Line) * M; }
    void validate () const;

    //element operations
    bool_ref operator() (int i)       { return _bit(i); }
    bool     operator() (int i) const { return _bit(i); }
    bool contains (int i) const { return _bit(i); }
    inline void insert (int i);
    inline void remove (int i);
    inline void merge  (int i, int j);
    void insert_all ();

    //entire operations
    void zero ();
    bool operator == (const dense_set& other) const;
    bool disjoint    (const dense_set& other) const;
    void operator += (const dense_set& other);
    void operator *= (const dense_set& other);
    void set_union   (const dense_set& s, const dense_set& t);
    void set_diff    (const dense_set& s, const dense_set& t);
    void set_insn    (const dense_set& s, const dense_set& t);
    void set_nor     (const dense_set& s, const dense_set& t);
    void merge       (const dense_set& dep);
    bool merge       (const dense_set& dep, dense_set& diff);
    bool ensure      (const dense_set& dep, dense_set& diff);
    //returns true if anything in rep changes

    //================ iteration ================
    class iterator
    {
        typedef       iterator& Ref;
        typedef const iterator& const_Ref;

        int  m_i;
        int  m_rem;
        int  m_quot;
        Line m_mask;
        const dense_set& m_set;

        //coordinate access
    public:
        int rem  () const { return m_rem; }
        int quot () const { return m_quot; }
        int mask () const { return m_mask; }
        const dense_set& set () const { return m_set; };

        //comparison
        bool operator == (const_Ref other) const { return m_i == other.m_i; }
        bool operator != (const_Ref other) const { return m_i != other.m_i; }
        bool operator <  (const_Ref other) const { return m_i <  other.m_i; }
        bool operator <= (const_Ref other) const { return m_i <= other.m_i; }
        bool operator >  (const_Ref other) const { return m_i >  other.m_i; }
        bool operator >= (const_Ref other) const { return m_i >= other.m_i; }
        //operator int () const { return m_i; }

        //traversal
    private:
        void _next_block ();
    public:
        inline void begin ();
        void next ();
        Ref operator ++ () { next(); return *this; }
        operator bool () const { return m_i; }
        bool done () const { return not m_i; }
        void finish () { m_i = 0; }

        //dereferencing
    private:
        void _deref_assert () const
        { Assert5(not done(), "dereferenced done dense_set::iter"); }
    public:
        int        operator *  () const { _deref_assert(); return m_i; }
        const int* operator -> () const { _deref_assert(); return &m_i; }

        //constructors
        //WARNING: careful using these
        iterator (const dense_set* set) : m_set(*set) {}
        iterator (const dense_set& set) : m_set(set) { begin(); }
    };
    iterator begin () const { iterator i(this); i.begin(); return i; }
    iterator end   () const { return iterator(this); }
};
inline bool_ref dense_set::_bit (int i)
{
    Assert5(0<i and i<=N, "dense_set[i] index out of range: " << i);
    div_t I = div(i,LINE_STRIDE);
    return bool_ref(m_lines + I.quot, I.rem);
}
inline bool dense_set::_bit (int i) const
{
    Assert5(0<i and i<=N, "const dense_set[i] index out of range: " << i);
    div_t I = div(i,LINE_STRIDE);
    return m_lines[I.quot] & (1<<I.rem);
}
inline void dense_set::insert (int i)
{
    Assert5(0<i and i<=N, "dense_set::insert item out of range: " << i);
    Assert4(not contains(i),
            "tried to insert item " << i << " in dense_set twice");
    _bit(i).one();
}
inline void dense_set::remove (int i)
{
    Assert5(0<i and i<=N, "dense_set::remove item out of range: " << i);
    Assert4(contains(i),
            "tried to remove item " << i << " from dense_set twice");
    _bit(i).zero();
}
inline void dense_set::merge (int i, int j)
{
    Assert5(0<i and i<=N, "dense_set.merge(i,j) index i="<<i<<" out of range");
    Assert5(0<j and j<=N, "dense_set.merge(i,j) index j="<<j<<" out of range");
    Assert5(i!=j, "dense_set tried to merge item "<<i<<" into itself");
    Assert5(i>j, "dense_set tried to merge in wrong order: "<<i<<'>'<<j);
    Assert4(contains(i) and contains(j),
            "dense_set tried to merge uninserted items: "<<i<<","<<j);
    _bit(i).zero();
}
inline void dense_set::iterator::begin ()
{
    Assert4(m_set.m_lines, "tried to begin a null dense_set::iterator");
    m_quot = -1;
    _next_block();
    Assert5(done() or m_set.contains(m_i),
            "dense_set::iterator::begin landed on empty pos "<<m_i);
}

}

#endif

