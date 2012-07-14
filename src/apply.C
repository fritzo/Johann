
#include "apply.h"
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

namespace Apply
{

namespace O = Obs;
namespace CS = CombinatoryStructure;
using namespace _private;

//the app table
AppTable *g_app_table(NULL);

//top level
void init (Int num_obs, Int num_eqns, bool is_full)
{
    logger.debug() << "Initializing Applys" |0;
    Logging::IndentBlock block;

    Assert (g_app_table == NULL, "tried to init g_app_table twice");

    //init forests
    App::init(num_eqns, is_full);

    //init app table
    g_app_table = new AppTable(num_obs);
}
void clear ()
{
    logger.debug() << "Clearing Applys" |0;
    Logging::IndentBlock block;

    Assert (g_app_table != NULL, "tried to clear g_app_table twice");

    App::clear();

    delete g_app_table; g_app_table = NULL;
}
void resize (Int num_obs)
{
    logger.debug() << "Resizing app table" |0;
    Logging::IndentBlock block;

    Assert (g_app_table != NULL, "tried to resize uninitialized g_app_table");

    //XXX: this uses lots of memory
    AppTable *new_table = new AppTable(num_obs);
    new_table->move_from(*g_app_table);
    delete g_app_table;
    g_app_table = new_table;
}
void insert_all ();
void resize (Int num_obs, Int num_eqns,
             const Reorder::Reordering<Ob>& o_order,
             const Reorder::Reordering<App>& e_order)
{
    logger.debug() << "Resizing Applys" |0;
    Logging::IndentBlock block;

    Assert (g_app_table != NULL, "tried to resize uninitialized g_app_table");

    //free table
    delete g_app_table;

    //resize forests
    App::resize(App::numUsed(), e_order.new2old());
    for (App::sparse_iterator iter=App::sbegin(), end=App::send();
            iter!=end; ++iter) {
        App eqn = *iter;

        //move A,L,R
        set_app(eqn, o_order.old2new(get_app(eqn)));
        set_lhs(eqn, o_order.old2new(get_lhs(eqn)));
        set_rhs(eqn, o_order.old2new(get_rhs(eqn)));
    }

    //rebuild table and forests
    g_app_table = new AppTable(num_obs);
    insert_all();
}

//validation
void validate_forests (Int level);
void validate_app_table (Int level);
void validate_agreement (Int level);
void validate (Int level)
{
    validate_forests(level);
    validate_app_table(level);
    validate_agreement(level);
}

//minimal search tree interface
void insert (App eqn);
inline void _clear(App eqn);

//saving/loading
struct LittleApp
{
    Short a,l,r;
    LittleApp () {}
    LittleApp (App e) : a(e(APP)), l(e(LHS)), r(e(RHS)) {}
};
Int data_size () { return sizeof(LittleApp) * App::numUsed(); }
void save_to_file (FILE* file)
{
    logger.debug() << "Saving " << Pos::numUsed() << " Apps to file" |0;
    Logging::IndentBlock block;

    for (Pos::iterator i=Pos::begin(); i!=Pos::end(); ++i) {
        LittleApp e = *i;
        safe_fwrite(&e, sizeof(LittleApp), 1, file);
    }
}
void load_from_file (FILE* file)
{
    logger.debug() << "Loading " << Pos::numUsed() << " Apps from file" |0;
    Logging::IndentBlock block;

    for (Pos::iterator i=Pos::begin(); i!=Pos::end(); ++i) {
        LittleApp e;
        safe_fread(&e, sizeof(LittleApp), 1, file);

        Pos pos = *i;
        _clear(pos);
        pos(APP) = e.a;
        pos(LHS) = e.l;
        pos(RHS) = e.r;
        insert(pos);
    }
}

//forward declarations for forests
inline void insert_Axx (Pos pos) { ALR::insert(pos); ARL::insert(pos); }
inline void remove_Axx (Pos pos) { ALR::remove(pos); ARL::remove(pos); }

//event functions for the app table
void remove_value (int _rem)
{
    LOG_DEBUG1( "app table is removing " << _rem );

    App rem(_rem);
    Assert3(rem.isUsed(), "tried to remove unused app value");
    remove_Axx(rem);
    delete_(rem);
}
void merge_values (int _dep, int _rep)
{
    LOG_DEBUG1( "app table is merging " << _dep << " --> " << _rep );

    App dep(_dep), rep(_rep);
    Assert3(dep.isUsed(), "tried to merge from unused app value");
    Assert3(rep.isUsed(), "tried to merge to unused app value");
    remove_Axx(dep);
    CS::merge(dep, rep);
    delete_(dep);
}
void move_value (int _moved, int _lhs, int _rhs)
{
    LOG_DEBUG1( "app table is moving " << _moved );

    App moved(_moved);
    remove_Axx(moved);
    set_lhs(moved, Ob(_lhs));
    set_rhs(moved, Ob(_rhs));
    insert_Axx(moved);
    CS::enforce(moved);
}

//eqn/ob-level interface
inline void _clear (App eqn)
{
    eqn.reset();
    eqn[IS_USED] = true;
}
App create (Ob app, Ob lhs, Ob rhs)
{//create an app
    if (App::full()) CS::grow_apps();
    App eqn = App::alloc();
    _clear(eqn);

    //define obs
    set_app(eqn, app);
    set_lhs(eqn, lhs);
    set_rhs(eqn, rhs);

    insert(eqn);
    return eqn;
}
void insert (App eqn)
{//insert a node into both forests and app table
    LOG_DEBUG1( "inserting eqn" );

    g_app_table->insert(Int(eqn(LHS)), Int(eqn(RHS)), Int(eqn));
    insert_Axx(eqn);
}
void remove (Ob rem)
{//remove an ob
    LOG_DEBUG1( "A: removing Ob" );
    Logging::IndentBlock block;

    //remove occurrences as LHS & RHS
    g_app_table->remove(Int(rem), remove_value);

    //remove occurrences as APP
    std::vector<App> changed_eqns; //TODO maybe make this static
    for (Alr_Iterator iter(rem); iter.ok(); iter.next()) {
        Assert3(iter.app() == rem, "eqn has wrong APP before removing");
        changed_eqns.push_back(*iter);
    }
    //  traverse through: remove, delete
    for (int i=0, size=changed_eqns.size(); i!=size; ++i) {
        App eqn = changed_eqns[i];
        g_app_table->remove(Int(eqn(LHS)), Int(eqn(RHS)));
        delete_(eqn);
    }
}
void merge (Ob dep, Ob rep)
{//merges two obs
    LOG_DEBUG1( "A: merging Obs " << dep << "-->" << rep );
    Logging::IndentBlock block;

    //merge occurrences as LHS & RHS
    g_app_table->merge(Int(dep), Int(rep), merge_values, move_value);

    //merge occurrences as APP
    std::vector<App> changed_eqns; //TODO maybe make this static
    for (Alr_Iterator iter(dep); iter.ok(); iter.next()) {
        Assert3(iter.app() == dep, "eqn has wrong APP before rerooting");
        changed_eqns.push_back(*iter);
    }
    //  traverse through: remove, reroot, insert
    for (int i=0, size=changed_eqns.size(); i!=size; ++i) {
        App eqn = changed_eqns[i];

        ALR::remove(eqn);
        ARL::remove(eqn);
        set_app(eqn, rep);
        ALR::insert(eqn);
        ARL::insert(eqn);

        CS::enforce(eqn);
    }
}
void insert_all ()
{//reconstructs table and forests from ALR triples
    logger.debug() << "Building app table and forests" |0;
    Logging::IndentBlock block;

    for (Ob::sparse_iterator iter=Ob::sbegin(); iter!=Ob::send(); ++iter) {
        Ob ob = *iter;
        ob(O::ALR_ROOT) = App(0);
        ob(O::ARL_ROOT) = App(0);
    }
    for (App::sparse_iterator iter=App::sbegin(); iter!=App::send(); ++iter) {
        insert(*iter);
    }
}

//app table validation
void validate_app_table (Int level)
{
    logger.debug() << "Validating App table" |0;
    Logging::IndentBlock block;

    Assert (g_app_table->sup_capacity() == Ob::capacity(),
            "app table has wrong size: " << g_app_table->sup_capacity());
    if (level < 3) return;
    g_app_table->validate();
}
void validate_agreement (Int level)
{
    logger.debug() << "Validating App forests-table agreement" |0;
    Logging::IndentBlock block;

    Assert (g_app_table->count_items() == App::numUsed(),
            "invalid: g_app_table has wrong number of eqns: "
            << g_app_table->count_items() << ", should be " << App::numUsed());
    if (level < 3) return;

    for (App::sparse_iterator iter=App::sbegin(); iter!=App::send(); ++iter) {
        App eqn = *iter;
        Int lhs = Int(eqn(LHS)), rhs = Int(eqn(RHS));
        Assert (g_app_table->contains(lhs,rhs),
                "app table does not support pair " << lhs << "," << rhs);
        Assert (Int(g_app_table->get_value(lhs,rhs)) == eqn,
                "app table has wrong value at (" << lhs << "," << rhs
                << "): " << g_app_table->get_value(lhs,rhs)
                << " should be " << eqn);
    }
}

// app table iteration
template<int X> LineIterator<X>::LineIterator ()
    : Base(g_app_table) {}
template<int X> LineIterator<X>::LineIterator (Ob fixed)
    : Base(g_app_table, int(fixed)) {}
RRxx_Iter::RRxx_Iter () : Base(g_app_table) {}
LRxx_Iter::LRxx_Iter () : Base(g_app_table) {}
LLxx_Iter::LLxx_Iter () : Base(g_app_table) {}
RRxx_Iter::RRxx_Iter (Ob x, Ob y) : Base(g_app_table) { begin(x, y); }
LRxx_Iter::LRxx_Iter (Ob f, Ob x) : Base(g_app_table) { begin(f, x); }
LLxx_Iter::LLxx_Iter (Ob f, Ob g) : Base(g_app_table) { begin(f, g); }

// validation & testing tools
template<int X> Ob  ob (Pos p) { return X ? get_rhs(p) : get_lhs(p); }
template<int X> void test_line_contains (App eqn)
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
            Assert (*iter2 == find_app_eqn(iter2.lhs(), iter2.rhs()),
                    "line iterator contains wrong value");
        }
    }
}

//validation & testing interface
void test_contains ()
{
    logger.info() << "Testing forest containment" |0;
    Logging::IndentBlock block;
    for (App::sparse_iterator iter=App::sbegin(); iter!=App::send(); ++iter) {
        App eqn = *iter;

        ALR::test_contains(eqn);
        ARL::test_contains(eqn);

        ALR::test_range_contains(eqn);
        ARL::test_range_contains(eqn);

        test_line_contains<AppTable::LHS_FIXED>(eqn);
        test_line_contains<AppTable::RHS_FIXED>(eqn);
    }
}
void validate_forests (Int level)
{
    if (level < 2) return;
    logger.debug() << "Validating App forests" |0;
    Logging::IndentBlock block;

    //validate obs are used
    for (App::sparse_iterator iter=App::sbegin(); iter!=App::send(); ++iter) {
        App eqn = *iter;
        Assert (get_app(eqn), "invalid: eqn's app is null");
        Assert (get_lhs(eqn), "invalid: eqn's lhs is null");
        Assert (get_rhs(eqn), "invalid: eqn's rhs is null");
        Assert (get_app(eqn).isUsed(), "invalid: eqn's app was freed");
        Assert (get_lhs(eqn).isUsed(), "invalid: eqn's lhs was freed");
        Assert (get_rhs(eqn).isUsed(), "invalid: eqn's rhs was freed");
    }

    //validate forests
    if (level >= 3) {
        ALR::validate_forest();
        ARL::validate_forest();
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
    test_line_find<AppTable::LHS_FIXED>();
    test_line_find<AppTable::RHS_FIXED>();
}

//================ explicit template instantiation ================

namespace _private
{

template LineIterator<0>::LineIterator ();
template LineIterator<1>::LineIterator ();
template LineIterator<0>::LineIterator (Ob);
template LineIterator<1>::LineIterator (Ob);

}

} // namespace Apply
