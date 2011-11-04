
#include "definitions.h"
#include "aligned_alloc.h"
#include <cstdlib> //for posix_memalign, free
#include <cstring> //for memset

//#ifdef MAC_HACKS
//    #define posix_memalign(base,_,size) ((*base = malloc(size))==NULL)
//#endif

//log levels
#define LOG_DEBUG1(mess)
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << message |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

namespace nonstd
{

using Logging::logger;

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
    logger.debug() << "Allocating " << numBlocks
                   << " blocks of size " << blockSize << 'B' |0;
    Logging::IndentBlock block;

    size_t alignment = roundDown(blockSize);
    size_t numBytes = blockSize * numBlocks;
    void* base;
#ifdef MAC_HACKS
    base = malloc(numBytes);
    AssertW(reinterpret_cast<size_t>(base) % alignment == 0,
            "bad alignment of block of size " << numBytes << 'B');
    return base;
#else
    if (posix_memalign(&base, alignment, numBytes)) {
        logger.warning() << "posix_memalign failed" |0;
        return NULL;
    } else {
        return base;
    }
#endif
}
void free_blocks (void* base)
{//just wraps free()
    logger.debug() << "Freeing blocks" |0;
    Logging::IndentBlock block;

    free(base);
}
void clear_block (void* base, size_t blockSize)
{//sets data to zero, wraps memset
    LOG_DEBUG1( "Clearing block of size " << blockSize << 'B' )
    Logging::IndentBlock block;

    //std::memset(base, 0, blockSize);
    bzero(base, blockSize);
}
void copy_blocks (void* destin_base, const void* source_base,
                  size_t blockSize, size_t numBlocks)
{//justs wraps for memcpy
    LOG_DEBUG1( "Copying blocks" )
    Logging::IndentBlock block;

    memcpy(destin_base, source_base, blockSize * numBlocks);
}

}

