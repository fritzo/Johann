#ifndef JOHANN_THREAD_TOOLS_H
#define JOHANN_THREAD_TOOLS_H

#include "definitions.h"
#include <pthread.h>

namespace ThreadTools
{

const Logging::Logger logger("thread", Logging::DEBUG);

template<class T>
class ThreadSafe
{
    T m_val;
    mutable pthread_mutex_t m_mutex;
    friend class Lock;
public:
    typedef T Type;
    ThreadSafe () { pthread_mutex_init( &m_mutex, NULL ); }
    ThreadSafe (T t) : m_val(t) { pthread_mutex_init( &m_mutex, NULL ); }
    ~ThreadSafe () { pthread_mutex_destroy( &m_mutex ); }

    T get () const
    {
        pthread_mutex_lock( &m_mutex );
        T result = m_val;
        pthread_mutex_unlock( &m_mutex );
        return result;
    }
    void set (T t)
    {
        pthread_mutex_lock( &m_mutex );
        m_val = t;
        pthread_mutex_unlock( &m_mutex );
    }
    T operator() () const { return get(); }
    void operator() (T t) { set(t); }

    //unsafe access, to be used with Locks
          T& operator*  ()       { return m_val; }
    const T& operator*  () const { return m_val; }
          T* operator-> ()       { return &m_val; }
    const T* operator-> () const { return &m_val; }
};

class Lock
{
    pthread_mutex_t* m_mutex;
public:
    template<class T>
    Lock (const ThreadSafe<T>& t) : m_mutex(&(t.m_mutex))
             { pthread_mutex_lock( m_mutex ); }
    ~Lock () { pthread_mutex_unlock( m_mutex ); }
};

void new_thread (void* (*f)(void*), void* x=NULL);

}

#endif
