
#include <vector>
#include <list>
#include <set>
#include <map>
#include <utility>
#include <iostream>
#include <cmath>

//standard output
//pairs
template <class F, class S>
inline std::ostream& operator << (std::ostream& os, std::pair<F,S> pair)
{ return os << "(" << pair.first << ", " << pair.second << ")"; }
//vectors
template <class T>
inline std::ostream& operator << (std::ostream& os, std::vector<T> vector)
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
inline std::ostream& operator << (std::ostream& os, std::list<T> list)
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
inline std::ostream& operator << (std::ostream& os, std::set<T> set)
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
inline std::ostream& operator << (std::ostream& os, std::map<K,V> map)
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
inline std::ostream& operator << (std::ostream& os, std::multimap<K,V> multimap)
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

//strict linear-order pair iterator
template <class T>
class SLOP_iterator
{
    typedef SLOP_iterator& Ref;
    typedef std::set<T> Data;
    Data m_data;
    typename Data::iterator m_first, m_second;
    void increment ();
public:
    template<class iterator> SLOP_iterator (iterator begin, iterator end)
        : m_data(begin, end),
          m_first(m_data.begin()), m_second(m_first)
    {
        std::cout << "created SLOP_iterator over " << m_data << std::endl;
    }
    void reset () { m_first = m_data.begin(); m_second = m_first; }
    void insert (T item) { m_data.insert(item); }
    void remove (T item);
    inline std::pair<T,T> pop ();
};
template<class T>
void SLOP_iterator<T>::remove(T item)
{
    typename Data::iterator iter = m_data.find(item);
    //Assert (iter != m_data.end(), "tried to remove element not in range");
    while ((m_first==iter) || (m_second==iter)) increment();
    m_data.erase(iter);
}
template <class T>
inline std::pair<T,T> SLOP_iterator<T>::pop ()
{
    std::pair<T, T> result(*m_first, *m_second);
    increment();
    return result;
}
template <class T>
void SLOP_iterator<T>::increment ()
{
    ++m_first;
    //Assert (m_first != m_data.end(), "pair iterator reached end of data");
    if (m_second != m_data.begin()) {
        --m_second;
    } else {
        m_second = m_first;
        m_first = m_data.begin();
    }
}

//weak linear-order pair iterator
template <class K, class V>
class WLOP_iterator
{
    typedef WLOP_iterator& Ref;
    typedef std::set<V> Block;
    typedef std::map<K, Block> Data;
    Data m_data;
    typename Data::iterator  m_first, m_second;
    typename Block::iterator m_first_val, m_second_val;
    typename Block::iterator m_first_end, m_second_end;
    void increment ();
public:
    template<class iterator, class IndexFun>
    WLOP_iterator (iterator begin, iterator end, IndexFun indexFun)
    {
        for (iterator iter = begin; iter != end; ++iter) {
            V value = *iter;
            K key = indexFun(value);
            insert(key, value);
        }
        reset();
        std::cout << "created WLOP_iterator over " << m_data << std::endl;
    }
    inline void reset ();
    inline void insert (K key, V value);
           void remove (K key, V value);
    inline std::pair<V,V> pop ();
};
template <class K, class V>
inline void WLOP_iterator<K,V>::reset ()
{
    m_first = m_data.begin();
    m_second = m_first;
    m_first_val = m_first->second.begin();
    m_second_val = m_first_val;

    m_first_end = m_first->second.end();
    m_second_end = m_first_end;
}
template <class K, class V>
void WLOP_iterator<K,V>::insert (K key, V value)
{
    //m_data[key].push_back(value); //for vectors or lists
    m_data[key].insert(value); //for sets
}
template <class K, class V>
void WLOP_iterator<K,V>::remove(K key, V value)
{
    Block& block = m_data[key];
    typename Block::iterator iter = block.find(value);
    //Assert (iter != block.end(), "tried to remove element not in range");
    //if ((m_first->first==key)||(m_second->first==key)) { //necessary?
    while ((m_first_val==iter) || (m_second_val==iter)) increment();
    //}
    block.erase(iter);
    if (block.empty) m_data.erase(key);
}
template <class K, class V>
inline std::pair<V,V> WLOP_iterator<K,V>::pop ()
{
    std::pair<V, V> result(*m_first_val, *m_second_val);
    increment();
    return result;
}
template <class K, class V>
void WLOP_iterator<K,V>::increment ()
{
    //try to increment m_second_val
    ++m_second_val;
    if (m_second_val != m_second_end) return;

    //try to increment m_first_val and reset m_second_val
    ++m_first_val;
    if (m_first_val != m_first_end) {
        m_second_val = m_second->second.begin();
        return;
    }

    //move to next (key, key) block
    ++m_first;
    //Assert (m_first != m_data.end(), "pair iterator reached end of data");
    if (m_second != m_data.begin()) {
        --m_second;
    } else {
        m_second = m_first;
        m_first = m_data.begin();
    }
    m_first_val = m_first->second.begin();
    m_first_end = m_first->second.end();
    m_second_val = m_second->second.begin();
    m_second_end = m_second->second.end();
}

//testing
template<class T>
inline T sum(std::pair<T,T> pair) { return pair.first + pair.second; }
void SLOP_test ()
{
    std::cout << "Testing SLOP_iterator" << std::endl;

    std::vector<int> data(7);
    data[0] = 0;
    data[1] = 1;
    data[2] = 2;
    data[3] = 3;
    data[4] = 4;
    data[5] = 5;
    data[6] = 6;

    std::cout<< "defining SLOP_iterator" << std::endl;
    SLOP_iterator<int> iter(data.begin(), data.end());

    std::cout<< "Iterating through pairs:" << std::endl;
    std::pair<int, int> prev = iter.pop();
    std::cout << "  " << prev << std::endl;
    for (int i=0; i<20; ++i) {
        std::pair<int, int> curr = iter.pop();
        if ((sum(prev)>sum(curr))) std::cout << "-- out of order --" << std::endl;
        std::cout << "  " << curr << std::endl;
        prev = curr;
    }
}
struct Index { inline int operator () (int x) { return x/2; } };
void WLOP_test ()
{
    std::cout << "Testing WLOP_iterator" << std::endl;

    std::vector<int> data(7);
    data[0] = 0;
    data[1] = 1;
    data[2] = 2;
    data[3] = 3;
    data[4] = 4;
    data[5] = 5;
    data[6] = 6;

    std::cout<< "defining WLOP_iterator" << std::endl;
    WLOP_iterator<int,int> iter(data.begin(), data.end(), Index());

    std::cout<< "Iterating through pairs:" << std::endl;
    std::pair<int, int> prev = iter.pop();
    std::cout << "  " << prev << std::endl;
    for (int i=0; i<30; ++i) {
        std::pair<int, int> curr = iter.pop();
        std::cout << "  " << curr << " --> ("
                  << curr.first/2 << ", " << curr.second/2 << ")" << std::endl;
        prev = curr;
    }
}

int main ()
{
    SLOP_test();
    WLOP_test();

    return 0;
}

