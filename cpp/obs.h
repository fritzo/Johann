#ifndef JOHANN_OBS_H
#define JOHANN_OBS_H

#include "definitions.h"
#include "symbols.h"
#include "nodes.h"
#include "reorder.h"

namespace Obs
{

const Logging::Logger logger("obs", Logging::DEBUG);

using namespace Heap;
using namespace Symbols;

//================================ fields ================================
//(see node_field_usage.text)

//naming
const TypedIndex<ObName*>
    NAME(0x0);

//rep trees
const TypedIndex<Ob>
    REP(0x1);

//boolean properties
const TypedIndex<Int>
    BOOL_PROPERTIES(0x2);

//equivalence splay tree roots
//WARNING: should match Indexing::root in apply.h, compose.h, and join.h
const TypedIndex<App>
    ALR_ROOT(0x3), ARL_ROOT(0x4);
const TypedIndex<Comp>
    CLR_ROOT(0x5), CRL_ROOT(0x6);
const TypedIndex<Join>
    JLR_ROOT(0x7);

//================================ interface ================================

//boolean properties
inline BoolPropertyMask allProps (Ob ob)
{ return BoolPropertyMask(ob(BOOL_PROPERTIES)); }
inline void assumeProps  (Ob ob, Int props) { ob(BOOL_PROPERTIES) |= props; }
inline bool injects      (Ob ob) { return ob(BOOL_PROPERTIES) & INJECTIVE; }
inline bool linear       (Ob ob) { return ob(BOOL_PROPERTIES) & LINEAR; }
inline bool nonconst     (Ob ob) { return ob(BOOL_PROPERTIES) & NONCONST; }
inline bool deterministic(Ob ob) { return ob(BOOL_PROPERTIES) & DETERMIN; }
inline bool sequential   (Ob ob) { return ob(BOOL_PROPERTIES) & SEQUENTIAL; }
inline bool isInCore     (Ob ob) { return ob(BOOL_PROPERTIES) & IN_CORE; }
inline void addToCore    (Ob ob) { ob(BOOL_PROPERTIES) |= IN_CORE; }

//marking
inline bool marked (Ob ob) { return ob(BOOL_PROPERTIES) & MARKED; }
inline void mark   (Ob ob) { ob(BOOL_PROPERTIES) |= MARKED; }
inline void unmark (Ob ob) { ob(BOOL_PROPERTIES) &= ~MARKED; }
void mark_all ();
void unmark_all ();

//properties
inline bool isNamed      (Ob ob) { return ob(NAME) != NULL; }
inline bool isDepricated (Ob ob)
{//WARNING: REP fields must not be otherwise used
    return ob(REP) != ob;
}
inline bool isUnkn     (Ob ob) { return not ob(ALR_ROOT); }
inline bool isPrunable (Ob ob)
{ return not (isNamed(ob) or isInCore(ob) or marked(ob)); }

//creation/insertion
Ob _create ();
inline Ob create_atom (Int properties=0)
{//atom-specific creation
    Ob atom = _create();

    //init properties
    atom(BOOL_PROPERTIES) |= properties;

    return atom;
}
inline Ob create_app () { return _create(); }
inline Ob create_comp () { return _create(); }
inline Ob create_join () { return _create(); }
void insert_app (App eqn);
void insert_comp (Comp eqn);
void insert_join (Join eqn);

//removal, merging, renaming
void merge (Ob  dep, Ob  rep);
void delete_ (Ob ob);
void rename_obs (const Reorder::Reordering<Ob>& o_order);

//rep trees
inline Ob& rep (Ob ob) { return ob(REP); }
//union-find with initial branch inlined
Ob _getRep (Ob& ob);
inline Ob getRep (Ob ob) { Ob& rep = ob[REP];
                           return rep == ob ? ob : _getRep(rep); }

//saving/loading
Int data_size ();
void save_to_file (FILE* file);
void load_from_file (FILE* file);

//validation
void validate (Int level);

}

#endif
