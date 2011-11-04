
template<class T>
struct A
{
  template<int x>
  void f () {}
};

template<class T>
struct B
{
  T t;
  template<int x>
  void f () { t.f<x>(); }
};

int main ()
{
  B<A<int> > b;
  b.f<0>();

  return 0;
}

