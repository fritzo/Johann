
#include "large_set.h"

namespace nonstd
{

const std::set<Elt> LargeSet::_empty_set;

//operations
LargeSet LargeSet::operator+ (const LargeSet& s) const
{
    if (empty()) return s;
    if (s.empty()) return *this;
    Set *r = new Set(*m_set);
    for (Iter i=s.begin(), e=s.end(); i!=e; ++i) {
        r->insert(*i);
    }
    return LargeSet(r);
}
LargeSet LargeSet::operator* (const LargeSet& s) const
{
    if (empty() or s.empty()) return LargeSet();
    Set *r = new Set();
    for (Iter i=s.begin(), e=s.end(); i!=e; ++i) {
        if (contains(*i)) r->insert(*i);
    }
    if (*r)   return LargeSet(r);
    delete r; return LargeSet();
}
LargeSet LargeSet::operator- (const LargeSet& s) const
{
    if (empty()) return LargeSet();
    if (s.empty()) return *this;
    Set *r = new Set();
    for (Iter i=begin(), e=end(); i!=e; ++i) {
        if (not s.contains(*i)) r->insert(*i);
    }
    if (*r)   return LargeSet(r);
    delete r; return LargeSet();
}
LargeSet LargeSet::operator^ (const LargeSet& s) const
{
    if (empty()) return s;
    if (s.empty()) return *this;
    Set *r = new Set();
    for (Iter i=begin(), e=end(); i!=e; ++i) {
        if (not s.contains(*i)) r->insert(*i);
    }
    for (Iter i=s.begin(), e=s.end(); i!=e; ++i) {
        if (not contains(*i)) r->insert(*i);
    }
    if (*r)   return LargeSet(r);
    delete r; return LargeSet();
}

//element access
void LargeSet::insert (Elt elt)
{
    if (empty()) { m_set.set(*(new Set(elt))); return; }
    if (contains(elt)) return;
    Set *r = new Set(*m_set);
    r->insert(elt);
    m_set.set(*r);
}
void LargeSet::remove (Elt elt)
{
    if (not contains(elt)) return;
    Set *r = new Set(*m_set);
    r->remove(elt);
    if (*r) { m_set.set(*r); return; }
    delete r; m_set.clear();
}

}

