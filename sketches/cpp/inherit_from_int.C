#include <iostream>
using namespace std;

class Int
{
    int m_value;
public:
    //conversions only
    Int (int value) : m_value(value) {}
    Int& operator = (int value) { m_value = value; return *this; }
    operator int& () { return m_value; } //this may not be efficient?
};

int main ()
{
    for (Int I = 0; I < 32; ++I) {
        cout << "I = " << I << endl;
    }

    return 0;
}

