
#include <iostream>
#include <cmath>

using namespace std;

float get_infinity ()
{
    float x = 2.0f;
    while (x != x*x) { x = x*x; }
    return x;
}

float infinity = get_infinity();

int main ()
{
    float inf = infinity;
    
    cout << "inf = " << inf << endl;
    
    cout << "inf + 1 = " << inf + 1 << endl;
    cout << "1 + inf = " << 1 + inf << endl;
    cout << "inf - 1 = " << inf - 1 << endl;
    cout << "1 - inf = " << 1 - inf << endl;
    cout << "inf * 1 = " << inf * 1 << endl;
    cout << "1 * inf = " << 1 * inf << endl;
    cout << "inf / 1 = " << inf / 1 << endl;
    cout << "1 / inf = " << 1 / inf << endl;

    cout << "log(inf) = "    << log(inf) << endl;
    cout << "log(0) = "      << log(0.0f) << endl;
    cout << "log(-0) = "      << log(-0.0f) << endl;
    cout << "exp(inf) = "  << exp(inf) << endl;
    cout << "exp(-inf) = " << exp(-inf) << endl;

    return 0;
}

