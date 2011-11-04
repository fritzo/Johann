
template<int SHIFT, int SCALE>
struct AffineTransform {
    static const int shift = SHIFT;
    static const int scale = SCALE;
};

typedef AffineTransform<1,2> t1;
typedef AffineTransform<2,3> t2;

template<class T>
int move(int x);

