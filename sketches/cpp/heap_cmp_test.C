
#include <stdint.h>
#include <cstdlib>
#include <iostream>

typedef uint32_t Int;

//version with loops
Int heap_Hindex_1  (Int pos)
{//calculates value which increases in inorder iteration, i.e., bit reversal
    Int index = pos & 1;
    for (int i=0; i < 31; ++i) {
        index <<= 1;
        pos   >>= 1;
        index |= pos & 1;
    }
    return index;
}

Int heap_Hindex_2  (Int x)
{//from http://paul.rutgers.edu/~rhoads/Code/rev.long.c
    x = ((x & 0xAAAAAAAA) >>  1) | ((x & 0x55555555) <<  1);
    x = ((x & 0xCCCCCCCC) >>  2) | ((x & 0x33333333) <<  2);
    x = ((x & 0xF0F0F0F0) >>  4) | ((x & 0x0F0F0F0F) <<  4);
    x = ((x & 0xFF00FF00) >>  8) | ((x & 0x00FF00FF) <<  8);
    return (x >> 16) | (x << 16);
}

//copied from definitions.h
template <class T> inline int cmp(const T& lhs, const T& rhs)
{ return int(lhs>rhs) - int(lhs<rhs); }

int main ()
{
    /*simple tests
    std::cout << "cmp(5,7) = " << cmp(5,7) << std::endl;
    std::cout << "cmp(5,5) = " << cmp(5,5) << std::endl;
    std::cout << "cmp(7,5) = " << cmp(7,5) << std::endl;

    Int size = 31;
    for (Int i=1; i<=size; ++i) {
        std::cout << i << '\t' << _inorderIndex(i) << std::endl;
    }
    */

    /*makes sure functions do the same thing
    for (Int i=1; i<10000; ++i) {
        Int idx1 = heap_Hindex_1(i);
        Int idx2 = heap_Hindex_2(i);
        if (idx1 != idx2) {
            std::cout << "fuck, it don't work:\n"
                      << i << '\t' << idx1 << '\t' << idx2 << std::endl;
            exit(0);
        }
    }
    */

    //for profiling
    Int N = 1<<25;
    for (Int i=1; i<N; ++i) {
        heap_Hindex_2(i);
    }

    return 0;
}
