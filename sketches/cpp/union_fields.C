#include <iostream>
using namespace std;

typedef unsigned int Int;
typedef float Real;

class Pos
{
private:
    Int m_int;
public:
    inline Pos& operator = (const Int _int) { m_int = _int << 2; }
    friend inline ostream& operator << (ostream& os, Pos pos);
};
inline ostream& operator << (ostream& os, Pos pos)
{
    return os << (pos.m_int >> 2);
}

union Word
{
Int m_int;
Real m_real;
Pos m_pos;
};

enum IntFieldIndex  {I0, I1};
enum RealFieldIndex {R0, R1};
enum PosFieldIndex  {P0, P1};

class Node
{
private:
    Word m_fields[2];
public:
    inline Int& operator [] (const IntFieldIndex index)
    { return m_fields[index].m_int; }
    inline Real& operator [] (const RealFieldIndex index)
    { return m_fields[index].m_real; }
    inline Pos& operator [] (const PosFieldIndex index)
    { return m_fields[index].m_pos; }
};

int main()
{
    Node n;
    n[I0] = 7u;
    n[R1] = 7.0f;
    cout << "n[I0] = " << n[I0] << "\n";
    cout << "n[R1] = " << n[R1] << "\n";
    cout << "n[I1] = " << n[I1] << "\n";
    cout << "n[R0] = " << n[R0] << "\n";
    n[P1] = 7;
    cout << "n[P1] = " << n[P1] << "\n";
    cout << "n[I1] = " << n[I1] << "\n";
    
    return 0;
}
