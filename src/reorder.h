#ifndef JOHANN_REORDER_H
#define JOHANN_REORDER_H

#include "definitions.h"
#include "node_heap.h" //for TypedIndex

namespace Reorder
{

const Logging::Logger logger("reord", Logging::DEBUG);

using namespace Heap;

//note: all arrays use 1-based indexing
template<class Pos>
class Reordering
{
    size_t m_total;
    size_t m_used;
    oid_t * const m_old2new;
    oid_t * const m_new2old;

    void _compact();
    template<class T>
    void _sort(const typename Pos::template array<T>& rank);

    Reordering (const Reordering<Pos>&) : m_old2new(NULL), m_new2old(NULL)
    { logger.error() << "tried to copy a reordering" |0; }
public:
    Reordering ()
        : m_total(Pos::capacity()), m_used(Pos::numUsed()),
          m_old2new(new oid_t[m_total + 1]),
          m_new2old(new oid_t[m_used + 1])
    {
        m_old2new[0] = 0;
        m_new2old[0] = 0;
        _compact();
    }
    template<class T>
    Reordering (const typename Pos::template array<T>& rank)
        : m_total(Pos::capacity()), m_used(Pos::numUsed()),
          m_old2new(new oid_t[m_total+1]),
          m_new2old(new oid_t[m_used +1])
    {
        m_old2new[0] = 0;
        m_new2old[0] = 0;
        _sort(rank);
    }
    ~Reordering () { delete[] m_old2new; delete[] m_new2old; }

    //indexing
    Pos old2new (Pos old_pos) const
    {
        oid_t i(old_pos);
        Assert4(i <= m_total, "old index out of bounds: "<<i<<" > "<<m_total);
        oid_t j = m_old2new[i];
        Assert4(j <= m_used, "new index out of bounds: "<<j<<" > "<<m_total);
        return Pos(j);
    }
    Pos new2old (Pos new_pos) const
    {
        oid_t j(new_pos);
        Assert4(j <= m_used, "new index out of bounds: "<<j<<" > "<<m_total);
        oid_t i = m_new2old[j];
        Assert4(i <= m_total, "old index out of bounds: "<<i<<" > "<<m_total);
        return Pos(i);
    }

    //direct array access, 1-based but compatable with 0-based
    const oid_t * old2new () const { return m_old2new; }
    const oid_t * new2old () const { return m_new2old; }
};

} // namespace Reorder

#endif // JOHANN_REORDER_H
