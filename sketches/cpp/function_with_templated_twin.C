
int f (int x) { return x+1; }

template<class T>
int f (int x) {
    if (x < 0) {
        return x;
    } else {
        return f<T>(T::shift + x);
    }
}

class T1 { public: enum {shift = -1}; };
class T2 { public: enum {shift = -2}; };

int main ()
{
   int x=0;
   x = f(x);
   x = f<T1>(x);

   return  0;
}
