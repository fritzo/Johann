
typedef unsigned char Int8;
typedef unsigned short Int16;

class Fixed8;
class Fixed16;
class Fixed32;

class Fixed8
{//8-bit fixed-point number in [0,1)
    Int8 m_value;
public:
    Fixed8 () {}
    explicit Fixed8 (Int8 value) : m_value(value) {}
    Fixed8 (float value) : m_value(value * 256) {}
    Fixed8 (Fixed16 value) : m_value(value.value() / 256) {}
    Fixed8 operator + (Fixed8& other)
    { return Fixed8(m_value + other.m_value); }
    Fixed16 operator * (Fixed8&other)
    { return Fixed16(Int16(m_value) * Int16(other.m_value)); }
};

class PMF8
{
    int m_size;
    Fixed8* m_masses;
public:
    PMF8 (int size) : m_size(size), m_masses(new(nothrow) Fixed8[size])
    { if (m_masses == NULL) exit(0); } //XXX: this should be an assertion
    int size () const { return m_size; }
    Fixed8& operator [] (int n)       { return m_masses[n]; }
    Fixed8  operator [] (int n) const { return m_masses[n]; }
};

void calc_conjunction (const PMF8& lhs, const PMF8& rhs, PMF8& result)
{
    Fixed16 total = 0.0;
    int N = lhs.size();
    for (int n=0; n<N; ++n) {
        total += lhs[n] * rhs[n];
    }
    for (int n=0; n<N; ++n) {
        result = (lhs[n] * rhs[n]) / total;
    }
}

