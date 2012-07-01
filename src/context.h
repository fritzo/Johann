#ifndef JOHANN_CONTEXT_H
#define JOHANN_CONTEXT_H

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <utility>
#include "definitions.h"
#include "symbols.h"
#include "expressions.h"
#include "statements.h"
#include "hash_map.h"
#include <utility>
#include <map>

namespace Contexts
{

const Logging::Logger logger("context", Logging::INFO);

enum Depth { MAX_DEPTH = 0x7FFFFFFF };

//contexts
class Context : public Expressions::Definable
{
    struct RawDef
    {
        PattHdl patt;
        ExprHdl meaning;
        RawDef (PattHdl p, ExprHdl m) : patt(p), meaning(m) {}
    };
    typedef std::vector<RawDef> RawDefs;
    typedef std::unordered_map<Var*, ExprHdl> Defs;
    typedef std::map<Expr*, VarHdl> Names;

    RawDefs m_raw;
    Defs    m_defs, m_simple;
    Names   m_names;

public:
    Context () {}
    virtual ~Context () { clear(); }

    //total access
    bool empty () const { return m_defs.empty(); }
    void clear ()
        { m_raw.clear(); m_defs.clear(); m_simple.clear(); m_names.clear(); }
    void simplify (bool create=false);

    //definitions
    virtual bool operator() (VarHdl name, ExprHdl meaning);
    bool define (PattHdl patt, ExprHdl meaning);
    void name   (VarHdl name, ExprHdl meaning);
    ExprHdl meaning_of (VarHdl name) const;     //returns defn or Undefined
    ExprHdl simple_meaning (VarHdl name) const; //returns defn or Undefined
    VarHdl name_of (ExprHdl meaning) const;     //returns name or Null handle
    inline bool contains (VarHdl name) const
    { return m_defs.find(&*name) != def_end(); }
private:
    void _define (VarHdl name, ExprHdl meaning);
    void _name (VarHdl name, ExprHdl meaning);
    Context (const RawDefs& raw, Int before)
    { for (Int i=0; i<before; ++i) define(raw[i].patt, raw[i].meaning); }
    Context operator[] (Int i) const { return Context(m_raw, i); }
public:

    //expansion & compression
    ExprHdl expand   (ExprHdl small, Int depth=MAX_DEPTH) const;
    ExprHdl compress (ExprHdl large, Int depth=0) const;

    //iteration
    typedef Defs::iterator        def_iter;
    typedef Defs::const_iterator  def_citer;
    def_iter   def_begin  ()       { return m_defs.begin(); }
    def_iter   def_end    ()       { return m_defs.end(); }
    def_citer  def_begin  () const { return m_defs.begin(); }
    def_citer  def_end    () const { return m_defs.end(); }
    def_iter  simp_begin  ()       { return m_simple.begin(); }
    def_iter  simp_end    ()       { return m_simple.end(); }
    def_citer simp_begin  () const { return m_simple.begin(); }
    def_citer simp_end    () const { return m_simple.end(); }

    //output
    //  brief:
    friend ostream& operator<< (ostream& os, Context& T);
    //  complete:
    void write_to (ostream& os);
    void save_to (ostream& os);
    void write_stats_to (ostream& os);
};

}

using Contexts::Context;

#endif
