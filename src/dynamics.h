#ifndef JOHANN_DYNAMICS_H
#define JOHANN_DYNAMICS_H

#include "definitions.h"
#include "meas_lite.h"
#include <cmath>

namespace Dynamics
{

const Logging::Logger logger("dynamic", Logging::DEBUG);

using MeasLite::S;
using MeasLite::D;

class System
{
public:
    virtual ~System () {}
    virtual void step (Float dt=1.0f) = 0;
};

class Life : public System
{
    Float eat, shit, fuck, die;
public:
    D pop, food, fed, fucked;
    Life ();
    virtual ~Life () {}
    virtual void step (Float dt=1.0f);
};

}

#endif
