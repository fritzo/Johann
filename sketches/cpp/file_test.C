
#include <iostream> //for cout
#include <fstream>  //for fstream
#include <cstdlib>  //for exit
#include <cstdio>   //for remove

using namespace std;

int main ()
{
    fstream file;
    //file.open("test.file",ios::out|ios::binary);
    //if (!file) {
    //    cout << "error opening the ofile\n";
    //    exit(1);
    //}
    //file.close();
    file.open("test.file",ios::out|ios::in|ios::binary|ios::trunc);
    if (!file) {
        cout << "error opening the iofile\n";
        exit(1);
    }
    int size = 10;
    int array[size];
    for (int i=1; i<size; ++i) {
        array[i] = i;
    }
    file.seekp(0);
    file.write((char*)(array),sizeof(int)*size);
    file.close();
    remove("test.file");
}

