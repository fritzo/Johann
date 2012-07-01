#ifndef JOHANN_VERSION_H
#define JOHANN_VERSION_H

#include <stdint.h>

struct Version
{
    uint8_t a,b,c,d;
    Version (uint8_t _a = 0, uint8_t _b = 0, uint8_t _c = 0, uint8_t _d = 0)
        : a(_a), b(_b), c(_c), d(_d)
    {}

    //comparison
    uint32_t num () const { return a << 24 | b << 16 | c << 8 | d; }
    bool operator == (const Version& v) const { return num() == v.num(); }
    bool operator != (const Version& v) const { return num() != v.num(); }
    bool operator <= (const Version& v) const { return num() <= v.num(); }
    bool operator >= (const Version& v) const { return num() >= v.num(); }

    //output
    friend inline ostream& operator<< (ostream& os, const Version& v)
    { return os << Int(v.a) << '.'
                << Int(v.b) << '.'
                << Int(v.c) << '.'
                << Int(v.d); }
};
const Version VERSION(0,9,1,16);

#endif
