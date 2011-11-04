
#include <iostream>

#define LOG(mess) { std::cout << mess << std::endl; }

extern "C" int A_fun (void *);

class A
{
  int method () { LOG("A.method()"); }
public:
  friend int A_fun (void * a) { return ((A *)a) -> method(); }
};

int call (int (*fun) (void *), void * data) { return fun(data); }

int main ()
{

  A a;
  call(A_fun, &a);

  return 0;
}

