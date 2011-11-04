
#include "template_method_include_lib.h"

int main ()
{
    Shift<0> shift;
    int x = 7;
    int y = shift(x);

    return 0;
}
