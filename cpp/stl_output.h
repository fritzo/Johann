
//LATER: add a header-wise include guard
#ifndef JOHANN_STL_OUTPUT_H
#define JOHANN_STL_OUTPUT_H

#include <vector>
#include <list>
#include <set>
#include <map>
#include <utility>
#include <iostream>

//pairs
template <class F, class S>
inline ostream& operator << (ostream& os, std::pair<F,S> pair)
{ return os << "(" << pair.first << ", " << pair.second << ")"; }
//vectors
template <class T>
inline ostream& operator << (ostream& os, std::vector<T> vector)
{
    if (vector.empty()) return os << "<>";
    typename std::vector<T>::iterator iter = vector.begin();
    os << "<" << *iter;
    for (++iter; iter != vector.end(); ++iter) {
        os << ", " << *iter;
    }
    return os << ">";
}
//lists
template <class T>
inline ostream& operator << (ostream& os, std::list<T> list)
{
    if (list.empty()) return os << "[]";
    typename std::list<T>::iterator iter = list.begin();
    os << "[" << *iter;
    for (++iter; iter != list.end(); ++iter) {
        os << ", " << *iter;
    }
    return os << "]";
}
//sets
template <class T>
inline ostream& operator << (ostream& os, std::set<T> set)
{
    if (set.empty()) return os << "{}";
    typename std::set<T>::iterator iter = set.begin();
    os << "{" << *iter;
    for (++iter; iter != set.end(); ++iter) {
        os << ", " << *iter;
    }
    return os << "}";
}
//maps
template <class K, class V>
inline ostream& operator << (ostream& os, std::map<K,V> map)
{
    if (map.empty()) return os << "{(map)}";
    typename std::map<K,V>::iterator iter = map.begin();
    os << "{(map)\n\t" << iter->first << ":\t" << iter->second;
    for (++iter; iter != map.end(); ++iter) {
        os << ",\n\t" << iter->first << ":\t" << iter->second;
    }
    return os << "\n}";
}
//multimaps
template <class K, class V>
inline ostream& operator << (ostream& os, std::multimap<K,V> multimap)
{
    if (multimap.empty()) return os << "{(multimap)}";
    typename std::multimap<K,V>::iterator iter = multimap.begin();
    os << "{(multimap)\n\t" << iter->first << ":\t" << iter->second;
    K prev_key = iter->first;
    for (++iter; iter != multimap.end(); ++iter) {
        if (iter->first != prev_key) {
            prev_key = iter->first;
            os << ",\n\t" << iter->first << ":\t" << iter->second;
        } else {
            os << ", " << iter->second;
        }
    }
    return os << "\n}";
}

#endif

