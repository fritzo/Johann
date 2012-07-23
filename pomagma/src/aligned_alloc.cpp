
#include "util.hpp"
#include "aligned_alloc.hpp"
#include <cstdlib> //for posix_memalign, free
#include <cstring> //for memset

//log levels
#define LOG_DEBUG1(mess)
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {POMAGMA_DEBUGmessage);}

namespace pomagma
{

// allocates an aligned array, wraps posix_memalign
void * alloc_blocks (size_t block_size, size_t block_count, size_t alignment)
{
    POMAGMA_DEBUG("Allocating " << block_count
                   << " blocks of size " << block_size << 'B');

    size_t byte_count = block_size * block_count;
    void * base;
    int info = posix_memalign(& base, alignment, byte_count);

    POMAGMA_ASSERT(info == 0,
            "posix_memalign failed to allocate " << byte_count << 'B');

    return base;
}

// just wraps free()
void free_blocks (void * base)
{
    POMAGMA_DEBUG("Freeing blocks");

    free(base);
}

} // namespace pomagma
