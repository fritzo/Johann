
#include "substitute.h"
#include "parser.h"
#include <sstream>

namespace Substitute
{

namespace EX = Expressions;

typedef Expr::VarSet::iterator var_iter;

bool Subs::operator() (VarHdl var, ExprHdl meaning)
{
    logger.debug() << "defining " << var << " := " << meaning |0;
    if (*var == *meaning) {
        (*m_err) << "trivial def: " << var << " := " << meaning << '\n';
        well_defined = false;
    }
    if (*(act(var)) != *var) {
        (*m_err) << "multiple defs: " << var << " := " << meaning << '\n';
        return false;
    }
    m_defs[&*var] = meaning;
    return true;
}
bool Subs::define (PattHdl patt, ExprHdl meaning, bool local)
{
    Assert (!m_closed, "tried to define in a closed subs");
    Assert (!local, "local defs are not implemented");

    if (patt->isBad()) return false;
    m_raw.push_back(RawDef(patt, meaning));
    return true;
}
void Subs::define (SubsHdl s, bool local)
{
    Assert (!m_closed, "tried to define in a closed subs");
    Assert (!local, "local defs are not implemented");

    m_raw    .insert(m_raw.end(),   s->m_raw    .begin(), s->m_raw    .end());
    m_imports.insert(               s->m_imports.begin(), s->m_imports.end());
    m_using  .insert(m_using.end(), s->m_using  .begin(), s->m_using  .end());
}
void Subs::_import (SubsHdl s, bool local)
{
    m_defs.insert(s->m_defs.begin(), s->m_defs.end());
}
void Subs::import (string fname, bool local)
{
    logger.debug() << "importing " << fname |0;
    Assert (!m_closed, "tried to import in a closed subs");
    Assert (!local, "local defs are not implemented");

    //XXX this misses cross-module name conflicts
    m_imports[fname] = SubsHdl();
}
void Subs::use (string name, bool local)
{
    logger.debug() << "using " << name |0;
    Assert (!m_closed, "tried to import in a closed subs");
    Assert (!local, "local defs are not implemented");

    EX::define_atom(name);
    m_using.push_back(EX::build_const(name));
}
bool Subs::close (ostream& err)
{//builds tables and ensures closure: increasing and idempotent
    Assert (!m_closed, "tried to close a substitution twice");
    m_err = &err;
    well_defined = true;
    m_closed = true;

    //imports
    for (Imports::iterator i=m_imports.begin(); i!=m_imports.end(); ++i) {
        string fname = i->first;
        library.add_file(fname, err);
        SubsHdl s = library.get_file(fname);
        if (s) _import(s);
        else well_defined = false;
    }

    //using
    for (unsigned i=0; i<m_using.size(); ++i) {
        ExprHdl atom = m_using[i];
        VarHdl  var = EX::build_var(atom->str());
        (*this)(var, atom);
    }

    //defs
    //TODO implement recursion
    for (RawDefs::const_iterator i = m_raw.begin(); i!=m_raw.end(); ++i) {
        i->patt->define(*this, i->meaning);
    }

    m_err = NULL;
    AssertW(well_defined, "error closing subs, see message");
    return well_defined;
}

//action
ExprHdl Subs::act (VarHdl var) const
{
    Assert2(m_closed, "subs must be closed before acting");

    Defs::const_iterator i = m_defs.find(&*var);
    return i == m_defs.end() ? var : i->second;
}
ExprHdl Subs::act (ExprHdl expr) const
{
    Assert1(m_closed, "subs must be closed before acting");

    Expr::VarSet vars = expr->vars();
    for (var_iter i=vars.begin(); i!=vars.end(); ++i) {
        VarHdl var(static_cast<Var*>(*i));
        ExprHdl meaning = act(var);
        //if (meaning != var) expr = expr->substitute(var, meaning);
        if (meaning != var) expr = EX::let(var, meaning, expr);
    }

    return expr;
}

//output
string Subs::str () const { std::ostringstream o; write_to(o); return o.str(); }
void Subs::write_to (ostream& os) const
{
    if (empty()) { os << "[]"; return; }

    os << "[\n";
    if (not m_using.empty()) {
        os << "\t!import";
        for (unsigned i=0; i<m_using.size(); ++i) {
            os << ' ' << m_using[i];
        }
        os << ".\n";
    }
    if (not m_imports.empty()) {
        os << "\t!import";
        int counter=0;
        for (Imports::const_iterator i=m_imports.begin();
                i!=m_imports.end(); ++i) {
            os << ' ' << i->first;
            if (++counter % 5 == 0) os << ".\n\t!import"; //at most 5 per line
        }
        os << ".\n";
    }
    for (RawDefs::const_iterator i=m_raw.begin(); i!=m_raw.end(); ++i) {
        os << '\t' << i->patt << " := " << i->meaning << ".\n";
    }
    os << ']';
}

//input
PP::Driver g_parser;
string parse_errors;
SubsHdl parse (istream& in, ostream& err)
{
    parse_errors.clear();
    return g_parser.parse_subs(in,err);
}
SubsHdl parse (string s, ostream& err)
{
    std::istringstream in(s);
    return parse(in,err);
}
SubsHdl parse (string s)
{
    std::istringstream in(s);
    std::ostringstream err;
    SubsHdl result = parse(in,err);
    parse_errors = err.str();
    AssertW(parse_errors.empty(), parse_errors);
    return result;
}

//================ Library ================

//static instance
Library library;

void Library::add_file (string fname, ostream& err)
{
    if (m_files.find(fname) != m_files.end()) return;
    logger.info() << "adding " << fname << " to library" |0;
    err << "#reading " << fname << "\n";
    m_files[fname] = Subs::id(); //avoid cycling

    //open file; try jtext then jcode
    ifstream file;  PP::FileType type;
    string sf = "scripts/" + fname;
    file.open((sf + ".jtext").c_str()); type = PP::JTEXT; if (not file) {
    file.open((sf + ".jcode").c_str()); type = PP::JCODE; if (not file) {
        err << "could not find " << fname << '\n';
        logger.error() << "could not find " << fname |0;
        return;
    }}

    //parse file with a new instance
    SubsHdl s = PP::Driver(fname.c_str()).parse_subs(file, err, type);
    file.close();
    if (not s) {
        err << "failed to parse " << fname << '\n';
        logger.error() << "failed to parse " << fname |0;
        return;
    }

    //add subs and dependencies
    m_files[fname] = s;
    for (Subs::Imports::const_iterator i=s->m_imports.begin();
            i!=s->m_imports.end(); ++i) {
        m_depends.insert(std::make_pair(&*s,&*(i->second)));
    }
}
SubsHdl Library::get_file (string fname) const
{
    Files::const_iterator i = m_files.find(fname);
    return i == m_files.end() ? SubsHdl() : i->second;
}
void Library::list (ostream& os) const
{
    for (Files::const_iterator i=m_files.begin(); i!=m_files.end(); ++i) {
        os << ' ' << i->first;
    }
    os << '\n';
}
std::vector<string> Library::list () const
{
    std::vector<string> result;
    for (Files::const_iterator i=m_files.begin(); i!=m_files.end(); ++i) {
        result.push_back(i->first);
    }
    return result;
}

void Library::update (ostream& err)
{
    logger.info() << "updating library" |0;
    Files old; old.swap(m_files);
    m_depends.clear();

    for (Files::iterator i=old.begin(); i!=old.end(); ++i) {
        i->second.clear();
        add_file(i->first, err);
    }
}
void Library::clear ()
{
    logger.debug() << "clearing library" |0;
    m_depends.clear();
    m_files.clear();
}

}


