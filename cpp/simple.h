#ifndef JOHANN_SIMPLE_H
#define JOHANN_SIMPLE_H

#include "definitions.h"
#include "expressions.h"
#include "thread_tools.h"
#include <cmath> //for INFINITY
#include <vector>
#include <map>
#include <utility>

namespace Simple {

const Logging::Logger logger("simple", Logging::DEBUG);

namespace TT = ThreadTools;
using TT::Lock;

/* Invariant: simplify(-) should
 * (1) be idempotent and
 * (2) return only linear normal forms
 */

class Simple
{
    //invariant: the map can only grow, so iterators are not invalidated
    typedef std::pair<ExprHdl,ExprHdl> Pair;
    typedef std::map<Expr*,Pair> Map;
    TT::ThreadSafe<Map> m_map;
public:
    typedef Map::iterator map_iter;
    map_iter map_begin () { Lock lock(m_map); return m_map->begin(); }
    map_iter map_end   () { Lock lock(m_map); return m_map->end(); }
    void     map_next  (map_iter& i) { Lock lock(m_map); ++i; }

    typedef Map::const_iterator map_citer;
    map_citer map_begin () const { Lock lock(m_map); return m_map->begin(); }
    map_citer map_end   () const { Lock lock(m_map); return m_map->end(); }
    void      map_next  (map_citer& i) const { Lock lock(m_map); ++i; }

private:
    typedef std::pair<ExprHdl, Float> ExprMass;
    typedef std::map<Expr*, ExprMass> Basis;
    Basis m_basis;
    Float m_app;
public:
    typedef Basis::const_iterator bas_iter;
    bas_iter bas_begin () const { Lock lock(m_map); return m_basis.begin(); }
    bas_iter bas_end   () const { Lock lock(m_map); return m_basis.end(); }
    void     bas_next  (bas_iter& i) const { Lock lock(m_map); ++i; }

    //basis access
    void  app (Float a) { m_app = a; }
    Float app () const { return m_app; }
    void  mass (ExprHdl e, Float m) { m_basis[&*e] = ExprMass(e,m); }
    Float mass (ExprHdl e) const
    {
        bas_iter i=m_basis.find(&*e);
        return i==bas_end() ? 0 : i->second.second;
    }

    //expr access
private:
    void _add (ExprHdl big, ExprHdl small);
    Float _size (ExprHdl e) const;
    ExprHdl _simplify (ExprHdl e) const;
public:
    Float add (ExprHdl lhs, ExprHdl rhs); //returns size decrease
    Float size (ExprHdl e) const { return _size(e->as_comb()); }
    ExprHdl simplify (ExprHdl e) const;
    ExprHdl operator() (ExprHdl e) { return simplify(e); }

    //stats
    void write_stats (ostream& out) const;
    void write_lang (ostream& out) const;

    //high-level interface
    Simple () : m_app(0) {}
    Simple (std::vector<ExprMass> basis, Float app);
    void update (); //try to keep idempotent

    //clearing is done only at the end, and invalidates iterators
    //locks are thus worthless
    void clear () { m_map->clear(); m_basis.clear(); }
};

}

#endif
