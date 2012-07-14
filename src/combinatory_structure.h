#ifndef JOHANN_COMBINATORY_STRUCTURE_H
#define JOHANN_COMBINATORY_STRUCTURE_H

#include "definitions.h"
#include "symbols.h"
#include "nodes.h"
#include "order.h"
#include "reorder.h"
#include <vector>

namespace CombinatoryStructure
{

const Logging::Logger logger("struct", Logging::DEBUG);

using namespace Symbols;
namespace OR = Order;

typedef Reorder::Reordering<Ob>   ORing;
typedef Reorder::Reordering<App>  ARing;
typedef Reorder::Reordering<Comp> CRing;
typedef Reorder::Reordering<Join> JRing;

//======================== combinator tools ========================

//WARNING: most operations should be followed by process_queues().

//ob primitives
std::vector<ObHdl> get_basis ();
void contract_to_core ();
Ob   make_atom   (string name, Int properties=0);
App  make_app    (Ob lhs, Ob rhs);
Comp make_comp   (Ob lhs, Ob rhs);
Join make_join   (Ob lhs, Ob rhs);
App  make_app    (Ob app, Ob lhs, Ob rhs);
Comp make_comp   (Ob comp, Ob lhs, Ob rhs);
Join make_join   (Ob join, Ob lhs, Ob rhs);
Ob   make_unkn   (string & name, Int properties=0);
Ob   update_unkn (string & name, Ob ob);

//safe creation/pruning
void create_app (Ob lhs, Ob rhs);
void create_comp (Ob lhs, Ob rhs);
void create_join (Ob lhs, Ob rhs);
void prune_ob (Ob toPrune);

//creation
Ob find_atom (const string& name);
const string* find_atom_name (Ob ob);
bool name_atom (Ob atom, const string& name);
bool forget_atom (const string& name);
std::vector<ObHdl> get_atoms ();
std::vector<string> get_atom_names ();
//use AE::find_app
Ob get_app (Ob lhs, Ob rhs);
Ob get_comp (Ob lhs, Ob rhs);
Ob get_join (Ob lhs, Ob rhs);

//equational theory tools
inline bool areEquiv   (Ob x, Ob y) { return x == y; }
inline bool isLessThan (Ob x, Ob y) { return (x==y) or OR::contains_pos(x,y); }
inline bool isNLessThan(Ob x, Ob y) { return (x!=y) and OR::contains_neg(x,y); }
inline bool areDistinct(Ob x, Ob y) { return isNLessThan(x,y)
                                          or isNLessThan(y,x); }
Trool query_reln (Ob lhs, Relation reln, Ob rhs);
bool assume_equiv(Ob ob1, Ob ob2);
bool assume_app  (Ob app, Ob lhs, Ob rhs);
bool assume_comp (Ob cmp, Ob lhs, Ob rhs);
bool assume_join (Ob join, Ob lhs, Ob rhs);
bool assume_less (Ob smaller, Ob larger);
bool assume_nless(Ob smaller, Ob larger);
bool assume_reln (Ob lhs, Relation reln, Ob rhs);

//enforcement tools for external enforcement
bool saturation_pending();
void saturate ();
//WARNING: these do not call saturate
bool ensure_equiv (Ob ob1, Ob ob2);
bool ensure_less  (Ob smaller, Ob larger);
bool ensure_nless (Ob smaller, Ob larger);
bool ensure_app   (Ob app, Ob lhs, Ob rhs);
bool ensure_comp  (Ob cmp, Ob lhs, Ob rhs);
bool ensure_join  (Ob join, Ob lhs, Ob rhs);

//======================== structural operations ========================

//properties & flags
bool isInitialized ();
bool isCompacted ();

//coarse-level structural operations
void initialize  (
        size_t num_obs,
        size_t num_apps = 0,
        size_t num_comps = 0,
        size_t num_joins = 0,
        bool full = false);
void resize (size_t num_obs);
void clear ();
void compact (
        const ORing& o_order,
        const ARing& a_order,
        const CRing& c_order,
        const JRing& j_order);
void validate (size_t level);

//saving/loading atom table
size_t num_atoms ();
size_t data_size ();
void save_to (ostream & os);
void save_to_file (FILE * file);
void load_from_file (FILE * file, size_t num_atoms);

//wrappers for resizing
const float RESIZE_FACTOR = 1.1;
inline void grow_obs () { resize(size_t(Ob::capacity() * RESIZE_FACTOR + 1)); }
inline void shrink_obs () { resize(size_t(Ob::capacity() / RESIZE_FACTOR)); }
inline void grow_apps ()
{
    size_t new_size = size_t(App::capacity() * RESIZE_FACTOR + 1);
    App::resize(new_size);
}
inline void grow_comps ()
{
    size_t new_size = size_t(Comp::capacity() * RESIZE_FACTOR + 1);
    Comp::resize(new_size);
}
inline void grow_joins ()
{
    size_t new_size = size_t(Join::capacity() * RESIZE_FACTOR + 1);
    Join::resize(new_size);
}

//merging & moving
void merge   (Ob    dep, Ob    rep);
void merge   (App   dep, App   rep);
void merge   (Comp  dep, Comp  rep);
void merge   (Join  dep, Join  rep);
void enforce (App  eqn);
void enforce (Comp eqn);
void enforce (Join eqn);
void enforce_less (oid_t lhs, oid_t rhs);
void enforce_nless (oid_t lhs, oid_t rhs);
ObHdl enforce_atom (const string & name);

//diagnostics
void dump (string filename, size_t struct_type=0, bool verbose=false);
float log_stats (Long time=0); //returns ord table fill
void write_stats_to (ostream& os);
void write_params_to (ostream& os);
bool vis_eqn_table (size_t size = 0);
bool vis_ord_table (size_t size = 0);
bool stats_page (size_t size = 0);
void die_quietly ();

}

#endif

