
#include <iostream>

int main ()
{
    //note: '\033' is ESC character
    //char* to_R = "\033[\031m";
    char* to_R = "\033[31m";
    char* to_G = "\033[32m";
    char* to_B = "\033[34m";
    char* to_C = "\033[36m";
    char* to_M = "\033[35m";
    char* to_Y = "\033[33m";
    char* to_K = "\033[37m";

    std::cout << to_R << "text" << std::endl;
    std::cout << to_G << "text" << std::endl;
    std::cout << to_B << "text" << std::endl;
    std::cout << to_C << "text" << std::endl;
    std::cout << to_M << "text" << std::endl;
    std::cout << to_Y << "text" << std::endl;
    std::cout << to_K << "text" << std::endl;

    return 0;
}
