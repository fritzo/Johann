#define alignof __alignof__

//test_compile.h
#include <vector>
#include <string>

namespace Compile
{
typedef std::vector<std::string> Stack;
extern Stack g_errors;
void process_errors();

class Assert
{
public:
    Assert (bool condition, const std::string& message)
    { if (!condition) g_errors.push_back(message); }
};

}

//test_compile.C
#include <iostream>
namespace Compile
{

Stack g_errors;

void process_errors ()
{
    if (g_errors.empty()) return;
    for (Stack::iterator message = g_errors.begin(); message != g_errors.end(); ++message) {
        std::cout << std::endl << "Compile Error: " << *message;
    }
    exit(0);
}

}

//some_file.C
//#include test_compile.h
Compile::Assert _a1(alignof(double) >= 4, "doubles cross partitions");

struct MyStruct { double m_double[3]; };
Compile::Assert _a2(sizeof(MyStruct) == 4, "MyStruct incorrect size");
Compile::Assert _a3(alignof(MyStruct) == 16, "MyStruct incorrect alignent");

//in main_file.C
//#include test_compile.h
int main ()
{
    Compile::process_errors();

    return 0;
}

