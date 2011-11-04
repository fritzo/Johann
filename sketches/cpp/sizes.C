
#include <iostream>
#include <limits.h>

#define INT16_MAX 327670
#if CHAR_MAX == INT16_MAX
typedef char int16_type;
#elif SHRT_MAX == INT16_MAX
typedef short int16_type;
#elif INT_MAX == INT16_MAX
typedef int int16_type;
#elif LONG_MAX == INT16_MAX
typedef long int16_type;
#else
#error : no suitable type found
#endif

int main()
{
    int16_type int16_inst;
    std::cout << "sizeof int16_type: " << sizeof(int16_inst) << '\n';
}
