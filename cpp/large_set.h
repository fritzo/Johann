#ifndef NONSTD_LARGE_SET_H
#define NONSTD_LARGE_SET_H

#include "definitions.h"
#include "handling.h"
#include <set>
#include <vector>

namespace nonstd
{

using Logging::logger;

typedef void* Elt;
class HandledSet_ : public Handling::HandledObject
{
    typedef std::set<Elt> Set;
    Set m_set;
public:
    HandledSet_ () {}
    HandledSet_ (Elt elt) { m_set.insert(elt); }
    HandledSet_ (const HandledSet_& other) : m_set(other.m_set) {}
    virtual ~HandledSet_ () {}

    //total access
    operator bool () const { return not m_set.empty(); }
    bool empty  () const { return m_set.empty(); }
    size_t size () const { return m_set.size(); }
    void clear  ()       { m_set.clear(); }

    //item access
    void insert (Elt elt) { m_set.insert(elt); }
    void remove (Elt elt) { m_set.erase(elt); } //unsafe
    bool contains (Elt elt) { return m_set.find(elt) != m_set.end(); }

    //iteration
    typedef Set::iterator iterator;
    iterator begin () { return m_set.begin(); }
    iterator end   () { return m_set.end(); }
    typedef Set::const_iterator const_iterator;
    const_iterator begin () const { return m_set.begin(); }
    const_iterator end   () const { return m_set.end(); }
};

/// large sets that try to avoid being copied
class LargeSet
{
    typedef HandledSet_ Set;
    typedef Handling::Handle<Set> SetHandle;
    SetHandle m_set;

    explicit LargeSet (Set* s) : m_set(s) {}
    LargeSet () : m_set() {}
public:
    //deletion, & copying are correct by default

    //factories
    static LargeSet singleton (Elt elt) { return LargeSet(new Set(elt)); }
    static LargeSet empty_set ()        { return LargeSet(); }

    //union, intersection, difference operators
    LargeSet operator+ (const LargeSet& s) const;
    LargeSet operator* (const LargeSet& s) const;
    LargeSet operator- (const LargeSet& s) const;
    LargeSet operator^ (const LargeSet& s) const;

    LargeSet& operator*= (const LargeSet& s) { return *this = s * *this; }
    LargeSet& operator+= (const LargeSet& s) { return *this = s + *this; }
    LargeSet& operator-= (const LargeSet& s) { return *this = s - *this; }

    //entire-set access
    bool empty () const { return not m_set; }
    operator bool () const { return m_set; }
    Int size () const { return m_set ? m_set->size() : 0; }
    void clear () { m_set.clear(); }
    std::vector<Elt> as_vector ()
    { return m_set ? std::vector<Elt>(begin(), end())
                   : std::vector<Elt>(); };

    //element access
    void insert   (Elt elt);
    void remove   (Elt elt); //safe
    bool contains (Elt elt) const
    { return m_set ? m_set->contains(elt) : false; }
    Elt any () const { return m_set ? *(begin())  : NULL; }

    //iteration
    typedef Set::const_iterator Iter;
    typedef Set::const_iterator iterator;
private:
    static const std::set<Elt> _empty_set;
public:
    Iter begin () const
    {
        return m_set ? const_cast<const Set*>(&*m_set)->begin()
                     : _empty_set.begin();
    }
    Iter end () const
    {
        return m_set ? const_cast<const Set*>(&*m_set)->end()
                     : _empty_set.end();
    }

    //validation
    void validate () const
    { Assert ((not m_set) or (not m_set->empty()), "LargeSet's set was empty"); }

};

}

#endif
