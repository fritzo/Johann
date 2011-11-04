
#include "definitions.h"
#include "meas_lite.h"
#include "animator.h"
#include "dynamics.h"

Logging::Logger logger("toy");

namespace M = MeasLite;

/** XXX This does not display anything, and I don't know why. XXX
 *
 * BUG: nothing is displayed
 * BUG: display and system update are not synchronized (eg oooo|oooooooo||oo|oo)
 */
int main (int argc, char **argv)
{
    Logging::switch_to_log("test.log");
    Logging::title("Johann Toy");

    M::load("../data/skj-big");

    Dynamics::Life sys;

    Animator::Window window(argc, argv);
    window.init(M::o_size,
                sys.pop.data(),
                sys.food.data(),
                M::prior().data());
    window.start();

    while (true) {

        window.lock();
        sys.step();
        window.unlock();

        window.update();

        //DEBUG
        cout << 'o' << std::flush;
    }

    return 0;
}

