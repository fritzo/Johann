
#include <iostream>

class MyClass
{
    const int m_int;
public:
    MyClass (int i) : m_int(i) {}
    int get_int () const { return m_int; }
};

template <class T>
class MyClass
{
    const int m_int;
    const T   m_T;
public:
    MyClass (int i, T t) : m_int(i), m_T(t) {}
    int get_int () const { return m_int; }
    T   get_T   () const { return T; }
};

int main ()
{
    std::cout << "sizeof(MyClass) = "       << sizeof(MyClass) << std::endl;
    std::cout << "sizeof(MyClass<int>) = "  << sizeof(MyClass<int>) << std::endl;
    std::cout << "sizeof(MyClass<long>) = " << sizeof(MyClass<long>) << std::endl;

    return 0;
}
