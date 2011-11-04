
#include <iostream>

const char * space = "                                " + 32;
#define LOG(mess) { std::cout << space << mess << std::endl; }
#define RUN(cmd) LOG(#cmd ";"); space-=2; cmd; space+=2;
#define RETURN(cmd) LOG("return " #cmd ";"); return cmd;

class Eg
{
public:
  Eg (const Eg & eg) { LOG("copy constructor"); }
  Eg () { LOG("default constructor"); }
  ~Eg () { LOG("destructor"); }
  const Eg & operator= (const Eg & eg) { LOG("copy"); }
};

Eg fun1 ()
{
   RETURN( Eg() )
}

Eg fun2 ()
{
  RUN( Eg x )
  RUN( Eg y = x )
  RETURN( x )
}

Eg fun3 ()
{
  RUN( Eg x )
  RUN( Eg y = x )
  RETURN( y )
}

int main ()
{
  LOG("main");
  {
    space-=2;

    RUN( Eg a )
    RUN( Eg b = a )
    RUN( Eg c )
    RUN( c = a )
    RUN( Eg d = fun1() )
    RUN( Eg e = fun2() )
    RUN( Eg f = fun3() )

    space+=2;
  }

  return 0;
}

