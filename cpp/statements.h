#ifndef JOHANN_STATEMENT_H
#define JOHANN_STATEMENT_H

#include "definitions.h"
#include "handling.h"
#include "expressions.h"
#include "symbols.h"
#include <vector>

namespace Statements
{

const Logging::Logger logger("stmt", Logging::DEBUG);

namespace EX = Expressions;
using namespace Symbols;

//the language
class Relationship;
class Conjunction;
class Disjunction;
class Forall;
class Exists;
class Negation;
class Implication;
class Definition;

class Guesser
{
public:
    virtual ~Guesser () {}
    virtual Prob less (ExprHdl lhs, ExprHdl rhs) = 0;
    virtual Prob equal (ExprHdl lhs, ExprHdl rhs) = 0;
};

class Statement : public Handling::HandledObject
{
protected:
    typedef Handling::Handle<Statement> StmtHdl;
    virtual ~Statement () {}
public:

    //specific access, for Relationships and Conjunctions
    virtual ExprHdl lhs () const { return ExprHdl(); }
    virtual ExprHdl rhs () const { return ExprHdl(); }

    //uniform access
    virtual Expr::VarSet vars ();
    virtual std::vector<ExprHdl> relevant_exprs ();
    virtual StmtHdl map (const EX::ExprFun&);
    virtual Prob guess (Guesser& g);

    //query normal forms
    /// query normal forms are not,and,or,exists terms over =,[=,![=
    virtual StmtHdl query_nf () = 0;
    virtual StmtHdl _neg ();
    virtual StmtHdl _abstract (PattHdl patt);
    virtual StmtHdl _where (PattHdl patt, ExprHdl mean);
    virtual StmtHdl _assuming (ExprHdl test);
    /// assumption normal forms are conjunctions of =,[=,![=
    virtual StmtHdl _assume_nf () { return StmtHdl(); }
    StmtHdl assume_nf () { return query_nf()->_assume_nf(); }

    //reflection
    virtual StmtHdl negation_nf () = 0;
    virtual ExprHdl to_semi ();
    virtual ExprHdl to_bool ();

    //dynamic typing
    virtual Relationship* as_reln () { return NULL; }
    virtual Conjunction*  as_conj () { return NULL; }
    virtual Disjunction*  as_disj () { return NULL; }
    virtual Forall*       as_fall () { return NULL; }
    virtual Exists*       as_exts () { return NULL; }

    //output
    virtual StmtHdl pretty () { return StmtHdl(this); }
    virtual ostream& write_to (ostream& os) const = 0;
};
typedef Handling::Handle<Statement> StmtHdl;

//----- query normal form statements -----

class Relationship : public Statement
{
    ExprHdl m_lhs, m_rhs;
    Relation m_rel;
public:
    Relationship (ExprHdl lhs, Relation rel, ExprHdl rhs)
        : m_lhs(lhs), m_rhs(rhs), m_rel(rel)
    {}
    virtual ~Relationship () {}

    //specific access
    virtual ExprHdl lhs () const { return m_lhs; }
    virtual ExprHdl rhs () const { return m_rhs; }
    Relation rel () const { return m_rel; }

    //uniform access
    virtual Expr::VarSet vars ();
    virtual std::vector<ExprHdl> relevant_exprs ();
    virtual StmtHdl map (const EX::ExprFun& fun);
    virtual Prob guess (Guesser& g);

    //query normal forms
    virtual StmtHdl query_nf ();
    virtual StmtHdl _neg ();
    virtual StmtHdl _abstract (PattHdl patt);
    virtual StmtHdl _where (PattHdl patt, ExprHdl mean);
    virtual StmtHdl _assuming (ExprHdl test);
    virtual StmtHdl _assume_nf ();

    //reflection
    virtual StmtHdl negation_nf ();
    virtual ExprHdl to_semi ();
    virtual ExprHdl to_bool ();

    //dynamic typing
    virtual Relationship* as_reln () { return this; }

    //output
    virtual StmtHdl pretty ();
    virtual ostream& write_to (ostream& os) const;
};

class Conjunction : public Statement
{
    std::vector<StmtHdl> m_terms;
public:
    Conjunction () : m_terms(0) {}
    virtual ~Conjunction () {}

    //specific access
    void add (StmtHdl stmt);
    const std::vector<StmtHdl>& terms () const { return m_terms; }
    virtual ExprHdl lhs () const
    {
        Assert (!m_terms.empty(), "empty conjunction has no lhs");
        return m_terms.front()->lhs();
    }
    virtual ExprHdl rhs () const
    {
        Assert (!m_terms.empty(), "empty conjunction has no rhs");
        return m_terms.back()->rhs();
    }

    //uniform access
    virtual Expr::VarSet vars ();
    virtual std::vector<ExprHdl> relevant_exprs ();
    virtual StmtHdl map (const EX::ExprFun& fun);
    virtual Prob guess (Guesser& g);

    //query normal forms
    virtual StmtHdl query_nf ();
    virtual StmtHdl _neg ();
    virtual StmtHdl _abstract (PattHdl patt);
    virtual StmtHdl _where (PattHdl patt, ExprHdl mean);
    virtual StmtHdl _assuming (ExprHdl test);
    virtual StmtHdl _assume_nf ();

    //reflection
    virtual StmtHdl negation_nf ();
    virtual ExprHdl to_semi ();
    virtual ExprHdl to_bool ();

    //dynamic typing
    virtual Conjunction* as_conj () { return this; }

    //output
    virtual ostream& write_to (ostream& os) const;
};
class Disjunction : public Statement
{
    std::vector<StmtHdl> m_terms;
public:
    Disjunction () : m_terms(0) {}
    virtual ~Disjunction () {}

    //specific access
    void add (StmtHdl stmt);
    const std::vector<StmtHdl>& terms () const { return m_terms; }

    //uniform access
    virtual Expr::VarSet vars ();
    virtual std::vector<ExprHdl> relevant_exprs ();
    virtual StmtHdl map (const EX::ExprFun& fun);
    virtual Prob guess (Guesser& g);

    //query normal forms
    virtual StmtHdl query_nf ();
    virtual StmtHdl _neg ();
    virtual StmtHdl _abstract (PattHdl patt);
    virtual StmtHdl _where (PattHdl patt, ExprHdl mean);
    virtual StmtHdl _assuming (ExprHdl test);

    //reflection
    virtual StmtHdl negation_nf ();
    virtual ExprHdl to_semi ();
    virtual ExprHdl to_bool ();

    //dynamic typing
    virtual Disjunction* as_disj () { return this; }

    //output
    virtual ostream& write_to (ostream& os) const;
};

//----- quantifier-normal statements -----

class Forall : public Statement
{
    PattHdl m_patt;
    StmtHdl m_stmt;
public:
    Forall (PattHdl patt, StmtHdl stmt) : m_patt(patt), m_stmt(stmt) {}
    virtual ~Forall () {}

    //reflection
    virtual StmtHdl negation_nf ();
    virtual ExprHdl to_semi ();
    virtual ExprHdl to_bool ();

    //dynamic typing
    virtual Forall* as_fall () { return this; }

    virtual StmtHdl query_nf ();
    virtual StmtHdl pretty ();
    virtual ostream& write_to (ostream& os) const;
};
class Exists : public Statement
{
    PattHdl m_patt;
    StmtHdl m_stmt;
public:
    Exists (PattHdl patt, StmtHdl stmt) : m_patt(patt), m_stmt(stmt) {}
    virtual ~Exists () {}

    //reflection
    virtual StmtHdl negation_nf ();
    virtual ExprHdl to_semi ();
    virtual ExprHdl to_bool ();

    //dynamic typing
    virtual Exists* as_exts () { return this; }

    virtual StmtHdl query_nf ();
    virtual ostream& write_to (ostream& os) const;
};
class Binder : public Expressions::Binder
{
public:
    Binder (Symbols::BinderType type, PattHdl patt)
        : Expressions::Binder(type,patt)
    {}

    ExprHdl bind (ExprHdl term) { return Expressions::Binder::bind(term); }
    StmtHdl bind (StmtHdl stmt);
private:
    StmtHdl forall (StmtHdl stmt);
};

//----- non-normal statements -----

class Negation : public Statement
{
    StmtHdl m_neg;
public:
    Negation (StmtHdl stmt) : m_neg(stmt) {}
    virtual ~Negation () {}

    virtual StmtHdl query_nf ();
    virtual StmtHdl negation_nf ();
    virtual ostream& write_to (ostream& os) const;
};

class Implication : public Statement
{
    StmtHdl m_hyp, m_conc;
public:
    Implication (StmtHdl hyp, StmtHdl conc) : m_hyp(hyp), m_conc(conc) {}
    virtual ~Implication () {}

    virtual StmtHdl query_nf ();
    virtual StmtHdl negation_nf ();
    virtual ostream& write_to (ostream& os) const;
};

class Definition : public Statement
{
    PattHdl m_patt;
    ExprHdl m_mean;
    StmtHdl m_stmt;
public:
    Definition (PattHdl p, ExprHdl m, StmtHdl s)
        : m_patt(p), m_mean(m), m_stmt(s) {}
    virtual ~Definition () {}

    virtual StmtHdl query_nf ();
    virtual StmtHdl negation_nf ();
    virtual ostream& write_to (ostream& os) const;
};

//factories
inline StmtHdl build_reln (ExprHdl lhs, Relation rel, ExprHdl rhs)
{
    return new Relationship(lhs, rel, rhs);
}
inline StmtHdl build_equation (ExprHdl lhs, ExprHdl rhs)
{
    return new Relationship(lhs, EQUAL, rhs);
}
StmtHdl build_equation (ExprHdl pair);
inline StmtHdl build_conj () { return new Conjunction(); }
inline StmtHdl build_disj () { return new Disjunction(); }
inline StmtHdl And (StmtHdl lhs, StmtHdl rhs)
{
    StmtHdl result = build_conj();
    result->as_conj()->add(lhs);
    result->as_conj()->add(rhs);
    return result;
}
inline StmtHdl Or (StmtHdl lhs, StmtHdl rhs)
{
    StmtHdl result = build_disj();
    result->as_disj()->add(lhs);
    result->as_disj()->add(rhs);
    return result;
}
inline StmtHdl Implies (StmtHdl hyp, StmtHdl conc)
{
    return new Implication(hyp, conc);
}
inline StmtHdl Not (StmtHdl s) { return s->_neg(); }

inline ostream& operator<< (ostream& os, const Statements::Statement& stmt)
{ return stmt.write_to(os); }
inline ostream& operator<< (ostream& os, const StmtHdl& handle)
{ return handle ? os << *handle : os << "???"; }

}

typedef Statements::Statement Stmt;
using Statements::StmtHdl;

#endif
