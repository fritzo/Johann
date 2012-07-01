
#include "thread_tools.h"

namespace ThreadTools
{

//threading
class Thread
{
    pthread_t p_thread;
    void* (*m_f)(void*);
    void* m_x;

    static void* run (void* _me);
    ~Thread () {} //self-deleting
public:
    Thread (void* (*f)(void*), void* x=NULL);
};
Thread::Thread (void* (*f)(void*), void* x)
    : m_f(f), m_x(x)
{
    AssertW(pthread_create( &p_thread, NULL, run, this) == 0,
            "pthread_create() failed");
}
void* Thread::run (void* _me)
{
    Thread* me = static_cast<Thread*>(_me);
    me->m_f(me->m_x);
    delete me;
    return NULL;
}
void new_thread (void* (*f)(void*), void* x) { new Thread(f,x); }

}

