
#include "dynamics.h"

namespace Dynamics
{

namespace M = MeasLite;
using namespace MeasLite;

Life::Life ()
    : eat   (0.3),
      shit  (0.2),
      fuck  (0.1),
      die   (0.2),

      pop   ("pop"),
      food  ("food"),
      fed   ("fed"),
      fucked("fucked")

{
    pop.clear();
    food = normal(dense(app(exp(M::random()),prior())));
    fed.clear();
    fucked.clear();
}
void Life::step (Float dt)
{
    dt = 1.0f - expf(-dt); //bound dt in [0,1]

    fed = app(pop,food);
    fucked = app(pop,pop);
    pop = normal( (dt * eat)  * fed
                + (dt * shit) * prior()
                + (dt * fuck) * fucked
                + (1.0f - dt * die) * pop );
}

}

