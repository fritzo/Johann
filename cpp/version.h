#ifndef JOHANN_VERSION_H
#define JOHANN_VERSION_H

struct Version
{
    unsigned char a,b,c,d;
    Version (unsigned char _a=0,
             unsigned char _b=0,
             unsigned char _c=0,
             unsigned char _d=0)
        : a(_a), b(_b), c(_c), d(_d) {}

    //comparison
    uint32_t num () const { return *reinterpret_cast<const uint32_t*>(this); }
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
const Version VERSION(0,9,1,13);

#endif
