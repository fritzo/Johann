
#include "compose.h"
#include "obs.h"
#include "combinatory_structure.h"
#include <vector> //for merge<X>
#include <utility>
#include <algorithm>

//log levels
#define LOG_DEBUG1(mess);
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << mess |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

namespace Compose
{

namespace O = Obs;
namespace CS = CombinatoryStructure;
using namespace _private;

//the comp table
CompTable *g_comp_table(NULL);

//top level
void init (Int num_obs, Int num_eqns, bool is_full)
{
    logger.debug() << "Initializing Composes" |0;
    Logging::IndentBlock block;

    Assert (g_comp_table == NULL, "tried to init g_comp_table twice");

    //init forests
    Comp::init(num_eqns, is_full);

    //init comp table
    g_comp_table = new CompTable(num_obs);
}
void clear ()
{
    logger.debug() << "Clearing Composes" |0;
    Logging::IndentBlock block;

    Assert (g_comp_table != NULL, "tried to clear g_comp_table twice");

    Comp::clear();

    delete g_comp_table; g_comp_table = NULL;
}
void resize (Int num_obs)
{
    logger.debug() << "Resizing comp table" |0;
    Logging::IndentBlock block;

    Assert (g_comp_table != NULL, "tried to resize uninitialized g_comp_table");

    //XXX: this uses lots of memory
    CompTable *new_table = new CompTable(num_obs);
    new_table->move_from(*g_comp_table);
    delete g_comp_table;
    g_comp_table = new_table;
}
void insert_all ();
void resize (Int num_obs, Int num_eqns,
             const Reorder::Reordering<Ob>& o_order,
             const Reorder::Reordering<Comp>& e_order)
{
    logger.debug() << "Resizing Composes" |0;
    Logging::IndentBlock block;

    Assert (g_comp_table != NULL, "tried to resize uninitialized g_comp_table");

    //free table
    delete g_comp_table;

    //resize forests
    Comp::resize(Comp::numUsed(), e_order.new2old());
    for (Comp::sparse_iterator iter=Comp::sbegin(), end=Comp::send();
            iter!=end; ++iter) {
        Comp eqn = *iter;

        //move A,L,R
        set_comp(eqn, o_order.old2new(get_comp(eqn)));
        set_lhs(eqn, o_order.old2new(get_lhs(eqn)));
        set_rhs(eqn, o_order.old2new(get_rhs(eqn)));
    }

    //rebuild table and forests
    g_comp_table = new CompTable(num_obs);
    insert_all();
}

//validation
void validate_forests (Int level);
void validate_comp_table (Int level);
void validate_agreement (Int level);
void validate (Int level)
{
    validate_forests(level);
    validate_comp_table(level);
    validate_agreement(level);
}

//minimal search tree interface
void insert (Comp eqn);
inline void _clear(Comp eqn);

//saving/loading
struct LittleComp
{
    Short a,l,r;
    LittleComp () {}
    LittleComp (Comp e) : a(e(COMP)), l(e(LHS)), r(e(RHS)) {}
};
Int data_size () { return sizeof(LittleComp) * Comp::numUsed(); }
void save_to_file (FILE* file)
{
    logger.debug() << "Saving " << Pos::numUsed() << " Comps to file" |0;
    Logging::IndentBlock block;

    for (Pos::iterator i=Pos::begin(); i!=Pos::end(); ++i) {
        LittleComp e = *i;
        safe_fwrite(&e, sizeof(LittleComp), 1, file);
    }
}
void load_from_file (FILE* file)
{
    logger.debug() << "Loading " << Pos::numUsed() << " Comps from file" |0;
    Logging::IndentBlock block;

    for (Pos::iterator i=Pos::begin(); i!=Pos::end(); ++i) {
        LittleComp e;
        safe_fread(&e, sizeof(LittleComp), 1, file);

        Pos pos = *i;
        _clear(pos);
        pos(COMP) = e.a;
        pos(LHS) = e.l;
        pos(RHS) = e.r;
        insert(pos);
    }
}

//forward declarations for forests
inline void insert_Cxx (Pos pos) { CLR::insert(pos); CRL::insert(pos); }
inline void remove_Cxx (Pos pos) { CLR::remove(pos); CRL::remove(pos); }

//event functions for the comp table
void remove_value (int _rem)
{
    LOG_DEBUG1( "comp table is removing " << _rem );

    Comp rem(_rem);
    Assert3(rem.isUsed(), "tried to remove unused comp value");
    remove_Cxx(rem);
    delete_(rem);
}
void merge_values (int _dep, int _rep)
{
    LOG_DEBUG1( "comp table is merging " << _dep << " --> " << _rep );

    Comp dep(_dep), rep(_rep);
    Assert3(dep.isUsed(), "tried to merge from unused comp value");
    Assert3(rep.isUsed(), "tried to merge to unused comp value");
    remove_Cxx(dep);
    CS::merge(dep, rep);
    delete_(dep);
}
void move_value (int _moved, int _lhs, int _rhs)
{
    LOG_DEBUG1( "comp table is moving " << _moved );

    Comp moved(_moved);
    remove_Cxx(moved);
    set_lhs(moved, Ob(_lhs));
    set_rhs(moved, Ob(_rhs));
    insert_Cxx(moved);
    CS::enforce(moved);
}

//eqn/ob-level interface
inline void _clear (Comp eqn)
{
    eqn.reset();
    eqn[IS_USED] = true;
}
Comp create (Ob comp, Ob lhs, Ob rhs)
{//create an comp
    if (Comp::full()) CS::grow_comps();
    Comp eqn = Comp::alloc();
    _clear(eqn);

    //define obs
    set_comp(eqn, comp);
    set_lhs(eqn, lhs);
    set_rhs(eqn, rhs);

    insert(eqn);
    return eqn;
}
void insert (Comp eqn)
{//insert a node into both forests and comp table
    LOG_DEBUG1( "inserting eqn" );

    g_comp_table->insert(Int(eqn(LHS)), Int(eqn(RHS)), Int(eqn));
    insert_Cxx(eqn);
}
void remove (Ob rem)
{//remove an ob
    LOG_DEBUG1( "C: removing Ob" );
    Logging::IndentBlock block;

    //remove occurrences as LHS & RHS
    g_comp_table->remove(Int(rem), remove_value);

    //remove occurrences as COMP
    std::vector<Comp> changed_eqns; //TODO maybe make this static
    for (Clr_Iterator iter(rem); iter.ok(); iter.next()) {
        Assert3(iter.comp() == rem, "eqn has wrong COMP before removing");
        changed_eqns.push_back(*iter);
    }
    //  traverse through: remove, delete
    for (int i=0, size=changed_eqns.size(); i!=size; ++i) {
        Comp eqn = changed_eqns[i];
        g_comp_table->remove(Int(eqn(LHS)), Int(eqn(RHS)));
        delete_(eqn);
    }
}
void merge (Ob dep, Ob rep)
{//merges two obs
    LOG_DEBUG1( "C: merging Obs " << dep << "-->" << rep );
    Logging::IndentBlock block;

    //merge occurrences as LHS & RHS
    g_comp_table->merge(Int(dep), Int(rep), merge_values, move_value);

    //merge occurrences as COMP
    std::vector<Comp> changed_eqns; //TODO maybe make this static
    for (Clr_Iterator iter(dep); iter.ok(); iter.next()) {
        Assert3(iter.comp() == dep, "eqn has wrong COMP before rerooting");
        changed_eqns.push_back(*iter);
    }
    //  traverse through: remove, reroot, insert
    for (int i=0, size=changed_eqns.size(); i!=size; ++i) {
        Comp eqn = changed_eqns[i];

        CLR::remove(eqn);
        CRL::remove(eqn);
        set_comp(eqn, rep);
        CLR::insert(eqn);
        CRL::insert(eqn);

        CS::enforce(eqn);
    }
}
void insert_all ()
{//reconstructs table and forests from CLR triples
    logger.debug() << "Building comp table and forests" |0;
    Logging::IndentBlock block;

    for (Ob::sparse_iterator iter=Ob::sbegin(); iter!=Ob::send(); ++iter) {
        Ob ob = *iter;
        ob(O::CLR_ROOT) = Comp(0);
        ob(O::CRL_ROOT) = Comp(0);
    }
    for (Comp::sparse_iterator iter=Comp::sbegin(); iter!=Comp::send(); ++iter) {
        insert(*iter);
    }
}

//comp table validation
void validate_comp_table (Int level)
{
    logger.debug() << "Validating Comp table" |0;
    Logging::IndentBlock block;

    Assert (g_comp_table->sup_capacity() == Ob::capacity(),
            "comp table has wrong size: " << g_comp_table->sup_capacity());
    if (level < 3) return;
    g_comp_table->validate();
}
void validate_agreement (Int level)
{
    logger.debug() << "Validating Comp forests-table agreement" |0;
    Logging::IndentBlock block;

    Assert (g_comp_table->count_items() == Comp::numUsed(),
            "invalid: g_comp_table has wrong number of eqns: "
            << g_comp_table->count_items() << ", should be " << Comp::numUsed());
    if (level < 3) return;

    for (Comp::sparse_iterator iter=Comp::sbegin(); iter!=Comp::send(); ++iter) {
        Comp eqn = *iter;
        Int lhs = Int(eqn(LHS)), rhs = Int(eqn(RHS));
        Assert (g_comp_table->contains(lhs,rhs),
                "comp table does not support pair " << lhs << "," << rhs);
        Assert (Int(g_comp_table->get_value(lhs,rhs)) == eqn,
                "comp table has wrong value at (" << lhs << "," << rhs
                << "): " << g_comp_table->get_value(lhs,rhs)
                << " should be " << eqn);
    }
}

//comp table iteration
template<int X> LineIterator<X>::LineIterator ()
    : Base(g_comp_table) {}
template<int X> LineIterator<X>::LineIterator (Ob fixed)
    : Base(g_comp_table, Int(fixed)) {}
RRxx_Iter::RRxx_Iter () : Base(g_comp_table) {}
LRxx_Iter::LRxx_Iter () : Base(g_comp_table) {}
LLxx_Iter::LLxx_Iter () : Base(g_comp_table) {}
RRxx_Iter::RRxx_Iter (Ob x, Ob y) : Base(g_comp_table) { begin(x, y); }
LRxx_Iter::LRxx_Iter (Ob f, Ob x) : Base(g_comp_table) { begin(f, x); }
LLxx_Iter::LLxx_Iter (Ob f, Ob g) : Base(g_comp_table) { begin(f, g); }

//validation & testing tools
template<int X> Ob  ob (Pos p) { return X ? get_rhs(p) : get_lhs(p); }
template<int X> void test_line_contains (Comp eqn)
{
    Ob start = ob<X>(eqn);
    LineIterator<X> iter(start);

    bool found = false;
    for (; iter.ok(); iter.next()) {
        if (*iter == eqn) { found = true; break; }
    }
    Assert (found,
            "invalid (1): eqn not contained in own line <" << X << ">");

    found = false;
    for (iter.begin(start); iter.ok(); iter.next()) {
        if (*iter == eqn) { found = true; break; }
    }
    Assert (found,
            "invalid (2): eqn not contained in own line <" << X << ">");
}
template<int X> void test_line_find ()
{
    for (Ob::sparse_iterator iter=Ob::sbegin(); iter!=Ob::send(); ++iter) {
        LineIterator<X> iter2;
        for (iter2.begin(*iter); iter2.ok(); iter2.next()) {
            Assert (*iter2 == find_comp_eqn(iter2.lhs(), iter2.rhs()),
                    "line iterator contains wrong value");
        }
    }
}

//validation & testing interface
void test_contains ()
{
    logger.info() << "Testing forest containment" |0;
    Logging::IndentBlock block;
    for (Comp::sparse_iterator iter=Comp::sbegin(); iter!=Comp::send(); ++iter) {
        Comp eqn = *iter;

        CLR::test_contains(eqn);
        CRL::test_contains(eqn);

        CLR::test_range_contains(eqn);
        CRL::test_range_contains(eqn);

        test_line_contains<CompTable::LHS_FIXED>(eqn);
        test_line_contains<CompTable::RHS_FIXED>(eqn);
    }
}
void validate_forests (Int level)
{
    if (level < 2) return;
    logger.debug() << "Validating Comp forests" |0;
    Logging::IndentBlock block;

    //validate obs are used
    for (Comp::sparse_iterator iter=Comp::sbegin(); iter!=Comp::send(); ++iter) {
        Comp eqn = *iter;
        Assert (get_comp(eqn), "invalid: eqn's comp is null");
        Assert (get_lhs(eqn), "invalid: eqn's lhs is null");
        Assert (get_rhs(eqn), "invalid: eqn's rhs is null");
        Assert (get_comp(eqn).isUsed(), "invalid: eqn's comp was freed");
        Assert (get_lhs(eqn).isUsed(), "invalid: eqn's lhs was freed");
        Assert (get_rhs(eqn).isUsed(), "invalid: eqn's rhs was freed");
    }

    //validate forests
    if (level >= 3) {
        CLR::validate_forest();
        CRL::validate_forest();
    }

    if (level >= 5) {
        test_iterators();
    }
}
void test_iterators ()
{
    logger.info() << "Testing forest iterators" |0;
    Logging::IndentBlock block;

    test_contains ();
    test_line_find<CompTable::LHS_FIXED>();
    test_line_find<CompTable::RHS_FIXED>();
}

//================ explicit template instantiation ================

namespace _private
{

template LineIterator<0>::LineIterator ();
template LineIterator<1>::LineIterator ();
template LineIterator<0>::LineIterator (Ob);
template LineIterator<1>::LineIterator (Ob);

}

} // namepace Compose
