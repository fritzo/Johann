
#include <iostream> //for cout
#include <fstream>  //for fstream
#include <cstdio>   //for remove

using namespace std;

template<class T, int blockSize = 64>
class FileCache
{
private:
    int fileSize; //in blocks
    int blockNum; //current block
    fstream file;
    T *buffer;
public:
    FileCache (int _fileSize, T initValue)
        : fileSize(_fileSize),
          blockNum(0)
    {
        file.open("FileCache.temp",ios::out|ios::in|ios::binary|ios::trunc);
        if (!file) {
            cout << "error opening iofile\n";
            exit(0);
        }
        buffer = new T[blockSize];
        for (int i=0; i<blockSize; ++i) {
            buffer[i] = initValue;
        }
        for (blockNum=0; blockNum<fileSize/blockSize; ++blockNum) {
            //WARNING: only works if fileSize % blockSize == 0
            writeBuffer();
        }
        blockNum = 0;
        readBuffer(); //not really necessary
    }
    ~FileCache ()
    {
        file.close();
        delete[] buffer;
        remove("FileCache.temp");
    }
    inline int size () { return fileSize; }

    T get (int index) {
        //if ((index < 0) || (fileSize <= index)) {
        //    throw RangeError;
        //}
        if (index/blockSize != blockNum) {
            writeBuffer();
            blockNum = index/blockSize;
            readBuffer();
        }
        return buffer[index % blockSize];
    }
    void set (int index, T value) {
        //if ((index < 0) || (fileSize <= index)) {
        //    throw RangeError;
        //}
        if (index/blockSize != blockNum) {
            writeBuffer();
            blockNum = index/blockSize;
            readBuffer();
        }
        buffer[index % blockSize] = value;
    }
    
private:
    void writeBuffer () {
        cout << "FileCache: writing block " << blockNum << "\n";
        file.seekp(blockNum * blockSize * sizeof(T));
        file.write((char*)(buffer),blockSize * sizeof(T));
    }
    void readBuffer () {
        cout << "FileCache: reading block " << blockNum << "\n";
        file.seekg(blockNum * blockSize * sizeof(T));
        file.read((char*)(buffer),blockSize * sizeof(T));
    }
};

int main ()
{
    FileCache<int> x(256,0);
    for (int i=0; i<x.size(); ++i) {
        x.set(i,i);
    }
    for (int i=x.size()-1; i>=0; --i) {
        cout << i << "->" << x.get(i) << " ";
    }
    cout << "\n";
}
    

