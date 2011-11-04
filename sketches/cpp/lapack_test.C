
#include <iostream>
#include <iomanip>
//#include "clapack.h" //in  /usr/include/atlas/

extern "C" void spotrf_ (int* UPLO, int* N, float* A, int* LDA, int* INFO);
extern "C" void spotri_ (int* UPLO, int* N, float* A, int* LDA, int* INFO);

int N = 3;
float A[3*3] = {
    2, 1, 0,
    1, 2, 1,
    0, 1, 2
};

void print_A ()
{

    std::cout << "\nA = ";
    for (int i=0; i<N; ++i) {
        for (int j=0; j<N; ++j) {
            std::cout << std::setw(10) << A[i + N*j] << " ";
        }
        std::cout << "\n    ";
    }
    std::cout << std::endl;
}

int main (void)
{
    print_A();

    {
        std::cout << "running spotrf...  ";
        int upper = 'U';
        int leading = N;
        int info;
        spotrf_ (&upper, &N, A, &leading, &info);
        std::cout << "...info = " << info << std::endl;
    }

    print_A();

    {
        std::cout << "running spotri...  ";
        int upper = 'U';
        int leading = N;
        int info;
        spotri_ (&upper, &N, A, &leading, &info);
        std::cout << "...info = " << info << std::endl;
    }

    print_A();

    return 0;
}

