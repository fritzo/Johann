
#include "join.h"
#include "obs.h"
#include "combinatory_structure.h"
#include <vector>
#include <utility>
#include <algorithm>

//log levels
#define LOG_DEBUG1(mess);
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << mess |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

namespace JoinEqn
{

namespace O = Obs;
namespace CS = CombinatoryStructure;

//the join table
JoinTable *g_join_table(NULL);

//top level
void init (Int num_obs, Int num_eqns, bool is_full)
{
    logger.debug() << "Initializing Joins" |0;
    Logging::IndentBlock block;

    Assert (g_join_table == NULL, "tried to init g_join_table twice");

    //init forests
    Join::init(num_eqns, is_full);

    //init join table
    g_join_table = new JoinTable(num_obs);
}
void clear ()
{
    logger.debug() << "Clearing Joins" |0;
    Logging::IndentBlock block;

    Assert (g_join_table != NULL, "tried to clear g_join_table twice");

    Join::clear();

    delete g_join_table; g_join_table = NULL;
}
void resize (Int num_obs)
{
    logger.debug() << "Resizing join table" |0;
    Logging::IndentBlock block;

    Assert (g_join_table != NULL, "tried to resize uninitialized g_join_table");

    //XXX: this uses lots of memory
    JoinTable *new_table = new JoinTable(num_obs);
    new_table->move_from(*g_join_table);
    delete g_join_table;
    g_join_table = new_table;
}
void insert_all ();
void resize (Int num_obs, Int num_eqns,
             const Reorder::Reordering<Ob>& o_order,
             const Reorder::Reordering<Join>& e_order)
{
    logger.debug() << "Resizing Joins" |0;
    Logging::IndentBlock block;

    Assert (g_join_table != NULL, "tried to resize uninitialized g_join_table");

    //free table
    delete g_join_table;

    //resize forests
    Join::resize(Join::numUsed(), e_order.new2old());
    for (Join::sparse_iterator iter=Join::sbegin(), end=Join::send();
            iter!=end; ++iter) {
        Join eqn = *iter;

        //move J,L,R
        Ob join = o_order.old2new(get_join(eqn));
        Ob lhs  = o_order.old2new(get_lhs (eqn));
        Ob rhs  = o_order.old2new(get_rhs (eqn));
        sort(lhs,rhs);
        set_join(eqn, join);
        set_lhs (eqn, lhs );
        set_rhs (eqn, rhs );
    }

    //rebuild table and forests
    g_join_table = new JoinTable(num_obs);
    insert_all();
}

//validation
void validate_forest (Int level);
void validate_join_table (Int level);
void validate_agreement (Int level);
void validate (Int level)
{
    validate_forest(level);
    validate_join_table(level);
    validate_agreement(level);
}

//minimal search tree interface
void insert (Join eqn);
inline void _clear(Join eqn);

//saving/loading
struct LittleJoin
{
    Short a,l,r;
    LittleJoin () {}
    LittleJoin (Join e) : a(e(JOIN)), l(e(LHS)), r(e(RHS)) {}
};
Int data_size () { return sizeof(LittleJoin) * Join::numUsed(); }
void save_to_file (FILE* file)
{
    logger.debug() << "Saving " << Pos::numUsed() << " Joins to file" |0;
    Logging::IndentBlock block;

    for (Pos::iterator i=Pos::begin(); i!=Pos::end(); ++i) {
        LittleJoin e = *i;
        safe_fwrite(&e, sizeof(LittleJoin), 1, file);
    }
}
void load_from_file (FILE* file)
{
    logger.debug() << "Loading " << Pos::numUsed() << " Joins from file" |0;
    Logging::IndentBlock block;

    for (Pos::iterator i=Pos::begin(); i!=Pos::end(); ++i) {
        LittleJoin e;
        safe_fread(&e, sizeof(LittleJoin), 1, file);

        Pos pos = *i;
        _clear(pos);
        pos(JOIN) = e.a;
        pos(LHS) = e.l;
        pos(RHS) = e.r;
        insert(pos);
    }
}

//event functions for the join table
void remove_value (int _rem)
{
    LOG_DEBUG1( "join table is removing " << _rem );

    Join rem(_rem);
    Assert3(rem.isUsed(), "tried to remove unised join value");
    JLR::remove(rem);
    delete_(rem);
}
void merge_values (int _dep, int _rep)
{
    LOG_DEBUG1( "join table is merging " << _dep << " --> " << _rep );

    Join dep(_dep), rep(_rep);
    Assert3(dep.isUsed(), "tried to merge from unused join value");
    Assert3(rep.isUsed(), "tried to merge to unused join value");
    JLR::remove(dep);
    CS::merge(dep, rep);
    delete_(dep);
}
void move_value (int _moved, int _lhs, int _rhs)
{
    LOG_DEBUG1( "join table is moving " << _moved );

    Join moved(_moved);
    JLR::remove(moved);
    set_lhs(moved, Ob(_lhs));
    set_rhs(moved, Ob(_rhs));
    JLR::insert(moved);
    CS::enforce(moved);
}

//eqn/ob-level interface
inline void _clear (Join eqn)
{
    eqn.reset();
    eqn[IS_USED] = true;
}
Join create (Ob join, Ob lhs, Ob rhs)
{//create an join
    sort(lhs,rhs);
    Assert3 (lhs <= rhs, "created out-of order join equation");
    if (Join::full()) CS::grow_joins();
    Join eqn = Join::alloc();
    _clear(eqn);

    //define obs
    set_join(eqn, join);
    set_lhs(eqn, lhs);
    set_rhs(eqn, rhs);

    insert(eqn);
    return eqn;
}
void insert (Join eqn)
{//insert a node into both forests and join table
    LOG_DEBUG1( "inserting eqn" );

    g_join_table->insert(Int(eqn(LHS)), Int(eqn(RHS)), Int(eqn));
    JLR::insert(eqn);
}
void remove (Ob rem)
{//remove an ob
    LOG_DEBUG1( "J: removing Ob" );
    Logging::IndentBlock block;

    //remove occurrences as LHS & RHS
    g_join_table->remove(Int(rem), remove_value);

    //remove occurrences as JOIN
    std::vector<Join> changed_eqns; //TODO maybe make this static
    for (Jlr_Iterator iter(rem); iter.ok(); iter.next()) {
        Assert3(iter.join() == rem, "eqn has wrong JOIN before removing");
        changed_eqns.push_back(*iter);
    }
    //  traverse through: remove, delete
    for (int i=0, size=changed_eqns.size(); i!=size; ++i) {
        Join eqn = changed_eqns[i];
        g_join_table->remove(Int(eqn(LHS)), Int(eqn(RHS)));
        delete_(eqn);
    }
}
void merge (Ob dep, Ob rep)
{//merges two obs
    LOG_DEBUG1( "J: merging Obs " << dep << "-->" << rep );
    Logging::IndentBlock block;

    //merge occurrences as LHS & RHS
    g_join_table->merge(Int(dep), Int(rep), merge_values, move_value);

    //merge occurrences as JOIN
    std::vector<Join> changed_eqns; //TODO maybe make this static
    for (Jlr_Iterator iter(dep); iter.ok(); iter.next()) {
        Assert3(iter.join() == dep, "eqn has wrong JOIN before rerooting");
        changed_eqns.push_back(*iter);
    }
    //  traverse through: remove, reroot, insert
    for (int i=0, size=changed_eqns.size(); i!=size; ++i) {
        Join eqn = changed_eqns[i];

        JLR::remove(eqn);
        set_join(eqn, rep);
        JLR::insert(eqn);

        CS::enforce(eqn);
    }
}
void insert_all ()
{//reconstructs table and forests from JLR triples
    logger.debug() << "Building join table and forests" |0;
    Logging::IndentBlock block;

    for (Ob::sparse_iterator iter=Ob::sbegin(); iter!=Ob::send(); ++iter) {
        Ob ob = *iter;
        ob(O::JLR_ROOT) = Join(0);
    }
    for (Join::sparse_iterator iter=Join::sbegin(); iter!=Join::send(); ++iter) {
        insert(*iter);
    }
}

//join table validation
void validate_join_table (Int level)
{
    logger.debug() << "Validating Join table" |0;
    Logging::IndentBlock block;

    Assert (g_join_table->sup_capacity() == Ob::capacity(),
            "join table has wrong size: " << g_join_table->sup_capacity());
    if (level < 3) return;
    g_join_table->validate();
}
unsigned numUsed ()
{
    unsigned result = 0;
    for (Join::sparse_iterator iter=Join::sbegin();
            iter!=Join::send(); ++iter) {
        ++result;
        if (get_lhs(*iter) != get_rhs(*iter)) ++result;
    }
    return result;
}
void validate_agreement (Int level)
{
    logger.debug() << "Validating Join forest-table agreement" |0;
    Logging::IndentBlock block;

    unsigned num_used = numUsed();
    Assert (g_join_table->count_pairs() == num_used,
            "invalid: g_join_table has wrong number of eqns: "
            << g_join_table->count_pairs() << ", should be " << num_used);
    if (level < 3) return;

    for (Join::sparse_iterator iter=Join::sbegin(); iter!=Join::send(); ++iter) {
        Join eqn = *iter;
        Int lhs = Int(eqn(LHS)), rhs = Int(eqn(RHS));
        Assert (g_join_table->contains(lhs,rhs),
                "join table does not support pair " << lhs << "," << rhs);
        Assert (Int(g_join_table->get_value(lhs,rhs)) == eqn,
                "join table has wrong value at (" << lhs << "," << rhs
                << "): " << g_join_table->get_value(lhs,rhs)
                << " should be " << eqn);
    }
}

//join table iteration
Lrj_Iterator::Lrj_Iterator () : Base(g_join_table) {}
Lrj_Iterator::Lrj_Iterator (Ob fixed) : Base(g_join_table, Int(fixed)) {}
LLrj_Iterator::LLrj_Iterator () : Base(g_join_table) {}
LLrj_Iterator::LLrj_Iterator (Ob f, Ob g) : Base(g_join_table) { begin(f, g); }

//validation & testing tools
void test_line_contains (Join eqn)
{
    Ob start = JLR::get_root(eqn);
    Lrj_Iterator iter(start);

    bool found = false;
    for (; iter.ok(); iter.next()) {
        if (*iter == eqn) { found = true; break; }
    }
    Assert (found, "invalid (1): eqn not contained in own line");

    found = false;
    for (iter.begin(start); iter.ok(); iter.next()) {
        if (*iter == eqn) { found = true; break; }
    }
    Assert (found, "invalid (2): eqn not contained in own line");
}
void test_line_find ()
{
    for (Ob::sparse_iterator iter=Ob::sbegin(); iter!=Ob::send(); ++iter) {
        Lrj_Iterator iter2;
        for (iter2.begin(*iter); iter2.ok(); iter2.next()) {
            Assert (*iter2 == find_join_eqn(iter2.fixed(), iter2.moving()),
                    "line iterator contains wrong value");
        }
    }
}

//validation & testing interface
void test_contains ()
{
    logger.info() << "Testing forest containment" |0;
    Logging::IndentBlock block;
    for (Join::sparse_iterator iter=Join::sbegin(); iter!=Join::send(); ++iter) {
        Join eqn = *iter;

        JLR::test_contains(eqn);
        JLR::test_range_contains(eqn);
        test_line_contains(eqn);
    }
}
void validate_forest (Int level)
{
    if (level < 2) return;
    logger.debug() << "Validating Join forest" |0;
    Logging::IndentBlock block;

    //validate obs are used
    for (Join::sparse_iterator iter=Join::sbegin(); iter!=Join::send(); ++iter) {
        Join eqn = *iter;
        Assert (get_join(eqn), "invalid: eqn's join is null");
        Assert (get_lhs(eqn), "invalid: eqn's lhs is null");
        Assert (get_rhs(eqn), "invalid: eqn's rhs is null");
        Assert (get_join(eqn).isUsed(), "invalid: eqn's join was freed");
        Assert (get_lhs(eqn).isUsed(), "invalid: eqn's lhs was freed");
        Assert (get_rhs(eqn).isUsed(), "invalid: eqn's rhs was freed");
    }

    //validate forests
    if (level >= 3) JLR::validate_forest();
    if (level >= 5) test_iterators();
}
void test_iterators ()
{
    logger.info() << "Testing forest iterators" |0;
    Logging::IndentBlock block;

    test_contains ();
    test_line_find();
}

}

