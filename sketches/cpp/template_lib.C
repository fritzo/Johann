
#include "template_lib.h"

template<class T>
int move(int x) { return T::shift + x * T::scale; }

int (*move1)(int) = move<t1>;
int (*move2)(int) = move<t2>;

