#ifndef JOHANN_ORDER_H
#define JOHANN_ORDER_H

#include "definitions.h"
#include "nodes.h"
#include <pomagma/dense_bin_rel.hpp>

namespace CombinatoryStructure
{
extern void enforce_less (int lhs, int rhs);
extern void enforce_nless(int lhs, int rhs);
}

namespace Order
{
const Logging::Logger logger("order", Logging::DEBUG);

namespace CS = CombinatoryStructure;

typedef pomagma::dense_set Set;
using pomagma::bool_ref;
typedef pomagma::dense_bin_rel OrdTable;
extern OrdTable *g_pos_table, *g_neg_table; //should be private
enum { NUM_TEMP_SETS = 4 };
extern Set* g_temp_sets[NUM_TEMP_SETS]; //should be private
inline Set& temp_set (unsigned n=0)
{
    Assert (n < NUM_TEMP_SETS, "too few temp sets");
    return *(g_temp_sets[n]);
}

//iterator types
enum { POS = true, NEG = false };
template<int _dir, int sign> struct Indexing
{
    enum Dir  { dir = _dir };
    enum Sign { is_pos = sign };
};
typedef Indexing< OrdTable::LHS_FIXED, POS> LRpos;
typedef Indexing< OrdTable::LHS_FIXED, NEG> LRneg;
typedef Indexing< OrdTable::RHS_FIXED, POS> RLpos;
typedef Indexing< OrdTable::RHS_FIXED, NEG> RLneg;

//======================== data structures ========================

//ord class
class Ord
{//just the coordinates of a single bit
    Ob m_lhs, m_rhs;

    //ctors & dtor
public:
    explicit Ord (Int i) : m_lhs(i), m_rhs(i) {}
    Ord (Ob lhs, Ob rhs) : m_lhs(lhs), m_rhs(rhs) {}
    operator bool () const { return m_lhs or m_rhs; }

    //table allocation
    static void init   (Int num_obs, bool is_full=false);
    static void resize (Int num_obs, const Int* old2new=NULL);
    static void clear ();
    static inline unsigned long capacity () { return g_pos_table->capacity(); }
    static inline unsigned long size_pos () { return g_pos_table->size(); }
    static inline unsigned long size_neg () { return g_neg_table->size(); }
    static inline unsigned long size     () { return size_pos() + size_neg(); }

    //comparison, for containers
    bool operator == (const Ord& rhs) const
    { return m_lhs == rhs.m_lhs and m_rhs == rhs.m_rhs; }
    bool operator != (const Ord& rhs) const
    { return m_lhs != rhs.m_lhs or m_rhs != rhs.m_rhs; }
    bool operator < (const Ord& rhs) const
    {
        return (m_lhs < rhs.m_lhs)
            or (m_lhs == rhs.m_lhs and (m_rhs < rhs.m_rhs));
    }

    //access
    Ob lhs () const { return m_lhs; }
    Ob rhs () const { return m_rhs; }
    bool_ref value_pos ();
    bool_ref value_neg ();
    bool     value_pos () const;
    bool     value_neg () const;

    //pretty printing
    void write_to_stream (ostream& os) const
    { os << m_lhs << " [= " << m_rhs; }

    //table iteration
    template<int parity>
    class iterator
    {//just a wrapper around dense_bin_rel::iterator
        OrdTable::iterator m_iter;

    public:
        iterator () : m_iter(parity ? g_pos_table : g_neg_table) {}

        //traversal
        void next () { m_iter.next(); }
        bool ok () const { return m_iter.ok(); }

        //dereferencing
        const Ord& operator *  () const
        { return (reinterpret_cast<const Ord*>(&(*m_iter)))[0]; }
        const Ord* operator -> () const
        { return reinterpret_cast<const Ord*>(&(*m_iter)); }
        Ob lhs () { return Ob(m_iter.lhs()); }
        Ob rhs () { return Ob(m_iter.rhs()); }
    };
};
struct Rel
{
    Order::Ord ord;
    bool less; //parity
    Rel (Ob lhs, Ob rhs, bool _less) : ord(lhs,rhs), less(_less) {}
    Ob lhs () const { return ord.lhs(); }
    Ob rhs () const { return ord.rhs(); }
};

//======================== interface ========================

//ord table access
inline bool contains_pos (Ord ord)
    { return g_pos_table->contains(ord.lhs(), ord.rhs()); }
inline bool contains_neg (Ord ord)
    { return g_neg_table->contains(ord.lhs(), ord.rhs()); }
inline bool contains (Ord ord){return contains_neg(ord) or contains_pos(ord); }
inline bool contains_pos (Ob ob1, Ob ob2) { return contains_pos(Ord(ob1,ob2)); }
inline bool contains_neg (Ob ob1, Ob ob2) { return contains_neg(Ord(ob1,ob2)); }
inline bool contains (Ob ob1, Ob ob2) { return contains(Ord(ob1,ob2)); }
inline bool ensure_less (Ord ord)
    { return not g_pos_table->ensure_inserted(ord.lhs(), ord.rhs()); }
inline bool ensure_nless (Ord ord)
    { return not g_neg_table->ensure_inserted(ord.lhs(), ord.rhs()); }
//WARNING: these vectorized versions do not call O::getRep(*iter)
inline void ensure_less (Ob x, const Set& ys)
    { g_pos_table->ensure_inserted(x,ys, CS::enforce_less); }
inline void ensure_less (const Set& xs, Ob y)
    { g_pos_table->ensure_inserted(xs,y, CS::enforce_less); }
inline void ensure_nless (Ob x, const Set& ys)
    { g_neg_table->ensure_inserted(x,ys, CS::enforce_nless); }
inline void ensure_nless (const Set& xs, Ob y)
    { g_neg_table->ensure_inserted(xs,y, CS::enforce_nless); }

//set access
inline const Set above (Ob x)
    { return Set(Ob::capacity(), g_pos_table->get_Lx_line(x)); }
inline const Set below (Ob x)
    { return Set(Ob::capacity(), g_pos_table->get_Rx_line(x)); }
inline const Set nabove (Ob x)
    { return Set(Ob::capacity(), g_neg_table->get_Lx_line(x)); }
inline const Set nbelow (Ob x)
    { return Set(Ob::capacity(), g_neg_table->get_Rx_line(x)); }

//ob support (used obs) access
inline void insert (Ob ob) { g_pos_table->insert(ob); g_neg_table->insert(ob); }
inline void remove (Ob ob) { g_pos_table->remove(ob); g_neg_table->remove(ob); }
inline void merge (Ob dep, Ob rep)
{
    Assert1(dep > rep, "Ord::merge: dep <= rep");

    //when a ord is moved over, it must be enforced
    g_pos_table->merge(dep, rep, CS::enforce_less);
    g_neg_table->merge(dep, rep, CS::enforce_nless);
}

//validation
void validate (Int level);
void test ();

//saving/loading
Int data_size ();
void save_to_file (FILE* file);
void load_from_file (FILE* file);

//======================== internals for iterator ========================

//line iteration
template<class idx>
class Iterator
{//given an ob x, iterates through all obs y [= x (or y =] x or either)
    OrdTable::Iterator<idx::dir> m_iter;

public:
    Iterator (Ob ob)
        : m_iter(static_cast<int>(ob), idx::is_pos ? g_pos_table : g_neg_table)
    { Assert3(ob.isUsed(), "Order Iterator created at unused ob"); }
    Iterator () : m_iter(idx::is_pos ? g_pos_table : g_neg_table) {}

    //traversal
    void begin () { m_iter.begin(); }
    void begin (Ob ob)
    {
        Assert3(ob.isUsed(), "Order Iterator began at unused ob");
        m_iter.begin(static_cast<int>(ob));
    }
    bool ok () const { return m_iter.ok(); }
    void next () { m_iter.next(); }

    //dereferencing
    const Ord& operator *  () const
    { return reinterpret_cast<const Ord*>(&*m_iter)[0]; }
    const Ord* operator -> () const
    { return reinterpret_cast<const Ord*>(&*m_iter); }

    //access
    Ob other () const { return Ob(m_iter.moving()); };
    Ob lhs   () const { return Ob(m_iter.lhs()); }
    Ob rhs   () const { return Ob(m_iter.rhs()); }
};

}

//names
template<> inline const char* nameof<Order::LRpos> () { return "LRpos"; }
template<> inline const char* nameof<Order::LRneg> () { return "LRneg"; }
template<> inline const char* nameof<Order::RLpos> () { return "RLpos"; }
template<> inline const char* nameof<Order::RLneg> () { return "RLneg"; }

using Order::Ord;

inline ostream& operator << (ostream& os, const Ord& ord)
{ ord.write_to_stream(os); return os; }

#endif

