
#include <cstdlib>

struct A
{
  float act (float x) { return 1 / (x * x); }
};

struct B
{
  virtual float act (float x) = 0;
};

struct C : public B
{
  virtual float act (float x) { return 1 / (x * x); }
};

int main (int argc, char ** argv)
{
  if (argc < 2) return 1;

  int N = atoi(argv[1]);

  if (N > 0) {
    A * f = new A();
    float x = 0;
    for (unsigned i = 0; i < N; ++i) x += f->act(i);
    return 0;
  }

  if (N < 0) {
    N = -N;
    B * f = new C();
    float x = 0;
    for (unsigned i = 0; i < N; ++i) x += f->act(i);
    return 0;
  }

  return 1;
}

