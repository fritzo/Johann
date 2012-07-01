#ifndef JOHANN_EXPRESSIONS_H
#define JOHANN_EXPRESSIONS_H

#include "definitions.h"
#include "symbols.h"
#include "large_set.h"
#include "handling.h"
#include <vector>
#include <set>
#include <map>
#include <utility>

//syntax coordination
#define LOCK_SYNTAX_DEBUG {\
    logger.info() << "locking... " <<__FILE__<<" : "<<__LINE__ |0;\
    _lock_syntax();\
    logger.warning() << "LOCKED " <<__FILE__<<" : "<<__LINE__ |0;\
}
#define UNLOCK_SYNTAX_DEBUG {\
    logger.info() << "unlocking... " <<__FILE__<<" : "<<__LINE__ |0;\
    _unlock_syntax();\
}
#define LOCK_SYNTAX _lock_syntax();
#define UNLOCK_SYNTAX _unlock_syntax();
void _lock_syntax   ();
void _unlock_syntax ();

//hash functions
#ifdef __GNUG__
    #include "hash_map.h"
    #define MAP_TYPE std::unordered_map
namespace Expressions { class Expr; class Var; }
namespace std
{//hash template specialization for expr pointers
template<> struct hash<Expressions::Expr*>
{
    size_t operator()(Expressions::Expr* __x) const
    {
        return reinterpret_cast<size_t>(__x);
    }
};
template<> struct hash<Expressions::Var*>
{
    size_t operator()(Expressions::Var* __x) const
    {
        return reinterpret_cast<size_t>(__x);
    }
};
template<> struct hash<pair<Expressions::Expr*,Expressions::Expr*> >
{
    size_t operator()(pair<Expressions::Expr*, Expressions::Expr*> __x) const
    {
        size_t x = reinterpret_cast<size_t>(__x.first);
        size_t y = reinterpret_cast<size_t>(__x.second);
        size_t shift = sizeof(size_t) / 2;
        return (y << shift) ^ (y >> shift) ^ x;
    }
};
}
#else
    #include <map>
    #define MAP_TYPE std::map
#endif

namespace Expressions
{

const Logging::Logger logger("expr", Logging::DEBUG);

//top level interface
void initialize ();
void initialize (string atoms); //space delimited list
void initialize (std::vector<string> atoms);
void clear ();
void define_atom (const string name);
void forget_atom (const string name);
void write_stats_to (ostream& os);
inline void validate () {} //nothing yet

//naming
void write_consts_to (ostream& os);

//================ expression objects ================

class Var;
class VarHdl;
class PattHdl;
class StructlFun;
enum SpecialType
{
    NOTHING,
    UNDEF, ERROR, BLANK,
    NUM_SPECIAL_TYPES = 3
};
enum Operator
{
    oATOM, oCOMP, oAPP, oARROW, oRAND, oJOIN,
    oDOT, oLAMBDA, oFORALL, oEXISTS, oNONE
};

//abstract expression class
class Expr : public Handling::HandledObject
{
    static Int s_num_exprs;
protected:
    typedef Handling::Handle<Expr> ExprHdl;

    //variables
public:
    typedef nonstd::LargeSet VarSet; //sets of pointers to vars
protected:
    const VarSet m_vars; //all variables, = free + bound
public:

    Expr (VarSet vars) : m_vars(vars) { ++s_num_exprs; }
    virtual ~Expr () { --s_num_exprs; }
    static Int num_exprs () { return s_num_exprs; }

    //flyweight comparison
    bool operator== (const Expr& other) const { return this == &other; }
    bool operator!= (const Expr& other) const { return this != &other; }

    //dynamic subtypes
    virtual ExprHdl as_comb () = 0;
    virtual ExprHdl as_pure ();
    friend inline ExprHdl bad (); // type error

    //combinator operations
    virtual ExprHdl _reduce (Int steps=0);
    virtual ExprHdl _sample (Int steps=0);
    ExprHdl reduce (Int steps=0) { return as_comb()->_reduce(steps); }
    ExprHdl sample (Int steps=0) { return as_comb()->_sample(steps); }
    virtual ExprHdl substitute (VarHdl var, ExprHdl expr);
    virtual ExprHdl abstract (VarHdl var); // \var. self
    virtual ExprHdl where (VarHdl var, ExprHdl mean);  // var:=mean. self
    ExprHdl where (PattHdl patt, ExprHdl mean);  // patt:=mean. self

    //dispatch
    inline  bool isPure  () const;
    virtual bool isApp   () const { return false; }
    virtual bool isComp  () const { return false; }
    virtual bool isJoin  () const { return false; }
    virtual bool isConst () const { return false; }
    virtual bool isVar   () const { return false; }
    virtual bool isBad   () const { return false; }

    //properties
    const VarSet& vars () const { return m_vars; }
    inline bool contains (VarHdl var) const;
    virtual const string* name () const { return NULL; }
    virtual Var*  var () { return NULL; }
    virtual Expr* lhs () const { return NULL; }
    virtual Expr* rhs () const { return NULL; }
    virtual Symbols::BinderType binder () const { return Symbols::NO_BINDER; }
    virtual const std::vector<ExprHdl>* terms () const { return NULL; }
    virtual SpecialType special () const { return NOTHING; }
    virtual void _called_by (StructlFun& fun);

    //input/output
    virtual ExprHdl pretty (const VarSet& avoid);
    ExprHdl pretty (unsigned steps=0);
    typedef Int Valence;
    virtual bool print (ostream& os, Valence left=oATOM, Valence right=oATOM, bool letter=false) const = 0;
    void write_to (ostream& os) const { print(os, oNONE, oDOT-1); };
    string str () const;
};
typedef Handling::Handle<Expr> ExprHdl;
inline ExprHdl bad ();

//input
ExprHdl parse (istream& in, ostream& err);
ExprHdl parse (string s, ostream& err);
ExprHdl parse (string s);
extern string parse_errors;

//atoms
inline ExprHdl build_const (const string& name);
inline VarHdl  build_var   (const string& name);
inline ExprHdl build_atom  (const string& name);
inline ExprHdl build_nat (Int n);
inline ExprHdl build_sel (Int n, Int N);
class Const : public Expr // K
{
    const string m_name;

    //unique instances
    typedef MAP_TYPE<string, Const*> Table;
    static Table s_table;
    Const (const string& name) : Expr(VarSet::empty_set()), m_name(name)
    {
        Assert2(s_table.find(m_name) == s_table.end(), "duplicate Consts");
        s_table[name] = this;
    }
    virtual ~Const ()
    {
        Assert2(s_table.find(m_name) != s_table.end(), "missing Const");
        s_table.erase(m_name);
    }

    //factories
    static Const* build (const string& name);
public:
    friend inline ExprHdl build_const (const string& name)
    { return Const::build(name); }
    friend inline ExprHdl build_atom (const string& name);
    friend void define_atom (const string name);
    static Const* find (const string& name)
    {
        Table::iterator iter=s_table.find(name);
        return iter == s_table.end() ? NULL : iter->second;
    }

    //dynamic subtypes
    virtual ExprHdl as_comb ();
    virtual ExprHdl as_pure ();

    //combinator operations
    virtual ExprHdl _reduce (Int steps=0);
    virtual ExprHdl _sample (Int steps=0);
    virtual ExprHdl substitute (VarHdl var, ExprHdl expr);
    virtual ExprHdl abstract (VarHdl var);
    virtual ExprHdl where (VarHdl var, ExprHdl mean);

    //structure
    virtual bool isConst () const { return true; }
    virtual const string* name () const { return &m_name; }
    virtual void _called_by (StructlFun& fun);

    //output
    virtual ExprHdl pretty (const VarSet& avoid);
    virtual bool print (ostream& os, Valence left=oATOM, Valence right=oATOM, bool letter=false) const;
    friend void write_consts_to (ostream& os);
};
class Var : public Expr // x
{
    const string m_name;

    //unique instances
    typedef MAP_TYPE<string, Var*> Table;
    static Table s_table;
    Var (const string& name)
        : Expr(VarSet::singleton(static_cast<void*>(this))), m_name(name)
    {
        Assert2(s_table.find(m_name) == s_table.end(), "duplicate Vars");
        s_table[name] = this;
    }
    virtual ~Var ()
    {
        Assert2(s_table.find(m_name) != s_table.end(), "missing Vars");
        s_table.erase(m_name);
    }

    //factories
    static Var* build (const string& name);
public:
    friend inline VarHdl build_var (const string& name);
    friend inline ExprHdl build_atom (const string& name);

    //fresh factories
    static VarHdl get_fresh (const VarSet& vars);
    static std::vector<VarHdl> get_fresh (const VarSet& vars, Int num);

    //dynamic subtypes
    virtual ExprHdl as_comb ();
    virtual ExprHdl as_pure ();

    //combinator operations
    virtual ExprHdl _reduce (Int steps=0);
    virtual ExprHdl _sample (Int steps=0);
    virtual ExprHdl substitute (VarHdl var, ExprHdl expr);
    virtual ExprHdl abstract (VarHdl var);
    virtual ExprHdl where (VarHdl var, ExprHdl mean);

    //structure
    virtual bool isVar () const { return true; }
    virtual const string* name () const { return &m_name; }
    virtual Var* var () { return this; }
    virtual void _called_by (StructlFun& fun);

    //output
    virtual bool print (ostream& os, Valence left=oATOM, Valence right=oATOM, bool letter=false) const;
};
class VarHdl : public Handling::Handle<Var>
{
    typedef Handling::Handle<Var> Base;
public:
    VarHdl () : Base () {}
    VarHdl (Var* var) : Base(var) {}
    virtual ~VarHdl () {}
    operator ExprHdl () const { return ExprHdl(m_object); }
};
class Nat : public Expr // 22
{
    const Int m_num;

    //unique instances
    typedef MAP_TYPE<Int, Nat*> Table;
    static Table s_table;
    Nat (Int n) : Expr(VarSet::empty_set()), m_num(n)
    {
        Assert3(s_table.find(n) == s_table.end(), "duplicated Nats");
        s_table[n] = this;
    }
    virtual ~Nat ()
    {
        Assert3(s_table.find(m_num) != s_table.end(), "missing Nat");
        s_table.erase(m_num);
    }

    //factories
    static Nat* build (Int n);
public:
    friend inline ExprHdl build_nat (Int n) { return Nat::build(n); }

    //dynamic subtypes
    virtual ExprHdl as_comb ();

    //output
    virtual bool print (ostream& os, Valence left=oATOM, Valence right=oATOM, bool letter=false) const;
};
class Sel : public Expr // [3/4]
{
    const Int m_num, m_out_of;

    //unique instances
    typedef std::pair<Int,Int> Key;
    typedef MAP_TYPE<Key, Sel*> Table;
    static Table s_table;
    Sel (Int n, Int N) : Expr(VarSet::empty_set()), m_num(n), m_out_of(N)
    {
        Assert ( 1 <= n and n <= N, "selector index out of range");
        Key key(n,N);
        Assert3(s_table.find(key) == s_table.end(), "duplicated Sels");
        s_table[key] = this;
    }
    virtual ~Sel ()
    {
        Key key(m_num, m_out_of);
        Assert3(s_table.find(key) != s_table.end(), "missing Nat");
        s_table.erase(key);
    }

    //factories
    static Sel* build (Int n, Int N);
public:
    friend inline ExprHdl build_sel (Int n, Int N)
    { return (1 <= n and n <= N) ? Sel::build(n,N) : bad(); }

    //dynamic subtypes
    virtual ExprHdl as_comb ();

    //output
    virtual bool print (ostream& os, Valence left=oATOM, Valence right=oATOM, bool letter=false) const;
};

//infix operations
inline ExprHdl build_app (ExprHdl lhs, ExprHdl rhs);
inline ExprHdl build_comp (ExprHdl lhs, ExprHdl rhs);
inline ExprHdl build_join (ExprHdl lhs, ExprHdl rhs);
inline ExprHdl build_rand (ExprHdl lhs, ExprHdl rhs);
inline ExprHdl build_arrow (ExprHdl lhs, ExprHdl rhs);
inline ExprHdl operator* (ExprHdl lhs, ExprHdl rhs);
inline ExprHdl operator* (VarHdl lhs, VarHdl rhs);
inline ExprHdl operator% (ExprHdl lhs, ExprHdl rhs);
inline ExprHdl operator% (VarHdl lhs, VarHdl rhs);
inline ExprHdl operator| (ExprHdl lhs, ExprHdl rhs);
inline ExprHdl operator| (VarHdl lhs, VarHdl rhs);
inline ExprHdl operator+ (ExprHdl lhs, ExprHdl rhs);
inline ExprHdl operator+ (VarHdl lhs, VarHdl rhs);
class App : public Expr // M N
{
    const ExprHdl m_lhs, m_rhs;

    //unique instances
    typedef std::pair<Expr*, Expr*> Key;
    typedef MAP_TYPE<Key, App*> Table;
    static Table s_table;
    static Table::iterator _find (const ExprHdl& lhs, const ExprHdl& rhs)
    {
        return s_table.find(Key(&(*lhs), &(*rhs)));
    }
    App (const ExprHdl& lhs, const ExprHdl& rhs)
        : Expr(lhs->vars() + rhs->vars()), m_lhs(lhs), m_rhs(rhs)
    {
        Key key(&*lhs, &*rhs);
        Assert3(s_table.find(key) == s_table.end(), "duplicated Apps");
        s_table[key] = this;
    }
    virtual ~App ()
    {
        Key key(&*m_lhs, &*m_rhs);
        Assert3(s_table.find(key) != s_table.end(), "missing App");
        s_table.erase(key);
    }

    //factories
    static App* build (const ExprHdl& lhs, const ExprHdl& rhs);
public:
    friend inline ExprHdl build_app (ExprHdl lhs, ExprHdl rhs)
    { return App::build(lhs,rhs); }
    friend inline ExprHdl operator* (ExprHdl lhs, ExprHdl rhs)
    { return App::build(lhs, rhs); }
    friend inline ExprHdl operator* (VarHdl lhs, VarHdl rhs)
    { return App::build(lhs, rhs); }

    //dynamic subtypes
    virtual ExprHdl as_comb ();
    virtual ExprHdl as_pure ();

    //combinator operations
    virtual ExprHdl _reduce (Int steps=0);
    virtual ExprHdl _sample (Int steps=0);
    virtual ExprHdl substitute (VarHdl var, ExprHdl expr);
    virtual ExprHdl abstract (VarHdl var);
    virtual ExprHdl where (VarHdl var, ExprHdl mean);

    //structure
    virtual bool isApp () const { return true; }
    virtual Expr* lhs () const { return &(*m_lhs); }
    virtual Expr* rhs () const { return &(*m_rhs); }
    virtual void _called_by (StructlFun& fun);

    //output
    virtual ExprHdl pretty (const VarSet& avoid);
    virtual bool print (ostream& os, Valence left=oATOM, Valence right=oATOM, bool letter=false) const;
};
class Comp : public Expr // M * N
{
    const ExprHdl m_lhs, m_rhs;

    //unique instances
    typedef std::pair<Expr*,Expr*> Key;
    typedef MAP_TYPE<Key, Comp*> Table;
    static Table s_table;
    static Table::iterator _find (const ExprHdl& lhs, const ExprHdl& rhs)
    {
        return s_table.find(Key(&(*lhs), &(*rhs)));
    }
    Comp (const ExprHdl& lhs, const ExprHdl& rhs)
        : Expr(lhs->vars() + rhs->vars()), m_lhs(lhs), m_rhs(rhs)
    {
        Key key(&*lhs, &*rhs);
        Assert3(s_table.find(key) == s_table.end(), "duplicated Comps");
        s_table[key] = this;
    }
    virtual ~Comp ()
    {
        Key key(&*m_lhs, &*m_rhs);
        Assert3(s_table.find(key) != s_table.end(), "missing Comp");
        s_table.erase(key);
    }

    //factories
    static Comp* build (const ExprHdl& lhs, const ExprHdl& rhs);
public:
    friend inline ExprHdl build_comp (ExprHdl lhs, ExprHdl rhs)
    { return Comp::build(lhs,rhs); }
    friend inline ExprHdl operator% (ExprHdl lhs, ExprHdl rhs)
    { return Comp::build(lhs, rhs); }
    friend inline ExprHdl operator% (VarHdl lhs, VarHdl rhs)
    { return Comp::build(lhs, rhs); }

    //dynamic subtypes
    virtual ExprHdl as_comb ();
    virtual ExprHdl as_pure ();

    //combinator operations
    virtual ExprHdl _reduce (Int steps=0);
    virtual ExprHdl _sample (Int steps=0);
    virtual ExprHdl substitute (VarHdl var, ExprHdl expr);
    virtual ExprHdl abstract (VarHdl var);
    virtual ExprHdl where (VarHdl var, ExprHdl mean);

    //structure
    virtual bool isComp () const { return true; }
    virtual Expr* lhs () const { return &(*m_lhs); }
    virtual Expr* rhs () const { return &(*m_rhs); }
    virtual void _called_by (StructlFun& fun);

    //output
    virtual ExprHdl pretty (const VarSet& avoid);
    virtual bool print (ostream& os, Valence left=oATOM, Valence right=oATOM, bool letter=false) const;
};
class Join : public Expr // M | N
{
    const ExprHdl m_lhs, m_rhs;

    //unique instances
    typedef std::pair<Expr*,Expr*> Key;
    typedef MAP_TYPE<Key, Join*> Table;
    static Table s_table;
    static Table::iterator _find (const ExprHdl& lhs, const ExprHdl& rhs)
    {
        return s_table.find(Key(&(*lhs), &(*rhs)));
    }
    Join (const ExprHdl& lhs, const ExprHdl& rhs)
        : Expr(lhs->vars() + rhs->vars()), m_lhs(lhs), m_rhs(rhs)
    {
        Key key(&*lhs, &*rhs);
        Assert3(s_table.find(key) == s_table.end(), "duplicated Joins");
        s_table[key] = this;
    }
    virtual ~Join ()
    {
        Key key(&*m_lhs, &*m_rhs);
        Assert3(s_table.find(key) != s_table.end(), "missing Join");
        s_table.erase(key);
    }

    //factories
    static Join* build (const ExprHdl& lhs, const ExprHdl& rhs);
public:
    friend inline ExprHdl build_join (ExprHdl lhs, ExprHdl rhs)
    { return Join::build(lhs,rhs); }
    friend inline ExprHdl operator| (ExprHdl lhs, ExprHdl rhs)
    { return Join::build(lhs, rhs); }
    friend inline ExprHdl operator| (VarHdl lhs, VarHdl rhs)
    { return Join::build(lhs, rhs); }

    //dynamic subtypes
    virtual ExprHdl as_comb ();
    virtual ExprHdl as_pure ();

    //combinator operations
    virtual ExprHdl _reduce (Int steps=0);
    virtual ExprHdl _sample (Int steps=0);
    virtual ExprHdl substitute (VarHdl var, ExprHdl expr);
    virtual ExprHdl abstract (VarHdl var);
    virtual ExprHdl where (VarHdl var, ExprHdl mean);

    //structure
    virtual bool isJoin () const { return true; }
    virtual Expr* lhs () const { return &(*m_lhs); }
    virtual Expr* rhs () const { return &(*m_rhs); }
    virtual void _called_by (StructlFun& fun);

    //output
    virtual ExprHdl pretty (const VarSet& avoid);
    virtual bool print (ostream& os, Valence left=oATOM, Valence right=oATOM, bool letter=false) const;
};
class Rand : public Expr // M + N
{
    const ExprHdl m_lhs, m_rhs;

    //unique instances
    typedef std::pair<Expr*,Expr*> Key;
    typedef MAP_TYPE<Key, Rand*> Table;
    static Table s_table;
    static Table::iterator _find (const ExprHdl& lhs, const ExprHdl& rhs)
    {
        return s_table.find(Key(&(*lhs), &(*rhs)));
    }
    Rand (const ExprHdl& lhs, const ExprHdl& rhs)
        : Expr(lhs->vars() + rhs->vars()), m_lhs(lhs), m_rhs(rhs)
    {
        Key key(&*lhs, &*rhs);
        Assert3(s_table.find(key) == s_table.end(), "duplicated Rands");
        s_table[key] = this;
    }
    virtual ~Rand ()
    {
        Key key(&*m_lhs, &*m_rhs);
        Assert3(s_table.find(key) != s_table.end(), "missing Rand");
        s_table.erase(key);
    }

    //factories
    static Rand* build (const ExprHdl& lhs, const ExprHdl& rhs);
public:
    friend inline ExprHdl build_rand (ExprHdl lhs, ExprHdl rhs)
    { return Rand::build(lhs,rhs); }
    friend inline ExprHdl operator+ (ExprHdl lhs, ExprHdl rhs)
    { return Rand::build(lhs, rhs); }
    friend inline ExprHdl operator+ (VarHdl lhs, VarHdl rhs)
    { return Rand::build(lhs, rhs); }

    //dynamic subtypes
    virtual ExprHdl as_comb ();

    //output
    virtual ExprHdl pretty (const VarSet& avoid);
    virtual bool print (ostream& os, Valence left=oATOM, Valence right=oATOM, bool letter=false) const;
};
class Arrow : public Expr // M -> N
{
    const ExprHdl m_lhs, m_rhs;

    //unique instances
    typedef std::pair<Expr*,Expr*> Key;
    typedef MAP_TYPE<Key, Arrow*> Table;
    static Table s_table;
    static Table::iterator _find (const ExprHdl& lhs, const ExprHdl& rhs)
    {
        return s_table.find(Key(&(*lhs), &(*rhs)));
    }
    Arrow (const ExprHdl& lhs, const ExprHdl& rhs)
        : Expr(lhs->vars() + rhs->vars()), m_lhs(lhs), m_rhs(rhs)
    {
        Key key(&*lhs, &*rhs);
        Assert3(s_table.find(key) == s_table.end(), "duplicated Arrows");
        s_table[key] = this;
    }
    virtual ~Arrow ()
    {
        Key key(&*m_lhs, &*m_rhs);
        Assert3(s_table.find(key) != s_table.end(), "missing Arrow");
        s_table.erase(key);
    }

    //factories
    static Arrow* build (const ExprHdl& lhs, const ExprHdl& rhs);
public:
    friend inline ExprHdl build_arrow (ExprHdl lhs, ExprHdl rhs)
    { return Arrow::build(lhs,rhs); }

    //dynamic subtypes
    virtual ExprHdl as_comb ();

    //output
    virtual ExprHdl pretty (const VarSet& avoid);
    virtual bool print (ostream& os, Valence left=oATOM, Valence right=oATOM, bool letter=false) const;
};

//container expressions
class Vector : public Expr // <x0,x1,...,xn>
{
    std::vector<ExprHdl> m_terms;

    void _push (ExprHdl expr) { m_terms.push_back(expr); }
    static VarSet all_vars (const std::vector<ExprHdl>& terms);
public:
    Vector (const ExprHdl& elt) : Expr(elt->vars()) { _push(elt); }
    Vector (const ExprHdl& x, const ExprHdl& y)
         : Expr(x->vars() + y->vars()) { _push(x); _push(y); }
    Vector (const std::vector<ExprHdl>& terms)
        : Expr(all_vars(terms))
    {
        m_terms.insert(m_terms.end(), terms.begin(), terms.end());
    }
    Vector (const ExprHdl& head, const std::vector<ExprHdl>& tail)
        : Expr(head->vars() + all_vars(tail))
    {
        _push(head);
        m_terms.insert(m_terms.end(), tail.begin(), tail.end());
    }
    virtual ~Vector () {}

    //combinator operations

    //dynamic subtypes
    virtual ExprHdl as_comb ();

    //structure
    virtual const std::vector<ExprHdl>* terms () const { return &m_terms; }

    //output
    virtual ExprHdl pretty (const VarSet& avoid);
    virtual bool print (ostream& os, Valence left=oATOM, Valence right=oATOM, bool letter=false) const;
};

//patterns
class Definable
{
public:
    virtual ~Definable () {}
    virtual bool operator() (VarHdl name, ExprHdl meaning) = 0;
};
class Pattern : public Handling::HandledObject
{
protected:
    typedef Expr::VarSet VarSet;

    VarSet m_vars;
    ExprHdl m_type, m_test, m_pred;

    Pattern (const VarSet& vars) : m_vars(vars) {}
public:
    Pattern () : m_vars(VarSet::empty_set()) {}
    virtual ~Pattern () {}

    const Expr::VarSet vars () const { return m_vars; }
    void type (ExprHdl type) { m_type = type; m_vars += type->vars(); }
    void test (ExprHdl test) { m_test = test; m_vars += test->vars(); }
    void pred (ExprHdl pred) { m_pred = pred; m_vars += pred->vars(); }
protected:
    ExprHdl type () const;
    ExprHdl test () const;
    ExprHdl pred () const;
    ExprHdl wrap_in (ExprHdl in) const;
    ExprHdl wrap_patt (ExprHdl patt) const;
public:

    virtual ExprHdl expr () const = 0;
    virtual ExprHdl lambda (ExprHdl in) const = 0;
    ExprHdl forall (ExprHdl in) const;
    ExprHdl exists (ExprHdl in) const;
    virtual bool define (Definable& def, ExprHdl meaning) const = 0;

    virtual Var* var () { return NULL; }
    virtual PattHdl as_comb () = 0;
    virtual bool isBad () const;

    virtual PattHdl pretty (const VarSet& avoid) = 0;
    virtual void print (ostream& os) const = 0;
    void write_to (ostream& os) const { print(os); };
    string str () const;
};
class PattHdl : public Handling::Handle<Pattern>
{
    typedef Handling::Handle<Pattern> Base;
public:
    PattHdl () : Base () {}
    PattHdl (Pattern* patt) : Base(patt) {}
    virtual ~PattHdl () {}
};

class BlankPatt : public Pattern
{
public:
    BlankPatt () {}
    virtual ~BlankPatt () {}

    virtual ExprHdl expr () const;
    virtual ExprHdl lambda (ExprHdl in) const;
    virtual bool define (Definable& def, ExprHdl meaning) const { return true; }

    virtual PattHdl as_comb ();
    virtual bool isBad () const;

    virtual PattHdl pretty (const VarSet& avoid);
    virtual void print (ostream& os) const;
};
class VarPatt : public Pattern
{
    VarHdl m_var;
public:
    VarPatt (VarHdl var) : Pattern(var->vars()), m_var(var) {}
    virtual ~VarPatt () {}

    virtual ExprHdl expr () const;
    virtual ExprHdl lambda (ExprHdl in) const;
    virtual bool define (Definable& def, ExprHdl meaning) const;

    virtual Var* var () { return &*m_var; }
    virtual PattHdl as_comb ();

    virtual PattHdl pretty (const VarSet& avoid);
    virtual void print (ostream& os) const;
};
class VectPatt : public Pattern
{
    std::vector<PattHdl> m_patts;
public:
    void push (const PattHdl& p) { m_patts.push_back(p); m_vars += p->vars(); }

    VectPatt () {}
    VectPatt (PattHdl p) { push(p); }
    VectPatt (PattHdl p, PattHdl q) { push(p); push(q); }
    VectPatt (const std::vector<PattHdl> p)
    {
        for (unsigned i=0; i<p.size(); ++i) push(p[i]);
    }
    virtual ~VectPatt () {}

    //const std::vector<Pattern> patts () { return m_patts; }

    virtual ExprHdl expr () const;
    virtual ExprHdl lambda (ExprHdl in) const;
    virtual bool define (Definable& def, ExprHdl meaning) const;

    virtual PattHdl as_comb ();
    virtual bool isBad () const;

    virtual PattHdl pretty (const VarSet& avoid);
    virtual void print (ostream& os) const;
};

//binders
class Definition : public Expr // p := X. M
{
    PattHdl m_patt;
    ExprHdl m_mean, m_in;
public:
    Definition (const PattHdl& patt, const ExprHdl& mean, const ExprHdl& in)
        : Expr(patt->vars() + mean->vars() + in->vars()),
          m_patt(patt), m_mean(mean), m_in(in)
    {}
    virtual ~Definition () {}

    virtual ExprHdl as_comb ();

    //output
    virtual ExprHdl pretty (const VarSet& avoid);
    virtual bool print (ostream& os, Valence left=oATOM, Valence right=oATOM, bool letter=false) const;
};
class Lambda : public Expr // \p. M
{
    PattHdl m_patt;
    ExprHdl m_in;
public:
    Lambda (const PattHdl& patt, const ExprHdl& in)
        : Expr(patt->vars() + in->vars()), m_patt(patt), m_in(in)
    {}
    virtual ~Lambda () {}

    //dynamic subtypes
    virtual ExprHdl as_comb ();

    //structure
    virtual Symbols::BinderType binder () const { return Symbols::LAMBDA; }

    //output
    virtual ExprHdl pretty (const VarSet& avoid);
    virtual bool print (ostream& os, Valence left=oATOM, Valence right=oATOM, bool letter=false) const;
};
class Forall : public Expr // /\p. M
{
    PattHdl m_patt;
    ExprHdl m_in;
public:
    Forall (const PattHdl& patt, const ExprHdl& in)
        : Expr(patt->vars() + in->vars()), m_patt(patt), m_in(in) {}
    virtual ~Forall () {}

    //dynamic subtypes
    virtual ExprHdl as_comb ();

    //structure
    virtual Symbols::BinderType binder () const { return Symbols::FORALL; }

    //output
    virtual ExprHdl pretty (const VarSet& avoid);
    virtual bool print (ostream& os, Valence left=oATOM, Valence right=oATOM, bool letter=false) const;
};
class Exists : public Expr // \/x. M
{
    PattHdl m_patt;
    ExprHdl m_in;
public:
    Exists (const PattHdl& patt, const ExprHdl& in)
        : Expr(patt->vars() + in->vars()), m_patt(patt), m_in(in) {}
    virtual ~Exists () {}

    //dynamic subtypes
    virtual ExprHdl as_comb ();

    //structure
    virtual Symbols::BinderType binder () const { return Symbols::EXISTS; }

    //output
    virtual ExprHdl pretty (const VarSet& avoid);
    virtual bool print (ostream& os, Valence left=oATOM, Valence right=oATOM, bool letter=false) const;
};

//buffered binders
class Binder
{
protected:
    Symbols::BinderType m_type;
    typedef std::vector<PattHdl> Patts;
    Patts m_patts;
public:
    void push (PattHdl patt) { m_patts.push_back(patt); }

    Binder (Symbols::BinderType type, PattHdl patt)
        : m_type(type) { push(patt); }

    Symbols::BinderType type  () const { return m_type; }
    const Patts&        patts () const { return m_patts; }

    ExprHdl bind (ExprHdl term);
private:
    ExprHdl lambda (ExprHdl term);
    ExprHdl forall (ExprHdl term);
    ExprHdl exists (ExprHdl term);
};

//special (non-)expressions: XXX ??? -
class Special : public Expr
{
    const SpecialType m_type;

    //unique instances
    static Special* s_table[1+NUM_SPECIAL_TYPES];
    Special (SpecialType type) : Expr(VarSet::empty_set()), m_type(type)
    {
        Assert (1 <= type and type <= NUM_SPECIAL_TYPES,
                "invalid special type");
        s_table[m_type] = this;
    }
    virtual ~Special () { s_table[m_type] = NULL; }
public:

    //factories
    static Special* build (SpecialType type);
    friend inline ExprHdl bad () { return build(ERROR); }

    //dynamic subtyping
    virtual ExprHdl as_comb ();

    //structure
    virtual bool isBad () const { return m_type == ERROR; }
    virtual SpecialType special () const { return m_type; }
    virtual void _called_by (StructlFun& fun);

    //output
    virtual ExprHdl pretty (const VarSet& avoid);
    virtual bool print (ostream& os, Valence left=oATOM, Valence right=oATOM, bool letter=false) const;
};

//======== factories ========

inline ExprHdl let (PattHdl patt, ExprHdl mean, ExprHdl in)
{
    return new Definition(patt,mean,in);
}
inline ExprHdl let (VarHdl var, ExprHdl mean, ExprHdl in)
{
    PattHdl patt = new VarPatt(var);
    return new Definition(patt,mean,in);
}
inline ExprHdl lam (VarHdl var, ExprHdl in)
{
    PattHdl patt = new VarPatt(var);
    return new Lambda(patt,in);
}
ExprHdl lam (const std::vector<VarHdl>& vars, ExprHdl in);
ExprHdl lam_blank (ExprHdl in);

inline VarHdl build_var (const string& name) { return Var::build(name); }
inline ExprHdl build_atom (const string& name)
{//finds an existing atom or makes a new variable
    Const::Table::iterator i = Const::s_table.find(name);
    return i != Const::s_table.end() ? ExprHdl(i->second) : build_var(name);
}

inline ExprHdl build_sgtn (ExprHdl elt) { return new Vector(elt); }
inline ExprHdl build_pair (ExprHdl x, ExprHdl y) { return new Vector(x,y); }
inline PattHdl build_pair (PattHdl p, PattHdl q) { return new VectPatt(p,q); }
inline ExprHdl build_vector (const std::vector<ExprHdl>& terms)
{
    return new Vector(terms);
}
inline ExprHdl build_vector (ExprHdl head, const std::vector<ExprHdl>& tail)
{
    return new Vector(head,tail);
}

ExprHdl build_tuple (const std::vector<ExprHdl>& terms);
PattHdl build_tuple (const std::vector<PattHdl>& patts);

inline ExprHdl build_undef   () { return Special::build(UNDEF); }
inline ExprHdl build_erron   () { return Special::build(ERROR); }
inline ExprHdl build_blank   () { return Special::build(BLANK); }

//combinator factories
ExprHdl close (ExprHdl a);                  //just (V a)
ExprHdl above (ExprHdl a);                  //just (J a)
ExprHdl power (ExprHdl a);                  //just (P a)
ExprHdl semi  (ExprHdl i);                  //just (semi i)
ExprHdl test  (ExprHdl t);                  //just (semi*t)
ExprHdl tests (ExprHdl s, ExprHdl t);       //just S (test s) (test t)
ExprHdl id ();                              //just I

//======== delayed inlined functions ========

inline bool Expr::contains (VarHdl var) const { return m_vars.contains(&*var); }
inline bool Expr::isPure () const { return m_vars.empty(); }

inline ostream& operator<< (ostream& os, const Expr& expr)
{ expr.write_to(os); return os; }
inline ostream& operator<< (ostream& os, const ExprHdl& handle)
{ return handle ? os << *handle : os << "???"; }

inline ostream& operator<< (ostream& os, const Var& var)
{ var.write_to(os); return os; }
inline ostream& operator<< (ostream& os, const VarHdl& handle)
{ return handle ? os << *handle : os << "???"; }

inline ostream& operator<< (ostream& os, const Pattern& patt)
{ patt.write_to(os); return os; }
inline ostream& operator<< (ostream& os, const PattHdl& handle)
{ return handle ? os << *handle : os << "???"; }

//======== virtual function classes ========

class ExprFun
{
public:
    virtual ~ExprFun () {}
    virtual ExprHdl operator() (const ExprHdl& expr) const = 0;
};
class substitutor : public ExprFun
{//via replacement
protected:
    const VarHdl m_old;
    const ExprHdl m_new;
public:
    substitutor (VarHdl _old, ExprHdl _new) : m_old(_old), m_new(_new) {}
    virtual ~substitutor () {}
    virtual ExprHdl operator() (const ExprHdl& expr) const;
};

class StructlFun
{
protected:
    void _call_me (Expr& expr) { expr._called_by(*this); }
    virtual ~StructlFun () {}
public:
    void operator() (Expr& expr) { _call_me(expr); }
    virtual void of_atom    (const string& atom) {};
    virtual void of_var     (VarHdl var) {};
    virtual void of_app     (Expr& lhs, Expr& rhs) {};
    virtual void of_comp    (Expr& lhs, Expr& rhs) {};
    virtual void of_join    (Expr& lhs, Expr& rhs) {};
    virtual void of_special (SpecialType type) {};
    virtual void of_pretty  (Expr& expr) {
        Error("StructlFun called on pretty: " << expr);
    };
};

}

inline ostream& operator<< (ostream& os, const Expressions::Expr::VarSet& vars)
{
    if (vars.empty()) return os << "{}";

    Expressions::Expr::VarSet::Iter i=vars.begin();
    os << '{' << *static_cast<Expressions::Var*>(*i);
    for (++i; i!=vars.end(); ++i) {
        os << ", " << *static_cast<Expressions::Var*>(*i);
    }
    return os << '}';
}

using Expressions::Expr;
using Expressions::ExprHdl;
using Expressions::Var;
using Expressions::VarHdl;
using Expressions::PattHdl;

#endif
