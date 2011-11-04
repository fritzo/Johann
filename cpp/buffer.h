#ifndef NONSTD_BUFFER_H
#define NONSTD_BUFFER_H

#include <set>

namespace nonstd
{

//buffers for unhandled objects
template <class T>
class buffer
{
    std::set<T*> m_buffer;
    typedef typename std::set<T*>::iterator Iter;
public:
    void insert (T* t) { m_buffer.insert( t); }
    void clear ();
};
template <class T>
void buffer<T>::clear ()
{
    for (Iter i=m_buffer.begin(); i!=m_buffer.end(); ++i) delete *i;
    m_buffer.clear();
}

}

#endif
