
#include <iostream>

using namespace std;

class X
{
private:
    int size;
public:
    X(int _size) : size(_size) {}
    inline int next_mod(int t) { return (t+1) % size; }
    static inline int next(int t)            { return  (t+1); }
};

int main ()
{
    X x(10);
    cout << "x.next(9) = " << x.next(9) << "\n";
    cout << "x.next_mod(9) = " << x.next_mod(9) << "\n";
}
