
#include <fstream>
#include <iostream>

int main ()
{
    std::ofstream log("test.log");
    for (int i=0; i<10; ++i) {
        log << std::endl << "writing line " << i;
    }

    std::string line;
    while (std::cin >> line) { log << std::endl << line; }

    log.close();

    return 0;
}

