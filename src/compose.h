#ifndef JOHANN_COMPOSE_H
#define JOHANN_COMPOSE_H

#include "definitions.h"
#include "nodes.h"
#include <pomagma/dense_bin_fun.hpp>
#include <pomagma/splay_forest.hpp>
#include "reorder.h"
//#include <set>

namespace Compose
{

const Logging::Logger logger("compose", Logging::DEBUG);

using namespace Heap;

typedef pomagma::dense_set Set;
typedef pomagma::dense_bin_fun CompTable;
extern CompTable *g_comp_table; //this should be private

//======================== field layout ========================
//(see node_field_usage.text)

//these are the components of each equation
const TypedIndex<void*>
    NAME(0x0);
const TypedIndex<Int>
    IS_USED(0x1);

//indices
//there's not enough room to store 3 Obs, so we store Shorts instead
const TypedIndex<Short>
    COMP(0x0), LHS(0x1), RHS(0x2);

//a block of tree neighbor pointers
const TypedIndex<Comp>
    CLR_UP(0x2), CLR_LEFT(0x3), CLR_RIGHT(0x4),
    CRL_UP(0x5), CRL_LEFT(0x6), CRL_RIGHT(0x7);

//Indexings for splay trees and the table
template<int key, int value, int indexOffset>
class Indexing
{//neighbor fields of each multimap indexing type
    enum IndexFields {
        OROOT = 0,
        KEY   = key,
        VALUE = value,
    };
    enum RelativeFields {
        EROOT = 5 + indexOffset, //WARNING: this should match CLR_ROOT in obs.h
        UP    = 2 + 3*indexOffset, //WARNING: this should match CLR_UP above
        LEFT  = UP + 1,
        RIGHT = LEFT + 1
    };

public:
    typedef ::Ob Ob;
    typedef Comp Pos;

    //tree roots
    typedef TypedIndex<Pos> TIP;
    static inline Pos& root  (Ob ob) { return ob(TIP(EROOT)); }
    static inline Pos& root  (Pos p) { return root(Ob(p(TIO(OROOT)))); }

    //key,value indices
    typedef TypedIndex<Short> TIO;
    static inline Ob get_root (Pos p) { return Ob(p(TIO(OROOT))); }
    static inline Ob get_key  (Pos p) { return Ob(p(TIO(KEY))); }
    static inline Ob get_val  (Pos p) { return Ob(p(TIO(VALUE))); }

    //tree node relationships
    static inline Pos& up   (Pos p) { return p(UP); }
    static inline Pos& left (Pos p) { return p(LEFT); }
    static inline Pos& right(Pos p) { return p(RIGHT); }
};
#define L 1
#define R 2
typedef pomagma::splay_forest<Indexing< L,R, 0> > CLR;
typedef pomagma::splay_forest<Indexing< R,L, 1> > CRL;
#undef L
#undef R

//======================== interface ========================

//top level
void init (Int num_obs, Int num_eqns, bool is_full=false);
void clear ();
void resize (Int num_obs);
void resize (Int num_obs, Int num_eqns,
             const Reorder::Reordering<Ob>& o_order,
             const Reorder::Reordering<Comp>& e_order);
void validate (Int level);

//finding functions (inlines defined below
inline Comp find_comp_eqn (Ob lhs, Ob rhs); //only used in Ob parsing
inline Ob find_comp  (Ob lhs, Ob rhs);
inline Ob find_linv_comp (Ob lhs, Ob comp);
inline Ob find_rinv_comp (Ob comp, Ob rhs);
inline const Set Lx_support (Ob ob) { return g_comp_table->get_Lx_set(ob); }
inline const Set Rx_support (Ob ob) { return g_comp_table->get_Rx_set(ob); }

//insertion/creation, deletion
Comp create (Ob comp, Ob lhs, Ob rhs);
inline void delete_ (Comp eqn) { eqn(NAME) = NULL; Comp::free(eqn); }

//insertion, removal, merging
void remove (Ob rem);
void merge (Ob dep, Ob rep);

//forest diagnostics
void test_iterators ();

//saving/loading
Int data_size ();
void save_to_file (FILE* file);
void load_from_file (FILE* file);

//eqn<-->short conversion for indexing
inline Ob get_comp (Comp eqn) { return Ob(eqn(COMP)); }
inline Ob get_lhs (Comp eqn) { return Ob(eqn(LHS)); }
inline Ob get_rhs (Comp eqn) { return Ob(eqn(RHS)); }
inline void set_comp (Comp eqn, Ob ob) { eqn(COMP) = Short(ob); }
inline void set_lhs (Comp eqn, Ob ob) { eqn(LHS) = Short(ob); }
inline void set_rhs (Comp eqn, Ob ob) { eqn(RHS) = Short(ob); }

//======================== internals for iterator ========================

typedef Comp Pos;

namespace _private
{

//tree iteration
template<class X> class Iterator : public X::Iterator
{
public:
    Iterator () : X::Iterator() {}
    Iterator (Ob root_ob) : X::Iterator(root_ob) {}

    //dereferencing
    Ob comp () const { return get_comp(**this); }
    Ob lhs  () const { return get_lhs (**this); }
    Ob rhs  () const { return get_rhs (**this); }
};

//the derived range iterator
template<class X> class RangeIterator : public X::RangeIterator
{
public:
    RangeIterator () : X::RangeIterator() {}
    RangeIterator (Ob root_ob, Ob key_ob)
        : X::RangeIterator(root_ob, key_ob) {}

    //dereferencing
    Ob comp () const { return get_comp(**this); }
    Ob lhs  () const { return get_lhs (**this); }
    Ob rhs  () const { return get_rhs (**this); }
};

//comp table iteration
template<int X> class LineIterator : public CompTable::Iterator<X>
{
    typedef CompTable::Iterator<X> Base;
public:
    LineIterator ();
    LineIterator (Ob fixed);

    //iteration
    void begin () { Base::begin(); }
    void begin (Ob ob) { Base::begin(Int(ob)); }

    //dereferencing
    Ob lhs () const { return Ob(Base::lhs()); }
    Ob rhs () const { return Ob(Base::rhs()); }
    Ob comp () const { return get_comp(Comp(Base::value())); }
    Comp operator * () const { return Comp(Base::value()); }
};
class LLxx_Iter : public CompTable::LLxx_Iter
{
    typedef CompTable::LLxx_Iter Base;
public:
    LLxx_Iter ();
    LLxx_Iter (Ob f, Ob g);

    //iteration
    void begin () { Base::begin(); }
    void begin (Ob f, Ob g) { Base::begin(Int(f), Int(g)); }

    //dereferencing
    Ob rhs  () const { return Ob(Base::rhs()); }
    Ob comp1 () const { return get_comp(Comp(Base::value1())); }
    Ob comp2 () const { return get_comp(Comp(Base::value2())); }
};
class LRxx_Iter : public CompTable::LRxx_Iter
{
    typedef CompTable::LRxx_Iter Base;
public:
    LRxx_Iter ();
    LRxx_Iter (Ob f, Ob x);

    //iteration
    void begin () { Base::begin(); }
    void begin (Ob f, Ob x) { Base::begin(Int(f), Int(x)); }

    //dereferencing
    Ob rhs1 () const { return Ob(Base::rhs1()); }
    Ob lhs2 () const { return Ob(Base::lhs2()); }
    Ob comp1 () const { return get_comp(Comp(Base::value1())); }
    Ob comp2 () const { return get_comp(Comp(Base::value2())); }
};
class RRxx_Iter : public CompTable::RRxx_Iter
{
    typedef CompTable::RRxx_Iter Base;
public:
    RRxx_Iter ();
    RRxx_Iter (Ob x, Ob y);

    //iteration
    void begin () { Base::begin(); }
    void begin (Ob x, Ob y) { Base::begin(Int(x), Int(y)); }

    //dereferencing
    Ob lhs  () const { return Ob(Base::lhs()); }
    Ob comp1 () const { return get_comp(Comp(Base::value1())); }
    Ob comp2 () const { return get_comp(Comp(Base::value2())); }
};

}

//the public iterators
typedef _private::Iterator<CLR>                      Clr_Iterator;
typedef _private::RangeIterator<CLR>                 CLr_Iterator;
typedef _private::RangeIterator<CRL>                 CRl_Iterator;
typedef _private::LineIterator<CompTable::LHS_FIXED> Lrc_Iterator;
typedef _private::LineIterator<CompTable::RHS_FIXED> Rlc_Iterator;
typedef _private::LLxx_Iter                          LLrc_Iterator;
typedef _private::LRxx_Iter                          LRxc_Iterator;
typedef _private::RRxx_Iter                          RRlc_Iterator;

//delayed inlines

//finding functions
#define CHECK_POS(pos) {Assert5((pos).isValid()and(pos).isUsed(),"bad position: " << pos);}
inline Comp find_comp_eqn (Ob lhs, Ob rhs)
{//returns an comp if it exists, 0 otherwise
    CHECK_POS(lhs)
    CHECK_POS(rhs)
    return Comp(g_comp_table->get_value(Int(lhs), Int(rhs)));
}
inline Ob find_comp (Ob lhs, Ob rhs)
{//returns an comp if it exists, 0 otherwise
    CHECK_POS(lhs)
    CHECK_POS(rhs)
    Comp eqn(g_comp_table->get_value(Int(lhs), Int(rhs)));
    return eqn ? get_comp(eqn) : Ob(0);
}
inline Ob find_linv_comp (Ob lhs, Ob comp)
{//returns an Ob if it exists, 0 otherwise
    CHECK_POS(lhs)
    CHECK_POS(comp)
    if (Comp eqn = CLR::find_key(comp,lhs)) return get_rhs(eqn);
    else return Ob(0);
}
inline Ob find_rinv_comp (Ob comp, Ob rhs)
{//returns an Ob if it exists, 0 otherwise
    CHECK_POS(comp)
    CHECK_POS(rhs)
    if (Comp eqn = CRL::find_key(comp,rhs)) return get_lhs(eqn);
    else return Ob(0);
}
#undef CHECK_POS
}

using Compose::get_comp;
using Compose::get_lhs;
using Compose::get_rhs;

//names
template<> inline const char* nameof<Compose::CLR> () { return "CLR"; }
template<> inline const char* nameof<Compose::CRL> () { return "CRL"; }

#endif
