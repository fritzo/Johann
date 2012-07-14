#ifndef POMAGMA_HASH_MAP_H
#define POMAGMA_HASH_MAP_H

#include <unordered_map>
#include <utility>

namespace std
{

template <>
struct hash<pair<uint32_t, uint32_t> >
{
    uint32_t operator()(pair<uint32_t, uint32_t> __x) const
    {
        uint32_t x = __x.first;
        uint32_t y = __x.second;
        return (y << 16) ^ (y >> 16) ^ x;
    }
};

template <>
struct hash<pair<uint64_t, uint64_t> >
{
    uint64_t operator()(pair<uint64_t, uint64_t> __x) const
    {
        uint64_t x = __x.first;
        uint64_t y = __x.second;
        return (y << 32) ^ (y >> 32) ^ x;
    }
};

}

#endif // POMAGMA_HASH_MAP_H
