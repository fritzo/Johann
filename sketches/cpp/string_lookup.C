
#include<iostream>
#include<string>
#include<map>
#include <utility>

using namespace std;

class LookupError {};

class NameTable
{
private:
    map<string,double> table;
    typedef map<string,double>::iterator Iter;
public:
    NameTable ()
    {
        insert("zero", 0.0);
        insert("one", 1.0);
    }
    void insert(string key, double value)
    {
        Iter pos = table.find(key);
        if (pos == table.end()) {
            cout << "  inserting new element\n";
            table.insert(make_pair(key, value));
        } else {
            cout << "  replacing old element\n";
            pos->second = value;
        }
    }
    void remove(string key)
    {
        cout << "  removing old element\n";
        Iter pos = table.find(key);
        if (pos == table.end()) {
            cout << "  key not found\n";
            throw LookupError();
        }
        table.erase(pos);
    }
    double operator [] (string key)
    {
        cout << "  finding old element\n";
        Iter pos = table.find(key);
        if (pos == table.end()) {
            cout << "  NameTable::operator[] error: key not found\n";
            throw LookupError();
        }
        return pos->second;
    }
};

int main ()
{
    NameTable table;
    cout << "finding element 1:\n";
    double one = table["one"];
    cout<< "one = " << one << "\n";
    
    cout << "adding element 2:\n";
    table.insert("two",2.0);

    cout << "removing element 2:\n";
    table.remove("two");

    cout << "finding element 2 (should be gone):\n";
    double two = table["two"];
    cout<< "two = " << two << "\n";
}
