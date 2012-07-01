
#include "lambda_theories.h"
#include "obs.h"
#include "apply.h"
#include "compose.h"
#include "join.h"
#include "combinatory_structure.h"
#include "syntax_semantics.h"
#include "priority_queue.h"
#include <set>
#include <algorithm>
#include <unistd.h> //for fork
#include <sys/wait.h> //for wait
#include <signal.h> //for kill

//log levels
#define LOG_DEBUG1(mess);
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << mess |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

namespace LambdaTheories
{

namespace O = Obs;
namespace AE = Apply;
namespace CE = Compose;
namespace JE = JoinEqn;
namespace OR = Order;
namespace EX = Expressions;
namespace ST = Statements;
namespace CS = CombinatoryStructure;

//======== named atoms ========

//Note: atoms are listed with their core file header atom offsets

//lattice atoms
ObHdl Atoms::Bot;
ObHdl Atoms::Top;
ObHdl Atoms::J;
ObHdl Atoms::R;

//lambda atoms
ObHdl Atoms::I;
ObHdl Atoms::K;
ObHdl Atoms::B;
ObHdl Atoms::C;
ObHdl Atoms::W;
ObHdl Atoms::S;

//extension atoms
//  SK-definable atoms
ObHdl Atoms::Y;
ObHdl Atoms::S2;
ObHdl Atoms::CB;
ObHdl Atoms::CI;
ObHdl Atoms::KI;
ObHdl Atoms::SI;

//  SKJ-definable atoms
ObHdl Atoms::U;
ObHdl Atoms::V;
ObHdl Atoms::P;

//  atoms used with types
ObHdl Atoms::Inl;
ObHdl Atoms::Inr;
ObHdl Atoms::J2;

//types
ObHdl Atoms::Div;
ObHdl Atoms::Unit;
ObHdl Atoms::Semi;
ObHdl Atoms::Bool;
ObHdl Atoms::Maybe;
ObHdl Atoms::Sum;
ObHdl Atoms::Sset;

//collections of atoms
typedef std::map<const string, ObHdl*> AtomDefs;
AtomDefs atom_defs;
std::set<string> required_atoms;

//functions
std::vector<ObHdl*> Atoms::s_defined_obs;
inline void Atoms::_define_atom(ObHdl& atom, const char* name, bool required)
{
    Ob ob = CS::find_atom(name);
    if (!ob) {
        if (not required) return;
        ob = CS::make_atom(name);
    }
    atom.set(ob);
    s_defined_obs.push_back(&atom);
}
inline void Atoms::_define_app(ObHdl& app, Ob lhs, Ob rhs)
{
    //don't call CS::get_app, or CS will saturate
    Ob ob = AE::find_app(lhs, rhs);
    if (!ob) ob = AE::get_app(CS::make_app(lhs, rhs));
    app.set(ob);
    s_defined_obs.push_back(&app);
}
void Atoms::clear ()
{
    for (Int i=0; i<s_defined_obs.size(); ++i) {
        ObHdl* ob = s_defined_obs[i];
        ob->clear();
    }
    s_defined_obs.clear();
}

//================================ theories ================================

/** Theories
 * Theories specify their atoms and variables statically;
 *  new names are stored elsewhere.
 */

inline ObHdl operator * (const ObHdl& lhs, const ObHdl& rhs)
{//application
    return ObHdl(CS::get_app(*lhs, *rhs));
}
inline ObHdl operator % (const ObHdl& lhs, const ObHdl& rhs)
{//composition
    Ob Bx = CS::get_app(*Atoms::B, *lhs);
    return ObHdl(CS::get_app(Bx, *rhs));
}

//======== general non-associative theory ========

MagmaTheory *MagmaTheory::s_unique_instance = NULL;

MagmaTheory::MagmaTheory ()
{
    logger.debug() << "creating MagmaTheory" |0;
    Assert (s_unique_instance == NULL, "tried to create a second theory");
    s_unique_instance = this;
}
MagmaTheory::~MagmaTheory ()
{
    logger.debug() << "deleting MagmaTheory" |0;
    s_unique_instance = NULL;

    atom_defs.clear();
    required_atoms.clear();
}
void MagmaTheory::clear ()
{
    Atoms::clear();
    m_thms.clear();
    m_axms.clear();
}

void MagmaTheory::init_basis ()
{
    for (AtomDefs::iterator i=atom_defs.begin(); i!=atom_defs.end(); ++i) {
        const string& name = i->first;
        ObHdl& ob = *(i->second);
        bool required = required_atoms.find(name) != required_atoms.end();
        _define_atom(ob, name.c_str(), required);
    }
}
bool MagmaTheory::define_atom (string name)
{//returns whether atom is recognized and new
    AtomDefs::iterator i=atom_defs.find(name);
    if (i == atom_defs.end()) {
        if (not CS::find_atom(name)) CS::make_atom(name);
        return false;
    } else {
        ObHdl& ob = *(i->second);
        _define_atom(ob, name.c_str());
        return true;
    }
}
bool MagmaTheory::forget_atom (string name)
{
    AtomDefs::iterator i=atom_defs.find(name);
    if (i == atom_defs.end()) {
        return false;
    } else {
        ObHdl &ob = *(i->second);
        ob.clear();
        return true;
    }
}

//standard  tools
MagmaTheory::Axm& MagmaTheory::AxmIter::operator* ()
{
    return s_unique_instance->m_axms[i];
}
MagmaTheory::Axm* MagmaTheory::AxmIter::operator-> ()
{
    return &(s_unique_instance->m_axms[i]);
}
MagmaTheory::Thm& MagmaTheory::ThmIter::operator* ()
{
    return s_unique_instance->m_thms[i];
}
MagmaTheory::Thm* MagmaTheory::ThmIter::operator-> ()
{
    return &(s_unique_instance->m_thms[i]);
}
inline int safe_fork ()
{
    logger.debug() << "forking kernel" |0;
    int pid = fork();
    Assert(pid >= 0, "failed to fork in proof by contradiction");
    return pid;
}
Trool MagmaTheory::_query_contradiction (StmtHdl stmt)
{//proof by contradiction

    int assume_pos;
    if ((assume_pos = safe_fork()) == 0) {
        CS::die_quietly();
        logger.info() << "making positive assumption" |0;
        _assume(stmt, false);
        logger.info() << "positive assumption was consistent" |0;
        _exit(0);
    }

    int assume_neg;
    if ((assume_neg = safe_fork()) == 0) {
        CS::die_quietly();
        logger.info() << "making negative assumption" |0;
        _assume(ST::Not(stmt), false);
        logger.info() << "negative assumption was consistent" |0;
        _exit(0);
    }

    while (assume_pos or assume_neg) {

        int status;
        int pid = wait(&status);
        int exited = WIFEXITED(status);
        int exitstatus = WEXITSTATUS(status);
        bool consistent = exited and exitstatus == 0;

        if (pid == assume_pos) {
            assume_pos = 0;

            if (!consistent) {
                if (assume_neg) kill(assume_neg, 9);
                logger.info() << "positive assumption was inconsistent" |0;
                return FALSE;
            }

        } else if (pid == assume_neg) {
            assume_neg = 0;

            if (!consistent) {
                if (assume_pos) kill(assume_pos, 9);
                logger.info() << "negative assumption was inconsistent" |0;
                return TRUE;
            }

        }
    }

    return UNKNOWN;
}
Trool MagmaTheory::query (StmtHdl stmt, unsigned effort)
{
    StmtHdl s = stmt->query_nf();
    if (!s) {
        logger.warning() << "statement has no query normal form" |0;
        return UNKNOWN;
    }

    Trool result = UNKNOWN;
    if (ST::Relationship* r = s->as_reln()) {
        result = query_reln(r->lhs(), r->rel(), r->rhs(), effort);
    } else if (ST::Conjunction* c = s->as_conj()) {
        result = TRUE;
        for (unsigned i=0,I=c->terms().size(); i<I; ++i) {
            result = And(query(c->terms()[i], effort), result);
        }
    } else if (ST::Disjunction* d = s->as_disj()) {
        result = FALSE;
        for (unsigned i=0,I=d->terms().size(); i<I; ++i) {
            result = Or(query(d->terms()[i], effort), result);
        }
    } else {
        logger.error() << "queried bad statement type: " << stmt |0;
        return UNKNOWN;
    }

    if (result == UNKNOWN and effort >= 2) {
        result = _query_contradiction(s);
    }

    return result;
}
Trool MagmaTheory::check (StmtHdl stmt, string comment, unsigned effort)
{
    //try to break conjunctions into parts
    if (ST::Conjunction* c = stmt->as_conj()) {
        Trool result = TRUE;
        for (unsigned i=0,I=c->terms().size(); i<I; ++i) {
            std::ostringstream comment_i;
            comment_i << comment << '(' << 1+i << '/' << I << ')';
            result = And(check(c->terms()[i], comment_i.str(), effort), result);
        }
        return result;
    }

    Trool truth = query(stmt, effort);
    if (truth == TRUE) _assume(stmt, false); //don't add to axioms,core
    m_thms.push_back(Thm(Prop(stmt,comment),truth));
    return truth;
}
bool MagmaTheory::assume (StmtHdl stmt, string comment)
{
    bool result = _assume(stmt, true);
    if (result) m_axms.push_back(Prop(stmt,comment));
    return result;
}
bool MagmaTheory::_assume (StmtHdl stmt, bool core)
{
    StmtHdl s = stmt->assume_nf();
    if (!s) {
        logger.warning() << "statement has no assume normal form" |0;
        return false;
    }

    bool result = true;
    if (ST::Relationship* r = s->as_reln()) {
        result = assume_reln(r->lhs(), r->rel(), r->rhs(), core);
    } else if (ST::Conjunction* c = s->as_conj()) {
        for (unsigned i=0,I=c->terms().size(); i<I; ++i) {
            result = _assume(c->terms()[i], core) and result;
        }
    } else {
        logger.error() << "assumed bad statement type: " << s |0;
        result = false;
    }
    return result;
}
void MagmaTheory::review (AxmIter i)
{
    StmtHdl stmt = i->stmt;
    _assume(stmt, true);
}
void MagmaTheory::review (ThmIter i)
{
    StmtHdl& stmt = i->first.stmt;
    Trool& truth = i->second;
    if (truth == TRUE) _assume(stmt, false);
}
void MagmaTheory::review ()
{
    logger.info() << "Reviewing all axioms and theorems" |0;
    Logging::IndentBlock block;

    for (AxmIter i=axms_begin(); i!=axms_end(); ++i) review(i);
    for (ThmIter i=thms_begin(); i!=thms_end(); ++i) review(i);
}
void MagmaTheory::recheck (ThmIter i, unsigned effort)
{
    Prop& prop = i->first;
    StmtHdl& stmt = prop.stmt;
    Trool& truth = i->second;

    if (truth != UNKNOWN) return;
    truth = query(prop.stmt, effort);
    if (truth == TRUE) {
        _assume(stmt, false);
    } else if (truth == FALSE) {
        logger.warning() << "failed check:\n\t" << prop |0;
    }
}
void MagmaTheory::recheck (unsigned effort)
{
    logger.info() << "Rechecking all problems" |0;
    Logging::IndentBlock block;

    for (ThmIter i=thms_begin(); i!=thms_end(); ++i) recheck(i,effort);
}
std::vector<StmtHdl> MagmaTheory::get_axioms ()
{
    std::vector<StmtHdl> result;
    for (AxmIter i=axms_begin(); i!=axms_end(); ++i) {
        result.push_back(i->stmt);
    }
    return result;
}
std::vector<StmtHdl> MagmaTheory::get_problems (Trool truth)
{
    std::vector<StmtHdl> result;
    for (ThmIter i=thms_begin(); i!=thms_end(); ++i) {
        Prop& prop = i->first;
        Trool& tr = i->second;

        if (tr == truth) result.push_back(prop.stmt);
    }
    return result;
}
#define BLOCK_SIZE 5
void MagmaTheory::print_problems (ostream& os, Trool truth)
{
    os << "#Problems (" << Symbols::TroolNames[truth] << "). \n";
    Int b = 0;
    for (ThmIter i=thms_begin(); i!=thms_end(); ++i) {
        Prop& prop = i->first;
        Trool& tr = i->second;

        if (tr != truth) continue;
        if ((b++)%BLOCK_SIZE == 0) os << '\n';
        os << "!check " << prop << '\n';
    }
    os << std::flush;
}
void MagmaTheory::save_to (ostream& os)
{
    os << "#\\section{Theory}\n\n";

    os << "#\\subsection{Axioms}\n";
    Int b = 0;
    for (AxmIter i=axms_begin(); i!=axms_end(); ++i) {
        Prop& prop = *i;
        if ((b++)%BLOCK_SIZE == 0) os << '\n';
        os << "!assume " << prop << "\n";
    }
    os << "\n";

    os << "#\\subsection{Theorems and Problems}\n";
    b = 0;
    for (ThmIter i=thms_begin(); i!=thms_end(); ++i) {
        Prop& prop = i->first;
        Trool& truth = i->second;
        if ((b++)%BLOCK_SIZE == 0) os << '\n';
        os << "!check " << prop << ", " << Symbols::TroolNames[truth] << "\n";
    }
    os << std::endl;
}
void MagmaTheory::write_stats_to (ostream& os)
{
    os << '\t' << m_axms.size() << " axioms, "
               << m_thms.size() << " theorems+problems+errors" << std::endl;
}

//virtual assumption tools
Trool MagmaTheory::query_reln
    (ExprHdl lhs, Relation reln, ExprHdl rhs, unsigned effort)
{//returns true on success
    LOG_DEBUG1( "Checking " << lhs << " "<< RelationNames[reln] <<" " << rhs );
    LOG_INDENT_DEBUG1
    Assert (lhs->isPure() and rhs->isPure(), "impure expressions");

    if (effort == 0) { //only look up the answer if it's already there
        Ob lhs_ob = EX::find_expr(lhs);  if(!lhs_ob) return UNKNOWN;
        Ob rhs_ob = EX::find_expr(rhs);  if(!rhs_ob) return UNKNOWN;
        return CS::query_reln(lhs_ob, reln, rhs_ob);
    }

#ifdef PEDANTIC_QUERY
    //just convert to combinators
    lhs = lhs->as_comb();
    rhs = rhs->as_comb();
#else
    //convert to combinators, and linear reduce both sides
    lhs = lhs->reduce();
    rhs = rhs->reduce();
#endif

    //build both sides of the relation if they don't exist
    ObHdl lhs_ob = EX::get_expr(lhs);
    ObHdl rhs_ob = EX::get_expr(rhs);
    if (not (lhs_ob and rhs_ob)) return UNKNOWN;
    return CS::query_reln(*lhs_ob, reln, *rhs_ob);
}
bool MagmaTheory::assume_reln
    (ExprHdl lhs, Relation reln, ExprHdl rhs, bool core)
{//returns true on success
    LOG_DEBUG1( "Assuming " << lhs << " "<< RelationNames[reln] <<" " << rhs );
    LOG_INDENT_DEBUG1
    Assert (lhs->isPure() and rhs->isPure(), "impure expressions");

    ObHdl lhs_ob = EX::get_expr(lhs, core);
    ObHdl rhs_ob = EX::get_expr(rhs, core);
    if (not (lhs_ob and rhs_ob)) {
        logger.warning() << "tried to make assumption about unknown atoms" |0;
        return false;
    }

    if (CS::query_reln(*lhs_ob, reln, *rhs_ob) == TRUE) {
        LOG_DEBUG1( "already true" );
    } else {
        LOG_DEBUG1( "not yet true" );
        CS::assume_reln(*lhs_ob, reln, *rhs_ob);
        Assert1(CS::query_reln(*lhs_ob, reln, *rhs_ob) == TRUE,
                "assumption not true after assumption");
    }
    return true;
}

//======== order-theoretic lattice theory ========

LatticeTheory::LatticeTheory() : MagmaTheory()
{
    logger.debug() << "creating LatticeTheory" |0;
    atom_defs["_"] = &Bot;  required_atoms.insert("_");
    atom_defs["T"] = &Top;  required_atoms.insert("T");
    atom_defs["J"] = &J;    required_atoms.insert("J");
    atom_defs["R"] = &R;
}
void LatticeTheory::init_basis ()
{
    MagmaTheory::init_basis();

    Int i = INJECTIVE, s = SEQUENTIAL, d = DETERMIN;
    if (Bot) O::assumeProps(*Bot, s|d  );
    if (Top) O::assumeProps(*Top,   d  );
    if (J)   O::assumeProps(*J,     d|i);
    if (R)   O::assumeProps(*R,   s|  i);
}
bool LatticeTheory::define_atom (string name)
{
    if (not MagmaTheory::define_atom(name)) return false;

    Int i = INJECTIVE, s = SEQUENTIAL, d = DETERMIN;
    if (name == "_") O::assumeProps(*Bot, s|d  );
    if (name == "T") O::assumeProps(*Top,   d  );
    if (name == "J") O::assumeProps(*J,     d|i);
    if (name == "R") O::assumeProps(*R,     d|i);

    return true;
}

//======== lambda-theory with variables & abstraction ========

 LambdaTheory::LambdaTheory () : LatticeTheory()
{
    logger.debug() << "creating LambdaTheory" |0;

    atom_defs["I"] = &I;  required_atoms.insert("I");
    atom_defs["K"] = &K;  required_atoms.insert("K");
    atom_defs["B"] = &B;  required_atoms.insert("B");
    atom_defs["C"] = &C;  required_atoms.insert("C");
    atom_defs["W"] = &W;  required_atoms.insert("W");
    atom_defs["S"] = &S;  required_atoms.insert("S");

    Int i = INJECTIVE, s = SEQUENTIAL, d = DETERMIN;
    if (I) O::assumeProps(*I, s|d|i);
    if (K) O::assumeProps(*K, s|d|i);
    if (B) O::assumeProps(*B, s|d|i);
    if (C) O::assumeProps(*C, s|d|i);
    if (W) O::assumeProps(*W, s|d  );
    if (S) O::assumeProps(*S, s|d  );
}

//axiom tools
class simple_closure
{
    ExprHdl m_lhs, m_rhs;
    Expr::VarSet m_vars;
public:
    simple_closure (ExprHdl lhs, ExprHdl rhs)
        : m_lhs(lhs), m_rhs(rhs), m_vars(lhs->vars() + rhs->vars())
    {
        for (Expr::VarSet::Iter i=m_vars.begin(); i!=m_vars.end(); ++i) {
            VarHdl var(static_cast<EX::Var*>(*i));
            m_lhs = m_lhs->abstract(var);
            m_rhs = m_rhs->abstract(var);
        }
    }

    Expr::VarSet vars () { return m_vars; }
    ExprHdl lhs () { return m_lhs; }
    ExprHdl rhs () { return m_rhs; }
};

class eta_iterator
{//iterates over Hindley's substitutions: [x/x], [x/a], [x/x a]
    ExprHdl m_lhs, m_rhs, m_lhs_subs, m_rhs_subs;
    Expr::VarSet m_vars;
    Int m_subs;
    VarHdl fresh;

    void _set_subs ();
public:
    eta_iterator (ExprHdl lhs, ExprHdl rhs);

    //iteration
    operator bool () const { return m_subs; }
    bool done () const { return not m_subs; }
    void next () { --m_subs; _set_subs(); }

    //dereferencing
    ExprHdl lhs () { return m_lhs_subs; }
    ExprHdl rhs () { return m_rhs_subs; }
};
eta_iterator::eta_iterator (ExprHdl lhs, ExprHdl rhs)
    : m_lhs(lhs), m_rhs(rhs),
      m_vars(lhs->vars() + rhs->vars()),
      m_subs(powi(3, m_vars.size())), // power of 3
      fresh(EX::Var::get_fresh(m_vars))
{
    next();
}
void eta_iterator::_set_subs ()
{
    m_lhs_subs = m_lhs;
    m_rhs_subs = m_rhs;
    Expr::VarSet::Iter i = m_vars.begin();
    for (int subs = m_subs; subs; ++i, subs/=3) {
        Int subs_type = subs % 3; if (!subs_type) continue;

        VarHdl var(static_cast<EX::Var*>(*i));

        switch (subs_type) {
            case 0:
                m_lhs_subs = m_lhs_subs->substitute(var, fresh);
                m_rhs_subs = m_rhs_subs->substitute(var, fresh);
                break;
            case 1:
                //do nothing
                break;
            case 2:
                m_lhs_subs = m_lhs_subs->substitute(var, var * fresh);
                m_rhs_subs = m_rhs_subs->substitute(var, var * fresh);
                break;
        }
    }
    m_lhs_subs = m_lhs_subs->abstract(fresh);
    m_rhs_subs = m_rhs_subs->abstract(fresh);
}

class closure_perm_iterator
{//iterates over all closing permutations
    ExprHdl m_lhs, m_rhs, m_lhs_abs, m_rhs_abs;
    Expr::VarSet m_vars;
    Expr::VarSet::iterator m_abs;
    closure_perm_iterator* m_iter;
    VarHdl fresh;

    void _set_abs ();
public:
    closure_perm_iterator (ExprHdl lhs, ExprHdl rhs);
    ~closure_perm_iterator () { if (m_iter) delete m_iter; } //for early term'n

    //iteration
    operator bool () const { return m_abs != m_vars.end(); }
    bool done () const { return m_abs == m_vars.end(); }
    void next ();

    //dereferencing
    ExprHdl lhs () { return m_iter ? m_iter->lhs() : m_lhs_abs; }
    ExprHdl rhs () { return m_iter ? m_iter->rhs() : m_rhs_abs; }
};
closure_perm_iterator::closure_perm_iterator (ExprHdl lhs, ExprHdl rhs)
    : m_lhs(lhs), m_rhs(rhs),
      m_vars(lhs->vars() + rhs->vars()),
      m_abs(m_vars.begin()),
      m_iter(NULL),
      fresh(EX::Var::get_fresh(m_vars))
{
    Assert1(not m_vars.empty(),
            "tried to create closure iterator over closed terms");
    _set_abs();
}
void closure_perm_iterator::_set_abs ()
{//abstracts out one of the variables from lhs and rhs
    m_lhs_abs = m_lhs->abstract(static_cast<EX::Var*>(*m_abs));
    m_rhs_abs = m_rhs->abstract(static_cast<EX::Var*>(*m_abs));
    if (m_lhs_abs->isPure() and m_rhs_abs->isPure()) return;
    m_iter = new closure_perm_iterator(m_lhs_abs, m_rhs_abs);
}
void closure_perm_iterator::next ()
{
    if (m_iter) {
        m_iter->next();
        if (not m_iter->done()) return;
        delete m_iter;  m_iter = NULL;
    }
    ++m_abs;
    if (not done()) _set_abs();
}

class closure_map_iterator
{//iterates over all closing abstractions, including variable coincidences
    ExprHdl m_lhs, m_rhs, m_lhs_abs, m_rhs_abs;
    Expr::VarSet m_vars;
    Int m_abs;
    closure_map_iterator* m_iter;
    VarHdl fresh;

    void _set_abs ();
public:
    closure_map_iterator (ExprHdl lhs, ExprHdl rhs);

    //iteration
    operator bool () const { return m_abs; }
    bool done () const { return not m_abs; }
    void next ();

    //dereferencing
    ExprHdl lhs () { return m_iter ? m_iter->lhs() : m_lhs_abs; }
    ExprHdl rhs () { return m_iter ? m_iter->rhs() : m_rhs_abs; }
};
closure_map_iterator::closure_map_iterator (ExprHdl lhs, ExprHdl rhs)
    : m_lhs(lhs), m_rhs(rhs),
      m_vars(lhs->vars() + rhs->vars()),
      m_abs(1<<m_vars.size()), //power of 2
      m_iter(NULL),
      fresh(EX::Var::get_fresh(m_vars))
{
    Assert1(not m_vars.empty(),
            "tried to create closure iterator over closed terms");
    next();
}
void closure_map_iterator::_set_abs ()
{//abstracts out some set of variables from lhs and rhs
    m_lhs_abs = m_lhs;
    m_rhs_abs = m_rhs;
    Expr::VarSet::Iter i = m_vars.begin();
    for (int abs = m_abs; abs; ++i, abs/=2) {
        Int abs_type = abs % 2; if (!abs_type) continue;

        VarHdl var(static_cast<EX::Var*>(*i));
        m_lhs_abs = m_lhs_abs->substitute(var, fresh);
        m_rhs_abs = m_rhs_abs->substitute(var, fresh);
    }
    m_lhs_abs = m_lhs_abs->abstract(fresh);
    m_rhs_abs = m_rhs_abs->abstract(fresh);
}
void closure_map_iterator::next ()
{
    if (m_iter) {
        m_iter->next();
        if (not m_iter->done()) return;
        delete m_iter;  m_iter = NULL;
    }
    if (not --m_abs) return;
    _set_abs();
    if (m_lhs_abs->isPure() and m_rhs_abs->isPure()) return;
    m_iter = new closure_map_iterator(m_lhs_abs, m_rhs_abs);
}

//new virtuals
void LambdaTheory::init_basis () { LatticeTheory::init_basis(); }
bool LambdaTheory::define_atom (string name)
{
    if (not LatticeTheory::define_atom(name)) return false;

    Int i = INJECTIVE, s = SEQUENTIAL, d = DETERMIN;
    if (name == "I") O::assumeProps(*I, s|d|i);
    if (name == "K") O::assumeProps(*K, s|d|i);
    if (name == "B") O::assumeProps(*B, s|d|i);
    if (name == "C") O::assumeProps(*C, s|d|i);
    if (name == "W") O::assumeProps(*W, s|d  );
    if (name == "S") O::assumeProps(*S, s|d  );

    return true;
}
Trool LambdaTheory::query_reln (ExprHdl lhs, Relation reln, ExprHdl rhs,
        unsigned effort)
{//returns true on success
    (effort ? logger.info() : logger.debug())
        << "Checking " << lhs << " " << RelationNames[reln] << " " << rhs |0;
    Logging::IndentBlock block;

#ifdef PEDANTIC_QUERY
    //just convert to combinators
    lhs = lhs->as_comb();
    rhs = rhs->as_comb();
#else
    //convert to combinators, and linear reduce both sides
    lhs = lhs->reduce();
    rhs = rhs->reduce();
#endif

    if (lhs->isPure() and rhs->isPure()) {
        return MagmaTheory::query_reln(lhs, reln, rhs, effort);
    }
    //The statement's truth value is invariant WRT permutations of free vars;
    //thus we can stop as soon as one permutation's closure is known:
    for (closure_perm_iterator c(lhs,rhs); not c.done(); c.next()) {
        Trool result = MagmaTheory::query_reln(c.lhs(), reln, c.rhs(), effort);
        if (result != UNKNOWN) return result;
    }
    return UNKNOWN;
}
bool LambdaTheory::assume_reln
    (ExprHdl lhs, Relation reln, ExprHdl rhs, bool core)
{//returns true on success
    logger.info() << "Assuming "
            << lhs << " " << RelationNames[reln] << " " << rhs |0;
    Logging::IndentBlock block;

    if (lhs->isPure() and rhs->isPure()) {
        return MagmaTheory::assume_reln(lhs, reln, rhs, core);
    }

    //avoid blowup of closures
    unsigned max_vars_to_expand = 3;
    unsigned max_vars_to_permute = 5;
    simple_closure closure(lhs,rhs);
    Expr::VarSet vars = closure.vars();
    if (vars.size() > max_vars_to_permute) {
        logger.warning() << "refusing to permute closure over " << vars |0;;
        return MagmaTheory::assume_reln(
                closure.lhs(), reln, closure.rhs(), core);
    }
    if (vars.size() > max_vars_to_expand) {
        logger.warning() << "refusing to expand closure over " << vars |0;;
        for (closure_perm_iterator c(lhs,rhs); not c.done(); c.next()) {
        if (not MagmaTheory::assume_reln(c.lhs(), reln, c.rhs(), core)) {
            return false;
        } }
    }

    if (IsPositive[reln]) {
        //positive statements are true under any variable substitution
        for (eta_iterator         e(  lhs,    rhs  ); not e.done(); e.next()) {
        for (closure_map_iterator c(e.lhs(),e.rhs()); not c.done(); c.next()) {
        if (not MagmaTheory::assume_reln(c.lhs(), reln, c.rhs(), core)) {
            return false;
        } } }
    } else {
        //negative statements are true only under fully general substitutions
        for (closure_perm_iterator c(lhs,rhs); not c.done(); c.next()) {
        if (not MagmaTheory::assume_reln(c.lhs(), reln, c.rhs(), core)) {
            return false;
        } }
    }

    if (DEBUG_LEVEL >= 4) {
        OR::validate(DEBUG_LEVEL);
    }

    return true;
}

//solving statements
std::vector<Ob> LambdaTheory::solve (StmtHdl stmt, bool tryhard)
{
    logger.info() << "Solving " << stmt |0;
    Logging::IndentBlock block;

    //simplify as much as possible first
    stmt = stmt->query_nf();
    stmt = stmt->map(EX::simplifier());

    //find one free variable.  TODO deal with multiple free vars
    Expr::VarSet vars = stmt->vars();
    if (vars.empty()) {
        logger.error() << "no free variables to solve for" |0;
        return std::vector<Ob>(0);
    }
    if (vars.size() > 1) {
        logger.error() << "too many free variables to solve for: "
                       << vars.size() |0;
        return std::vector<Ob>(0);
    }
    VarHdl var(static_cast<EX::Var*>(vars.any()));
    logger.debug() << "solving for variable " << var |0;

    //use one of two methods to solve equation
    if (tryhard) {
        return solve(var, stmt);
    }
    if (ST::Relationship *reln = stmt->as_reln()) {
        return solve(var, reln);
    }
    logger.warning() << "reverting to slow solver for compound statement" |0;
    return solve(var, stmt);
}
std::vector<Ob> LambdaTheory::solve (VarHdl var, ST::Relationship* r)
{//fast & limited
    ExprHdl lhs = r->lhs();
    ExprHdl rhs = r->rhs();
    Relation rel = r->rel();
    logger.info() << "Solving for " << var << " satisfying relation: "
                  << lhs << ' ' << RelationNames[rel] << ' ' << rhs |0;
    Logging::IndentBlock block;

    //abstract out the free variable
    lhs = EX::simplify(lhs)->abstract(var);
    rhs = EX::simplify(rhs)->abstract(var);
    ObHdl f = EX::get_expr(lhs);
    ObHdl g = EX::get_expr(rhs);
    if (not (f and g)) {
        logger.error() << "failed to solve relation with unknown atoms" |0;
        return std::vector<Ob>(0);
    }

    //solve for {x | f x rel g x}
    std::vector<Ob> result;
    for (AE::LLra_Iterator iter(*f,*g); iter; iter.next()) {
        Ob fx = iter.app1();
        Ob gx = iter.app2();
        Ob x  = iter.rhs();
        if (CS::query_reln(fx, rel, gx) == TRUE) result.push_back(x);
    }
    return result;
}
std::vector<Ob> LambdaTheory::solve (VarHdl var, StmtHdl stmt)
{
    logger.info() << "Solving for " << var << " satisfying statement: "
                  << stmt |0;
    Logging::IndentBlock block;

    //search through all substitution instances
    std::vector<Ob> result;
    for (Ob::sparse_iterator iter=Ob::sbegin(); iter!=Ob::send(); ++iter) {
        Ob x = *iter;
        ExprHdl expr = EX::parse_ob(x);
        StmtHdl instance = stmt->map(EX::substitutor(var, expr));
        if (query(instance,0) == TRUE) result.push_back(x);
    }
    return result;
}
typedef TypedTheory::ObWNum ObWNum;
struct FewerInhabs_
{
    bool operator() (const ObWNum& l, const ObWNum& r) const
    { return l.second<r.second; }
};
std::vector<ObWNum> TypedTheory::type_of
    (std::vector<ExprHdl> inhab_exprs)
{//finds smallest few types of all inhabitants
    logger.info() << "Finding common types of inhabitants" |0;
    Logging::IndentBlock block;

    //eval inhabs
    logger.info() << "evaluating inhabitants" |0;
    std::vector<ObHdl> inhab_obHdls;
    Int Ninhabs = inhab_exprs.size();
    for (Int i=0; i<Ninhabs; ++i) {
        ObHdl inhab = EX::get_expr(inhab_exprs[i]);
        if (!inhab) {
            logger.warning() << "could not parse expr: " << inhab_exprs[i] |0;
            continue;
        }
        inhab_obHdls.push_back(inhab);
    }
    Ninhabs = inhab_obHdls.size();
    std::vector<Ob> inhab_obs;
    for (Int i=0; i<Ninhabs; ++i) {
        inhab_obs.push_back(*(inhab_obHdls[i]));
    }

    //find all types
    logger.info() << "finding all types" |0;
    std::vector<bool> is_type(Ob::capacity(), false);
    for (Ob::sparse_iterator iter=Ob::sbegin(); iter!=Ob::send(); ++iter) {
        Ob t = *iter;
        Ob Vt = AE::find_app(*V, t); if (Vt != t) continue;

        bool inhabits = true;
        for (Int i=0; i<Ninhabs; ++i) {
            Ob x = inhab_obs[i];
            if (AE::find_app(t,x) != x) {
                inhabits = false;
                break;
            }
        }
        if (inhabits) is_type[Int(t)] = true;
    }

    //sort according to size
    logger.info() << "sorting according to total inhabitants" |0;
    std::vector<ObWNum> result;
    for (Int i=0; i<Ob::capacity(); ++i) {
        if (not is_type[i]) continue;
        Int n_inhabs = 0;
        Ob t = Ob(i);
        for (AE::Lra_Iterator tx_iter(t); not tx_iter.done(); tx_iter.next()) {
            Ob x  = tx_iter.rhs();
            Ob tx = tx_iter.app();
            if (tx == x) ++n_inhabs;
        }
        result.push_back(ObWNum(t,n_inhabs));
    }
    std::sort(result.begin(), result.end(), FewerInhabs_());
    return result;
}

//======== extension-theory ========

ExtnTheory::ExtnTheory () : LambdaTheory()
{
    logger.debug() << "creating ExtnTheory" |0;

    //derived atoms, already in <S,K>
    atom_defs["Y"]   = &Y;   required_atoms.insert("Y");
    atom_defs["S'"]  = &S2;

    //additional atoms in <S,K,J>
    atom_defs["U"] = &U;
    atom_defs["V"] = &V;
    atom_defs["P"] = &P;

    //atoms used with types
    atom_defs["inl"]  = &Inl;
    atom_defs["inr"]  = &Inr;
    atom_defs["J'"]   = &J2;
}

void ExtnTheory::init_basis ()
{
    LambdaTheory::init_basis();

    //useful temporaries
    _define_app(CB, *C, *B);
    _define_app(CI, *C, *I);
    _define_app(KI, *K, *I);
    _define_app(SI, *S, *I);
}
bool ExtnTheory::define_atom (string name)
{
    if (not LambdaTheory::define_atom(name)) return false;

    if (name == "J") {}

    return true;
}

//======== typed theory ========

TypedTheory::TypedTheory () : ExtnTheory()
{
    logger.debug() << "creating TypedTheory" |0;

    //types
    atom_defs["div"]   = &Div;
    atom_defs["unit"]  = &Unit;
    atom_defs["semi"]  = &Semi;
    atom_defs["bool"]  = &Bool;
    atom_defs["sum"]   = &Sum;
    atom_defs["maybe"] = &Maybe;
    atom_defs["sset"]  = &Sset;
}

void TypedTheory::init_basis ()
{
    ExtnTheory::init_basis();
}
bool TypedTheory::define_atom (string name)
{
    if (not ExtnTheory::define_atom(name)) return false;

    if (name == "div")  {}
    if (name == "unit") {
        Assert (I,  "unit defined before I");
    }
    if (name == "semi") {
        Assert (I,  "semi defined before I");
    }
    if (name == "bool") {
        Assert (K,  "bool defined before K");
        Assert (KI, "bool defined before F=KI");
    }
    if (name == "maybe") {
        Assert (K, "maybe defined before none=K");
        Assert (Inr, "maybe defined before some=inr");
    }
    if (name == "sum") {
        Assert (Inl, "sum defined before inl");
        Assert (Inr, "sum defined before inr");
    }
    if (name == "sset") {
        Assert (CI, "sum defined before CI");
        Assert (J2, "sum defined before J'");
    }

    return true;
}

}

ostream& operator<< (ostream &os, const LambdaTheories::MagmaTheory::Prop& p)
{
    return os << p.stmt << ". #" << p.comment;
}

