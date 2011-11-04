
#include <iostream>
#include "template_lib.h"

using namespace std;

int main ()
{
    int x = 1;
    cout << "x = " << x << "\n";

    cout << "...move<t1>...\n";
    x = move<t1>(x);
    cout << "x = " << x << "\n";

    cout << "...move<t2>...\n";
    x = move<t2>(x);
    cout << "x = " << x << "\n";
}


