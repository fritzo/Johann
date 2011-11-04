
#include <iostream>

using namespace std;

template<unsigned s>
class K
{
public:
    static const unsigned t;
};
template<unsigned s>
const unsigned K<s>::t(s+1);

int main ()
{
    K<1> k1;
    K<2> k2;

    cout << "k1.t = " << k1.t << "\n";
    cout << "k2.t = " << k2.t << "\n";
}
