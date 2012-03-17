#ifndef JOHANN_MAIN_H
#define JOHANN_MAIN_H

#include "definitions.h"
#include "symbols.h"
#include "expressions.h"
#include "statements.h"
#include "context.h"
#include <vector>

namespace Kernel
{
const Logging::Logger logger("kernel", Logging::DEBUG);

using namespace Symbols;

//syntax coordination
//#define DEBUG_DEADLOCK
#ifdef DEBUG_DEADLOCK
#undef LOCK_SYNTAX
#undef UNLOCK_SYNTAX
#define LOCK_SYNTAX LOCK_SYNTAX_DEBUG
#define UNLOCK_SYNTAX LOCK_SYNTAX_DEBUG
#endif

//command processing
void set_thinking (bool whether);   //called by user
void set_paused (bool whether);     //called by parser
void no_more_cmds ();
void cancel_task ();

//top-level interface
void restart ();
void restart (std::vector<string>& basis);
bool save (string filename);
bool load (string filename);
void dump (string filename, Int type=0);
bool validate (Int level=3);
bool test     (Int level=3);

//expressions
ExprHdl expand (ExprHdl expr, int depth=Contexts::MAX_DEPTH);
StmtHdl expand (StmtHdl stmt, int depth=Contexts::MAX_DEPTH);
ExprHdl compress (ExprHdl expr, int depth=0);
ExprHdl simplify (ExprHdl expr, Float create=0.0f);
ExprHdl express (ExprHdl expr);

//talking to server
void send_lang ();
void send_eqns ();

//basis
bool with_atoms    (std::vector<string> names);
bool without_atoms (std::vector<string> names);
bool start_using   (std::vector<string> names);
bool name_expr (string name, ExprHdl meaning, string comment); //true on success

//db internals interface
void set_size        (Int s);
void set_granularity (Int s);
void set_time_scale  (Int s);
void set_density     (Float d);
void set_temperature (Float t);
void set_elegance    (Float e);
void think_user (Int num_cycles=1); //called by user;   acts in background
void think_auto (Int num_cycles=1); //called by script; acts in foreground
bool think_in (std::vector<ExprHdl> exprs); //true if successful
bool think_in (std::vector<std::pair<ExprHdl, Float> > exprs); // same
void set_P_app (Float P_app);
void set_P_comp (Float P_comp);
void set_P_join (Float P_join);
void think_about_everything ();
void think_about_exprs (std::vector<ExprHdl> exprs);
void think_about_context ();
void think_about_theory ();

//params
void write_lang_to (ostream& os);
void write_stats_to (ostream& os);
void save_basis_to (ostream& os);
void save_theory_to (ostream& os);
void save_params_to (ostream& os);
void write_params_to (ostream& os);
void plot_funs_of_eps (Int num_points=32);
enum StatType { STAT_WIN=1, VIS_EQNS=2, VIS_ORDS=3, VIS_MASS=4, MAP_3D=5 };
bool save_stats (StatType type); //returns true on error

//measure interface
void set_guessing (bool whether);
Float get_symbols (ExprHdl expr);
Float get_relevance (ExprHdl expr);
void print_simplest_to (ostream& os, Int N=0);
void print_most_relevant_to (ostream& os, Int N=0);
void print_sketchiest_to (ostream& os, Int N=0);
void print_conjectures_to (ostream& os, Int N=64);

//lambda theory
unsigned get_default_effort ();
Trool query_stmt (StmtHdl stmt);
Trool check_stmt (StmtHdl stmt, string comment, Int effort);
bool assume_stmt (StmtHdl stmt, string comment);
Prob guess_stmt (StmtHdl stmt);
void solve_and_print (ostream& os, StmtHdl& stmt, bool tryhard=false);
bool mark   (ExprHdl expr);
bool unmark (ExprHdl expr);
void recheck ();
void review ();
void problems (ostream& os, Trool truth=Symbols::UNKNOWN);
void clear_theory ();
void mark_all   ();
void unmark_all ();

//context
Context& context ();
void recompile_context (Int tryhard=0); //called after any mods
void update (Int tryhard=0);

//language
bool retract (ExprHdl expr);
bool extend (ExprHdl expr);
void set_rule_weight (Symbols::PropRule name, Float weight=1.0f);

//napping and resting
void set_resting (bool whether);
void nap ();
void rest ();

}

#endif
