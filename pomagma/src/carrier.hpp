#ifndef POMAGMA_CARRIER_HPP
#define POMAGMA_CARRIER_HPP

#include "util.hpp"
#include "dense_set.hpp"
#include <functional>
#include <algorithm>

namespace pomagma
{

// WARNING nonstandard use of constness:
// insert, find are const
// merge, remove are nonconst

class Carrier
{
    dense_set m_support;
    dense_set m_unsupport;
    size_t m_item_count;
    size_t m_rep_count;
    mutable oid_t * m_reps;

public:

    Carrier (size_t item_dim);
    void move_from (const Carrier & other, const oid_t * new2old);

    size_t item_dim () const { return m_support.item_dim(); }
    size_t item_count () const { return m_item_count; }
    size_t rep_count () const { return m_rep_count; }
    bool contains (oid_t oid) const { return m_support.contains(oid); }

    // merge trees
private:
    oid_t _find (oid_t & oid) const;
public:
    oid_t find (oid_t oid) const
    {
        POMAGMA_ASSERT5(contains(oid),
                "tried to find unsupported object " << oid);
        oid_t & rep = m_reps[oid];
        return rep == oid ? oid : _find(rep);
        // TODO this could be more clever
    }

    void insert (oid_t oid) // WARNING not thread safe
    {
        POMAGMA_ASSERT1(not contains(oid), "double insertion: " << oid);
        POMAGMA_ASSERT1(not m_reps[oid], "double insertion: " << oid);

        m_support.insert(oid);
        m_unsupport.remove(oid);
        m_reps[oid] = oid;
    }

    oid_t insert (); // WARNING not thread safe

    void remove (oid_t oid); // WARNING not thread safe

    void merge (oid_t dep, oid_t rep) const
    {
        POMAGMA_ASSERT2(dep < rep,
                "out of order merge: " << dep << "," << rep);
        POMAGMA_ASSERT2(m_support.contains(dep), "bad merge dep " << dep);
        POMAGMA_ASSERT2(m_support.contains(rep), "bad merge rep " << rep);

        m_reps[find(dep)] = find(rep); // ATOMIC
    }

    void validate () const;
};

} // namespace pomagma

#endif // POMAGMA_CARRIER_HPP
