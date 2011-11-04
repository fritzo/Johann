
#include <iostream>

typedef uint16_t Short;
typedef uint32_t Int;

int main ()
{
    Int x = 1;
    Short *p = reinterpret_cast<Short*>(&x);
    std::cout << "test for endian-ness: \n"
              << "  1 = 0,1 is natural (big-endian)\n"
              << "  1 = 1,0 is backwards (little-endian)\n"
              << x << " = " << p[0] << "," << p[1] << std::endl;

    return 0;
}

