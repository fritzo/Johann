#ifndef NONSTD_ALIGNED_ALLOC_H
#define NONSTD_ALIGNED_ALLOC_H

#include "util.hpp"

namespace pomagma
{

void * alloc_blocks (
        size_t block_size,
        size_t block_count,
        size_t alignment = 32);

template<class T> T * alloc_blocks (size_t block_count)
{
  return static_cast<T *>(alloc_blocks(sizeof(T), block_count));
}

void  free_blocks (void * base);

template<class T>
class AlignedBuffer : noncopyable
{
    enum { default_size = 1024 };
    size_t m_size;
    T * m_data;

public:

    AlignedBuffer ()
        : m_size(default_size),
          m_data(alloc_blocks<T>(m_size))
    {
    }

    ~AlignedBuffer ()
    {
        free_blocks(m_data);
    }

    T * operator() (size_t size)
    {
        if (m_size < size) {
            while (m_size < size) {
                m_size *= 2;
            }
            free_blocks(m_data);
            m_data = alloc_blocks<T>(m_size);
        }
        return m_data;
    }
};

} // namespace pomagma

#endif // NONSTD_ALIGNED_ALLOC_H
