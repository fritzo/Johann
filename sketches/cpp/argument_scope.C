
int f (int) {}

int main ()
{
  // this works
  int y;
  f (y = 0);

  // but this doesn't
  // f (int y = 0);

  return f(y);
}

