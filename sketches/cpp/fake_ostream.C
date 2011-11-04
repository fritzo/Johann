
#include <iostream>

using namespace std;

class fake_ostream
{
    const bool m_live;
public:
    fake_ostream (bool live) : m_live(live) {}
    template <class Message>
    const fake_ostream& operator << (const Message& message) const
    { if (m_live) { cout << message; } return *this; }
};

const fake_ostream live_out(true), dead_out(false);

class Number
{
    int m_num;
public:
    Number (int num) : m_num(num) {}
    friend inline ostream& operator<< (ostream& os, const Number& num)
    { return os << num.m_num; }
};

int main ()
{
    Number num(5);
    live_out << num;
    dead_out << num;

    return 0;
}

