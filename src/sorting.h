#ifndef NONSTD_SORTING
#define NONSTD_SORTING

#include <set>
#include "definitions.h" //for assert

namespace nonstd
{

using Logging::logger;

/// A sieve to get the N best elements from some list.
template<class Item, class Worse>
class N_Best
{
    typedef std::set<Item, Worse> Set;
    Set m_set;
    const Int m_N;
    Item m_worst;
    Worse worse;

    //insertion
public:
    void insert (Item item)
    {
        if (!m_N) return;
        if (m_set.size() >= m_N) {
            if (worse(item, m_worst)) return;
            m_set.erase(m_worst);
        }
        m_set.insert(item);
        m_worst = *(m_set.begin());
        Assert4(m_set.size() <= m_N, "N_Best set got too big");
    }
    template<class _iterator> void insert (_iterator _begin, _iterator _end)
    {
        for (_iterator iter=_begin; iter!=_end; ++iter) insert(*iter);
    }

    //construction
    N_Best (Int N) : m_N(N) {}
    N_Best (Int N, Worse _worse) : m_N(N), worse(_worse) {}

    template<class _iterator>
    N_Best (Int N, _iterator begin, _iterator end)
        : m_N(N)
    { insert(begin, end); }

    template<class _iterator>
    N_Best (Int N, Worse _worse, _iterator begin, _iterator end)
        : m_set(_worse), m_N(N), worse(_worse)
    { insert(begin, end); }

    void clear () { m_set.clear(); }

    //iteration
    typedef typename Set::reverse_iterator iterator;
    iterator begin () const { return m_set.rbegin(); }
    iterator end   () const { return m_set.rend(); }
};

}

#endif
