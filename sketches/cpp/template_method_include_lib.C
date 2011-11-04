
#include "template_method_include_lib.h"

template<int SHIFT> int Shift<SHIFT>::operator () (int x)
{ return x + SHIFT; }

//int Shift<0>::operator () (int x);
Shift<0> instance_0;
