
#include <iostream>

#define LOG(mess) { std::cout << mess << std::endl; }

struct Base
{
  Base () { LOG("constructing a Base"); }
  virtual ~Base () { LOG("destructing a Base"); }
};

struct Derived : public Base
{
  Derived () { LOG("constructing a Derived"); }
  virtual ~Derived () { LOG("destructing a Derived"); }
};

int main ()
{
  Derived();

  return 0;
}

