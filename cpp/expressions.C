
#include "expressions.h"
#include "parser.h"
#include <cstdio> //for sprintf
#include <sstream>
#include <deque>
#include <stack>
#include <algorithm> //for find_if
#include <pthread.h> //for syntax lock

//log levels
#define LOG_DEBUG1(mess)
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << mess |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

//params
#define MAX_NAME_SIZE 64

//syntax coordination
namespace Expressions
{
pthread_mutex_t p_syntax_mutex = PTHREAD_MUTEX_INITIALIZER;
}
void _unlock_syntax () { pthread_mutex_unlock( &Expressions::p_syntax_mutex ); }
void _lock_syntax ()   { pthread_mutex_lock  ( &Expressions::p_syntax_mutex ); }

namespace Expressions
{

//================ commonly used atoms ================

ExprHdl Bot, Top, J, R;                         //for Lattice Theories
ExprHdl I, K, B, C, W, S;                       //for Lambda Theories
ExprHdl Y, F, U, V, P;                          //for Extension Theories
ExprHdl Div, Unit, Semi, Bool, Prod;            //for typed theories
ExprHdl Undefined, Erroneous, Blank;            //special exprs

//================ static data ================

Int Expr::s_num_exprs = 0;

#define TABLE(Class) Class::Table Class::s_table;
TABLE( Const )
TABLE( Var )
TABLE( Nat )
TABLE( Sel )
TABLE( App )
TABLE( Comp )
TABLE( Join )
TABLE( Rand )
TABLE( Arrow )
#undef TABLE

//collections of atoms
typedef MAP_TYPE<string, ExprHdl*> ConstDefs;
typedef std::vector<ExprHdl*> MiscConsts;

ConstDefs atom_defs;
MiscConsts misc_atoms;

//================ top-level interface ================

bool g_is_initialized = false;
void define_atom (const string name)
{
    ConstDefs::iterator i = atom_defs.find(name);
    if (i != atom_defs.end()) { //atom is known
        Expr* atom = Const::build(name);
        *(i->second) = atom;
    } else { //atom is unknown
        logger.debug() << "using unfamiliar atom " << name |0;
        misc_atoms.push_back(new ExprHdl(Const::build(name)));
    }
}
struct SameName_
{
    const string *m_name;
    SameName_ (const string& name) : m_name(&name) {}
    bool operator() (const ExprHdl* expr) const
    {
        const string* name = (*expr)->name();
        return name and *name == *m_name;
    }
};
void forget_atom (const string name)
{
    ConstDefs::iterator i = atom_defs.find(name);
    if (i != atom_defs.end()) { //atom is known
        logger.info() << "forgetting known atom" |0;
        ExprHdl& expr = *(i->second);
        expr.clear();
        return;
    }

    MiscConsts::iterator j = std::find_if(
            misc_atoms.begin(), misc_atoms.end(), SameName_(name));
    if (j != misc_atoms.end()) {
        logger.info() << "forgetting misc atom" |0;
        delete *j;
        misc_atoms.erase(j);
        return;
    }

    //LATER:check to make sure no references remain
}
void _initialize ()
{
    logger.debug() << "Initializing expressions" |0;
    Logging::IndentBlock block;
    Assert (not g_is_initialized, "tried to initialize expressions twice");
    Assert (Expr::num_exprs() == 0,
            Expr::num_exprs() << " Exprs exist before initializing");

    //for undefined exprs
    Undefined = build_undef();
    Erroneous = build_erron();
    Blank     = build_blank();

    //for lattice theories
    atom_defs["_"]  = &Bot;
    atom_defs["T"]  = &Top;
    atom_defs["J"]  = &J;
    atom_defs["R"]  = &R;

    //for lambda theories
    //  combinators
    atom_defs["I"] = &I;
    atom_defs["K"] = &K;
    atom_defs["B"] = &B;
    atom_defs["C"] = &C;
    atom_defs["W"] = &W;
    atom_defs["S"] = &S;

    //for extension theories
    atom_defs["Y"] = &Y;
    atom_defs["F"] = &F;
    atom_defs["U"] = &U;
    atom_defs["V"] = &V;
    atom_defs["P"] = &P;

    //for typed theories
    atom_defs["div"]   = &Div;
    atom_defs["unit"]  = &Unit;
    atom_defs["semi"]  = &Semi;
    atom_defs["bool"]  = &Bool;
    atom_defs["prod"]  = &Prod;

    for (ConstDefs::iterator i=atom_defs.begin(); i!=atom_defs.end(); ++i) {
        *(i->second) = Undefined;
    }
}
void initialize () { _initialize(); g_is_initialized = true; }
void initialize (std::vector<string> atoms)
{
    _initialize();
    //define all atoms in the combinatory structure
    for (Int i=0; i<atoms.size(); ++i) {
        define_atom(atoms[i]);
    }
    g_is_initialized = true;
}
inline bool isspace (char c) { return c==' ' or c=='\t' or c=='\n' or c=='\r'; }
void initialize (string atoms)
{
    _initialize();

    //define all atoms in the combinatory structure
    string atom;
    for (Int i=0; i<atoms.size(); ++i) {
        char c = atoms[i];
        if (not isspace(c)) {
            atom.push_back(c);
        } else {
            if (not atom.empty()) {
                define_atom(atom);
                atom.clear();
            }
        }
    }

    g_is_initialized = true;
}
void clear ()
{
    logger.debug() << "Clearing expressions" |0;
    Logging::IndentBlock block;
    Assert (g_is_initialized, "tried to clear expressions twice");

    //for undefined exprs
    Undefined.clear();
    Erroneous.clear();
    Blank.clear();

    //clear defined atoms
    for (ConstDefs::iterator i=atom_defs.begin(); i!=atom_defs.end(); ++i) {
        i->second->clear();
    }
    atom_defs.clear();
    for (Int i=0; i<misc_atoms.size(); ++i) {
        delete misc_atoms[i];
    }
    misc_atoms.clear();

    if (DEBUG_LEVEL and Expr::num_exprs() > 0) {
        logger.warning() << Handling::numHandles() << " handles remain" |0;
        logger.warning() << Handling::numHandledObjects()
            << " handled objects remain" |0;
    }
    Assert (Expr::num_exprs() == 0,
            Expr::num_exprs() << " Exprs remain after clearing");
    g_is_initialized = false;
}

//output
void write_consts_to (ostream& os)
{
    std::vector<string> consts;
    for (Const::Table::iterator i=Const::s_table.begin();
            i!=Const::s_table.end(); ++i) {
        consts.push_back(i->first);
    }

    std::sort(consts.begin(), consts.end(), compare_nocase());

    for (unsigned i=0; i<consts.size(); ++i) {
        os << consts[i] << ' ';
    }
}
void write_stats_to (ostream& os)
{
    os << '\t' << Expr::num_exprs() << "\texpressions\n";
}

//input
PP::Driver g_parser;
string parse_errors;
ExprHdl parse (istream& in, ostream& err)
{
    parse_errors.clear();
    return g_parser.parse_expr(in,err);
}
ExprHdl parse (string s, ostream& err)
{
    std::istringstream in(s);
    return parse(in,err);
}
ExprHdl parse (string s)
{
    std::istringstream in(s);
    std::ostringstream err;
    ExprHdl result = parse(in,err);
    parse_errors = err.str();
    AssertW(parse_errors.empty(), parse_errors);
    return result;
}

//================ factories ================

//buffered binders
ExprHdl Binder::bind (ExprHdl term)
{
    switch (m_type) {
    case Symbols::LAMBDA:  return lambda(term);
    case Symbols::FORALL:  return forall(term);
    case Symbols::EXISTS:  return exists(term);
    default:
        logger.error() << "unknown patt: " << Int(m_type) |0;
        return build_undef();
    }
}
ExprHdl Binder::lambda (ExprHdl term)
{
    logger.debug() << "binding lambda" |0;
    for (; not m_patts.empty(); m_patts.pop_back()) {
        term = m_patts.back()->lambda(term);
    }
    return term;
}
ExprHdl Binder::forall (ExprHdl term)
{
    logger.debug() << "binding forall" |0;
    for (; not m_patts.empty(); m_patts.pop_back()) {
        term = m_patts.back()->forall(term);
    }
    return term;
}
ExprHdl Binder::exists (ExprHdl term)
{
    logger.debug() << "binding forall" |0;
    for (; not m_patts.empty(); m_patts.pop_back()) {
        term = m_patts.back()->exists(term);
    }
    return term;
}

//================ factories ================

ExprHdl close (ExprHdl a) { return V * a; }
ExprHdl above (ExprHdl a) { return J * a; }
ExprHdl power (ExprHdl a) { return P * a; }
ExprHdl semi  (ExprHdl i) { return (Semi!=Undefined) ? Semi * i : Undefined; }
ExprHdl test  (ExprHdl t) { return (Semi!=Undefined) ? Semi % t : Undefined; }
ExprHdl tests (ExprHdl s, ExprHdl t) { return S * test(s) * test(t); }
ExprHdl id () { return I; }

ExprHdl lam_blank (ExprHdl in) { return new Lambda(new BlankPatt(),in); }
ExprHdl lam (const std::vector<VarHdl>& vars, ExprHdl in)
{
    ExprHdl result = in;
    for (Int i=0; i<vars.size(); ++i) {
        result = lam(vars[vars.size()-i-1], result);
    }
    return result;
}

Expr::VarSet Vector::all_vars (const std::vector<ExprHdl>& terms)
{
    VarSet result = VarSet::empty_set();
    for (Int i=0; i<terms.size(); ++i) {
        result += terms[i]->vars();
    }
    return result;
}

ExprHdl build_tuple (const std::vector<ExprHdl>& terms)
{
    if (terms.size() < 2) return Erroneous;
    unsigned n = terms.size() - 2;
    ExprHdl result = build_pair(terms[n], terms[n+1]);
    while (n>0) result = build_pair(terms[--n], result);
    return result;
}
PattHdl build_tuple (const std::vector<PattHdl>& patts)
{
    Assert (patts.size() > 0, "no patterns in tuple");
    if (patts.size() == 1) return patts[0];
    unsigned n = patts.size() - 2;
    PattHdl result = build_pair(patts[n], patts[n+1]);
    while (n>0) result = build_pair(patts[--n], result);
    return result;
}

Var* Var::build (const string& name)
{
    LOG_DEBUG1( "building Var" );
    Table::iterator pos = s_table.find(name);
    return pos != s_table.end() ? pos->second : new Var(name);
}
Nat* Nat::build (Int num)
{
    LOG_DEBUG1( "building Var" );
    Table::iterator pos = s_table.find(num);
    return pos != s_table.end() ? pos->second : new Nat(num);
}
Sel* Sel::build (Int n, Int N)
{
    LOG_DEBUG1( "building Var" );
    Table::iterator pos = s_table.find(std::make_pair(n,N));
    return pos != s_table.end() ? pos->second : new Sel(n,N);
}
Const* Const::build (const string& name)
{
    LOG_DEBUG1( "building Const" );
    Table::iterator pos = s_table.find(name);
    return pos != s_table.end() ? pos->second : new Const(name);
}
App* App::build (const ExprHdl& lhs, const ExprHdl& rhs)
{
    LOG_DEBUG1( "building App" );
    Table::iterator pos = _find(lhs,rhs);
    return pos != s_table.end() ? pos->second : new App(lhs,rhs);
}
Comp* Comp::build (const ExprHdl& lhs, const ExprHdl& rhs)
{
    LOG_DEBUG1( "building Comp" );
    Table::iterator pos = _find(lhs,rhs);
    return pos != s_table.end() ? pos->second : new Comp(lhs,rhs);
}
Join* Join::build (const ExprHdl& lhs, const ExprHdl& rhs)
{
    LOG_DEBUG1( "building Join" );
    Table::iterator pos = _find(lhs,rhs);
    return pos != s_table.end() ? pos->second : new Join(lhs,rhs);
}
Rand* Rand::build (const ExprHdl& lhs, const ExprHdl& rhs)
{
    LOG_DEBUG1( "building Rand" );
    Table::iterator pos = _find(lhs,rhs);
    return pos != s_table.end() ? pos->second : new Rand(lhs,rhs);
}
Arrow* Arrow::build (const ExprHdl& lhs, const ExprHdl& rhs)
{
    LOG_DEBUG1( "building Arrow" );
    Table::iterator pos = _find(lhs,rhs);
    return pos != s_table.end() ? pos->second : new Arrow(lhs,rhs);
}

Special* Special::s_table[1+NUM_SPECIAL_TYPES] =
{ NULL, NULL, NULL, NULL };
const char* SpecialNames[1+NUM_SPECIAL_TYPES] =
{ "", "???", "XXX", "-" };
Special* Special::build (SpecialType type)
{
    Assert (1 <= type and type <= NUM_SPECIAL_TYPES, "invalid special type");
    return s_table[type] ? s_table[type] : new Special(type);
}

//fresh variables
string get_varString (Int i)
{
    static char varName[MAX_NAME_SIZE];
    Assert (i<10000, "too many variables");
    char a = "abcdefghijklmnopqrstuvwxyz"[i%26];
    i /= 26;
    if (i) sprintf(varName, "%c%i", a, i);
    else   sprintf(varName, "%c", a);
    return varName;
}
VarHdl Var::get_fresh (const VarSet& vars)
{//returns a locally fresh variable
    for (Int i=0; true; ++i) {
        string varString = get_varString(i);
        VarHdl var = build(varString);
        if (Const::find(varString)) continue;
        if (vars.contains(static_cast<void*>(&*var))) continue;
        return var;
    }
}
typedef std::vector<VarHdl> VarList;
VarList Var::get_fresh (const VarSet& vars, Int num)
{//returns a list of locally fresh variables
    VarList result;
    for (Int i=0,n=0; n<num; ++i) {
        string varString = get_varString(i);
        VarHdl var = build(varString);
        if (Const::find(varString)) continue;
        if (vars.contains(static_cast<void*>(&*var))) continue;
        result.push_back(var);
        ++n;
    }
    return result;
}

//======== dynamic typing ========

//conversion to combinator
//  combinators are: constants, variables, and applications
ExprHdl Const::as_comb () { return this; }
ExprHdl Var::as_comb () { return this; }
ExprHdl App::as_comb ()
{
    ExprHdl lhs = m_lhs->as_comb(); if (lhs->isBad()) return bad();
    ExprHdl rhs = m_rhs->as_comb(); if (rhs->isBad()) return bad();
    return lhs * rhs;
}
ExprHdl Comp::as_comb ()
{
    ExprHdl lhs = m_lhs->as_comb(); if (lhs->isBad()) return bad();
    ExprHdl rhs = m_rhs->as_comb(); if (rhs->isBad()) return bad();
    return lhs % rhs;
}
ExprHdl Join::as_comb ()
{
    ExprHdl lhs = m_lhs->as_comb(); if (lhs->isBad()) return bad();
    ExprHdl rhs = m_rhs->as_comb(); if (rhs->isBad()) return bad();
    return m_lhs->as_comb() | m_rhs->as_comb();
}

//  all others are converted to consts, vars, and apps.
ExprHdl Nat::as_comb ()
{//binary form
    ExprHdl Zero = K * I,
            One = I,
            Two = W * B,
            Succ = S * B,
            Double = B * Two; //i.e. bitshift

    //check for degeneracy
    if (m_num == 0) return Zero;
    if (m_num == 1) return One;

    //remove leading zeros
    Int this_bit = 1<<31;
    while (!(m_num & this_bit)) this_bit >>= 1;

    //construct binary expansion
    ExprHdl result = Two;
    this_bit >>= 1;
    if (m_num & this_bit) result = Succ * result;
    while (this_bit >>= 1) {
        result = Double * result;
        if (m_num & this_bit) result = Succ * result;
    }

    return result;
}
ExprHdl Sel::as_comb()
{
    Int n = m_num, N = m_out_of;
    Assert (1 <= n and n <= N, "bad selector");

    if (n == N) {
        ExprHdl result = I;
        for (Int i=1; i<n; ++i) result = K * result;
        return result;
    }

    ExprHdl result = K;
    Int i=N;
    for (--i; i>n; --i) result = K % result;
    for (--i; i>0; --i) result = K * result;
    return result;
}
ExprHdl Rand::as_comb ()
{
    ExprHdl lhs = m_lhs->as_comb(); if (lhs->isBad()) return bad();
    ExprHdl rhs = m_rhs->as_comb(); if (rhs->isBad()) return bad();
    return R * m_lhs->as_comb() * m_rhs->as_comb();
}
ExprHdl Arrow::as_comb ()
{
    ExprHdl lhs = m_lhs->as_comb(); if (lhs->isBad()) return bad();
    ExprHdl rhs = m_rhs->as_comb(); if (rhs->isBad()) return bad();
    //any of these will work
    //return C * (B * B * (B * rhs)) * lhs;
    //return B * (B * rhs) * (C * B * lhs);
    return (B * rhs) % (C * B * lhs);
}

bool Pattern::isBad () const
{
    if (m_type and m_type->isBad()) return true;
    if (m_test and m_test->isBad()) return true;
    if (m_pred and m_pred->isBad()) return true;
    return false;
}
bool BlankPatt::isBad () const
{
    //blank patterns should not have any types or tests
    return m_type or m_test or m_pred;
}
bool VectPatt::isBad () const
{
    for (unsigned i=0; i<m_patts.size(); ++i) {
        if (m_patts[i]->isBad()) return true;
    }
    return Pattern::isBad();
}

PattHdl BlankPatt::as_comb () { return this; }
PattHdl VarPatt::as_comb ()
{
    if (!m_type and !m_test and !m_pred) return this;

    VarPatt *result = new VarPatt(m_var);
    if (m_type) result->type(m_type->as_comb());
    if (m_test) result->test(m_test->as_comb());
    if (m_pred) result->pred(m_pred->as_comb());
    return result;
}
PattHdl VectPatt::as_comb ()
{
    VectPatt *result = new VectPatt();
    for (unsigned i=0; i<m_patts.size(); ++i) {
        result->push(m_patts[i]->as_comb());
    }
    if (m_type) result->type(m_type->as_comb());
    if (m_test) result->test(m_test->as_comb());
    if (m_pred) result->pred(m_pred->as_comb());
    return result;
}

ExprHdl Definition::as_comb ()
{
    return m_in->where(m_patt, m_mean);
}
ExprHdl Lambda::as_comb () { return m_patt->lambda(m_in)->as_comb(); }
ExprHdl Forall::as_comb () { return m_patt->forall(m_in)->as_comb(); }
ExprHdl Exists::as_comb () { return m_patt->exists(m_in)->as_comb(); }

ExprHdl Vector::as_comb ()
{
    if (m_terms.empty()) return I;

    ExprHdl result = C * I * m_terms[0]->as_comb();
    for (unsigned i=1; i< m_terms.size(); ++i) {
        result = C * result * m_terms[i]->as_comb();
    }
    return result;
}

ExprHdl Special::as_comb () { return m_type == BLANK ? Bot : bad(); }

//purification
ExprHdl Expr::as_pure () { return as_comb()->as_pure(); }
ExprHdl Const::as_pure () { return this; }
ExprHdl Var::as_pure () { return bad(); }
ExprHdl App::as_pure ()
{
    ExprHdl lhs = m_lhs->as_pure(); if (lhs->isBad()) return bad();
    ExprHdl rhs = m_rhs->as_pure(); if (rhs->isBad()) return bad();
    return lhs * rhs;
}
ExprHdl Comp::as_pure ()
{
    ExprHdl lhs = m_lhs->as_pure(); if (lhs->isBad()) return bad();
    ExprHdl rhs = m_rhs->as_pure(); if (rhs->isBad()) return bad();
    return lhs % rhs;
}
ExprHdl Join::as_pure ()
{
    ExprHdl lhs = m_lhs->as_pure(); if (lhs->isBad()) return bad();
    ExprHdl rhs = m_rhs->as_pure(); if (rhs->isBad()) return bad();
    return lhs | rhs;
}

//========= patterns ========

//two levels of safety
#define WITH1(a) (a!=Undefined)
//#define WITH1(a) (false)
//#define WITH2(a) (a!=Undefined)
#define WITH2(a) (false)

ExprHdl Pattern::type () const
{
    if (!m_type) return I;
    ExprHdl result = m_type;
    if WITH1(V) result = V * result;
    return result;
}
ExprHdl Pattern::test () const
{
    if (!m_test) return K * I;
    ExprHdl result = m_test;
    if WITH1(Semi) result = Semi % result;
    return result;
}
ExprHdl Pattern::pred () const
{
    if (!m_pred) return K * K;
    ExprHdl result = m_pred;
    if WITH1(Bool) result = Bool % result;
    return result;
}
ExprHdl Pattern::wrap_in (ExprHdl in) const
{
    Assert (!m_pred, "not implemented");

    ExprHdl result = in;
    if (m_test) result = S * test() * result;
    if (m_type) result = result % type();
    return result;
}
ExprHdl Pattern::wrap_patt (ExprHdl p) const
{
    Assert (!m_pred, "not implemented");

    ExprHdl result = p;
    if (m_type) result = type() * result;
    if (m_test) result = W * test() * result;
    return result;
}

ExprHdl BlankPatt::expr () const { return Bot; }
ExprHdl VarPatt::expr () const { return &*m_var; }
ExprHdl VectPatt::expr () const
{
    std::vector<ExprHdl> exprs;
    for (unsigned i=0; i<m_patts.size(); ++i) {
        exprs.push_back(m_patts[i]->expr());
    }
    return build_vector(exprs);
}


ExprHdl BlankPatt::lambda (ExprHdl in) const { return K * in; }
ExprHdl VarPatt::lambda (ExprHdl in) const
{
    return wrap_in(in->abstract(m_var));
}
ExprHdl VectPatt::lambda (ExprHdl in) const
{
    ExprHdl result = in;
    for (unsigned i=m_patts.size(); i>0; --i) {
        result = m_patts[i-1]->lambda(result);
    }
    result = C * I * result;
    //LATER
    //unsigned N = m_patt.size;
    //if WITH2(Vect) result = (Vect * build_nat(N)) % result;
    return wrap_in(result);
}
ExprHdl Pattern::forall (ExprHdl in) const
{// /\i. M = \x,i. V M(x i) = S (\i. V M)
    if WITH1(V) in = V * in;
    ExprHdl i = expr();
    VarHdl  x = Var::get_fresh(vars() + in->vars());
    PattHdl x_ = new VarPatt(x);
    ExprHdl result = in * (x * i);
    result = lambda(result);
    result = x_->lambda(result);
    return result;
}
ExprHdl Pattern::exists (ExprHdl in) const
{// \/i. M = V \(i,x):prod. (i,V M x)
    ExprHdl i = expr();
    VarHdl  x = Var::get_fresh(vars() + in->vars());
    ExprHdl ix = build_pair(i,x);
    PattHdl i_ = PattHdl(const_cast<Pattern*>(this));
    PattHdl x_ = new VarPatt(x);            x_->type(in);
    PattHdl ix_ = build_pair(i_, x_);       if WITH2(Prod) ix_->type(Prod);
    ExprHdl result = ix_->lambda(ix);       if WITH1(V) result = V * result;
    return result;
}

bool VarPatt::define (Definable& def, ExprHdl meaning) const
{
    AssertW(!m_type, "ignoring type in patterned definition");
    AssertW(!m_test, "ignoring test in patterned definition");
    AssertW(!m_pred, "ignoring pred in patterned definition");

    return def(m_var, meaning);
}
bool VectPatt::define (Definable& def, ExprHdl meaning) const
{
    AssertW(!m_type, "ignoring type in patterned definition");
    AssertW(!m_test, "ignoring test in patterned definition");
    AssertW(!m_pred, "ignoring pred in patterned definition");

    for (unsigned n=0,N=m_patts.size(); n<N; ++n) {
        ExprHdl meaning_n = meaning * build_sel(n+1,N)->as_comb();
        if (not m_patts[n]->define(def, meaning_n)) return false;
    }
    return true;
}

#undef WITH1
#undef WITH2

//========= substitution ========

//substitution
ExprHdl Const::substitute (VarHdl var, ExprHdl expr) { return this; }
ExprHdl Var::substitute (VarHdl var, ExprHdl expr)
{
    return (*this == *var) ? expr : this;
}
ExprHdl App::substitute (VarHdl var, ExprHdl expr)
{
    if (not contains(var)) return this;
    return m_lhs->substitute(var, expr)
         * m_rhs->substitute(var, expr);
}
ExprHdl Comp::substitute (VarHdl var, ExprHdl expr)
{
    if (not contains(var)) return this;
    return m_lhs->substitute(var, expr)
         % m_rhs->substitute(var, expr);
}
ExprHdl Join::substitute (VarHdl var, ExprHdl expr)
{
    if (not contains(var)) return this;
    return m_lhs->substitute(var, expr)
         | m_rhs->substitute(var, expr);
}
ExprHdl Expr::substitute (VarHdl var, ExprHdl expr)
{
    return as_comb()->substitute(var,expr);
}

//abstract definition
ExprHdl Var::where (VarHdl var, ExprHdl mean)
{
    return *this == *var ? mean : this;
}
ExprHdl App::where (VarHdl var, ExprHdl def)
{//SW-eta definition
    if (m_lhs->contains(var)) {
        if (m_rhs->contains(var)) {
            if (m_rhs == var)       return W * m_lhs->abstract(var) * def;
            else                    return S * m_lhs->abstract(var)
                                             * m_rhs->abstract(var) * def;
        } else                      return m_lhs->where(var,def) * m_rhs;
    } else {
        if (m_rhs->contains(var))   return m_lhs * m_rhs->where(var,def);
        else                        return this;
    }
}
ExprHdl Comp::where (VarHdl var, ExprHdl def)
{//S-B-compose definition
    if (m_lhs->contains(var)) {
        if (m_rhs->contains(var))   return S * (B % m_lhs->abstract(var))
                                             * m_rhs->abstract(var) * def;
        else                        return m_lhs->where(var,def) % m_rhs;
    } else {
        if (m_rhs->contains(var))   return m_lhs % m_rhs->where(var,def);
        else                        return this;
    }
}
ExprHdl Join::where (VarHdl var, ExprHdl def)
{//join definition
    if (m_lhs->contains(var)) {
        if (m_rhs->contains(var))   return ( m_lhs->abstract(var)
                                           | m_rhs->abstract(var) ) * def;
        else                        return m_lhs->where(var,def) | m_rhs;
    } else {
        if (m_rhs->contains(var))   return m_lhs | m_rhs->where(var,def);
        else                        return this;
    }
}
ExprHdl Const::where   (VarHdl, ExprHdl) { return this; }
ExprHdl Expr::where (VarHdl var, ExprHdl def)
{
    return as_comb()->where(var, def);
}
ExprHdl Expr::where (PattHdl patt, ExprHdl mean)
{
    return (patt->lambda(this) * mean)->as_comb();
}

//======== abstraction ========

ExprHdl Const::abstract (VarHdl var) { return K * this; }
ExprHdl Var::abstract (VarHdl var) { return (*var == *this) ? I : K * this; }
ExprHdl App::abstract (VarHdl var)
{//IKCSW-compose-eta abstraction
    if (not contains(var))          return K * this;
    if (m_lhs->contains(var)) {
        ExprHdl lhs_abs = m_lhs->abstract(var);
        if (m_rhs->contains(var)) {
            if (*m_rhs == *var)     return W * lhs_abs;
            else                    return S * lhs_abs * m_rhs->abstract(var);
        }
        else                        return C * lhs_abs * m_rhs;
    } else {
        Assert (m_rhs->contains(var),
                "var in neither lhs nor rhs");
        if (*m_rhs == *var)         return m_lhs;
        else                        return m_lhs % m_rhs->abstract(var);
    }
}
ExprHdl Comp::abstract (VarHdl var)
{//KBCS-compose-eta abstraction
    if (not contains(var))          return K * this;
    if (m_lhs->contains(var)) {
        ExprHdl lhs_abs = m_lhs->abstract(var);
        if (m_rhs->contains(var))   return S * (B % lhs_abs)
                                             * m_rhs->abstract(var);
        else                        return (C * B * m_rhs) % lhs_abs;
    } else {
        Assert (m_rhs->contains(var),
                "var in neither lhs nor rhs");
        if (*m_rhs == *var)         return B * m_lhs;
        else                        return (B * m_lhs) % m_rhs->abstract(var);
    }
}
ExprHdl Join::abstract (VarHdl var)
{//K-compose-eta abstraction
    if (not contains(var))          return K * this;

    //Version 1: produces lots of K's and joins
    //                                return m_lhs->abstract(var)
    //                                     | m_rhs->abstract(var);

    //Version 2: produces lots of J's and compositions
    if (m_lhs->contains(var)) {
        if (m_rhs->contains(var))   return m_lhs->abstract(var)
                                         | m_rhs->abstract(var);
        if (*m_lhs == *var)         return J * m_rhs;
        else                        return (J * m_rhs) % m_lhs->abstract(var);
    } else {
        Assert (m_rhs->contains(var),
                "var in neither lhs nor rhs");
        if (*m_rhs == *var)         return J * m_lhs;
        else                        return (J * m_lhs) % m_rhs->abstract(var);
    }
}
ExprHdl Expr::abstract (VarHdl var) { return as_comb()->abstract(var); }

//======== combinator reduction ========

//curried function for head beta-eta-reduction
//TODO: fork joins into two separate reducers
//  this requires major refactoring:
//    an abstract Reduce type with two subclasses: ReduceStack and ReduceJoin
class Reduce
{
    ExprHdl m_fun;
    std::vector<ExprHdl> m_args;
    bool m_sampling;

    //arguments are numbered 1,...,N
    Int nargs () const { return m_args.size(); }
    ExprHdl& arg (Int i)       { return m_args[nargs()-i]; }
    ExprHdl  arg (Int i) const { return m_args[nargs()-i]; }

    //stack control
    ExprHdl top () const { return m_args.back(); }
    ExprHdl pop () { ExprHdl r = m_args.back(); m_args.pop_back(); return r; }
    void push (ExprHdl expr) { m_args.push_back(expr); }
    void clear () { m_args.clear(); }
    void set_fun (ExprHdl term)
    {
        while (term->isApp()) { push(term->rhs()); term = term->lhs(); }
        m_fun = term;
    }

    //expr <--> stack conversion
    ExprHdl as_expr ()
    {
        ExprHdl result = m_fun;
        for (Int i=1; i<=nargs(); ++i) result = result * arg(i);
        return result;
    }

    //reduction
    enum StepType { NO_STEP, LIN_STEP, NONLIN_STEP };
    StepType lin_step ();
    StepType nonlin_step ();

    bool linear_0 ();
    bool linear_1 ();
    bool linear_2 ();
    bool linear_3 ();
    bool nonlin_1 ();
    bool nonlin_2 ();
    bool nonlin_3 ();
public:

    //interface
    Reduce (Expr*    term, bool s=false) : m_sampling(s) { set_fun(term); }
    Reduce (ExprHdl& term, bool s=false) : m_sampling(s) { set_fun(term); }
    ExprHdl operator() (Int steps);
};

//reduction rules
bool Reduce::linear_0 ()
{
    if (m_fun->isJoin()) {
        ExprHdl lhs(m_fun->lhs());
        ExprHdl rhs(m_fun->rhs());

        if (lhs == Top) {            //(|) Top|x --> Top
            set_fun(Top);
            return true;
        }
        if (rhs == Top) {            //(|) Top|x --> Top
            set_fun(Top);
            return true;
        }
        if (lhs == Bot) {            //(|) _|x --> x
            set_fun(rhs);
            return true;
        }
        if (rhs == Bot) {            //(|) x|_ --> x
            set_fun(lhs);
            return true;
        }
        if (lhs == rhs) {             //(|) x|x --> x
            set_fun(lhs);
            return true;
        }
    }
    return false;
}
bool Reduce::linear_1 ()
{
    if (m_fun->isComp()) {           //(*) (x*y) z --> x (y z)
        ExprHdl a1(m_fun->lhs());
        ExprHdl a2(m_fun->rhs());
        ExprHdl a3 = pop();
        push(a2 * a3);
        set_fun(a1);
        return true;
    }
    if (m_fun == Bot) {             //(Bot1) Bot x1 ... xn --> Bot
        clear();
        return true;
    }
    if (m_fun == Top) {             //(Top1) Top x1 ... xn --> Top
        clear();
        return true;
    }
    if (m_fun == I) {               //(I1) I x --> x
        ExprHdl a1 = pop();
        set_fun(a1);
        return true;
    }
    if (m_fun == J) {
        ExprHdl a1 = top();
        if (a1 == Top) {            //(J1a) J Top --> Top
            pop();
            set_fun(Top);
            return true;
        }
        if (a1 == Bot) {            //(J1b) J Bot --> I
            pop();
            set_fun(I);
            return true;
        }
    }
    if (m_fun == V) {
        ExprHdl a = top();
        if (a == V) {               //(V1a) V V --> V
            pop();
            return true;
        }
        if (a->lhs() == &*V) {      //(V1b) V(V x) --> V x
            pop();
            set_fun(a);
            return true;
        }
    }
    if (m_fun == U) {
        ExprHdl a = top();
        if (a == U) {               //(U1a) U U --> U
            pop();
            return true;
        }
        if (a->lhs() == &*U) {      //(V1b) U(U x) --> U x
            pop();
            set_fun(a);
            return true;
        }
    }
    if (m_sampling and m_fun == R) {    //(R1) R x --> K x @ 1/2
        set_fun(random_bit() ? K : F);  //           + F x @ 1/2
    }
    return false;
}
bool Reduce::linear_2 ()
{
    if (m_fun == K) {               //(K2) K x y --> x
        ExprHdl a1 = pop();
        pop();
        set_fun(a1);
        return true;
    }
    if (m_fun == F) {               //(F2) F x y --> y
        pop();
        ExprHdl a2 = pop();
        set_fun(a2);
        return true;
    }
    if (m_fun == J) {               //(|) J x y --> x|y
        ExprHdl a1 = pop();
        ExprHdl a2 = pop();
        set_fun(a1 | a2);
        return true;
    }
    if (m_fun == B) {               //(*) B x y --> x*y
        ExprHdl a1 = pop();
        ExprHdl a2 = pop();
        set_fun(a1 % a2);
        return true;
    }
    if (m_fun == R) {
        ExprHdl a1 = arg(1);
        ExprHdl a2 = arg(2);
        if (a1 == a2) {             //(R2) R x x --> x
            pop(); pop();
            set_fun(a1);
            return true;
        }
    }

    return false;
}
bool Reduce::linear_3 ()
{
    if (m_fun == C) {               //(C3) C x y z --> x z y
        ExprHdl a1 = pop();
        ExprHdl a2 = pop();
        ExprHdl a3 = pop();
        push(a2);
        push(a3);
        set_fun(a1);
        return true;
    }

    return false;
}
bool Reduce::nonlin_1 ()
{
    if (m_fun->isJoin()) {           //(|) (x|y) z --> x z|y z
        ExprHdl a1(m_fun->lhs());
        ExprHdl a2(m_fun->rhs());
        ExprHdl a3 = pop();
        a1 = a1 * a3;
        a2 = a2 * a3;
        set_fun(a1 | a2);
        return true;
    }
    if (m_fun == Y) {               //(Y1) Y x --> x(Y x)
        ExprHdl a1 = pop();
        push(Y * a1);
        set_fun(a1);
        return true;
    }
    if (m_fun == V) {               //(V1c) V f --> I|f*(V f)
        ExprHdl f = pop();
        push(B*f*(V*f));
        push(I);
        set_fun(J);
        return true;
    }
    if (m_fun == U) {               //(U1c) U f --> f*(U f)|f
        ExprHdl f = top();
        push(B*f*(U*f));
        set_fun(J);
        return true;
    }

    return false;
}
bool Reduce::nonlin_2 ()
{
    if (m_fun == W) {               //(W2) W x y --> x y y
        ExprHdl a1 = pop();
        ExprHdl a2 = top();
        if (a1 == W and a2 == W) {  //(WWW) W W W --> Bot
            clear();
            set_fun(Bot);
            return true;
        }
        push(a2);
        set_fun(a1);
        return true;
    }

    return false;
}
bool Reduce::nonlin_3 ()
{
    if (m_fun == S) {               //(S3) S x y z --> x z(y z)
        ExprHdl a1 = pop();
        ExprHdl a2 = pop();
        ExprHdl a3 = pop();
        push( (a2*a3)->_reduce() );  //keep in linear NF
        push(a3);
        set_fun(a1);
        return true;
    }
    if (m_fun == R) {               //(R3) (x+y)z --> x z+y z
        ExprHdl a1 = pop();
        ExprHdl a2 = pop();
        ExprHdl a3 = pop();
        push(a2 * a3);
        push(a1 * a3);
        return true;
    }

    return false;
}

//stepping
Reduce::StepType Reduce::lin_step ()
{//returns true on success
    Int N = nargs();
                                if (linear_0()) return LIN_STEP;
    if (N < 1) return NO_STEP;  if (linear_1()) return LIN_STEP;
    if (N < 2) return NO_STEP;  if (linear_2()) return LIN_STEP;
    if (N < 3) return NO_STEP;  if (linear_3()) return LIN_STEP;

    return NO_STEP;
}
Reduce::StepType Reduce::nonlin_step ()
{//returns true on success
    Int N = nargs();

    if (linear_0()) return LIN_STEP;

    if (N < 1) return NO_STEP;
    if (linear_1()) return LIN_STEP;
    if (nonlin_1()) return NONLIN_STEP;

    if (N < 2) return NO_STEP;
    if (linear_2()) return LIN_STEP;
    if (nonlin_2()) return NONLIN_STEP;

    if (N < 3) return NO_STEP;
    if (linear_3()) return LIN_STEP;
    if (nonlin_3()) return NONLIN_STEP;

    return NO_STEP;
}
ExprHdl Reduce::operator() (Int steps)
{
    LOG_INDENT_DEBUG1
    for (bool done=false; m_fun->isConst() and not done;) {
        if (steps) {
            switch (nonlin_step()) {
                case NONLIN_STEP:
                    --steps;
                    LOG_DEBUG1("-nonlin-> " << as_expr());
                    break;
                case LIN_STEP:
                    LOG_DEBUG1("-linear-> " << as_expr());
                    break;
                case NO_STEP:
                    done = true;
            }
        } else {
            if (lin_step()) {
                LOG_DEBUG1("-linear-> " << as_expr());
            } else {
                done = true;
            }
        }
    }
    if (m_fun->isComp()) {
        ExprHdl lhs(m_fun->lhs()); lhs = lhs->_reduce(steps);
        ExprHdl rhs(m_fun->rhs()); rhs = rhs->_reduce(steps);
        set_fun(lhs % rhs);
    }
    if (m_fun->isJoin()) {
        ExprHdl lhs(m_fun->lhs()); lhs = lhs->_reduce(steps);
        ExprHdl rhs(m_fun->rhs()); rhs = rhs->_reduce(steps);
        set_fun(lhs | rhs);
    }
    for (Int i=1; i<=nargs(); ++i) {
        arg(i) = arg(i)->_reduce(steps);
    }
    return as_expr();
}

//reduction
ExprHdl Const::_reduce (Int steps) { return this; }
ExprHdl Var::_reduce   (Int steps) { return this; }
ExprHdl App::_reduce   (Int steps) { return Reduce(this)(steps); }
ExprHdl Comp::_reduce  (Int steps) { return Reduce(this)(steps); }
ExprHdl Join::_reduce  (Int steps) { return Reduce(this)(steps); }
ExprHdl Expr::_reduce  (Int steps) { return bad(); }

ExprHdl Const::_sample (Int steps)
{
    if (R and this == &*R) return random_bit() ? K : F;
    return this;
}
ExprHdl Var::_sample   (Int steps) { return this; }
ExprHdl App::_sample   (Int steps) { return Reduce(this,true)(steps); }
ExprHdl Comp::_sample  (Int steps) { return Reduce(this,true)(steps); }
ExprHdl Join::_sample  (Int steps) { return Reduce(this,true)(steps); }
ExprHdl Expr::_sample  (Int steps) { return bad(); }

//================ structure ================

ExprHdl substitutor::operator() (const ExprHdl& expr) const
{
    return expr->substitute(m_old, m_new);
}

//StructlFuns
void Expr   ::_called_by (StructlFun& fun) { fun.of_pretty(*this); }
void Const  ::_called_by (StructlFun& fun) { fun.of_atom(m_name); }
void Var    ::_called_by (StructlFun& fun) { fun.of_var (this); }
void App    ::_called_by (StructlFun& fun) { fun.of_app (*m_lhs, *m_rhs); }
void Comp   ::_called_by (StructlFun& fun) { fun.of_comp (*m_lhs, *m_rhs); }
void Join   ::_called_by (StructlFun& fun) { fun.of_join (*m_lhs, *m_rhs); }
void Special::_called_by (StructlFun& fun) { fun.of_special(m_type); }

//======== comb -> pretty ========

class Decompile
{
    //function and args
    typedef std::deque<ExprHdl> Exprs;
    Exprs m_exprs;
    Expr::VarSet m_vars;
    static unsigned s_steps;

    //buffered binders
    struct Binder {
        virtual ExprHdl bind (ExprHdl in) = 0;
        virtual ~Binder () {}
    };
    struct Lambda : public Binder
    {
        VarHdl m_var;
        Lambda (VarHdl var) : m_var(var) {}
        virtual ExprHdl bind (ExprHdl in)
        {
            return in->contains(m_var) ? lam(m_var,in) : lam_blank(in);
        }
        virtual ~Lambda () {}
    };
    struct Def : public Binder
    {
        VarHdl m_var;
        ExprHdl m_def;
        Def (VarHdl var, ExprHdl def) : m_var(var), m_def(def) {}
        virtual ExprHdl bind (ExprHdl in)
        {
            return in->contains(m_var) ? let(m_var,m_def,in) : in;
        }
        virtual ~Def () {}
    };
    std::stack<Binder*> m_binders;

    //methods
    Int args () const { return m_exprs.size()-1; }
    ExprHdl arg (Int pos)
    {
        Exprs::iterator i = m_exprs.begin();
        for (Int j=0; j<pos; ++j) if (++i == m_exprs.end()) return ExprHdl();
        return *i;
    }
    ExprHdl& front () { return m_exprs.front(); }
    ExprHdl pop ()
    { ExprHdl result = front();  m_exprs.pop_front();  return result; }
    void push_arg (ExprHdl expr) { m_exprs.push_front(expr); }
    void set_fun (ExprHdl expr)
    {
        while (expr->isApp()) {
            m_exprs.push_front(expr->rhs());
            expr = expr->lhs();
        }
        if (expr->isComp()) {
            m_exprs.push_front(expr->rhs());
            m_exprs.push_front(expr->lhs());
            expr = B;
        }
        if (expr->isJoin()) {
            m_exprs.push_front(expr->rhs());
            m_exprs.push_front(expr->lhs());
            expr = J;
        }
        m_exprs.push_front(expr);
    }
    VarHdl get_fresh ()
    {
        VarHdl var = Var::get_fresh(m_vars);
        m_vars.insert(&*var);
        return var;
    }
    void add_var ()
    {
        VarHdl var = get_fresh();
        m_binders.push(new Lambda(var));
        m_exprs.push_back(var);
    }
    void ensure (Int pos) { while (m_exprs.size() < pos) add_var(); }
    void ensure_var (Int pos)
    {   //adds vars if needed, o/w replaces pos'th arg with var
        if (m_exprs.size() >= pos) {
            Exprs::iterator i = m_exprs.begin();
            for (Int j=1; j<pos; ++j) ++i;
            ExprHdl& e = *i;
            if (e->isVar() or s_steps) { --s_steps; return; }
            VarHdl v = get_fresh();
            Def* e_def = new Def(v,e->pretty(m_vars));
            m_binders.push(e_def);
            m_vars += e_def->m_def->vars();
            e = v;
        } else while (m_exprs.size() < pos) add_var();
    }
public:
    static void set_steps (unsigned steps) { s_steps = steps; }
    Decompile (ExprHdl comb, const Expr::VarSet& avoid)
        : m_vars(comb->vars() + avoid)
    { set_fun(comb); }
    ~Decompile ()
    {
        while (not m_binders.empty()) {
            delete m_binders.top(); m_binders.pop();
        }
    }
    ExprHdl operator() ();
};
unsigned Decompile::s_steps;
ExprHdl Decompile::operator() ()
{
    //temporaries
    ExprHdl CB = C*B;
    ExprHdl CI = C*I;

    while (front()->isConst()) {
        if (front() == Top) return Top;
        if (front() == Bot) return Bot;
        if (front() == I) {
            pop(); ensure(1);
        } else if (front() == K) {
            pop(); ensure(2); ExprHdl x = pop(); pop();
            set_fun(x);
        } else if (front() == F) { //F is questionably basic
            pop(); ensure(2); pop(); ExprHdl y = pop();
            set_fun(y);
        } else if (front() == W) {
            pop(); ensure_var(2); ExprHdl x = pop(), y = front();
            push_arg(y); set_fun(x);
        } else if (front() == B) {

            //B(B b)(C B a) <- a->b
            if (ExprHdl x = arg(1)) { if (x->lhs() == &*B) {
            if (ExprHdl y = arg(2)) { if (y->lhs() == &*CB) {
                pop(); pop(); pop();
                set_fun(build_arrow(y->rhs(), x->rhs()));
                break;
            } } } }

            pop(); ensure(3); ExprHdl x = pop(), y = pop(), z = pop();
            push_arg(y * z); set_fun(x);
        } else if (front() == C) {
            if (ExprHdl a = arg(1)) {

                //C I x <- <x>
                if (a == I) {
                    pop(); pop(); ensure(1); ExprHdl x = pop();
                    set_fun(build_sgtn(x));
                    break;
                }

                //C <x> xs = C(C I x)xs <- <x,xs>
                if (a->lhs() == &*CI) {
                    pop(); pop(); ensure(1); ExprHdl xs = pop();
                    set_fun(build_pair(a->rhs(), xs));
                    break;
                }
            }
            pop(); ensure(3); ExprHdl x = pop(), y = pop(), z = pop();
            push_arg(y); push_arg(z); set_fun(x);
        } else if (front() == S) {
            pop(); ensure_var(3); ExprHdl x = pop(), y = pop(), z = pop();
            push_arg(y * z); push_arg(z); set_fun(x);
        } else if (front() == J) {
            pop(); ensure(2);
            ExprHdl x = pop(), y = pop();
            set_fun(x | y);
            break;
        } else if (front() == R) {
            pop(); ensure(2);
            ExprHdl x = pop(), y = pop();
            set_fun(x + y);
            break;
        } else break; //unrecognized constant
    }

    ExprHdl result = pop()->pretty(m_vars);
    m_vars += result->vars();
    while (not m_exprs.empty()) {
        result = result * pop()->pretty(m_vars);
        m_vars += result->vars();
    }
    while (not m_binders.empty()) {
        result = m_binders.top()->bind(result);
        delete m_binders.top();
        m_binders.pop();
    }
    return result;
}
ExprHdl Expr::pretty (unsigned steps)
{
    Decompile::set_steps(steps);
    return pretty(VarSet::empty_set());
}

//constants remain unchanged

ExprHdl Expr::pretty (const VarSet& avoid) { return this; }
ExprHdl Const::pretty (const VarSet& avoid)
{
    std::vector<VarHdl> x;
    if (*this == *I) {
        x = Var::get_fresh(avoid,1); return lam(x, x[0]);
    }
    if (*this == *K) {
        x = Var::get_fresh(avoid,2); return lam(x, x[0]);
    }
    if (*this == *F) { //F is questionably basic
        x = Var::get_fresh(avoid,2); return lam(x, x[1]);
    }
    if (*this == *W) {
        x = Var::get_fresh(avoid,2); return lam(x, x[0] * x[1] * x[1]);
    }
    if (*this == *B) {
        x = Var::get_fresh(avoid,3); return lam(x, x[0] * (x[1] * x[2]));
    }
    if (*this == *C) {
        x = Var::get_fresh(avoid,3); return lam(x, x[0] * x[2] * x[1]);
    }
    if (*this == *S) {
        x = Var::get_fresh(avoid,3); return lam(x, x[0]*x[2] * (x[1]*x[2]));
    }
    if (*this == *J) {
        x = Var::get_fresh(avoid,2); return lam(x, x[0] | x[1]);
    }
    if (*this == *R) {
        x = Var::get_fresh(avoid,2); return lam(x, x[0] + x[1]);
    }
    return this;
}
ExprHdl App::pretty (const VarSet& avoid) { return Decompile(this,avoid)(); }
ExprHdl Comp::pretty (const VarSet& avoid)
{
    ExprHdl lhs = m_lhs->pretty(avoid + m_rhs->vars());
    ExprHdl rhs = m_rhs->pretty(avoid + lhs->vars());
    return lhs % rhs;
}
ExprHdl Join::pretty (const VarSet& avoid)
{
    ExprHdl lhs = m_lhs->pretty(avoid + m_rhs->vars());
    ExprHdl rhs = m_rhs->pretty(avoid + lhs->vars());
    return lhs | rhs;
}
ExprHdl Rand::pretty (const VarSet& avoid)
{
    ExprHdl lhs = m_lhs->pretty(avoid + m_rhs->vars());
    ExprHdl rhs = m_rhs->pretty(avoid + lhs->vars());
    return lhs + rhs;
}
ExprHdl Arrow::pretty (const VarSet& avoid)
{
    ExprHdl lhs = m_lhs->pretty(avoid + m_rhs->vars());
    ExprHdl rhs = m_rhs->pretty(avoid + lhs->vars());
    return build_arrow(lhs, rhs);
}

PattHdl BlankPatt::pretty (const VarSet&) { return this; }
PattHdl VarPatt::pretty (const VarSet& avoid)
{
    Assert (!m_pred, "not implemented");

    if (!m_type and !m_test) return this;

    PattHdl result(new VarPatt(m_var));
    VarSet vars = m_vars + avoid;
    if (m_type) {
        if (m_test) {
            ExprHdl type = m_test->pretty(vars);
            result->type(type);
            result->test(m_test->pretty(vars + type->vars()));
        } else {
            result->type(m_type->pretty(vars));
        }
    } else {
        result->test(m_test->pretty(vars));
    }
    return result;
}
PattHdl VectPatt::pretty (const VarSet& avoid)
{
    Assert (!m_pred, "not implemented");

    VectPatt *result = new VectPatt();
    VarSet vars = m_vars + avoid;
    for (Int i=0; i<m_patts.size(); ++i) {
        PattHdl patt = m_patts[i]->pretty(vars);
        vars += patt->vars();
        result->push(patt);
    }
    if (m_type) {
        ExprHdl type = m_type->pretty(vars);
        vars += type->vars();
        result->type(type);
    }
    if (m_test) {
        result->test(m_test->pretty(vars));
    }
    return result;
}

ExprHdl Definition::pretty (const VarSet& avoid)
{
    PattHdl patt = m_patt->pretty(avoid + m_mean->vars() + m_in->vars());
    ExprHdl mean = m_mean->pretty(avoid + patt->vars() + m_in->vars());
    ExprHdl in   = m_in  ->pretty(avoid + patt->vars() + mean->vars());
    return new Definition(m_patt, mean, in);
}
ExprHdl Lambda::pretty (const VarSet& avoid)
{
    PattHdl patt = m_patt->pretty(avoid + m_in->vars());
    ExprHdl in   = m_in  ->pretty(avoid + patt->vars());
    return new Lambda(patt, in);
}
ExprHdl Forall::pretty (const VarSet& avoid)
{
    PattHdl patt = m_patt->pretty(avoid + m_in->vars());
    ExprHdl in   = m_in  ->pretty(avoid + patt->vars());
    return new Forall(patt, in);
}
ExprHdl Exists::pretty (const VarSet& avoid)
{
    PattHdl patt = m_patt->pretty(avoid + m_in->vars());
    ExprHdl in   = m_in  ->pretty(avoid + patt->vars());
    return new Exists(patt, in);
}

ExprHdl Vector::pretty (const VarSet& avoid)
{
    std::vector<ExprHdl> result;
    VarSet vars = m_vars + avoid;
    for (Int i=0; i<m_terms.size(); ++i) {
        ExprHdl term = m_terms[i]->pretty(vars);
        vars += term->vars();
        result.push_back(term);
    }
    return new Vector(result);
}

ExprHdl Special::pretty (const VarSet& avoid) { return this; }

//======== output ========

string Expr::str () const { std::ostringstream o; write_to(o); return o.str(); }

typedef Expr::Valence Val;

bool Const::print (ostream& os, Val left, Val right, bool letter) const
{
    if (letter) os << ' ';
    os << m_name;
    return true;
}
bool Var::print (ostream& os, Val left, Val right, bool letter) const
{
    if (letter) os << ' ';
    os << m_name;
    return true;
}
bool Nat::print (ostream& os, Val left, Val right, bool letter) const
{
    if (letter) os << ' ';
    os << m_num;
    return true;
}
bool Sel::print (ostream& os, Val left, Val right, bool letter) const
{
    os << '[' << m_num << '/' << m_out_of << ']';
    return false;
}

bool App::print (ostream& os, Val left, Val right, bool letter) const
{
    if (min(left,right) < oAPP) {
        os << '(';
        letter = m_lhs->print(os, oNONE, oAPP);
        m_rhs->print(os, oAPP-1, oNONE, letter);
        os << ')';
        return false;
    } else {
        letter = m_lhs->print(os, left, oAPP, letter);
        return m_rhs->print(os, oAPP-1, right, letter);
    }
}
bool Comp::print (ostream& os, Val left, Val right, bool letter) const
{
    if (min(left,right) < oCOMP) {
        os << '(';
        m_lhs->print(os, oNONE, oCOMP);
        os << '*';
        m_rhs->print(os, oCOMP, oDOT-1);
        os << ')';
        return false;
    } else {
        m_lhs->print(os, left, oCOMP, letter);
        os << '*';
        return m_rhs->print(os, oCOMP, min(right,Val(oDOT-1)));
    }
}
bool Join::print (ostream& os, Val left, Val right, bool letter) const
{
    if (min(left,right) < oJOIN) { //associative
        os << '(';
        m_lhs->print(os, oNONE, oJOIN);
        os << '|';
        m_rhs->print(os, oJOIN, oNONE);
        os << ')';
        return false;
    } else {
        m_lhs->print(os, left, oJOIN, letter);
        os << '|';
        return m_rhs->print(os, oJOIN, right);
    }
}
bool Arrow::print (ostream& os, Val left, Val right, bool letter) const
{
    if (min(left,right) < oARROW) {
        os << '(';
        m_lhs->print(os, oNONE, oARROW-1);
        os << "->";
        m_rhs->print(os, oARROW, oDOT-1);
        os << ')';
        return false;
    } else {
        m_lhs->print(os, left, oARROW-1, letter);
        os << "->";
        return m_rhs->print(os, oARROW, min(right,Val(oDOT-1)));
    }
}
bool Rand::print (ostream& os, Val left, Val right, bool letter) const
{
    if (min(left,right) <= oRAND) { //non-associative
        os << '(';
        m_lhs->print(os, oNONE, oRAND);
        os << '+';
        m_rhs->print(os, oRAND, oNONE);
        os << ')';
        return false;
    } else {
        m_lhs->print(os, left, oRAND, letter);
        os << '+';
        return m_rhs->print(os, oRAND, right);
    }
}

void BlankPatt::print (ostream& os) const { os << '-'; }
void VarPatt::print (ostream& os) const
{
    Assert (!m_pred, "not implemented");

    m_var->print(os);
    if (m_type) { os << ':';  m_type->print(os, oDOT-1, oDOT-1); }
    if (m_test) { os << "::"; m_test->print(os, oDOT-1, oDOT-1); }
}
void VectPatt::print (ostream& os) const
{
    Assert (!m_pred, "not implemented");

    os << '<';
    if (not m_patts.empty()) {
        m_patts[0]->print(os);
        for (unsigned i=1; i<m_patts.size(); ++i) {
            os << ',';
            m_patts[i]->print(os);
        }
    }
    os << '>';
    if (m_type) { os << ':';  m_type->print(os, oDOT-1, oDOT-1); }
    if (m_test) { os << "::"; m_test->print(os, oDOT-1, oDOT-1); }
}

bool Definition::print (ostream& os, Val left, Val right, bool letter) const
{
    if (right < oDOT) {
        os << '(';
        if (Var* var = m_patt->var()) {
            var->print(os);
        } else {
            os << "let ";
            m_patt->print(os);
        }
        os << " := ";
        m_mean->print(os, oDOT-1, oDOT-1);
        os << ". ";
        m_in->print(os, oNONE, oDOT);
        os << ')';
        return false;
    } else {
        if (left < oNONE) os << ". ";
        if (Var* var = m_patt->var()) {
            var->print(os);
        } else {
            os << "let ";
            m_patt->print(os);
        }
        os << " := ";
        m_mean->print(os, oDOT-1, oDOT-1);
        os << ". ";
        return m_in->print(os, oNONE, right);
    }
}
bool Lambda::print (ostream& os, Val left, Val right, bool letter) const
{
    if (left == oLAMBDA) {      os << ',';
    } else if (right < oDOT) {  os << "(\\";
    //} else if (left < oNONE) {  os << " \\";
    } else                      os << '\\';

    m_patt->print(os);

    if (m_in->binder() == Symbols::LAMBDA) {
        letter = m_in->print(os, oLAMBDA, oDOT);
    } else {
        os << ". ";
        letter = m_in->print(os, oNONE, oDOT);
    }

    if (right < oDOT) {
        os << ')';
        letter = false;
    }

    return letter;
}
bool Forall::print (ostream& os, Val left, Val right, bool letter) const
{
    if (left == oFORALL) {      os << ',';
    } else if (right < oDOT) {  os << "(/\\";
    //} else if (left < oNONE) {  os << " /\\";
    } else                      os << "/\\";

    m_patt->print(os);

    if (m_in->binder() == Symbols::FORALL) {
        letter = m_in->print(os, oFORALL, oDOT);
    } else {
        os << ". ";
        letter = m_in->print(os, oNONE, oDOT);
    }

    if (right < oDOT) {
        os << ')';
        letter = false;
    }

    return letter;
}
bool Exists::print (ostream& os, Val left, Val right, bool letter) const
{
    if (left == oEXISTS) {      os << ',';
    } else if (right < oDOT) {  os << "(\\/";
    //} else if (left < oNONE) {  os << " \\/";
    } else                      os << "\\/";

    m_patt->print(os);

    if (m_in->binder() == Symbols::EXISTS) {
        letter = m_in->print(os, oEXISTS, oDOT);
    } else {
        os << ". ";
        letter = m_in->print(os, oNONE, oDOT);
    }

    if (right < oDOT) {
        os << ')';
        letter = false;
    }

    return letter;
}

bool Vector::print (ostream& os, Val left, Val right, bool letter) const
{
    if (m_terms.empty()) { os << "<>"; return Val(); }

    os << '<';
    m_terms[0]->print(os, oNONE, oNONE);
    for (Int i=1; i<m_terms.size(); ++i) {
        os << ',';
        m_terms[i]->print(os, oNONE, oNONE);
    }
    os << '>';
    return false;
}

bool Special::print (ostream& os, Val left, Val right, bool letter) const
{
    if (letter) os << ' ';
    os << SpecialNames[m_type];
    return true;
}

}


