#ifndef NONSTD_PRIORITY_QUEUE
#define NONSTD_PRIORITY_QUEUE

#include "definitions.h" //for assert
#include <set>

namespace nonstd
{

using Logging::logger;

/// A priority queue supporting push,pop,merge,remove,rename
template<class T>
class priority_queue
{
    typedef std::set<T> DataType;
    DataType m_data;
public:
    int  size     ()       const { return m_data.size(); }
    bool empty    ()       const { return m_data.empty(); }
    operator bool ()       const { return not m_data.empty(); }
    void clear    ()             { m_data.clear(); }
    bool contains (T item) const { return m_data.find(item) == m_data.end(); }
    void push     (T item)       { m_data.insert(item); }
    void remove   (T item)       { m_data.erase(item); }
    inline T pop  ();                   ///< pops lowest element
    void merge    (T dep, T rep);       ///< replaces dep with rep
    void rename   (const T* old2new);   ///< renames all elements, assuming 1-1
    template<class Cond> void remove_if (const Cond& cond);
    inline void remove_safe (T item);
    inline void prune ();

    //iteration
    typedef typename DataType::iterator iterator;
    iterator begin () { return m_data.begin(); }
    iterator end   () { return m_data.end(); }
    void erase (iterator iter) { m_data.erase(iter); }

};
template<class T> inline T priority_queue<T>::pop ()
{//pops lowest element
    Assert1(!empty(), "tried to pop element off empty queue");
    iterator iter = m_data.begin();
    T item = *iter;
    m_data.erase(iter);
    return item;
}
template<class T> void priority_queue<T>::merge (T dep, T rep)
{//replaces old entry (dep) with its new representative (rep)
    iterator old_iter = m_data.find(dep);
    if (old_iter == m_data.end()) return;
    m_data.erase(old_iter);
    if (m_data.find(rep) == m_data.end()) {
        m_data.insert(rep);
    }
}
template<class T> void priority_queue<T>::rename (const T* old2new)
{//renames all elements, assuming 1-1 renaming
    DataType new_data;
    for (DataType iter = m_data.begin(); iter != m_data.end(); ++iter) {
        new_data.insert(old2new[*iter]);
    }
    m_data.swap(new_data);
}
template<class T> template<class Cond>
void priority_queue<T>::remove_if (const Cond& cond)
{
    for (iterator iter=m_data.begin(); iter!=m_data.end();) {
        if (cond(*iter)) {
            iterator prev = iter;
            ++iter;
            m_data.erase(prev);
        } else {
            ++iter;
        }
    }
}
template<class T> inline void priority_queue<T>::remove_safe(T item)
{
    iterator iter = m_data.find(item);
    if (iter != m_data.end()) m_data.erase(iter);
}
template<class T> inline void priority_queue<T>::prune ()
{
    Assert1(!empty(), "tried to prune element from empty queue");
    m_data.erase(m_data.rbegin());
}

}

#endif
