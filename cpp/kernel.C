
#include "kernel.h"
#include "version.h"
#include "nodes.h"
#include "obs.h"
#include "apply.h"
#include "compose.h"
#include "join.h"
#include "substitute.h"
#include "order.h"
#include "brain.h"
#include "lambda_theories.h"
#include "syntax_semantics.h"
#include "combinatory_structure.h"
#include "socket_tools.h"
#include "thread_tools.h"
#include <sstream>
#include <iomanip>
#include <cstdlib> //for atoi
#include <deque>
#include <pthread.h>

//from console.y
extern void* start_parser (void* init_script);
extern void set_quietly (bool);

namespace Expressions
{

namespace CS = CombinatoryStructure;
namespace K  = Kernel;

//calculating moments
struct Moments { Float symbols, relevance; };
inline Moments get_moments (Ob ob)
{
    Moments result;
    result.symbols   = TheBrain::brain().get_symbols(ob);
    result.relevance = TheBrain::brain().get_relevance(ob);
    return result;
}
class FindMoments : public StructlFun
{
    Moments& m_moments;
    Int& m_valid;

    FindMoments (Moments& moments, Int& defined)
        : m_moments(moments), m_valid(defined)
    {}
    virtual ~FindMoments () {}
public:
    static void call (Moments& temp, Int& defined, Expr& arg)
    {
        defined = true;
        FindMoments fun(temp, defined);
        fun(arg);
    }

    virtual void of_atom (const string& atom)
    {
        Ob atom_ob = CS::find_atom(atom);
        if (atom_ob) {
            m_moments = get_moments(atom_ob);
            return;
        }
        m_moments.symbols = INFINITY;
        m_moments.relevance = 0.0f;
    }
    virtual void of_app (Expr& lhs, Expr& rhs)
    {
        Moments lhs_mmts;
        call(lhs_mmts, m_valid, lhs);
        if (not m_valid) return;

        Moments rhs_mmts;
        call(rhs_mmts, m_valid, rhs);
        if (not m_valid) return;

        m_moments.symbols = lhs_mmts.symbols + rhs_mmts.symbols;
        m_moments.relevance = 0.0f;
    }
    virtual void of_comp (Expr& lhs, Expr& rhs)
    {
        Moments lhs_mmts;
        call(lhs_mmts, m_valid, lhs);
        if (not m_valid) return;

        Moments rhs_mmts;
        call(rhs_mmts, m_valid, rhs);
        if (not m_valid) return;

        m_moments.symbols = lhs_mmts.symbols + rhs_mmts.symbols;
        m_moments.relevance = 0.0f;
    }
    virtual void of_join (Expr& lhs, Expr& rhs)
    {
        Moments lhs_mmts;
        call(lhs_mmts, m_valid, lhs);
        if (not m_valid) return;

        Moments rhs_mmts;
        call(rhs_mmts, m_valid, rhs);
        if (not m_valid) return;

        m_moments.symbols = lhs_mmts.symbols + rhs_mmts.symbols;
        m_moments.relevance = 0.0f;
    }
    virtual void of_special (SpecialType type) { m_valid = false; }
};
inline Moments get_moments (ExprHdl expr)
{
    Moments mmts;
    Int valid;
    FindMoments::call(mmts, valid, *expr);
    if (not valid) {
        mmts.symbols = NAN;
        mmts.relevance = NAN;
    }
    return mmts;
}
}
namespace Kernel
{

namespace O = Obs;
namespace AE= Apply;
namespace CE= Apply;
namespace OR = Order;
namespace CS = CombinatoryStructure;
namespace LT = LambdaTheories;
typedef   LT::Atoms A;
namespace EX = Expressions;
namespace S = Substitute;
namespace TB = TheBrain;
namespace ST = SocketTools;
namespace TT = ThreadTools;

using TB::brain;
using Symbols::TRUE;
using Symbols::UNKNOWN;
using Symbols::FALSE;

//which theory to use?
//typedef LT::ExtnTheory Theory;
typedef LT::TypedTheory Theory;

//the global pointers
Theory *g_theory(NULL);
TB::Brain *g_system(NULL);
bool construct_system ()
{//returns whether system was created
    if (g_system == NULL) {
        g_theory = new Theory();
        g_system = new TB::Brain(g_theory);
        return true;
    }
    return false;
}

//printing tools
inline ExprHdl parse_ob (Ob ob)
{
    ExprHdl result = EX::parse_ob(ob);
    return result ? context().compress(result) : result;
}

//========================= Command Processing ============================
// Overview
//   run_command(cmd, arg)  - enqueues command and returns
//   run_function(cmd, arg) - enqueues command and waits until completion
//   run_syn_cmd, run_syn_fun - same, but wait until syntax is lockable
//
// WARNING: functions _explicitly_ using syntax are locked by the caller, and
//   should be wrapped with run_function; run_syn_fun is reserved for functions
//   using syntax _implicitly_ (not in arguments or return type).
//
//   TODO add priority, so that automatic rechecking doesn't lock out user

//abstract commands
typedef std::pair<long,long> Priority;
class Command
{
    //priority queuing
    const Priority m_priority;
    static long s_num_commands;
    struct prior {
        bool operator() (Command const* c, Command const* d) {
            return c->m_priority > d->m_priority;
        }
    };
    static std::set<Command*, prior> s_queue;
public:
    Command* pop () { TODO(); }

    //completion testing
    pthread_mutex_t m_completed;
    void _lock   () { pthread_mutex_lock(   &m_completed ); }
    void _unlock () { pthread_mutex_unlock( &m_completed ); }
protected:
    void done () { _unlock(); }
public:
    void wait () { _lock(); _unlock(); }

    //no copying allowed
private:
    Command (const Command&) {Assert(0, "tried to copy constuct a Command"); }
    Command& operator= (const Command&) {Assert(0, "tried to copy a Command"); }

    //classes are factories, and commands are responsible for their destruction
protected:
    Command (long priority=0) : m_priority(priority, -(s_num_commands++))
    {
        pthread_mutex_init( &m_completed, NULL);
        _lock();
        //s_queue.push(this);
    }
    virtual ~Command () { pthread_mutex_destroy( &m_completed ); }

    //interface for the command processor
public:
    virtual void run () = 0;
};
long Command::s_num_commands = 0;
std::set<Command*, Command::prior> Command::s_queue;
void push_cmd (Command* cmd);
bool g_processing_cmds = false;

//output commands
class OutputCommand : public Command
{
protected:
    ostream& m_os;
public:
    OutputCommand (ostream& os) : m_os(os) {}
    virtual ~OutputCommand () {}
};

//external commands
template<class T>
class ExternCommand : public Command
{
    void (*m_cmd)(T);
    T m_arg;
    ExternCommand (long priority, void cmd(T), T arg)
        : Command(priority), m_cmd(cmd), m_arg(arg) {}
    virtual ~ExternCommand () {}
public:
    virtual void run () { m_cmd(m_arg); delete this; }
    static void call (void cmd(T), T arg, long priority=0)
    { push_cmd(new ExternCommand<T>(priority, cmd, arg)); }
};
template<class T>
void run_command (void cmd(T), T arg, long priority=0)
{ ExternCommand<T>::call(cmd, arg, priority); }

//external commands using syntax
template<class T>
class SyntaxCommand : public Command
{
    void (*m_cmd)(T);
    T m_arg;
    SyntaxCommand (long priority, void cmd(T), T arg)
        : Command(priority), m_cmd(cmd), m_arg(arg) {}
    virtual ~SyntaxCommand () {}
public:
    virtual void run ()
    {
        LOCK_SYNTAX
        m_cmd(m_arg);
        UNLOCK_SYNTAX
        delete this;
    }
    static void call (void cmd(T), T arg, long priority=0)
    { push_cmd(new SyntaxCommand<T>(priority, cmd, arg)); }
};
template<class T>
void run_syn_cmd (void cmd(T), T arg, long priority=0)
{ SyntaxCommand<T>::call(cmd, arg, priority); }

//external functions
template<class Tin, class Tout>
class ExternFunction : public Command
{
    Tout (*m_cmd)(Tin);
    Tin m_arg;
    Tout m_result;
    ExternFunction (long priority, Tout cmd(Tin), Tin arg)
        : Command(priority), m_cmd(cmd), m_arg(arg) {}
public:
    virtual ~ExternFunction () {}
    virtual void run () { m_result = m_cmd(m_arg); done(); }
    Tout result () { wait(); return m_result; }
    static Tout call (Tout fun(Tin), Tin arg, long priority=0)
    {
        ExternFunction<Tin, Tout> cmd(priority,fun, arg);
        push_cmd(&cmd);
        return cmd.result();
    }
};
template<class Tin, class Tout>
Tout run_function (Tout cmd(Tin), Tin arg)
{
    return ExternFunction<Tin, Tout>::call(cmd, arg);
}

//external functions implicitly using syntax
template<class Tin, class Tout>
class SyntaxFunction : public Command
{
    Tout (*m_cmd)(Tin);
    Tin m_arg;
    Tout m_result;
    SyntaxFunction (long priority, Tout cmd(Tin), Tin arg)
        : Command(priority), m_cmd(cmd), m_arg(arg) {}
public:
    virtual ~SyntaxFunction () {}
    virtual void run ()
    {
        LOCK_SYNTAX
        m_result = m_cmd(m_arg);
        UNLOCK_SYNTAX
        done();
    }
    Tout result () { wait(); return m_result; }
    static Tout call (Tout fun(Tin), Tin arg, long priority=0)
    {
        SyntaxFunction<Tin, Tout> cmd(priority, fun, arg);
        push_cmd(&cmd);
        return cmd.result();
    }
};
template<class Tin, class Tout>
Tout run_syn_fun (Tout cmd(Tin), Tin arg, long priority=0)
{
    return SyntaxFunction<Tin, Tout>::call(cmd, arg, priority);
}


//=============================== Commands ================================

//expressions: Main converts between pretty and combinatory expressions
ExprHdl expand (ExprHdl expr, int depth)
{
    expr = expr->as_comb();
    expr = depth > 0 ? context().expand  (expr,  depth)
                     : context().compress(expr, -depth);
    expr = expr->as_comb(); //in case expanded term is undefined
    return expr;
}
struct Expander : public EX::ExprFun
{
    virtual ~Expander () {}
    virtual ExprHdl operator() (const ExprHdl& expr) const
    {
        ExprHdl result = expr;
        if (result) result = result->as_comb();
        if (result) result = expand(result);
        return result ? result : EX::build_erron();
    }
};
const Expander expander;
StmtHdl expand (StmtHdl stmt, int depth)
{
    if (stmt) stmt = stmt->query_nf();
    if (stmt) stmt = stmt->map(expander);
    return stmt;
}
ExprHdl compress (ExprHdl expr, int depth)
{
    expr = expr->as_comb();
    return depth < 0 ? context().expand(  expr, -depth)
                     : context().compress(expr,  depth);
}
int _simplify (std::pair<ExprHdl*,Float> args)
{
    *args.first = EX::simplify(*(args.first), args.second);
    return 0;
}
ExprHdl simplify (ExprHdl expr, Float create)
{
    expr = expand(expr);
    UNLOCK_SYNTAX
    run_syn_fun(_simplify, std::make_pair(&expr, create));
    LOCK_SYNTAX
    return expr;
}
int _express (ExprHdl *expr)
{
    *expr = EX::simplify(*expr, INFINITY, true);
    return 0;
}
ExprHdl express (ExprHdl expr)
{
    expr = expand(expr);
    UNLOCK_SYNTAX
    run_syn_fun(_express, &expr);
    LOCK_SYNTAX
    return expr;
}

//talking to server
void _send_lang (int) { brain().send_lang_to("/tmp/johann_socket"); }
void _send_eqns (int) { brain().send_eqns_to("/tmp/johann_socket"); }
void send_lang () { run_command(_send_lang,0); }
void send_eqns () { run_command(_send_eqns,0); }

//basis
bool _start_using (std::vector<string> names)
{//returns true if atom was created
    std::vector<ObHdl> new_atoms;
    for (Int i=0; i<names.size(); ++i) {
        string& name = names[i];
        if (CS::find_atom(name)) continue;
        logger.info() << "starting to use atom " << name |0;
        g_theory->define_atom(name);
        EX::define_atom(name);
        ObHdl atom = CS::enforce_atom(name);
        Assert (atom, "atom not found after having just been made");
        new_atoms.push_back(atom);
    }
    if (new_atoms.empty()) return false;

    brain().extend(new_atoms); //give the new atoms some mass
    return true;
}
bool start_using (std::vector<string> names)
{
    return run_syn_fun(_start_using, names);
}
bool _name_expr (std::pair< std::pair<string,ExprHdl*>, string*> args)
{//returns true on success
    string name = args.first.first;
    ExprHdl meaning = expand(*(args.first.second));
    string& comment = *(args.second);

    if (meaning->isBad() or not meaning->isPure()) {
        logger.warning() << "unclear meaning " << args.second |0;
        return false;
    }

    //add to core and simplify
    ExprHdl simplified = EX::simplify(meaning);
    ObHdl compiled = EX::get_expr(simplified, true); //adds to core
    if (!compiled) {
        logger.warning() << "cannot compile expression for naming" |0;
        return false;
    }

    //check for existing definition
    Ob old_ob = CS::find_atom(name);
    if (old_ob) {
        ExprHdl old_expr(EX::build_atom(name));
        StmtHdl stmt = Statements::build_equation(old_expr, simplified);
        return g_theory->assume(stmt, comment);
    }

    if (not CS::name_atom (*compiled, name)) return false;

    g_theory->define_atom(name);
    EX::define_atom(name);
    CS::enforce_atom(name);

    return true;
}
bool name_expr (string name, ExprHdl meaning, string comment)
{
    UNLOCK_SYNTAX
    bool result = run_syn_fun(
            _name_expr,
            std::make_pair( std::make_pair(name,&meaning), &comment)
    );
    LOCK_SYNTAX
    meaning.clear();
    return result;
}
bool _w_atoms (std::vector<string> names)
{
    for (Int i=0; i<names.size(); ++i) {
        if (not CS::find_atom(names[i])) return false;
    }
    return true;
}
bool _wo_atoms (std::vector<string> names)
{
    for (Int i=0; i<names.size(); ++i) {
        if (CS::find_atom(names[i])) return false;
    }
    return true;
}
bool with_atoms (std::vector<string> names)
{
    return run_function(_w_atoms, names);
}
bool without_atoms (std::vector<string> names)
{
    return run_function(_wo_atoms,names);
}

//shaping & thinking interface
void _set_size        (Int s) { brain().set_size(s); }
void set_size         (Int s) { run_command(_set_size, s); }
void _set_granularity (Int s) { brain().set_granularity(s); }
void set_granularity  (Int s) { run_command(_set_granularity, s); }
void _set_time_scale  (Int s) { brain().set_time_scale(s); }
void set_time_scale   (Int s) { run_command(_set_time_scale, s); }
void _set_density    (Float t){ brain().set_density(t); }
void set_density     (Float t){ run_command(_set_density, t); }
void _set_temperature(Float t){ brain().set_temperature(t); }
void set_temperature (Float t){ run_command(_set_temperature, t); }
void _set_elegance   (Float e){ brain().set_elegance(e); }
void set_elegance    (Float e){ run_command(_set_elegance, e); }
void _think     (Int num_cycles) { brain().think(num_cycles); }
void think_auto (Int num_cycles) { run_command(_think, num_cycles); }
bool _think_in_set (std::vector<ExprHdl>* es)
{
    std::vector<ExprHdl>& exprs = *es;
    std::vector<ObHdl> obs;
    for (Int i=0; i<exprs.size(); ++i) {
        ObHdl ob = EX::get_expr(expand(exprs[i]));
        if (ob) {
            obs.push_back(ob);
        } else {
            logger.warning() << "I can't think in terms of " << exprs[i] |0;
        }
    }
    brain().think_in(obs);
    return exprs.empty() or obs.size() == exprs.size();
}
bool think_in (std::vector<ExprHdl> exprs)
{
    UNLOCK_SYNTAX
    bool result = run_syn_fun(_think_in_set, &exprs);
    LOCK_SYNTAX
    exprs.clear();
    return result;
}
bool _think_in_pmf (std::vector<std::pair<ExprHdl,Float> >* es)
{
    std::vector<std::pair<ExprHdl,Float> >& exprs = *es;
    std::vector<std::pair<ObHdl, Float> > obs;
    for (Int i=0; i<exprs.size(); ++i) {
        ExprHdl expr = expand(exprs[i].first);
        ObHdl ob = EX::get_expr(expr);
        if (ob) {
            Float weight = exprs[i].second;
            obs.push_back(std::make_pair(ob, exprs[i].second));
            logger.debug() << "adding concept " << expr << " @ " << weight |0;
        } else {
            logger.warning() << "I can't think in terms of " << expr |0;
        }
    }
    brain().think_in(obs);
    return obs.size() == exprs.size();
}
bool think_in (std::vector<std::pair<ExprHdl,Float> > exprs)
{
    UNLOCK_SYNTAX
    bool result = run_syn_fun(_think_in_pmf, &exprs);
    LOCK_SYNTAX
    exprs.clear();
    return result;
}
void _set_P_app (Float P_app) { brain().set_P_app(P_app); }
void set_P_app (Float P_app) { run_command(_set_P_app, P_app); }
void _set_P_comp (Float P_comp) { brain().set_P_comp(P_comp); }
void set_P_comp (Float P_comp) { run_command(_set_P_comp, P_comp); }
void _set_P_join (Float P_join) { brain().set_P_join(P_join); }
void set_P_join (Float P_join) { run_command(_set_P_join, P_join); }
void _think_about_exprs (std::vector<ExprHdl> exprs)
{
    std::vector<ExprHdl> combs;
    for (Int i=0; i<exprs.size(); ++i) {
        combs.push_back(expand(exprs[i]));
    }
    brain().think_about_exprs(exprs);
}
void think_about_exprs (std::vector<ExprHdl> exprs)
{
    run_command(_think_about_exprs, exprs);
}
void _think_about_everything (int) { brain().think_about_everything(); }
void think_about_everything ()
{
    run_command(_think_about_everything, 0);
}

//measure interface
void _set_guessing (bool whether) { brain().set_guessing(whether); }
void set_guessing (bool whether) { run_command(_set_guessing, whether); }
Float _get_symbols (ExprHdl* e) { return EX::get_moments(expand(*e)).symbols; }
Float _get_rel (ExprHdl *e) { return EX::get_moments(*e).relevance; }
Float get_symbols (ExprHdl e)
{
    UNLOCK_SYNTAX
    Float result = run_syn_fun(_get_symbols, &e);
    LOCK_SYNTAX
    e.clear();
    return result;
}
Float get_relevance (ExprHdl e)
{
    UNLOCK_SYNTAX
    Float result = run_syn_fun(_get_rel, &e);
    LOCK_SYNTAX
    e.clear();
    return result;
}

class PrintHead : public OutputCommand
{
    Int m_size;
    void (*m_fun)(ostream&, Int);
    PrintHead (ostream& os, Int n, void fun(ostream&, Int))
        : OutputCommand(os), m_size(n), m_fun(fun) {}
public:
    virtual ~PrintHead () {}
    virtual void run ()
    {
        LOCK_SYNTAX
        m_fun(m_os, m_size);
        UNLOCK_SYNTAX
        done();
    }
    static void call (ostream& os, void fun(ostream&, Int), Int n=0)
    {
        PrintHead cmd(os, n, fun);
        push_cmd(&cmd);
        cmd.wait();
    }
};
void _print_simplest_to (ostream& os, Int N)
{
    if (N == 0) N = Ob::numUsed();
    std::vector<std::pair<Ob,Float> > obs(brain().get_simplest(N));
    N = obs.size();
    os << "list of " << N << " simplest of " << Ob::numUsed() << " Obs:";
    os << "\n\tsize [core named keep] Ob";
    os << "\n\t-------------------------";
    os << std::left;
    for (Int i=0; i<N; ++i) {
        Ob ob = obs[i].first;
        Float mass = obs[i].second;
        bool in_core  = O::isInCore(ob);
        bool is_named = O::isNamed(ob);
        bool prunable = O::isPrunable(ob);
        ExprHdl expr = parse_ob(ob);
        os << "\n\t" << std::setw(12) << mass
                     << '['
                     << (in_core  ? 'c' : ' ')
                     << (is_named ? 'n' : ' ')
                     << (prunable ? ' ' : 'k')
                     << "] "
                     << expr;
    }
    os << std::endl;
}
void print_simplest_to (ostream& os, Int N)
{ PrintHead::call(os, _print_simplest_to, N); }
void _print_most_relevant_to (ostream& os, Int N)
{
    if (N == 0) N = Ob::numUsed();
    std::vector<std::pair<Ob,Float> > obs(brain().get_most_relevant(N));
    N = obs.size();
    os << "list of " << N << " most relevant " << Ob::numUsed() << " Obs:";
    os << "\n\trelevance [core named keep] Ob";
    os << "\n\t------------------------------";
    os << std::left;
    for (Int i=0; i<N; ++i) {
        Ob ob = obs[i].first;
        Float mass = obs[i].second;
        bool in_core  = O::isInCore(ob);
        bool is_named = O::isNamed(ob);
        bool prunable = O::isPrunable(ob);
        ExprHdl expr = parse_ob(ob);
        os << "\n\t" << std::setw(12) << mass
                     << '['
                     << (in_core  ? 'c' : ' ')
                     << (is_named ? 'n' : ' ')
                     << (prunable ? ' ' : 'k')
                     << "] "
                     << expr;
    }
    os << std::endl;
}
void print_most_relevant_to (ostream& os, Int N)
{ PrintHead::call(os, _print_most_relevant_to, N); }
void _print_sketchiest_to (ostream& os, Int N)
{
    if (N == 0) N = Ob::numUsed();
    std::vector<std::pair<Ob,Float> > obs(brain().get_sketchiest(N));
    N = obs.size();
    os << "list of " << N << " sketchiest " << Ob::numUsed() << " Obs:";
    os << "\n\tsketchinesss Ob";
    os << "\n\t---------------";
    os << std::left;
    for (Int i=0; i<N; ++i) {
        Ob ob = obs[i].first;
        Float mass = obs[i].second;
        ExprHdl expr = parse_ob(ob);
        os << "\n\t" << std::setw(12) << mass << " " << expr;
    }
    os << std::endl;
}
void print_sketchiest_to (ostream& os, Int N)
{ PrintHead::call(os, _print_sketchiest_to, N); }
void _print_conjectures_to (ostream& os, std::vector<TB::Conjecture> conj_list)
{
    typedef TB::Conjecture Conj;
    Int N = conj_list.size();
    os << "list of " << N << " conjectures with most supporting evidence:";
    os << "\n\tevidence   conjecture\n\t------------------------";
    os << std::left;
    for (Int i=0; i<N; ++i) {
        Conj& conj = conj_list[i];
        ExprHdl lhs = parse_ob(conj.lhs);
        ExprHdl rhs = parse_ob(conj.rhs);
        os << "\n\t" << std::setw(10) << conj.evidence << " "
           << lhs << "\t [=  " << rhs ;
    }
    os << std::endl;
}
void _print_conjectures_to (ostream& os, Int N)
{
    typedef TB::Conjecture Conj;
    std::vector<Conj> conj_list(brain().get_conjectures(N));
    _print_conjectures_to (os, conj_list);

    //save to file if possible
    ofstream file("stats/conjectures.text");
    if (file) {
        logger.info() << "writing conjectures to stats/conjectures.text" |0;
        _print_conjectures_to (file, conj_list);
        file.close();
    } else {
        logger.warning() << "could not open stats/conjectures.text" |0;
    }
}
void print_conjectures_to (ostream& os, Int N)
{ PrintHead::call(os, _print_conjectures_to, N); }

//lambda theory
unsigned get_default_effort () { return LT::MAX_EFFORT; }
Trool _query_stmt (StmtHdl* s) { return g_theory->query(*s); }
Trool _check_stmt (std::pair< std::pair<StmtHdl*,Int>, string*> args)
{
    StmtHdl stmt = *(args.first.first);
    Int effort = args.first.second;
    string& comment = *(args.second);
    return g_theory->check(stmt,comment,effort);
}
bool _assume_stmt (std::pair<StmtHdl*,string*> args)
{
    StmtHdl& stmt = *(args.first);
    string& comment = *(args.second);

    int o_cost = -Ob::size();
    int e_cost = -App::size();
    bool success = g_theory->assume(stmt,comment);
    o_cost += Ob::size();
    e_cost += App::size();
    if (o_cost or e_cost) {
        logger.info() << "  cost = " << o_cost << " obs + "
                                     << e_cost << " eqns" |0;
    }
    return success;
}
Prob  _guess_stmt (StmtHdl* s) { return brain().guess(*s); }
Trool query_stmt (StmtHdl stmt)
{
    UNLOCK_SYNTAX
    Trool result = run_syn_fun(_query_stmt, &stmt);
    LOCK_SYNTAX
    stmt.clear();
    return result;
}
Prob guess_stmt (StmtHdl s)
{
    UNLOCK_SYNTAX
    Prob result = run_syn_fun(_guess_stmt, &s);
    LOCK_SYNTAX
    s.clear();
    return result;
}
Trool check_stmt (StmtHdl stmt, string comment, Int effort)
{
    UNLOCK_SYNTAX
    Trool result = run_syn_fun(
            _check_stmt,
            std::make_pair( std::make_pair(&stmt,effort), &comment)
    );
    LOCK_SYNTAX
    stmt.clear();
    return result;
}
bool assume_stmt (StmtHdl stmt, string comment)
{
    UNLOCK_SYNTAX
    bool result = run_syn_fun(_assume_stmt, std::make_pair(&stmt,&comment));
    LOCK_SYNTAX
    stmt.clear();
    return result;
}

//reviewing & rechecking.  XXX most of this is thread unsafe
unsigned g_review_base = 0, g_review_cost = 0;
unsigned g_recheck_base = 0, g_recheck_cost = 0;
void _review_tick  (int) { g_review_base = Ob::size(); }
void _review_tock  (int) { g_review_cost = Ob::size() - g_review_base; }
void _recheck_tick (int) { g_recheck_base = Ob::size(); }
void _recheck_tock (int) { g_recheck_cost = Ob::size() - g_recheck_base; }
void _review  (Theory::AxmIter i) { g_theory->review(i); }
void _review  (Theory::ThmIter i) { g_theory->review(i); }
void _recheck (Theory::ThmIter i) { g_theory->recheck(i); }
void review ()
{
    logger.info() << "Starting to review all axioms and theorems" |0;
    run_command(_review_tick, 0);

    Theory& t = *g_theory;
    for (Theory::AxmIter i=t.axms_begin(); i!=t.axms_end(); ++i) {
        run_syn_cmd(_review, i);
    }
    for (Theory::ThmIter i=t.thms_begin(); i!=t.thms_end(); ++i) {
        run_syn_cmd(_review, i);
    }

    run_command(_review_tock, 0);
}
void recheck ()
{
    logger.info() << "Starting to recheck all problems" |0;
    run_command(_recheck_tick, 0);

    Theory& t = *g_theory;
    for (Theory::ThmIter i=t.thms_begin(); i!=t.thms_end(); ++i) {
        run_syn_cmd(_recheck, i);
    }

    run_command(_recheck_tock, 0);
}
void _problems (ostream& os, Int tr) { g_theory->print_problems(os,Trool(tr)); }
void problems (ostream& os, Trool tr) { PrintHead::call(os, _problems, tr); }
void _clear_theory (int) { g_theory->clear_thms(); g_theory->clear_axms(); }
void clear_theory () { run_syn_cmd(_clear_theory, 0); }

//marking
bool _mark (ExprHdl* expr)
{
    ObHdl ob = EX::get_expr(expand(*expr));
    if (ob) { O::mark(*ob); return true; }
    else return false;
}
bool mark (ExprHdl expr)
{
    UNLOCK_SYNTAX
    bool result = run_syn_fun(_mark, &expr);
    LOCK_SYNTAX
    expr.clear();
    return result;
}
bool _unmark (ExprHdl* expr)
{
    Ob ob = EX::find_expr(expand(*expr));
    if (ob) { O::unmark(ob); return true; }
    else return false;
}
bool unmark (ExprHdl expr)
{
    UNLOCK_SYNTAX
    bool result = run_syn_fun(_unmark, &expr);
    LOCK_SYNTAX
    expr.clear();
    return result;
}
void _mark_all   (int=0) { O::mark_all(); }
void mark_all    ()      { run_command(_mark_all,0); }
void _unmark_all (int=0) { O::unmark_all(); }
void unmark_all  ()      { run_command(_unmark_all,0); }

//solving equations
struct SolveAndPrint
{
    ostream &os;
    StmtHdl &stmt;
    bool tryhard;
    SolveAndPrint (ostream& o, StmtHdl &s, bool t)
        : os(o), stmt(s), tryhard(t) {}
};
unsigned _solve_and_print (SolveAndPrint args)
{
    std::vector<Ob> soln_set = g_theory->solve(expand(args.stmt), args.tryhard);

    if (soln_set.empty()) { args.os << "{}" << std::endl; return 0; }

    args.os << '{' << parse_ob(soln_set[0]);
    for (Int i=1; i<soln_set.size(); ++i) {
        args.os << ", " << parse_ob(soln_set[i]);
    }
    args.os << "} #size = " << soln_set.size() << std::endl;

    return soln_set.size();
}
void solve_and_print (ostream& os, StmtHdl& stmt, bool tryhard)
{
    run_syn_fun(_solve_and_print, SolveAndPrint(os, stmt, tryhard));
}

//context
Context g_context;
Context& context () { return g_context; }
int _recompile_context (Int tryhard=0) { brain().update(tryhard); return 0; }
void recompile_context (Int tryhard)
{
    run_syn_fun(_recompile_context, tryhard);
}
void _update (Int tryhard=0)
{
    context().simplify();
    _recompile_context(tryhard);
}
void update (Int tryhard) { run_syn_cmd(_update, tryhard); }
void _think_about_context(int) { brain().think_about_context(context()); }
void think_about_context () { run_syn_cmd(_think_about_context, 0); }
void _think_about_theory(int) { brain().think_about_theory(); }
void think_about_theory () { run_syn_cmd(_think_about_theory, 0); }

//language
typedef std::vector<ExprHdl> ExprList;
bool _extend  (ExprHdl* expr)
{
    ObHdl ob = EX::get_expr(*expr);
    if (not ob) {
        logger.warning() << "concept not understood for adding" |0;
        return false;
    }
    return brain().extend(ob);
}
bool _retract (ExprHdl* expr)
{
    ObHdl ob = EX::get_expr(*expr);
    if (not ob) {
        logger.warning() << "concept not understood for removing" |0;
        return false;
    }
    return brain().retract(ob);
}
bool extend  (ExprHdl expr)
{
    UNLOCK_SYNTAX
    bool result = run_syn_fun(_extend, &expr);
    LOCK_SYNTAX
    expr.clear();
    return result;
}
bool retract (ExprHdl expr)
{
    UNLOCK_SYNTAX
    bool result = run_syn_fun(_retract, &expr);
    LOCK_SYNTAX
    expr.clear();
    return result;
}
void _set_rule_weight (std::pair<Symbols::PropRule, Float> args)
{
    brain().rules()[args.first] = args.second;
}
void set_rule_weight (Symbols::PropRule name, Float weight)
{
    run_command(_set_rule_weight, std::make_pair(name,weight));
}

//timer
pthread_mutex_t p_timer_mutex = PTHREAD_MUTEX_INITIALIZER;
Int g_numevents;
Float g_timing_since;
void start_timing ()
{
    pthread_mutex_lock( &p_timer_mutex );
    g_timing_since = get_elapsed_time();
    g_numevents = 0;
    pthread_mutex_unlock( &p_timer_mutex );
}
void tick_tock ()
{//advance clock by one event
    pthread_mutex_lock( &p_timer_mutex );
    ++g_numevents;
    pthread_mutex_unlock( &p_timer_mutex );
}

//napping (briefly) and resting (longer)
// * napping happens every ~100 events  or ~10 minutes (hard-coded)
// * resting happens every ~1000 ob prunings (the time-scale parameter)
TT::ThreadSafe<bool> resting(false);
void set_resting (bool w) { resting(w); }
Float g_nap_time = 0.0f;
bool time_for_nap ()
{
    const Int ENOUGH_EVENTS = 100; //every 100 events, or
    const Float ENOUGH_TIME = 600; //every 10 minutes, or
    const Float MAX_NAP_PART = 0.05f; //but don't nap more than 5% of the time

    bool result = false;
    pthread_mutex_lock( &p_timer_mutex );
    Float time_diff = get_elapsed_time() - g_timing_since;
    if (g_numevents > ENOUGH_EVENTS) result = true;
    result = result and (time_diff > ENOUGH_TIME);
    result = result and (time_diff > MAX_NAP_PART * g_nap_time);
    pthread_mutex_unlock( &p_timer_mutex );
    return result;
}
void _nap (int=0)
{
    logger.info() << "Napping..." |0;
    Logging::IndentBlock block;

    pthread_mutex_lock( &p_timer_mutex );
    g_nap_time = -get_elapsed_time();
    pthread_mutex_unlock( &p_timer_mutex );

    context().simplify();
    brain().update();

    pthread_mutex_lock( &p_timer_mutex );
    Float nap_time = (g_nap_time += get_elapsed_time());
    pthread_mutex_unlock( &p_timer_mutex );
    logger.info() << "nap took " << nap_time << " sec" |0;

    start_timing();
}
void nap () { run_syn_cmd(_nap, 0); }
void _rest (int=0) { brain().rest(); }
Float g_rest_time = 0.0f; //for user feedback only
void _tick_rest (int)
{
    g_rest_time = -get_elapsed_time();
}
void _tock_rest (int)
{
    g_rest_time += get_elapsed_time();
    logger.info() << "...rest took " << g_rest_time << " sec" |0;
}
void rest ()
{
    logger.info() << "Resting..." |0;

    //run these as many commands to reduce console latency
    run_command(_tick_rest,0);

    //review();
    //recheck();

    update(true);
    run_syn_cmd(_rest, 0);
    update();
    run_command(_tock_rest,0);
}
void rest_if_tired ()
{
    if (brain().time_to_rest()) {
        rest();
        start_timing();
    } else if (time_for_nap()) {
        nap();
        start_timing();
    }

    //                     yawn
    //fritz is tired too :()
}
void whine_if_tired ()
{
    if (brain().time_to_rest()) {
        logger.warning() << "time to rest" |0;
        start_timing();
    } else if (time_for_nap()) {
        logger.warning() << "time to nap" |0;
        start_timing();
    }

    //                     yawn
    //fritz is tired too :()
}

//params
void _write_lang_to (ostream& os, Int)
{
    os << "language:\n";
    brain().write_lang_to(os);
}
void write_lang_to (ostream& os) { PrintHead::call(os, _write_lang_to); }
void _write_stats_to (ostream& os, Int)
{
    os << "system statistics:\n";
    CS::       write_stats_to(os);
    EX::       write_stats_to(os);
    context(). write_stats_to(os);
    g_theory-> write_stats_to(os);
    brain().   write_stats_to(os);

    //cost of review & recheck
    if (g_review_cost + g_recheck_cost > 0) {
        os << "\treview cost = "  << g_review_cost
           << ", recheck cost = " << g_recheck_cost << std::endl;
    }

    //nap time
    pthread_mutex_lock( &p_timer_mutex );
    Float nap_time = g_nap_time;
    pthread_mutex_unlock( &p_timer_mutex );
    if (nap_time > 0) {
        os << "\tlast nap took " << nap_time << " sec." << std::endl;
    }

    //also in log file
    CS::log_stats();
}
void write_stats_to (ostream& os) { PrintHead::call(os, _write_stats_to); }
long am_thinking ();
void _save_basis_to (ostream& os,Int) { CS::save_to(os); }
void save_basis_to (ostream& os) { PrintHead::call(os, _save_basis_to); }
void _save_theory_to (ostream& os,Int) { g_theory->save_to(os); }
void save_theory_to (ostream& os) { PrintHead::call(os, _save_theory_to); }
void _save_params_to (ostream& os, Int)
{
    os << "#\\section{System Parameters}\n\n";

    //LOCK_SYNTAX
    //EX::save_params_to(os)
    brain().save_params_to(os);
    //UNLOCK_SYNTAX

    //loal params
    os << "#\\subsection{Task Management}\n\n";
    os << (resting() ? "!start" : "!stop") << " resting\n";
    os << (am_thinking() ? "!start" : "!stop") << " thinking\n";
}
void save_params_to (ostream& os)
{ PrintHead::call(os, _save_params_to); }
void _write_params_to (ostream& os, Int)
{
    os << "system parameters:\n";
    long thoughts = am_thinking();
    if (thoughts) {
        os << "\tcurrently thinking; " << thoughts << " thoughts enqueued\n";
    } else {
        os << "\tcurrently not thinking\n";
    }
    if (resting()) os << "\tresting when I want to\n";

    g_theory->write_params_to(os);
    CS::write_params_to(os);
    brain().write_params_to(os);
}
void write_params_to (ostream& os)
{ PrintHead::call(os, _write_params_to); }
void _plot_fe (Int p) { brain().plot_funs_of_eps(p); }
void plot_funs_of_eps (Int num_points) { run_command(_plot_fe, num_points); }
bool write_stats_page (Int size)
{//writes statistics page
    logger.info() << "writing stats window" |0;
    Logging::IndentBlock block;

    brain().compact();

    //write stats part
    ofstream o("stats/stats.html");
    if (not o) {
        return false;
        logger.warning() << "failed to write stats window" |0;
    }

    o << "\n<html><head>"
      << "\n<title>Database Statistics</title>"
      << "\n<link rel='stylesheet' type='text/css' href='stats.css'>"
      << "\n</head><body>"
      << "\n<table>"

      << "\n<tr><td>"
      << "\n\t<pre>\n";
    CS::write_stats_to(o);
    o << "\t</pre>"
      << "\n</td></tr>"

      << "\n<tr><td>"
      << "\n\t<a href='ob_mass.html'><img src='ob_mass_small.png'></a>"
      << "\n</td></tr>"

      << "\n<tr><td>"
      << "\n\t<a href='ord_table.html'><img src='ord_table_small.png'></a>"
      << "\n</td></tr>"

      << "\n<tr><td>"
      << "\n\t<a href='eqn_table.html'><img src='eqn_table_small.png'></a>"
      << "\n</td></tr>"

      << "\n</table>"
      << "\n</body></html>";
    o.close();

    //draw images
    brain().vis_ob_mass(size);
    CS::vis_eqn_table(size);
    CS::vis_ord_table(size);

    logger.info() << "done." |0;
    return true;
}
bool _save_stats (StatType type)
{//returns true on error
    const Int win_size = 480;
    switch (type) {
        case STAT_WIN: return not write_stats_page(win_size);
        case VIS_EQNS: return not CS::vis_eqn_table(win_size);
        case VIS_ORDS: return not CS::vis_ord_table(win_size);
        case VIS_MASS: return not brain().vis_ob_mass(win_size);
        case MAP_3D:   return brain().save_map();
        default:
            logger.error() << "unknown statistic:" << Int(type) |0;
            return true;
    }
}
bool save_stats (StatType type) { return run_function(_save_stats, type); }

//top-level interface
class Restart : public Command
{
    std::set<string> m_basis;
    void _fix_basis ();
    Restart () { _fix_basis(); }
    Restart (std::vector<string>& basis)
        : m_basis(basis.begin(),basis.end())
    { _fix_basis(); }
public:
    virtual ~Restart () {}
    virtual void _run ();
    virtual void run ();
    static void _call();
    static void call();
    static void call(std::vector<string>& basis);
};
void Restart::_fix_basis ()
{//adds minimum basis requirements
    //ensure minimum basis
    m_basis.insert("_");
    m_basis.insert("T");
    m_basis.insert("S");
    m_basis.insert("K");
    m_basis.insert("I");
    m_basis.insert("B");
    m_basis.insert("C");
    m_basis.insert("W");

    //some possible extensions include
    //m_basis.insert("F");
    //m_basis.insert("Y");

    //m_basis.insert("J");
    //m_basis.insert("R");
    //m_basis.insert("U");
    //m_basis.insert("V");
    //m_basis.insert("P");
}
void Restart::_run ()
{
    const Logging::fake_ostream &log = Logging::logger.info();
    log << "restarting with: ";
    typedef TB::Brain::BasisType BasisType;
    BasisType m_basis;
    for (BasisType::iterator i=m_basis.begin(); i!=m_basis.end(); ++i) {
        log << ' ' << *i;
    }
    log |0;
    Logging::IndentBlock block;

    if (not construct_system()) {
        context().clear();
        brain().clear();
    }
    const Int num_obs = 255; //default size

    brain().initialize(num_obs, m_basis);
    //WARNING: notation.jtext must be re-read now
}
void Restart::run () { LOCK_SYNTAX  _run();  UNLOCK_SYNTAX  done(); }
void Restart::_call() { Restart()._run(); }
void Restart::call()
{
    Restart cmd;
    push_cmd(&cmd);
    cmd.wait(); //since syntax is reset
}
void Restart::call(std::vector<string>& basis)
{
    Restart cmd(basis);
    push_cmd(&cmd);
    cmd.wait(); //since syntax is reset
}
void _restart () { Restart::_call(); }
void restart () { Restart::call(); }
void restart (std::vector<string> basis) { Restart::call(basis); }
bool _save (string filename)
{
    //review theory first
    g_theory->review();
    return brain().save(filename);
}
bool save (string filename) { return run_syn_fun(_save, filename); }
bool _load (string filename)
{
    construct_system();
    context().clear(); //so exprs can clear
    bool result = brain().load(filename);
    return result;
    //WARNING: notation.jtext must be re-read now
}
bool load (string filename) { return run_syn_fun(_load, filename); }
int _dump (std::pair<string,Int> args)
{
    CS::dump(args.first, args.second);
    return 0;
}
void dump (string filename, Int type)
{
    run_syn_fun(_dump, std::make_pair(filename, type));
}
bool _validate (Int level)
{
    start_validating();
    brain().validate(level);
    return everything_valid();
}
bool validate (Int level) { return run_syn_fun(_validate, level); }
bool _test (Int level)
{
    logger.info() << "Testing brain" |0;
    Logging::IndentBlock block;

    start_validating();
    if (level) {
        AE::test_iterators();
        CE::test_iterators();
    }
    return everything_valid();
}
bool test (Int level) { return run_syn_fun(_test, level); }

//command processing (implementation)
//  data
pthread_mutex_t p_task_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t p_task_pending = PTHREAD_COND_INITIALIZER;
bool g_expecting_cmds;
long g_am_thinking;
bool g_am_paused;
std::deque<Command*> g_pending_cmds;

//  task generation
#define AD_INFINITUM 0x70000000l
void push_cmd (Command* cmd)
{
    pthread_mutex_lock( &p_task_mutex );
    g_pending_cmds.push_back(cmd);
    pthread_cond_signal( &p_task_pending );
    pthread_mutex_unlock( &p_task_mutex );
}
void cancel_task ()
{
    logger.warning() << "cancel_task only partially implemented" |0;

    //just clears backlogged commands
    pthread_mutex_lock( &p_task_mutex );
    g_pending_cmds.clear();
    g_am_thinking = 0;
    pthread_cond_signal( &p_task_pending );
    pthread_mutex_unlock( &p_task_mutex );
}
long _am_thinking () { return g_am_paused ? 0 : g_am_thinking; }
long am_thinking ()
{
    pthread_mutex_lock( &p_task_mutex );
    long result = _am_thinking();
    pthread_mutex_unlock( &p_task_mutex );
    return result;
}
void set_thinking (bool whether)
{
    pthread_mutex_lock( &p_task_mutex );
    g_am_thinking = whether ? AD_INFINITUM : 0;
    bool thinking = _am_thinking();
    pthread_cond_signal( &p_task_pending );
    pthread_mutex_unlock( &p_task_mutex );
    if (thinking) start_timing();
}
void think_user (Int num_cycles)
{
    pthread_mutex_lock( &p_task_mutex );

    g_am_thinking += num_cycles * (1 + brain().granularity());
    if (g_am_thinking < 0) g_am_thinking = AD_INFINITUM;

    bool thinking = _am_thinking();
    pthread_cond_signal( &p_task_pending );
    pthread_mutex_unlock( &p_task_mutex );
    if (thinking) start_timing();
}
void pop_thought ()
{
    pthread_mutex_lock( &p_task_mutex );
    Assert (g_am_thinking > 0, "tried to pop thought off empty queue");
    --g_am_thinking;
    pthread_mutex_unlock( &p_task_mutex );
}
void set_paused (bool whether)
{
    pthread_mutex_lock( &p_task_mutex );
    g_am_paused = whether;
    bool thinking = _am_thinking();
    pthread_cond_signal( &p_task_pending );
    pthread_mutex_unlock( &p_task_mutex );
    if (thinking) start_timing();
}
void no_more_cmds ()
{
    pthread_mutex_lock( &p_task_mutex );
    g_expecting_cmds = false;
    pthread_cond_signal( &p_task_pending );
    pthread_mutex_unlock( &p_task_mutex );
}

//  task processing
//  Later: this would be cleaner with a polymorphic Task class...
enum TaskType { NO_TASK, COMMAND_TASK, THINK_TASK };
TaskType wait_for_task ()
{
    pthread_mutex_lock( &p_task_mutex );
    while (g_expecting_cmds and g_pending_cmds.empty() and not _am_thinking()) {
        pthread_cond_wait( &p_task_pending, &p_task_mutex );
    }
    TaskType result = NO_TASK;
    if (not g_pending_cmds.empty()) result = COMMAND_TASK;
    else if (_am_thinking())        result = THINK_TASK;
    pthread_mutex_unlock( &p_task_mutex );
    return result;
}
Command* pop_cmd ()
{
    Command* result = NULL;
    pthread_mutex_lock( &p_task_mutex );
    if (not g_pending_cmds.empty()) {
        result = g_pending_cmds.front();
        g_pending_cmds.pop_front();
    }
    pthread_mutex_unlock( &p_task_mutex );
    return result;
}
void process_cmds (void* script_name)
{
    logger.debug() << "begin processing commands" |0;

    //init data
    g_pending_cmds.clear();
    g_expecting_cmds = true;
    g_am_paused = false;
    g_am_thinking = 0;

    //start input thread
    pthread_t p_parser;
    int status = pthread_create( &p_parser, NULL, start_parser, script_name);
    Assert (status == 0, "return code from pthread_create() is " << status);

    //process commands
    g_processing_cmds = true;
    bool mid_thought = false;
    do { TaskType task = wait_for_task();

        switch (task) {
            case NO_TASK: {
                g_processing_cmds = false;
                return;
            } break;

            case COMMAND_TASK: {
                Command* cmd = pop_cmd();
                Assert (cmd, "no command pending");
                if (mid_thought) { mid_thought = false; brain().cleanup(); }
                cmd->run(); //command is responsible for its own deletion
            } break;

            case THINK_TASK: {
                mid_thought = true;
                pop_thought();
                if (not brain().think()) { //uses no exprs
                    logger.error() << "failed to think; stopping thinking" |0;
                    set_thinking(false);
                }
            } break;
        }

        tick_tock();

        if (resting()) rest_if_tired();
        else           whine_if_tired();

    } while (true);
}

}

namespace
{
const char* const help_message =
"Usage: johann [options] [file]\n\
  Starts algebra system with specified script [file]\n\
Options:\n\
    -c <db name>    Load database <db name>\n\
    -l <log name>   Write log messages to <log name>\n\
    -s <integer>    Use <integer> as random seed\n\
    -q              Run quietly\n\
    -v              Run verbosely\n\
    -               Do not read init.jcode\n\
    -h, --help      Display this message\n\
    -V, --version   Display version information\n\
Default values:\n\
    database:   $JOHANN/data/default.jdb\n\
    script:     $JOHANN/scripts/init.jcode\n\
    log file:   $JOHANN/log/default.log\n\
";
}

//main function main
int main (int argc, char** argv)
{
    std::ios::sync_with_stdio(false);

    namespace S = Substitute;
    namespace K =  Kernel;
    using K::logger;

    //parse command line arguments
    const char* db_name = NULL;
    const char* script_name = "scripts/init.jcode";
    const char* log_name = "default.log";
    int random_seed = 0;
    bool quietly = true;

    const string ARG_NONE   ("-"),
                      ARG_CORE   ("-c"),
                      ARG_LOG    ("-l"),
                      ARG_SEED   ("-s"),
                      ARG_QUIET  ("-q"),
                      ARG_VERBOSE("-v"),
                      ARG_H("-h"), ARG_HELP("--help"),
                      ARG_V("-V"), ARG_VERSION("--version");
    enum ArgType { SCRIPT_NAME, CORE_NAME, LOG_NAME , SEED_NUM};

    ArgType arg_type = SCRIPT_NAME; //the default
    for (int i=1; i<argc; ++i) {
        const char* arg = argv[i];

        //read boolean flags
        if (arg == ARG_NONE)   { script_name = NULL; continue; }
        if (arg == ARG_QUIET)  { quietly = true;     continue; }
        if (arg == ARG_VERBOSE){ quietly = false;    continue; }

        //read type flags
        if (arg == ARG_CORE) { arg_type = CORE_NAME; continue; }
        if (arg == ARG_LOG)  { arg_type = LOG_NAME;  continue; }
        if (arg == ARG_SEED) { arg_type = SEED_NUM;  continue; }

        //read info flags
        if (arg == ARG_V or arg == ARG_VERSION) {
            std::cout << "Johann " << VERSION << "\n";
            return 0;
        }
        if (arg == ARG_H or arg == ARG_HELP) {
            std::cout << help_message;
            return 0;
        }

        //read variable flags
        switch (arg_type) {
            case SCRIPT_NAME: script_name = arg; break;
            case CORE_NAME:   db_name     = arg; break;
            case LOG_NAME:    log_name    = arg; break;
            case SEED_NUM:    random_seed = atoi(arg); break;
        }
        arg_type = SCRIPT_NAME; //the default
    }

    //start logger
    if (log_name != NULL) { Logging::switch_to_log(log_name); }
    std::ostringstream title;
    title << "Johann " << VERSION;
    Logging::title(title.str());

    //initialize
    srand48(random_seed);
    if (random_seed) {
        Logging::logger.info() << "random seed = " << random_seed |0;
    }

    //start system
    if (db_name == NULL) K::_restart();
    else                 K::_load(db_name);

    //proccess commands in foreground and thoughts in background
    set_quietly(quietly);
    K::process_cmds(static_cast<void*>(const_cast<char*>(script_name)));

    //exit
    K::context().clear();
    S::library.clear();
    K::brain().clear();
    delete K::g_system;
    logger.info() << "bye!" |0;
    pthread_exit(NULL); //so existing threads can continue
    return 0;
}



