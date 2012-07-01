#ifndef NONSTD_SMALL_SET_H
#define NONSTD_SMALL_SET_H

#include "definitions.h"
#include <vector>

namespace nonstd
{

/// A small set class that fits into one word.
class SmallSet
{//bitfield set
    Int m_int;
public:
    enum { NUM_ELTS = 32 };
    //no constructors or destructors: this is used with reinterpret_cast

    //factories
    explicit SmallSet (Int i) : m_int(i) {} //how to avoid this?
    static SmallSet singleton (Int elt) { return SmallSet(1<<elt); }
    static SmallSet full_set  () { return SmallSet(0xFFFFFFFF); }
    static SmallSet empty_set () { return SmallSet(0); }

    //union, intersection, difference operators
    SmallSet& operator += (SmallSet s) { m_int |= s.m_int;  return *this; }
    SmallSet& operator *= (SmallSet s) { m_int &= s.m_int;  return *this; }
    SmallSet& operator -= (SmallSet s) { m_int &= ~s.m_int; return *this; }

    SmallSet operator + (SmallSet s) const { return SmallSet(m_int | s.m_int); }
    SmallSet operator * (SmallSet s) const { return SmallSet(m_int & s.m_int); }
    SmallSet operator - (SmallSet s) const { return SmallSet(m_int & ~s.m_int); }
    SmallSet operator ^ (SmallSet s) const { return SmallSet(m_int ^ s.m_int); }

    //entire-set access
    bool empty () const { return m_int == 0; }
    operator bool () const { return m_int != 0; }
    Int size () const;
    void clear () { m_int = 0; }
    std::vector<Int> as_vector () const;

    //element access
    bool contains (Int elt) const { return m_int & (1<<elt); }
    void insert   (Int elt)       { m_int |= (1<<elt); }
    void remove   (Int elt)       { m_int &= ~(1<<elt); }
    Int min () const;
    Int max () const;
};

}

ostream& operator<< (ostream& os, nonstd::SmallSet elts);

#endif
