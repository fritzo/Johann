
#include <cstdio>
#include <cstdlib>
#include <iostream>

#define FILENAME "seq_read.temp"

void normalize (float* a, unsigned Ns)
{
    double total = 0.0;
    for (int s=0; s < Ns; ++s) {
        total += a[s];
    }
    for (int s=0; s < Ns; ++s) {
        a[s] /= total;
    }
}

class FileTable
{
    unsigned m_Ns, m_Nt;
    FILE* m_file;
public:
    FileTable (unsigned Ns, unsigned Nt) : m_Ns(Ns), m_Nt(Nt)
    {
        std::cout << "initializing file table of size " << Nt << std::endl;
        //open write-only for initialization
        m_file = fopen(FILENAME, "wb");
        flockfile(m_file);
        for (unsigned t=1; t < m_Nt; ++t) {
            unsigned x[3] = { (2*t) % m_Ns ,
                              (3*t) % m_Ns ,
                              (5*t) % m_Ns };
            fwrite_unlocked(x, sizeof(unsigned), 3, m_file);
        }
        funlockfile(m_file);
        fclose(m_file);

        //reopen as read-only
        m_file = fopen(FILENAME, "rb");
        //setvbuf(m_file, NULL, _IOFBF, 4096); //neg'ble performance gain
    }
    ~FileTable ()
    {
        fclose(m_file);
        remove(FILENAME);
    }
    void actOn (float* a, float* b, float* c)
    {
        std::cout << "  file table acting on vectors" << std::endl;
        for (int s=0; s < m_Ns; ++s) {
            c[s] = 0.0f;
        }
        flockfile(m_file);
        rewind(m_file);
        for (int t=1; t < m_Nt; ++t) {
            unsigned x[3];
            fread_unlocked(x, sizeof(unsigned), 3, m_file);
            c[x[2]] += a[x[0]] + b[x[1]];
        }
        funlockfile(m_file);
        normalize(c,m_Ns);
    }
};

int main (int argc, char** argv)
{
    std::ios::sync_with_stdio(false);

    unsigned Ns = 1<<16, Nt = 1<<24;
    float* a = new float[Ns];
    float* b = new float[Ns];
    float* c = new float[Ns];

    //initialize
    std::cout << "initializing data of size " << Ns << std::endl;
    for (int s=0; s < Ns; ++s) {
        a[s] = 1.0f / (1+s);
        b[s] = 1.0f / (Ns-s);
    }
    normalize(a,Ns);
    normalize(b,Ns);
    FileTable f(Ns, Nt);

    //iterate
    double diff, tol = 1e-6;
    do {
        std::cout << "mapping data" << std::endl;
        
        f.actOn(a, b, c);

        diff = 0.0f;
        for (int s=0; s < Ns; ++s) {
            float d = b[s] - c[s];
            diff += d*d;
            b[s] = c[s];
        }
        std::cout << "  diff = " << diff << std::endl;
    } while (diff > tol);

    return 0;
}
