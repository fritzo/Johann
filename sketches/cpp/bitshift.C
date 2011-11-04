
#include <stdint.h>
#include <iostream>
#include <iomanip>

int main ()
{
    std::cout << std::hex;
    std::cout.setf(std::ios::showbase);
    //std::cout.setf(std::ios::uppercase);
    
    uint32_t i = 0xffffffff;
    std::cout << "i = " << i << std::endl;
    std::cout << "i >> 31 = " << (i>>31) << std::endl;
    std::cout << "i << 31 = " << (i<<31) << std::endl;

    return 0;
}

/* produces the following output
i = 0xffffffff
i >> 31 = 0x1
i << 31 = 0x80000000
*/

