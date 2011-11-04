
#include <string>
#include <iostream>

int main (int)
{
    while (true) {
        std::string line;
        std::cout << "\n> " << std::flush;
        std::cin >> line;
        std::cout << line << std::endl;

        for (unsigned i=0; i<line.size(); ++i) {
            std::cout << int(line[i]) << ", ";
        }
        std::cout << std::endl;
    }

    return 0;
}
