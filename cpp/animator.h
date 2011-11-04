#ifndef JOHANN_ANIMATOR_H
#define JOHANN_ANIMATOR_H

#include "definitions.h"

namespace Animator
{

const Logging::Logger logger("animate", Logging::DEBUG);

class Window
{
    static pthread_mutex_t s_mutex;
    pthread_t m_thread;
public:
    unsigned N;
    const Float *x, *y, *r;

    Window (int argc, char **argv);
    ~Window () {}

    void init (unsigned N_, const Float *_x, const Float *_y, const Float *_r)
    { N = N_, x = _x; y = _y; r = _r; }

    void lock () { pthread_mutex_lock( &s_mutex ); }
    void unlock () { pthread_mutex_unlock( &s_mutex ); }

    void start ();
    void update ();
};

}

#endif
