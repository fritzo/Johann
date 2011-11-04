
#include <iostream>

int primes[] {2, 3, 5, 7, 9, 11, 13, 17, 23, 29, 31, 37, 41};

inline int& operator* (const int i) { return primes[i]; }

void main ()
{
    for (int i=0; i<10; ++i) {
        std::cout << *i << "\n";
    }
}
    

