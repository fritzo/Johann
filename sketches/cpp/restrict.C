
#ifdef __GNUG__
  #define restrict __restrict__
#else
  #warning keyword 'restrict' ignored
  #define restrict
#endif

void add (float * lhs,
          float * rhs,
          float * restrict result,
          const int n)
{
    for (int i=0; i<n; ++i)  result[i] = lhs[i] + rhs[i];
}

int main ()
{
    float x[4] = {0,1,2,3};
    float y[4] = {1,4,9,16};
    float z[4];
    add(x, y, z, 4);

    return 0;
}
