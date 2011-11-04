#include <new>
#include <iostream>

using namespace std;

int main(void)
{
    int size = 1 << 30;
    int *x;
    try {
        x = new int[size];
    } catch (bad_alloc) {
        cout << "failed\n";
    }
    for (int i=0; i<size; ++i) {
        x[size] = 1;
    }
    int t=0;
    for (int i=0; i<size; ++i) {
        t += x[size];
    }
    cout << "x = " << x << "\n";
    cout << "t = " << t << "\n";
}
    
