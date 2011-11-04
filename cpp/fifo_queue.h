#ifndef NONSTD_FIFO_QUEUE
#define NONSTD_FIFO_QUEUE

#include "definitions.h" //for assert
#include <utility>

//hash functions
#ifdef __GNUG__
    #include "hash_map.h"
    #define MAP_TYPE std::unordered_map
#else
    #include <map>
    #define MAP_TYPE std::map
#endif

namespace nonstd
{

using Logging::logger;

/// A FIFO queue supporting push,pop,merge,remove.
template<class T> //REQUIREMENT: T must have null value T(0)
class fifo_queue
{
    struct DLL { T prev,next; DLL(T p=T(0),T n=T(0)) : prev(p), next(n) {} };
    typedef MAP_TYPE<T,DLL> Queue;
    typedef typename Queue::value_type Value;
    typedef std::pair<typename Queue::iterator, bool> Inserted;

    T m_front, m_back;
    Queue m_queue;

    T& next (T x) { return m_queue.find(x)->second.next; }
    T& prev (T x) { return m_queue.find(x)->second.prev; }
public:
    fifo_queue () : m_front(0), m_back(0) {}

    //item access
    operator bool () const { return m_front; }
    bool empty    () const { return !m_front; }
    inline void push  (T x);
    inline T    pop   ();
    inline void remove (T t);
    void merge (T dep, T rep) { remove(dep); push(rep); }

    //iteration
    class iterator
    {
        T m_x;
        fifo_queue<T> *m_queue;
    public:
        iterator () : m_x(0), m_queue(NULL) {}
        iterator (T x, fifo_queue<T> *q) : m_x(x), m_queue(q) {}

        bool operator== (const iterator &i) const { return m_x == i.m_x; }
        bool operator!= (const iterator &i) const { return m_x != i.m_x; }
        void operator++ () { m_x = m_queue->prev(m_x); }
        T operator++ (int) { T x = m_x; m_x = m_queue->prev(m_x); return x; }

        const T& operator*  () const { return m_x; }
        const T* operator-> () const { return &m_x; }
    };
    iterator begin () { return iterator(m_back, this); }
    iterator end   () { return iterator(); }
};
template<class T> inline void fifo_queue<T>::push (T x)
{//pushes element if not already equeued
    Assert2(x, "tried to push null x");
    if (m_queue.insert(Value(x,DLL(m_back))).second) { //x is new
        (m_back ? next(m_back) : m_front) = x;
        m_back = x;
    }
}
template<class T> inline T fifo_queue<T>::pop ()
{//pops oldest element
    Assert1(!empty(), "tried to pop element off empty queue");
    T x = m_front;
    typename Queue::iterator i = m_queue.find(x);
    Assert2(i != m_queue.end(), "front not in queue");
    const DLL& dll = i->second;
    ((m_front = dll.next) ? prev(m_front) : m_back) = T(0);
    m_queue.erase(i);
    return x;
}
template<class T> inline void fifo_queue<T>::remove (T x)
{//removes element if it exists
    typename Queue::iterator i = m_queue.find(x);
    if (i == m_queue.end()) return;
    const DLL& dll = i->second;
    (dll.prev ? next(dll.prev) : m_front) = dll.next;
    (dll.next ? prev(dll.next) : m_back ) = dll.prev;
    m_queue.erase(i);
}

}

#endif
