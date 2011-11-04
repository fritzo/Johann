#include <iostream>
using namespace std;

template<class Node>
class Pos
{
    static Node* s_base;
    int m_offset;
public:
    //conversions
    Int (int offset) : m_offset(offset) {}
    Int& operator = (int offset) { m_offset = offset; return *this; }
    operator int& () { return m_offset; } //this may not be efficient?

    //dereferencing
    inline Node  operator *  () { return s_base[m_offset]; }
    inline Node* operator -> () { return s_base + m_offset; }

    //base math, no allocation
    static void set_base (Node* base) { s_base = base; }

    //indexing
    //...
};

int main ()
{
    for (Int I = 0; I < 32; ++I) {
        cout << "I = " << I << endl;
    }

    return 0;
}

