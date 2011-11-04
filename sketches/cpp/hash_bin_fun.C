
#include <stdint.h>
#include <iostream>
#include <ext/hash_map>
#include <utility>

typedef uint32_t Int;
typedef uint64_t Long;

struct HashIntPair
{
    size_t operator() (std::pair<Int,Int> p) const
    {
        Int x = p.first;
        Int y = p.second;
        return ((y << 16) ^ (y >> 16)) ^ x;
    }
};

typedef __gnu_cxx::hash_map<std::pair<Int,Int>, Int, HashIntPair> Table;

inline Long pair_fun (Int x, Int y)
{
    Long m = x+y;
    return (m * (m+1) * y)/2;
}

inline bool contained (Int i, Int j)
{
    //slow version
    //return pair_fun(i,j) % 8;

    //equivalent fast version
    i &= 0xF;
    j &= 0xF;
    return not ((i * (i+1) * j) / 2) % 8;
}

int main (void)
{
    Table table;
    Int N = 1 << 13;

#define iterate for(int i=0; i<N; ++i) for(int j=0; j<N; ++j) if(contained(i,j))

    std::cout << "building table" << std::endl;
    iterate table[std::make_pair(i,j)] = i*j;

    std::cout << "testing key lookup" << std::endl;
    iterate if (table[std::make_pair(i,j)] != i*j) {
        std::cout << "  failed on key (" << i << "," << j << ")" << std::endl;
        return 0;
    }

    return 0;
}

