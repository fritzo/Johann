
#include "definitions.h"
#include "expressions.h"

using namespace Expressions;

void test_handles ()
{

    ExprHdl a1 = build_atom("a");

    //test atom lookup
    ExprHdl a2 = build_atom("a");
    Assert (&(*a1) == &(*a2), "two equivalent atoms created");

    //test copying
    ExprHdl a3 = a1;
    Assert (&(*a1) == &(*a3), "expressions copied incorrectly");

    //test app lokup
    ExprHdl b = build_atom("b");
    ExprHdl ab1 = a1 * b;
    ExprHdl ab2 = a2 * b;
    Assert (&(*ab1) == &(*ab2), "two equivalent apps created incorrectly");

    validate();
}

int main ()
{
    Logging::switch_to_log("test.log");
    Logging::title("Running Expressions Test");

    initialize();
    test_handles();
    validate();
    clear();

    return 0;
}


