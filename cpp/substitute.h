#ifndef JOHANN_SUBSTITUTE_H
#define JOHANN_SUBSTITUTE_H

#include "definitions.h"
#include "symbols.h"
#include "expressions.h"
//#include "statements.h" //LATER
#include <map>

namespace Substitute
{

const Logging::Logger logger("subs", Logging::DEBUG);

//locations
struct Location
{
    unsigned line;
    string file;
    Location (unsigned l=0, string f="") : line(l), file(f) {}
    friend inline ostream& operator<< (ostream& os, const Location& loc)
    { return os << loc.file << ':' << loc.line; }
};

/** Substitutions are closure operators acting on valuations, i.e. they are
 * (1) idempotent (applying them twice is the same as applying them once)
 * (2) increasing (so variables cannot be undefined or re-defined)
 */
class Subs : public Handling::HandledObject, public Expressions::Definable
{
    typedef Handling::Handle<Subs> SubsHdl;

    struct RawDef
    {
        PattHdl patt;
        ExprHdl meaning;
        RawDef (PattHdl p, ExprHdl m) : patt(p), meaning(m) {}
    };
    typedef std::vector<RawDef>         RawDefs;    RawDefs m_raw;
    typedef MAP_TYPE<Var*, ExprHdl>     Defs;       Defs    m_defs;
    typedef std::map<string,SubsHdl>    Imports;    Imports m_imports;
    typedef std::vector<ExprHdl>        Using;      Using   m_using;
    friend class Library;

    bool m_closed;
    ostream* m_err;
    bool well_defined; //a temporary

    //non-copiable
    Subs (const Subs&) { Error("copy-constructing a Subs"); }
    Subs (int) : m_closed(true), m_err(NULL) {}
public:
    Subs () : m_closed(false), m_err(NULL) {}
    bool empty () const { return m_defs.empty() and m_imports.empty()
                                                and m_using.empty(); }

    //construction interface, all true on success
    virtual bool operator() (VarHdl var, ExprHdl meaning);
private:
    void _import (SubsHdl subs, bool local=false);
public:
    bool define (PattHdl patt, ExprHdl meaning, bool local=false);
    void define (SubsHdl subs, bool local=false);
    void import (string fname, bool local=false);
    void use    (string name, bool local=false);
    void find   (ExprHdl expr, Location loc) { LATER(); }
    /* LATER
    void check  (StmtHdl stmt, Location loc) { LATER(); }
    void assume (StmtHdl stmt, Location loc) { LATER(); }
    */
    bool close (ostream& err);

    //action on expressions
    ExprHdl act (VarHdl var) const;
    ExprHdl act (ExprHdl expr) const;

    //factories
    static SubsHdl id () { return new Subs(0); }

    //i/o
    void write_to (ostream& os) const;
    string str () const;
};
typedef Handling::Handle<Subs> SubsHdl;

//output
inline ostream& operator<< (ostream& os, const Subs& subs)
{ subs.write_to(os); return os; }
inline ostream& operator<< (ostream& os, const SubsHdl& handle)
{ return handle ? os << *handle : os << "[]"; }

//input
SubsHdl parse (istream& in, ostream& err);
SubsHdl parse (string s, ostream& err);
SubsHdl parse (string s);
extern string parse_errors;

//================ library of substitutions ================
class Library
{
    typedef std::map<string, SubsHdl> Files;        Files m_files;
    typedef std::multimap<Subs*,Subs*> Depends;     Depends m_depends;
public:
    void    add_file (string fname, ostream& err);
    SubsHdl get_file (string fname) const;
    unsigned size () const { return m_files.size(); }
    void list (ostream& os) const;
    std::vector<string> list () const;

    void update (ostream& err);
    void clear ();
};
extern Library library;

}

using Substitute::Subs;
using Substitute::SubsHdl;

#endif
