
#include <cmath>
#include <cstdio>
#include <iostream>

inline float min (float x, float y) { return x<=y ? x : y; }
inline float log_add (float x, float y)
{
    //Version 1: BAD, causes overflow
    //return -log(exp(-x) + exp(-y));

    //Version 2: GOOD
    return x <= y ? x - logf(1.0f + expf(x - y))
                  : y - logf(1.0f + expf(y - x));

    //Version 3: SLOW
    //return min(x,y) - logf(1.0f + expf(fabs(x-y)));
}

class Cost
{
    float m;
public:
    Cost () : m(0) {}
    explicit Cost (float t) : m(t) {}

    double to_prob () const { return exp(-m); }
    void from_prob (float p) { m = -log(p); }

    //c+c' = Cost(prob(c) + prob(c')) = -log(exp(c) + exp(c'))
    Cost operator+ (Cost other) { return Cost(log_add(m, other.m)); }
    Cost operator+= (Cost other) { m = log_add(m, other.m); return *this; }

    Cost operator* (Cost other) { Cost(m + other.m); }
    Cost operator*= (Cost other) { m += other.m; return *this; }

    friend std::ostream& operator<< (std::ostream& o, const Cost& c)
    { return o << c.m; }
};

int main (int argc, char** argv)
{

    unsigned N = 1<<16;
    if (argc > 1) N = atoi(argv[1]);

    if (true) {
        Cost result(50);
        for (unsigned n=0; n<N; ++n) {
            result = result + Cost(1.0f);
        }
        std::cout << "result = " << result << std::endl;
    } else {
        std::cout << "using " << sizeof(long double) << " bytes" << std::endl;
        long double result(50);
        for (unsigned n=0; n<N; ++n) {
            result = result + 1;
        }
        std::cout << "result = " << result << std::endl;
    }

    return 0;
}

