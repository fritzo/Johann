#include <stdlib.h>
#include <iostream>

using namespace std;

class Int
{
private:
    int data;
public:
    Int (int _data) : data(_data) {}
    inline operator int () { return data; }
    void naechste ()
    {
        data = 1 + 3*data;
        while (!(data%2)) {
            data /= 2;
        }
    }
};

class Ints
{
private:
    int * const data;
public:
    Ints () : data (new int[4]) {}
    Int& operator [] (int i) { return reinterpret_cast<Int&>(data[i]); }
    Int operator [] (int i) const { return data[i]; }
};

union leader { float float_val; int int_val; };
enum IntIndex {I1, I2};
enum intIndex {i1, i2};
enum floatIndex {f1,f2};
class coalition
{
    leader data[2];
public:
    Int&   operator [] (IntIndex   i) { return reinterpret_cast<Int&>(data[i].int_val); }
    int&   operator [] (intIndex   i) { return data[i].int_val; }
    float& operator [] (floatIndex i) { return data[i].float_val; }
};

int main (int argc, char**argv)
{
    /*
    Ints marcel;
    if (argc < 2) return 0;
    marcel[3] = atoi(argv[1]);
    cout << marcel[3] << endl;
    while (marcel[3] > 1) {
        marcel[3].naechste();
        cout << marcel[3] << endl;
    }
    */

    coalition beads;
    if (argc < 2) return 0;
    beads[I2] = atoi(argv[1]);
    cout << beads[I2] << endl;
    while (beads[I2] > 1) {
        beads[I2].naechste();
        cout << beads[I2] << endl;
    }
}
