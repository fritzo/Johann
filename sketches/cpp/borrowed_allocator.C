
#include <vector>
#include <iostream>
#include <cstdlib>

int random_int () { return 1 + rand() % 16; }

int f1 (int x)
{//allocate-every-time
    std::vector<int> _vect;

    int end = random_int();
    for (int i=0; i<end; ++i) _vect.push_back(x*i);
    return _vect.back();
}

int f2 (int x)
{//keep a static copy
    static std::vector<int> _vect;
    _vect.clear();

    int end = random_int();
    for (int i=0; i<end; ++i) _vect.push_back(x*i);
    return _vect.back();
}

template<class T, int N>
class Borrowed
{
    static T s_bank[N];
    static unsigned s_usage;

    T* m_T;
    unsigned m_used;
public:
    Borrowed() : m_T(s_bank), m_used(1u)
    {
        while (s_usage && m_used) { ++m_T; m_used <<= 1u; }
        s_usage ^= m_used;
    }
    ~Borrowed() { s_usage ^= m_used; }

    T& operator *  () { return *m_T; }
    T* operator -> () { return m_T; }
};
template<class T, int N> T Borrowed<T,N>::s_bank[N];
template<class T, int N> unsigned Borrowed<T,N>::s_usage(0);

int f3 (int x)
{//get a borrowed version
    Borrowed<std::vector<int>,10> _vect;
    _vect->clear();

    int end = random_int();
    for (int i=0; i<end; ++i) _vect->push_back(x*i);
    return _vect->back();
}

int main (int argc, char** argv)
{
    int version = atoi(argv[1]);
    int runs = 1 << atoi(argv[2]);
    switch (version) {
    case 1:
        std::cout << "running f1 " << runs << " times" << std::endl;
        for (int x=0; x<runs; ++x) f1(x);
        break;
    case 2:
        std::cout << "running f2 " << runs << " times" << std::endl;
        for (int x=0; x<runs; ++x) f2(x);
        break;
    case 3:
        std::cout << "running f3 " << runs << " times" << std::endl;
        for (int x=0; x<runs; ++x) f3(x);
        break;
    default:
        std::cout << "unknown version: " << version << std::endl;
    }

    return 0;
}
