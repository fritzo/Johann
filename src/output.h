#ifndef NONSTD_OUTPUT_H
#define NONSTD_OUTPUT_H

#include <iostream>
#include <set>
#include <vector>

//output tools
template <class T> ostream& operator<< (ostream& os, const std::set<T>& s)
{
    os << '{';
    typename std::set<T>::iterator pos = s.begin();
    if (pos != s.end()) {
        os << *pos;
        for(++pos; pos != s.end(); ++pos) {
            os << ", " << *pos;
        }
    }
    return os << '}';
}
template <class T> ostream& operator<< (ostream& os, const std::vector<T>& v)
{
    os << '[';
    int N = v.size();
    if (N) {
        os << v[0];
        for(int i=1; i<N; ++i) {
            os << ", " << v[i];
        }
    }
    return os << ']';
}

#endif

