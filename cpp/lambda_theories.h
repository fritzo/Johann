#ifndef JOHANN_MERGING_H
#define JOHANN_MERGING_H

#include "definitions.h"
#include "symbols.h"
#include "expressions.h"
#include "statements.h"
#include "nodes.h"
#include "order.h"
#include <vector>
#include <map>

namespace LambdaTheories
{

//effort:
// 0 = only check what is already in db
// 1 = expand db with new expressions & check
// 2 = also try to prove by contradiction via fork-assume-wait
// XXX proof-by-contradiction seems to never prove anything
// XXX proof-by-contradiction spawns many processes; should be 2 at a time
static const unsigned MAX_EFFORT = 1;

const Logging::Logger logger("theory", Logging::DEBUG);

using namespace Symbols;

//======== named atoms ========
class Atoms
{
public:
    //lattice atoms
    static ObHdl Bot ;
    static ObHdl Top ;
    static ObHdl J ;
    static ObHdl R ;

    //lambda atoms
    static ObHdl I ;
    static ObHdl K ;
    static ObHdl B ;
    static ObHdl C ;
    static ObHdl W ;
    static ObHdl S ;

    //extension atoms
    static ObHdl Y ;
    static ObHdl S2 ;

    static ObHdl CB ;
    static ObHdl CI ;
    static ObHdl KI ;
    static ObHdl SI ;

    static ObHdl U ;
    static ObHdl V ;
    static ObHdl P ;

    //static ObHdl None ; // = K
    //static ObHdl Some ; // = Inr
    static ObHdl Inl ;
    static ObHdl Inr ;
    static ObHdl J2 ;

    //types
    static ObHdl Div ;
    static ObHdl Unit ;
    static ObHdl Semi ;
    static ObHdl Bool ;
    static ObHdl Maybe ;
    static ObHdl Sum ;
    static ObHdl Sset ;

    //functions
    virtual void clear ();
    virtual ~Atoms () {}
protected:
    inline void _define_atom(ObHdl& atom, const char* name, bool required=true);
    inline void _define_app (ObHdl& app, Ob lhs, Ob rhs);
private:
    static std::vector<ObHdl*> s_defined_obs;
};

//======== theories ========

class MagmaTheory : public Atoms
{//abstract nonassociative theory
    typedef MagmaTheory MyType;

    typedef std::vector<ObHdl> ObSet;

    //to ensure uniqueness
protected:
    static MyType *s_unique_instance;
public:
    friend inline MyType* theory ();
    MagmaTheory ();
    virtual ~MagmaTheory ();

    //for the algebra system
    virtual void init_basis ();
    virtual bool define_atom (string name); //true if recognized and new
    virtual bool forget_atom (string name); //true if recognized and new

    //for the theory manager
    struct Prop {
        StmtHdl stmt; string comment;
        Prop (StmtHdl &s, string &c) : stmt(s), comment(c) {}
    };
    typedef Prop Axm;                    typedef std::vector<Axm> Axms;
    typedef std::pair<Prop, Trool> Thm;  typedef std::vector<Thm> Thms;
private:
    Axms m_axms;
    Thms m_thms;
public:
    class AxmIter
    {
        unsigned i;
    public:
        AxmIter(unsigned _i) : i(_i) {}
        bool operator!= (const AxmIter& other) { return other.i != i; }
        AxmIter& operator++ () { ++i; return *this; }
        Axm& operator* ();
        Axm* operator-> ();
    };
    class ThmIter {
        unsigned i;
    public:
        ThmIter(unsigned _i) : i(_i) {}
        bool operator!= (const ThmIter& other) { return other.i != i; }
        ThmIter& operator++ () { ++i; return *this; }
        Thm& operator* ();
        Thm* operator-> ();
    };
    AxmIter axms_begin () { return AxmIter(0); }
    AxmIter axms_end   () { return AxmIter(m_axms.size()); }
    ThmIter thms_begin () { return ThmIter(0); }
    ThmIter thms_end   () { return ThmIter(m_thms.size());   }

    virtual bool assume_reln
        (ExprHdl lhs, Relation reln, ExprHdl rhs, bool core);
    virtual Trool query_reln
        (ExprHdl lhs, Relation reln, ExprHdl rhs, unsigned effort=MAX_EFFORT);
private:
    Trool _query_contradiction (StmtHdl stmt);
public:
    Trool query (StmtHdl stmt, unsigned effort=MAX_EFFORT);
    Trool check (StmtHdl stmt, string comment, unsigned effort=MAX_EFFORT);
    bool assume (StmtHdl stmt, string comment);
    bool _assume (StmtHdl stmt, bool core);
    void review ();
    void review  (AxmIter axm);
    void review  (ThmIter thm);
    void recheck (unsigned effort=MAX_EFFORT);
    void recheck (ThmIter thm, unsigned effort=MAX_EFFORT);
    void clear_thms () { m_thms.clear(); }
    void clear_axms () { m_axms.clear(); }
    std::vector<StmtHdl> get_axioms ();
    std::vector<StmtHdl> get_problems (Trool truth);
    void print_problems (ostream& os, Trool truth);
    virtual void save_to (ostream& os);
    virtual void clear ();

    //these do nothing by default
    virtual void enforce_O (Ob   ob)  const {}
    virtual void enforce_A (App  eqn) const {}
    virtual void enforce_C (Comp eqn) const {}
    virtual void enforce_J (Join eqn) const {}
    virtual void enforce_L (Ord  ord) const {}
    virtual void enforce_N (Ord  ord) const {}
    void validate (Int level);
protected:
    virtual void _validate (Int level) {};
public:
    virtual void write_params_to (ostream& os)
    { os << "\tmagma theory\n"; }
    void write_stats_to (ostream& os);
};

class LatticeTheory : public MagmaTheory
{
public:
    virtual void init_basis ();
    virtual bool define_atom (string name);
    virtual void enforce_O (Ob  ob)  const;
    virtual void enforce_A (App eqn) const;
    virtual void enforce_C (Comp eqn) const;
    virtual void enforce_J (Join eqn) const;
    virtual void enforce_L (Ord ord) const;
    virtual void enforce_N (Ord ord) const;
protected:
    virtual void _validate (Int level);
public:
    virtual void write_params_to (ostream& os)
    { os << "\tlattice theory\n"; }

    LatticeTheory ();
    virtual ~LatticeTheory () {}
};

class LambdaTheory : public LatticeTheory
{
    //an extensional lambda-K-theory with {S, K, I, B, C} axiom schemes
    //variables & abstraction
    static const Int s_numVars = 32;
    Ob _make_var (Int varNum, const char* name=NULL);
    void _close_terms (ObHdl& ob1, ObHdl& ob2) const;
public:
    static ObHdl get_var (Int varNum);
    static int numVars () { return s_numVars; }
    static bool isPure (ObHdl ob);

    //for the theory manager
    virtual bool assume_reln
        (ExprHdl lhs, Relation reln, ExprHdl rhs, bool core=true);
    virtual Trool query_reln
        (ExprHdl lhs, Relation reln, ExprHdl rhs, unsigned effort=MAX_EFFORT);
    std::vector<Ob> solve (StmtHdl stmt, bool tryhard);
protected:
    std::vector<Ob> solve (VarHdl var, StmtHdl stmt);
    std::vector<Ob> solve (VarHdl var, Statements::Relationship* r);
public:

    //for the algebra system
    virtual void init_basis ();
    virtual bool define_atom (string name);
    virtual void enforce_O (Ob  ob)  const;
    virtual void enforce_A (App eqn) const;
    virtual void enforce_C (Comp eqn) const;
protected:
    virtual void _validate (Int level);
public:
    virtual void write_params_to (ostream& os)
    { os << "\tlambda theory\n"; }

    LambdaTheory ();
    virtual ~LambdaTheory () {}
};

class ExtnTheory : public LambdaTheory
{//adds misc additional atoms Y,S',WI,KI
    //type,both,intersect,rand
public:
    //for the algebra system
    virtual void init_basis ();
    virtual bool define_atom (string name);
    virtual void enforce_A (App eqn) const;
    virtual void enforce_C (Comp eqn) const;
    virtual void enforce_L (Ord ord) const;
protected:
    virtual void _validate (Int level);
public:
    virtual void write_params_to (ostream& os)
    { os << "\textension theory\n"; }

    ExtnTheory ();
    virtual ~ExtnTheory () {}
};

class TypedTheory : public ExtnTheory
{//adds types Div,Unit,Semi,Bool,...
public:
    //for the algebra system
    virtual void init_basis ();
    virtual bool define_atom (string name);
    virtual void enforce_A (App eqn) const;
    virtual void enforce_C (Comp eqn) const;
    virtual void enforce_L (Ord ord) const;
    virtual void enforce_N (Ord ord) const;
protected:
    virtual void _validate (Int level);
public:
    virtual void write_params_to (ostream& os)
    { os << "\ttyped theory\n"; }

    TypedTheory ();
    virtual ~TypedTheory () {}

    //for the theory manager
    typedef std::pair<Ob,Int> ObWNum;
    std::vector<ObWNum> type_of (std::vector<ExprHdl> inhab_exprs);
};

inline MagmaTheory* theory ()
{ return MagmaTheory::s_unique_instance; }

void dump (string filename);

}

ostream& operator<< (ostream &os, const LambdaTheories::MagmaTheory::Prop& p);

#endif

