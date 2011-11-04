
#include "definitions.h"
#include "meas_lite.h"

Logging::Logger logger("test");

namespace M = MeasLite;

void test_evo (unsigned N=10)
{
    logger.info() << "Testing complexity calculation" |0;
    Logging::IndentBlock block;

    using namespace M;

    //variables
    D a("animal"), f("food"), p(prior());

    f = normal(app(exp(M::random()), p));
    float sleep = 0.6;
    float eat = 0.3;
    float shit = 0.2;
    float fuck = 0.1;
    for (unsigned n=0; n<N; ++n) {
        a = normal( sleep * a
                  + eat * app(a,f)
                  + shit * p
                  + fuck * app(a,a) );
        ::logger.info() << "entropy = " << a.entropy() |0;
    }
}

int main ()
{
    Logging::switch_to_log("test.log");
    Logging::title("Running Measures-Lite Test");

    M::load("../data/skj");

    test_evo(100);

    M::clear();

    return 0;
}

