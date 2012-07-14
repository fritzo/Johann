
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


unsigned roundUp (unsigned i)
{//rounds up to next power of two
    unsigned j = 1;
    for (i-=1; i; i>>=1) j <<= 1;
    return j;
}
unsigned roundDown (unsigned i)
{//rounds down to previous power of two
    unsigned j = 1;
    for (i>>=1; i; i>>=1) j <<= 1;
    return j;
}

void* alloc_blocks (size_t blockSize, size_t numBlocks)
{//allocates an aligned array, wraps posix_memalign
    POMAGMA_DEBUG("Allocating " << numBlocks
                   << " blocks of size " << blockSize << 'B');

    size_t alignment = max(16u, roundDown(blockSize));
    size_t numBytes = blockSize * numBlocks;
    void * base;
    if (posix_memalign(& base, alignment, numBytes)) {
        POMAGMA_WARN("posix_memalign failed");
        return NULL;
    } else {
        return base;
    }
}
void free_blocks (void* base)
{//just wraps free()
    POMAGMA_DEBUG("Freeing blocks");

    free(base);
}
void clear_block (void* base, size_t blockSize)
{//sets data to zero, wraps memset
    LOG_DEBUG1( "Clearing block of size " << blockSize << 'B' )

    //std::memset(base, 0, blockSize);
    bzero(base, blockSize);
}
void copy_blocks (void* destin_base, const void* source_base,
                  size_t blockSize, size_t numBlocks)
{//justs wraps for memcpy
    LOG_DEBUG1( "Copying blocks" )

    memcpy(destin_base, source_base, blockSize * numBlocks);
}

}

