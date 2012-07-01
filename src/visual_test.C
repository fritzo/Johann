
#include "definitions.h"
#include "visual.h"

int main ()
{
    using namespace Visual;

    Logging::switch_to_log("test.log");
    Logging::title("Running Visual Test");

    _private::test_grey();
    _private::test_color();

    return 0;
}

