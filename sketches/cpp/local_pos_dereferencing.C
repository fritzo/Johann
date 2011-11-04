
#include <iostream>

using namespace std;

class Pos(pos)
{
private:
    int m_offset;
public:
    Pos(int offset) : m_offset(offset) {}
    operator int () { return m_offset ; }
};

template<class T, int length>
class Tuple
{
private:
    T m_data[length];
public:
    inline T& operator [] (int index) { return m_data[index]; }
};

template<class T, int length>
class Container
{
public:
    typedef Tuple<T,length> TT;
private:
    const int m_size;
    const TT *m_data;
public:
    Container(int size) : m_size(size), m_data(new TT[size]) {}
    ~Container() { delete m_data; }

    inline T& operator [] (Pos pos, int index) { return m_data[pos][size]; }
};

int main()
{
    Container<double,2> data(5);
    .....

}
    
