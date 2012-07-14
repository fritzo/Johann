#ifndef NONSTD_ALIGNED_ALLOC_H
#define NONSTD_ALIGNED_ALLOC_H

namespace pomagma
{

unsigned roundUp (unsigned i);
unsigned roundDown (unsigned i);

//memory functions
void* alloc_blocks (size_t blockSize,
                    size_t numBlocks);
void  free_blocks (void* base);
void  clear_block (void* base,
                   size_t blockSize);
void copy_blocks (void*  destin_base,
                  const void*  source_base,
                  size_t blockSize,
                  size_t numBlocks);

template<class T> T * alloc_blocks (size_t numBlocks)
{
  return static_cast<T *>(alloc_blocks(sizeof(T), numBlocks));
}

}

#endif
