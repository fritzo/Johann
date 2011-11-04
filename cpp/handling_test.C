
#include "handling.h"
#include <iostream>

using namespace Handling;

//basic test
class TestObject : public HandledObject
{
public:
    int m_data;
    TestObject (int data = 0) : m_data(0) {}
    virtual ~TestObject () {}
};
inline ostream& operator << (ostream& os, const TestObject& to)
{ return os << "<test object>"; }
typedef Handle<TestObject> HTO;

long initial_numHandledObjects(0);
long initial_numHandles(0);

//basic test
void copy_test ()
{
    HTO a(new TestObject(1)), b = a, c = b;
    a->m_data = 2;
    Assert (a->m_data == c->m_data,
            "copied handles do not refer to the same object");
}
void basic_test ()
{
    int initial_numHandles = Handling::numHandles();
    copy_test();
    Assert (Handling::numHandles() == initial_numHandles,
            "basic_test: final Handle count does not match initial count");
}

//number of handledObjects
void initial_handledObjects_test ()
{
    initial_numHandledObjects = Handling::numHandledObjects();
    logger.debug() << "initial number of handled objects = "
                   << initial_numHandledObjects |0;
}
void final_handledObjects_test ()
{
    //assert no HandledObjects remaining
    long final_numHandledObjects = Handling::numHandledObjects();
    logger.debug() << "final number of handled objects = "
                   << final_numHandledObjects |0;
    Assert (final_numHandledObjects == initial_numHandledObjects,
            "final_handledObjects_test: final HandledObject count does not match initial count");
}

int main ()
{
    Logging::switch_to_log("test.log");
    Logging::title("Running Handling Test");

    //initialize
    initial_handledObjects_test();

    basic_test();
    copy_test();

    //do handledObject test
    final_handledObjects_test();

    return 0;
}
