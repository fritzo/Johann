
#include "simple.h"

namespace Simple
{

Simple::Simple (std::vector<ExprMass> basis, Float app)
    : m_app(app)
{
    for (Int i=0; i<basis.size(); ++i) {
        ExprHdl e(basis[i].first);
        Float   m(basis[i].second);
        mass(e,m);
    }
}

void Simple::_add (ExprHdl big, ExprHdl small)
{
    //logger.debug() << "adding " << big << " = " << small |0;
    Lock lock(m_map);
    (*m_map)[&*big] = Pair(big,small);
}
Float Simple::add (ExprHdl lhs, ExprHdl rhs)
{
    lhs = simplify(lhs->as_comb()); if (lhs->isBad()) return 0;
    rhs = simplify(rhs->as_comb()); if (rhs->isBad()) return 0;
    if (lhs == rhs) return 0;

    Float l = size(lhs);
    Float r = size(rhs);

    if (r > l) { _add(rhs,lhs); return r - l; }
    if (l > r) { _add(lhs,rhs); return l - r; }

    return 0;
}

ExprHdl Simple::_simplify (ExprHdl e) const
{//divide-and-conquer, look up cached value
    if (e->isApp()) e = _simplify(e->lhs())
                      * _simplify(e->rhs());

    map_citer i = m_map->find(&*e);
    return (i == m_map->end()) ? e : i->second.second;
}
#define MAX_STEPS 16
ExprHdl Simple::simplify (ExprHdl e) const
{//put in linear normal form, simplify, repeat
    logger.debug() << " simplifying " << e |0;
    e = e->as_comb()->reduce();
    Lock lock(m_map);
    for (Int i=0; i<MAX_STEPS; ++i) {
        e = _simplify(e);
        ExprHdl r = e->reduce();
        if (r == e) break;
        e = r;
    }
    return e;
}

Float Simple::_size (ExprHdl e) const
{
    bas_iter i = m_basis.find(&*e);
    if (i != m_basis.end()) return i->second.second;

    if (e->isApp()) {
        return m_app + size(e->lhs())
                     + size(e->rhs());
    }

    return INFINITY;
}

void Simple::update ()
{//try to keep idempotent
    for (map_iter i = map_begin(); i!=map_end(); map_next(i)) {
        LOCK_SYNTAX
        Pair& p = i->second;
        TODO(); //XXX what if p.second = p.first?
        p.second = simplify(p.first);
        UNLOCK_SYNTAX
    }
}

//output
void Simple::write_stats (ostream& out) const
{
    Lock lock(m_map);
    out << "basis size: " << m_basis.size() << '\n';
    out << "equations: " << m_map->size() << std::endl;
}
void Simple::write_lang (ostream& out) const
{
    out << "|app| = " << m_app << '\n';
    out << "basis = {";
    LOCK_SYNTAX
    for (bas_iter i = bas_begin(); i!=bas_end(); bas_next(i)) {
        out << "\n\t" << i->second.first << "\t@ " << i->second.second;
    }
    UNLOCK_SYNTAX
    out << "\n}" << std::endl;
}

}

