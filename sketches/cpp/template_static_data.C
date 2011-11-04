
#include "template_static_data.h"
#include <string> //needed for NULL

//template<> int* Foo_int::s_data; //this fails
template<> int* Foo_int::s_data = NULL; //this works

