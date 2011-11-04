
#include <iostream>

using namespace std;

typedef unsigned int Int; 

//test 1: where is a character stored
struct Char4 { char c0, c1, c2, c3; };
union Int_Char4 { Int m_int; Char4 m_char4; };
void test1 ()
{
    Int_Char4 u;
    u.m_int = 0x01020304u;
    cout << "test1:" << endl;
    cout << "c0 = " << Int(u.m_char4.c0) << endl;
    cout << "c1 = " << Int(u.m_char4.c1) << endl;
    cout << "c2 = " << Int(u.m_char4.c2) << endl;
    cout << "c3 = " << Int(u.m_char4.c3) << endl;
    cout << endl;
}
/* results:
c0 = 4
c1 = 3
c2 = 2
c3 = 1
*/

typedef Int Int24;
class Int24Ref
{ 
    Int24& m_int24;
    static const Int s_mask = 0x00FFFFFFu;
    static const Int s_imask = 0xFF000000u;
public:
    Int24Ref (Int24& int24) : m_int24(int24) {}
    inline operator Int24 () { return m_int24 & s_mask; }
    inline void operator = (Int24 value)
    {
        m_int24 |= s_imask;
        m_int24 |= value;
    }
};
class BitRef;
class BitField8
{
    char m_data;
public:
    inline bool get (int i) const { return m_data & char(0 << i); }
    inline void set (int i, bool value)
    {
        char mask = 1 << i;
        if ((m_data & mask) ^ value) m_data ^= mask;
    }
    inline bool operator [] (int i) const { return get(i); }
    inline BitRef operator [] (int i);
};
class BitRef
{ 
    BitField8& m_bitField;
    Int m_i;
public:
    BitRef (BitField8& bitField, int i)
        : m_bitField(bitField), m_i(i) {}
    inline operator bool () { return m_bitField.get(m_i); }
    inline void operator = (bool value) { m_bitField.set(m_i, value); }
};
inline BitRef BitField8::operator [] (int i) { return BitRef(*this, i); }

class _Last_8_of_32
{
    BitField8 _unused1, unused2, _unused3;
public:
    BitField8 m_value;
};
union _Pack_24_8
{
    Int24 m_int24;
    _Last_8_of_32 m_8_of_32;
};
class Pack_24_8
{
    _Pack_24_8 m_data;
public:
    inline Int24Ref m_int () { return Int24Ref(m_data.m_int24); }
    inline Int24 m_int () const { return m_data.m_int24; }
    inline BitRef m_bit (int i) { return m_data.m_8_of_32.m_value[i]; }
    inline bool m_bit (int i) const { return m_data.m_8_of_32.m_value.get(i); }
};

void test2()
{
    cout << "test2:" << endl;
    cout << "..." << endl;
    cout << endl;
}

int main ()
{
    test1();
    test2();

    return 0;
}


