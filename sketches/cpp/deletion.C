
#include <iostream>
#include <string>

using namespace std;

//indenting machinery
const int length = 24;
const int stride = 2;
const char* spaces = "                        " + length;
int level(0);
inline const char* indentation ()
{
    return spaces - level * stride;
}
inline void indent ()
{
    ++level;
    //Assert(level*stride < length, "indent level overflow");
}
inline void outdent ()
{
    --level;
    //Assert(level >= 0, "indent level underflow");
}

//section logging
class SectionLogger
{
public:
    SectionLogger (const char* name)
    {
        cout << '\n' << indentation() << "> " << name;
        indent();
    }
    ~SectionLogger () { outdent(); }
    inline ostream& operator () () { return cout << '\n' << indentation(); }
};

SectionLogger logger("top-level");

void fun1 ()
{
    SectionLogger logger("fun1");
    logger() << "this is in fun1";
}

void fun2 ()
{
    SectionLogger logger("fun2");
    logger() << "this is in fun2";
    fun1();
    logger() << "this is also in fun2";
}

int main ()
{
    SectionLogger logger("main");
    logger() << "this is in main";
    fun1();
    logger() << "this is also in main";
    fun2();
    logger() << "this is also in main";
}

