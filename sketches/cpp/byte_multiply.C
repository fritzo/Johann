
#include <iostream>
#include <cstdlib>

#define restrict __restrict__

typedef unsigned char  Int8;
typedef unsigned short Int16;
typedef unsigned int   Int32;

//results:
//
//
//

void mul1(const restrict Int8* a,
          const restrict Int8* b,
          restrict Int8* c,
          int N)
{
    for (int n=0; n<N; ++n) {
        const int A = a[n], B = b[n];
        c[n] = (A * B) >> 8;
    }
}

union Block
{
    Int32 int32;
    Int16 int16[2];
    Int8 int8[4];
};

void mul2(const restrict Int8* a,
          const restrict Int8* b,
          restrict Int8* c,
          int N)
{
    //XXX: this doesn't work.
    int N2 = N/2;
    for (int n2=0; n2<N2; ++n2) {
        int i = 2*n2, j = i+1;
        Block A;  A.int32 = a[i];  A.int8[2] = a[j];
        Block B;  B.int32 = b[i];  B.int8[2] = b[j];
        Block C;  C.int32 = A.int32 * B.int32;
        c[i] = C.int8[1];
        c[j] = C.int8[3];
    }
}

int main ()
{

    std::cout << "defining factor arrays" << std::endl;
    int N = 1<<16;
    Int8 *a = new Int8[N];
    Int8 *b = new Int8[N];
    for (int i=0; i<256; ++i) {
        for (int j=0; j<256; ++j) {
            int n = i + (j<<8);
            a[n] = i;
            b[n] = j;
        }
    }

    std::cout << "calculating product 1" << std::endl;
    Int8 *c1 = new Int8[N];
    mul1(a,b,c1,N);

    std::cout << "calculating product 2" << std::endl;
    Int8 *c2 = new Int8[N];
    mul2(a,b,c2,N);

    std::cout << "verifying equivalence" << std::endl;
    for (int n=0; n<N; ++n) {
        if (c1[n] != c2[n]) {
            int i = n % 256, j = n / 256;
            std::cout << "invalid product " << i << " x " << j
                      << ": " << c1[n] << " != " << c2[n] << std::endl;
            exit(0);
        }
    }

    return 0; 
}

