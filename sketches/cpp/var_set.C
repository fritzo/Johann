
#include <iostream>

using namespace std;
typedef unsigned int Int;

class VarSet
{//bitfield set
    Int m_int;
public:
    //no constructors or destructors: this is used with reinterpret_cast

    //factories
    explicit VarSet (Int i) : m_int(i) {} //how to avoid this?
    //operator Int () const { return m_int; }
    static VarSet singleton (Int varNum) { return VarSet(1<<varNum); }
    static VarSet full_set  () { return VarSet(0xFFFFFFFF); }
    static VarSet empty_set () { return VarSet(0); }

    //union, intersection, difference operators
    VarSet& operator += (VarSet other) { m_int |= other;  return *this; }
    VarSet& operator *= (VarSet other) { m_int &= other;  return *this; }
    VarSet& operator -= (VarSet other) { m_int &= ~other; return *this; }

    VarSet operator + (VarSet other) const
    { return VarSet(m_int | other.m_int); }
    VarSet operator * (VarSet other) const
    { return VarSet(m_int & other.m_int); }
    VarSet operator - (VarSet other) const
    { return VarSet(m_int & ~other.m_int); }

    //properties, insertion/removal
    void clear () { m_int = 0; }
    bool empty () { return m_int == 0; }
    operator bool () const { return m_int != 0; }
    bool contains (Int varNum) const { return m_int & (1<<varNum); }
    void insert   (Int varNum)       { m_int |= (1<<varNum); }
    void remove   (Int varNum)       { m_int &= ~(1<<varNum); }
};

ostream& operator<< (ostream& os, VarSet vars)
{
    static const char* varnames="abcdefghijklmnopqrstuvwxyz";
    const Int numVars = 7;
    os << "{";
    for (Int i=0; i<numVars; ++i) {
        if (vars.contains(i)) {
            os << varnames[i];
        }
    }
    return os << "}";
}

int main ()
{
    VarSet full  = VarSet::full_set(),
           empty = VarSet::empty_set();
    cout << "empty \t= " << empty << endl;
    cout << "full \t= " << full << endl;

    VarSet a = VarSet::singleton(0),
           b = VarSet::singleton(1),
           c = VarSet::singleton(2);
    cout << "{a} \t= " << a << endl;
    cout << "{b} \t= " << b << endl;
    cout << "{c} \t= " << c << endl;

    VarSet ab = a+b,
           abc = ab+c,
           ac = abc-b,
           a_ = ac * ab;
    cout << "{ab} \t= " << ab << endl;
    cout << "{abc} \t= " << abc << endl;
    cout << "{ac} \t= " << ac << endl;
    cout << "{a} \t= " << a_ << endl;

    cout << "is a in {a}?    " << (a.contains(0) ? "yes" : "no") << endl;
    cout << "is b in {a}?    " << (a.contains(1) ? "yes" : "no") << endl;
    cout << "is a in {b}?    " << (b.contains(0) ? "yes" : "no") << endl;
    cout << "is b in {b}?    " << (b.contains(1) ? "yes" : "no") << endl;
    cout << "is c in {abc}?  " << (abc.contains(2) ? "yes" : "no") << endl;
}

