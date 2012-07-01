
#include "small_set.h"

namespace nonstd
{

Int SmallSet::size () const
{
    Int result = 0;
    for (Int i=0; i<NUM_ELTS; ++i) {
        if (contains(i)) ++result;
    }
    return result;
}
std::vector<Int> SmallSet::as_vector () const
{
    std::vector<Int> result;
    for (Int i=0; i<NUM_ELTS; ++i) {
        if (contains(i)) result.push_back(i);
    }
    return result;
}


Int SmallSet::min () const
{
    for (Int i=0; i<NUM_ELTS; ++i) {
        if (contains(i)) return i;
    }
    return NUM_ELTS; //set was empty
}
Int SmallSet::max () const
{
    for (Int i=1; i<=NUM_ELTS; ++i) {
        if (contains(NUM_ELTS - i)) return NUM_ELTS - i;
    }
    return NUM_ELTS; //set was empty
}

}

ostream& operator<< (ostream& os, nonstd::SmallSet elts)
{
    const string alphabet = "abcdefghijklmnopqrstuvwxyz??????";
    os << "{";
    for (Int i=0; i<nonstd::SmallSet::NUM_ELTS; ++i) {
        if (elts.contains(i)) {
            os << alphabet[i] << ',';
        }
    }
    return os << "}";
}

