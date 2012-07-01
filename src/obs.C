
#include "obs.h"
#include "apply.h"
#include "compose.h"
#include "join.h"
#include "combinatory_structure.h"
#include <set> //for set
#include <map> //for multimap
#include <vector>
#include <utility> //for pair
#include <sstream>

//log levels
#define LOG_DEBUG1(mess)
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << message |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

namespace Obs
{

namespace CS = CombinatoryStructure;

typedef Ob Pos;

//properties
void mark_all ()
{
    logger.info() << "marking all obs" |0;
    for (Ob::sparse_iterator i=Ob::sbegin(), end=Ob::send(); i!=end; ++i) {
        mark(*i);
    }
}
void unmark_all ()
{
    logger.info() << "unmarking all obs" |0;
    for (Ob::sparse_iterator i=Ob::sbegin(), end=Ob::send(); i!=end; ++i) {
        unmark(*i);
    }
}

//interface
void merge (Ob dep, Ob rep)
{
    LOG_DEBUG1( "O: merging Obs" )
    Assert3(not isDepricated(rep),
            "tried to merge dep Ob in to depricated rep");

    //name
    if (dep(NAME)) {
        if (rep(NAME))  ObHdl::merge(dep(NAME), rep(NAME));
        else            dep(NAME)->move_to(rep);
        Assert3(dep(NAME) == NULL,
                "in O::merge, dep name was not forgotten");
    }

    //properties
    rep(BOOL_PROPERTIES) |= dep(BOOL_PROPERTIES);
}
void rename_obs (const Reorder::Reordering<Ob>& o_order)
{//moves names and reps
    for (Ob::sparse_iterator iter=Ob::sbegin(), end=Ob::send();
            iter!=end; ++iter) {
        Ob old_ob = *iter;

        //move name
        if (isNamed(old_ob)) {
            Ob new_ob = o_order.old2new(old_ob);
            old_ob(NAME)->move_to_tandem(new_ob);
        }

        //move rep
        Ob old_rep = old_ob(REP);
        Ob new_rep = o_order.old2new(old_rep);
        old_ob(REP) = new_rep;
    }
}

//creation/deletion
inline void _clear (Ob ob)
{//clears all fields
    ob.reset();
    ob[REP] = ob;
    ob[BOOL_PROPERTIES] = IS_USED;
}
Ob _create ()
{//generic creation
    if (Ob::full()) CS::grow_obs();
    Ob ob = Ob::alloc();
    _clear (ob);
    return ob;
}
void delete_ (Ob ob)
{
    Assert (not ob(NAME), "tried to delete_ a named ob");
    //Assert (isPrunable(ob), "tried to delete_ an unprunable Ob");

    Ob::free(ob);
}

//insertion & removal
void insert_app (App eqn)
{//inserts an app in rep and parse trees
    LOG_DEBUG1( "O: inserting app Ob" )
    Ob app = get_app(eqn); Assert3(!isDepricated(app), "insert_app: bad app");
    Ob lhs = get_lhs(eqn); Assert3(!isDepricated(lhs), "insert_app: bad lhs");
    Ob rhs = get_rhs(eqn); Assert3(!isDepricated(rhs), "insert_app: bad rhs");

    //update bool properties
    app(BOOL_PROPERTIES) |= STRUCTURAL & lhs(BOOL_PROPERTIES)
                                       & rhs(BOOL_PROPERTIES);
}
void insert_comp (Comp eqn)
{//inserts an comp in rep and parse trees
    LOG_DEBUG1( "O: inserting comp Ob" )
    Ob cmp = get_comp(eqn);Assert3(!isDepricated(cmp), "insert_comp: bad comp");
    Ob lhs = get_lhs(eqn); Assert3(!isDepricated(lhs), "insert_comp: bad lhs");
    Ob rhs = get_rhs(eqn); Assert3(!isDepricated(rhs), "insert_comp: bad rhs");

    //update bool properties
    cmp(BOOL_PROPERTIES) |= STRUCTURAL & lhs(BOOL_PROPERTIES)
                                       & rhs(BOOL_PROPERTIES);
}
void insert_join (Join eqn)
{//inserts an join in rep and parse trees
    LOG_DEBUG1( "O: inserting join Ob" )
    Ob j = get_join(eqn);   Assert3(!isDepricated(j), "insert_join: bad join");
    Ob l = get_lhs(eqn);    Assert3(!isDepricated(l), "insert_join: bad lhs");
    Ob r = get_rhs(eqn);    Assert3(!isDepricated(r), "insert_join: bad rhs");

    //update bool properties
    j(BOOL_PROPERTIES) |= STRUCTURAL & l(BOOL_PROPERTIES)
                                     & r(BOOL_PROPERTIES);
}

//rep trees
//union-find with initial branch inlined
Ob _getRep (Ob& ob) { return ob = getRep(ob[REP]); }
/*OLD
Ob getRep (Ob ob)
{//union-find: collapse all reps
    Ob& rep = ob[REP]; //use [] to allow traversal through unused nodes
    return rep == ob ? ob : (rep = getRep(rep));
}
*/
/* OLDER
Ob getRep (Ob ob)
{//union-find: traverses the rep chain to find rep at end
    //use [] to allow traversal through unused nodes
    Ob rep = ob[REP]; //even ob may be unused now
    while (rep[REP] != rep) rep = rep[REP];
    Assert3(rep, "end of rep chain is 0");
    Assert3(rep.isUsed(), "end of rep chain is not used");
    ob[REP] = rep;
    return rep;
}
*/
void validate_reps ()
{//fails if any depricated obs remain used
    logger.debug() << "Validating reps" |0;
    Logging::IndentBlock block;

    for (Ob::sparse_iterator iter=Ob::sbegin(); iter!=Ob::send(); ++iter) {
        Ob ob = *iter;
        Assert (not isDepricated(ob), "invalid: depricated ob found");
    }
}

//saving/loading
Int data_size ()
{
    const Int NUM_FIELDS_USED = 1; //BOOL_PROPERTIES
    const Int NODE_SIZE = sizeof(Int) * NUM_FIELDS_USED;
    return NODE_SIZE * Ob::size();
}
void save_to_file (FILE* file)
{
    logger.debug() << "Saving " << Pos::numUsed() << " Obs to file" |0;
    Logging::IndentBlock block;

    const size_t FIELD_SIZE = sizeof(Int);
    for (Pos::iterator i=Pos::begin(); i!=Pos::end(); ++i) {
        Pos pos = *i;

        safe_fwrite(&(pos(BOOL_PROPERTIES)), FIELD_SIZE, 1, file);
    }
}
void load_from_file (FILE* file)
{
    logger.debug() << "Loading " << Pos::numUsed() << " Obs from file" |0;
    Logging::IndentBlock block;

    const size_t FIELD_SIZE = sizeof(Int);

    //read data
    for (Pos::iterator i=Pos::begin(); i!=Pos::end(); ++i) {
        Pos pos = *i;
        _clear(pos);
        safe_fread(&(pos(BOOL_PROPERTIES)), FIELD_SIZE, 1, file);
    }
}

//validation
void validate (Int level)
{
    if (level >= 1) validate_reps();
}

}


