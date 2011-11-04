
#include <cstdlib>
#include <iostream>

int main (int argc, char** argv)
{
    int size = 1<<20;

    std::cout << "allocating array of " << size << "chars" << std::endl;
    char* data = new char[size];
    std::cout << "initializing data" << std::endl;
    for (int i=0; i<size; ++i) {
        data[i] = i%256;
    }
    std::cout << "done" << std::endl;
}

