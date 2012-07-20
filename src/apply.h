#ifndef JOHANN_APPLY_H
#define JOHANN_APPLY_H

#include "definitions.h"
#include "nodes.h"
#include <pomagma/dense_bin_fun.hpp>

#include <pomagma/splay_forest.hpp>
#include "reorder.h"
//#include <set>

namespace Apply
{

const Logging::Logger logger("apply", Logging::DEBUG);

using namespace Heap;

typedef pomagma::dense_set Set;
typedef pomagma::dense_bin_fun AppTable;
extern AppTable * g_app_table; // this should be private

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
    APP(0x0), LHS(0x1), RHS(0x2);

//a block of tree neighbor pointers
const TypedIndex<App>
    ALR_UP(0x2), ALR_LEFT(0x3), ALR_RIGHT(0x4),
    ARL_UP(0x5), ARL_LEFT(0x6), ARL_RIGHT(0x7);

//splay Indexings
template<int key, int value, int indexOffset>
class Indexing
{//neighbor fields of each multimap indexing type
    enum IndexFields {
        OROOT = 0,
        KEY   = key,
        VALUE = value,
    };
    enum RelativeFields {
        EROOT = 3 + indexOffset, //WARNING: this should match ALR_ROOT in obs.h
        UP    = 2 + 3*indexOffset, //WARNING: this should match ALR_UP above
        LEFT  = UP + 1,
        RIGHT = LEFT + 1
    };

public:
    typedef ::Ob Ob;
    typedef App Pos;

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
typedef pomagma::splay_forest<Indexing< L,R, 0> > ALR;
typedef pomagma::splay_forest<Indexing< R,L, 1> > ARL;
#undef L
#undef R

//======================== interface ========================

//top level
void init (Int num_obs, Int num_eqns, bool is_full=false);
void clear ();
void resize (Int num_obs);
void resize (Int num_obs, Int num_eqns,
             const Reorder::Reordering<Ob>& o_order,
             const Reorder::Reordering<App>& e_order);
void validate (Int level);

//finding functions (inlines defined below
inline App find_app_eqn (Ob lhs, Ob rhs); //only used in Ob parsing
inline Ob find_app  (Ob lhs, Ob rhs);
inline Ob find_ldiv (Ob lhs, Ob app);
inline Ob find_rdiv (Ob app, Ob rhs);
inline const Set Lx_support (Ob ob) { return g_app_table->get_Lx_set(ob); }
inline const Set Rx_support (Ob ob) { return g_app_table->get_Rx_set(ob); }

//insertion/creation, deletion
App create (Ob app, Ob lhs, Ob rhs);
inline void delete_ (App eqn) { eqn(NAME) = NULL; App::free(eqn); }

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
inline Ob get_app (App eqn) { return Ob(eqn(APP)); }
inline Ob get_lhs (App eqn) { return Ob(eqn(LHS)); }
inline Ob get_rhs (App eqn) { return Ob(eqn(RHS)); }
inline void set_app (App eqn, Ob ob) { eqn(APP) = Short(ob); }
inline void set_lhs (App eqn, Ob ob) { eqn(LHS) = Short(ob); }
inline void set_rhs (App eqn, Ob ob) { eqn(RHS) = Short(ob); }

//======================== internals for iterator ========================

typedef App Pos;

namespace _private
{

//tree iteration
template<class X> class Iterator : public X::Iterator
{
public:
    Iterator () : X::Iterator() {}
    Iterator (Ob root_ob) : X::Iterator(root_ob) {}

    //dereferencing
    Ob app () const { return get_app(**this); }
    Ob lhs () const { return get_lhs(**this); }
    Ob rhs () const { return get_rhs(**this); }
};

//the derived range iterator
template<class X> class RangeIterator : public X::RangeIterator
{
public:
    RangeIterator () : X::RangeIterator() {}
    RangeIterator (Ob root_ob, Ob key_ob)
        : X::RangeIterator(root_ob, key_ob) {}

    //dereferencing
    Ob app () const { return get_app(**this); }
    Ob lhs () const { return get_lhs(**this); }
    Ob rhs () const { return get_rhs(**this); }
};

//app table iteration
template<int X> class LineIterator : public AppTable::Iterator<X>
{
    typedef AppTable::Iterator<X> Base;
public:
    LineIterator ();
    LineIterator (Ob fixed);

    //iteration
    void begin () { Base::begin(); }
    void begin (Ob ob) { Base::begin(Int(ob)); }

    //dereferencing
    Ob lhs () const { return Ob(Base::lhs()); }
    Ob rhs () const { return Ob(Base::rhs()); }
    Ob app () const { return get_app(App(Base::value())); }
    App operator * () const { return App(Base::value()); }
};
class LLxx_Iter : public AppTable::LLxx_Iter
{
    typedef AppTable::LLxx_Iter Base;
public:
    LLxx_Iter ();
    LLxx_Iter (Ob f, Ob g);

    //iteration
    void begin () { Base::begin(); }
    void begin (Ob f, Ob g) { Base::begin(Int(f), Int(g)); }

    //dereferencing
    Ob rhs  () const { return Ob(Base::rhs()); }
    Ob app1 () const { return get_app(App(Base::value1())); }
    Ob app2 () const { return get_app(App(Base::value2())); }
};
class LRxx_Iter : public AppTable::LRxx_Iter
{
    typedef AppTable::LRxx_Iter Base;
public:
    LRxx_Iter ();
    LRxx_Iter (Ob f, Ob x);

    //iteration
    void begin () { Base::begin(); }
    void begin (Ob f, Ob x) { Base::begin(Int(f), Int(x)); }

    //dereferencing
    Ob rhs1 () const { return Ob(Base::rhs1()); }
    Ob lhs2 () const { return Ob(Base::lhs2()); }
    Ob app1 () const { return get_app(App(Base::value1())); }
    Ob app2 () const { return get_app(App(Base::value2())); }
};
class RRxx_Iter : public AppTable::RRxx_Iter
{
    typedef AppTable::RRxx_Iter Base;
public:
    RRxx_Iter ();
    RRxx_Iter (Ob x, Ob y);

    //iteration
    void begin () { Base::begin(); }
    void begin (Ob x, Ob y) { Base::begin(Int(x), Int(y)); }

    //dereferencing
    Ob lhs  () const { return Ob(Base::lhs()); }
    Ob app1 () const { return get_app(App(Base::value1())); }
    Ob app2 () const { return get_app(App(Base::value2())); }
};

}

//the public iterators
typedef _private::Iterator<ALR>                      Alr_Iterator;
typedef _private::RangeIterator<ALR>                 ALr_Iterator;
typedef _private::RangeIterator<ARL>                 ARl_Iterator;
typedef _private::LineIterator<AppTable::LHS_FIXED>  Lra_Iterator;
typedef _private::LineIterator<AppTable::RHS_FIXED>  Rla_Iterator;
typedef _private::LLxx_Iter                          LLra_Iterator;
typedef _private::LRxx_Iter                          LRxa_Iterator;
typedef _private::RRxx_Iter                          RRla_Iterator;

//delayed inlines

//finding functions
#define CHECK_POS(pos) {Assert5((pos).isValid()and(pos).isUsed(),"bad position: " << pos);}
inline App find_app_eqn (Ob lhs, Ob rhs)
{//returns an app if it exists, 0 otherwise
    CHECK_POS(lhs)
    CHECK_POS(rhs)
    return App(g_app_table->get_value(Int(lhs), Int(rhs)));
}
inline Ob find_app (Ob lhs, Ob rhs)
{//returns an app if it exists, 0 otherwise
    CHECK_POS(lhs)
    CHECK_POS(rhs)
    App eqn(g_app_table->get_value(Int(lhs), Int(rhs)));
    return eqn ? get_app(eqn) : Ob(0);
}
inline Ob find_ldiv (Ob lhs, Ob app)
{//returns an Ob if it exists, 0 otherwise
    CHECK_POS(lhs)
    CHECK_POS(app)
    if (App eqn = ALR::find_key(app,lhs)) return get_rhs(eqn);
    else return Ob(0);
}
inline Ob find_rdiv (Ob app, Ob rhs)
{//returns an Ob if it exists, 0 otherwise
    CHECK_POS(app)
    CHECK_POS(rhs)
    if (App eqn = ARL::find_key(app,rhs)) return get_lhs(eqn);
    else return Ob(0);
}
#undef CHECK_POS
}

using Apply::get_app;
using Apply::get_lhs;
using Apply::get_rhs;

//names
template<> inline const char* nameof<Apply::ALR> () { return "ALR"; }
template<> inline const char* nameof<Apply::ARL> () { return "ARL"; }

#endif
