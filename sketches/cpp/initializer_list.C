
struct A { int x,y; };
struct B { int data[2] __attribute__ ((aligned (16))); };

int main ()
{
  A a{1,2};
  B b{1,2};

  return 0;
}

