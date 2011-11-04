
#include <iostream>

using namespace std;


class LRA {
public: 
    enum FieldNames {
        root  = 0,
        key   = 1,
        value = 2,
        left  = 3,
        right = 4
    };
};

class RLA {
public: 
    enum FieldNemes {
        root  = 4,
        key   = 3,
        value = 2,
        left  = 1,
        right = 0
    };
};

enum IntIndex { A,B,C,D,E,F };

class Fields
{
public:
    int m_fields[5];
    Fields () { for (int i=0; i<5; ++i) m_fields[i] = i; }
    inline int  operator [] (IntIndex index) {
        cout << "...calling operator [] (IntIndex)...";
        return m_fields[index];
    }
    inline bool operator [] (int index) {
        cout << "...calling operator [] (int)...";
        return !m_fields[index];
    }
};

template<class xxx>
int right (Fields& fields)
{
    return fields[xxx::right];
}

int main (void)
{
    Fields fields;
    cout << "right<LRA>(fields) = " << right<LRA>(fields) << "\n";
    cout << "right<RLA>(fields) = " << right<RLA>(fields) << "\n";
}
