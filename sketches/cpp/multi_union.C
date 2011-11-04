
#include <iostream>

using namespace std;

template<int _size>
struct Perm
{
    char elements[_size];
    void init ()
    {
        for (int i=0; i<_size; ++i) {
            elements[i] = "abcdefg"[i];
        }
    }
    void print ()
    {
        for (int i=0; i<_size; ++i) {
            cout << elements[i] << "   ";
        }
        cout << "\n";
    }
};

template<int _size, int _source, int _target>
struct Swap : public Perm<_size>
{
    inline void operator () (void)
    {
        char temp = elements[_target];
        elements[_target] = elements[_source];
        elements[_source] = temp;
    }
};

template<int _size, int s1, int t1, int s2, int t2>
union TwoSwap
{
    Perm<_size> p;
    Swap<_size,s1,t1> act1;
    Swap<_size,s2,t2> act2;
};

template<int _size>
ostream& operator << (ostream& os, Perm<_size>& p)
{
    for (int i=0; i<_size; ++i) {
        os << p.elements[i] << " ";
    }
    return os;
}

int main(void)
{
    char  *left  = " \\ /    |\n  X     |\n / \\    |\n",
          *right = "|    \\ /\n|     X\n|    / \\\n";
    TwoSwap<3,0,1,1,2> braid;
    braid.p.init();
    braid.p.print();
    for (int t=0; t<10; ++t) {
        braid.act1();
        cout << left;
        braid.p.print();
        
        braid.act2();
        cout << right;
        braid.p.print();
    }
}

