
#include "reorder.h"
#include "nodes.h" //for explicit instantiation
#include <vector>
#include <algorithm> //for sort/nth_element

namespace Reorder
{

template <class Pos>
void Reordering<Pos>::_compact ()
{
    oid_t j = 0;
    for (typename Pos::sparse_iterator iter=Pos::sbegin(), end=Pos::send();
            iter!=end; ++iter) {
        oid_t i(*iter);
        ++j;

        m_old2new[i] = j;
        m_new2old[j] = i;
    }
}

template<class T>
struct SortNode
{
    oid_t pos;
    T index;

    SortNode (oid_t p, T i) : pos(p), index(i) {}

    bool operator <  (const SortNode<T>& other) const
    { return index >  other.index; }
    bool operator <= (const SortNode<T>& other) const
    { return index >= other.index; }
};

template <class Pos> template<class T>
void Reordering<Pos>::_sort (const typename Pos::template array<T>& rank)
{
    //build unsorted array
    std::vector<SortNode<T> > sorted;
    sorted.reserve(m_used);
    for (typename Pos::sparse_iterator iter=Pos::sbegin(), end=Pos::send();
            iter!=end; ++iter) {
        Pos pos = *iter;
        sorted.push_back(SortNode<T>(oid_t(pos), rank(pos)));
    }

    //sort array
    std::sort(sorted.begin(), sorted.end());

    //create lookup tables
    for (oid_t i = 0; i <= m_total; ++i) {
        m_old2new[i] = 0;
    }
    m_new2old[0] = 0;
    for (oid_t j = 1; j <= m_used; ++j) {
        oid_t i = sorted[j-1].pos;
        m_old2new[i] = j;
        m_new2old[j] = i;
    }
}

//explicit template instantiation
template class Reordering<Nodes::Ob>;
template class Reordering<Nodes::App>;
template class Reordering<Nodes::Comp>;
template class Reordering<Nodes::Join>;
template Reordering<Nodes::Ob  >::Reordering (const Ob  ::array<Float>&);
template Reordering<Nodes::App >::Reordering (const App ::array<size_t>&);
template Reordering<Nodes::Comp>::Reordering (const Comp::array<size_t>&);
template Reordering<Nodes::Join>::Reordering (const Join::array<size_t>&);

} //  namespace Reorder
