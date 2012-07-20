#ifndef JOHANN_JOIN_H
#define JOHANN_JOIN_H

#include "definitions.h"
#include "nodes.h"
#ifdef SPARSE_JOINS
    #include "sparse_sym_fun.h"
#else
    #include <pomagma/dense_sym_fun.hpp>
#endif
#include <pomagma/splay_forest.hpp>
#include "reorder.h"
//#include <set>

//TODO implement sparse_sym_fun

namespace JoinEqn
{

const Logging::Logger logger("join", Logging::DEBUG);

using namespace Heap;

typedef pomagma::dense_set Set;
#ifdef SPARSE_JOINS
typedef pomagma::sparse_sym_fun JoinTable;
#else
typedef pomagma::dense_sym_fun JoinTable;
#endif
extern JoinTable *g_join_table; //this should be private

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
    JOIN(0x0), LHS(0x1), RHS(0x2);

//a block of tree neighbor pointers
const TypedIndex<Join>
    JLR_UP(0x2), JLR_LEFT(0x3), JLR_RIGHT(0x4);

//splay Indexings
template<int key, int value>
class Indexing
{//neighbor fields of each multimap indexing type
    enum IndexFields {
        OROOT = 0,
        KEY   = key,
        VALUE = value,
    };
    enum RelativeFields {
        EROOT = 7, //WARNING: this should match JLR_ROOT in obs.h
        UP    = 2, //WARNING: this should match JLR_UP above
        LEFT  = UP + 1,
        RIGHT = LEFT + 1
    };

public:
    typedef ::Ob Ob;
    typedef Join Pos;

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
typedef pomagma::splay_forest<Indexing<L,R> > JLR;
#undef L
#undef R

//======================== interface ========================

inline void sort (Ob& i, Ob& j) { if (j < i) { Ob k=j; j=i; i=k;}  }

//top level
void init (Int num_obs, Int num_eqns, bool is_full=false);
void clear ();
void resize (Int num_obs);
void resize (Int num_obs, Int num_eqns,
             const Reorder::Reordering<Ob>& o_order,
             const Reorder::Reordering<Join>& e_order);
void validate (Int level);

//finding functions (inlines defined below
inline Join find_join_eqn (Ob lhs, Ob rhs); //only used in Ob parsing
inline Ob find_join  (Ob lhs, Ob rhs);
inline const Set Lx_support (Ob ob) { return g_join_table->get_Lx_set(ob); }

//insertion/creation, deletion
Join create (Ob join, Ob lhs, Ob rhs);
inline void delete_ (Join eqn) { eqn(NAME) = NULL; Join::free(eqn); }

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
inline Ob get_join (Join eqn) { return Ob(eqn(JOIN)); }
inline Ob get_lhs (Join eqn) { return Ob(eqn(LHS)); }
inline Ob get_rhs (Join eqn) { return Ob(eqn(RHS)); }
inline void set_join (Join eqn, Ob ob) { eqn(JOIN) = Short(ob); }
inline void set_lhs (Join eqn, Ob ob) { eqn(LHS) = Short(ob); }
inline void set_rhs (Join eqn, Ob ob) { eqn(RHS) = Short(ob); }

//======================== internals for iterator ========================

typedef Join Pos;

//tree iteration
class Jlr_Iterator : public JLR::Iterator
{
public:
    Jlr_Iterator () : JLR::Iterator() {}
    Jlr_Iterator (Ob root_ob) : JLR::Iterator(root_ob) {}

    //dereferencing
    Ob join () const { return get_join(**this); }
    Ob lhs () const { return get_lhs(**this); }
    Ob rhs () const { return get_rhs(**this); }
};

//the derived range iterator
class JLr_Iterator : public JLR::RangeIterator
{
public:
    JLr_Iterator () : JLR::RangeIterator() {}
    JLr_Iterator (Ob root_ob, Ob key_ob) : JLR::RangeIterator(root_ob, key_ob) {}

    //dereferencing
    Ob join () const { return get_join(**this); }
    Ob lhs () const { return get_lhs(**this); }
    Ob rhs () const { return get_rhs(**this); }
};

//join table iteration
class Lrj_Iterator : public JoinTable::Iterator
{
    typedef JoinTable::Iterator Base;
public:
    Lrj_Iterator ();
    Lrj_Iterator (Ob fixed);
    Lrj_Iterator (Ob fixed, Set& subset);

    //iteration
    void begin () { Base::begin(); }
    void begin (Ob ob) { Base::begin(Int(ob)); }

    //dereferencing
    Ob fixed  () const { return Ob(Base::fixed()); }
    Ob moving () const { return Ob(Base::moving()); }
    Ob join   () const { return get_join(Join(Base::value())); }
    Join operator * () const { return Join(Base::value()); }
};
class LLrj_Iterator : public JoinTable::LLxx_Iter
{
    typedef JoinTable::LLxx_Iter Base;
public:
    LLrj_Iterator ();
    LLrj_Iterator (Ob f, Ob g);

    //iteration
    void begin () { Base::begin(); }
    void begin (Ob f, Ob g) { Base::begin(Int(f), Int(g)); }

    //dereferencing
    Ob fixed1 () const { return Ob(Base::fixed1()); }
    Ob fixed2 () const { return Ob(Base::fixed2()); }
    Ob moving () const { return Ob(Base::moving()); }
    Ob join1  () const { return get_join(Join(Base::value1())); }
    Ob join2  () const { return get_join(Join(Base::value2())); }
};

//delayed inlines

//finding functions
#define CHECK_POS(pos) {Assert5((pos).isValid()and(pos).isUsed(),"bad position: " << pos);}
inline Join find_join_eqn (Ob lhs, Ob rhs)
{//returns an join if it exists, 0 otherwise
    sort(lhs,rhs);
    CHECK_POS(lhs)
    CHECK_POS(rhs)
    return Join(g_join_table->get_value(Int(lhs), Int(rhs)));
}
inline Ob find_join (Ob lhs, Ob rhs)
{//returns an join if it exists, 0 otherwise
    sort(lhs,rhs);
    CHECK_POS(lhs)
    CHECK_POS(rhs)
    Join eqn(g_join_table->get_value(Int(lhs), Int(rhs)));
    return eqn ? get_join(eqn) : Ob(0);
}
#undef CHECK_POS
}

using JoinEqn::get_join;
using JoinEqn::get_lhs;
using JoinEqn::get_rhs;

#endif
