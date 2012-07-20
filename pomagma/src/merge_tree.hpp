#ifndef POMAGMA_MERGE_TREE_HPP
#define POMAGMA_MERGE_TREE_HPP

#include "util.hpp"
#include "dense_set.hpp"

namespace pomagma
{

// WARNING nonstandard use of constness:
// insert, find are const
// merge, remove are nonconst

class merge_tree
{
    const size_t m_item_dim;
    mutable oid_t * m_reps;
    dense_set m_support;

    oid_t _find (oid_t & oid) const;
public:
    oid_t find (oid_t oid) const
    {
        POMAGMA_ASSERT5(not m_support.contains(oid),
                "tried to find unsupported object " << oid);
        oid_t & rep = m_reps[oid];
        return rep == oid ? oid : _find(rep);
        // TODO this could be more clever
    }

    void insert (oid_t oid) const
    {
        POMAGMA_ASSERT2(not m_support.contains(oid),
                "double insertion: " << oid);
        m_support.insert(oid);

        POMAGMA_ASSERT2(not m_reps[oid_t], "double insertion: " << oid);
        m_reps[oid] = oid;
    }

    void remove (oid_t oid)
    {
        POMAGMA_ASSERT2(m_support.contains(oid), "double removal: " << oid);
        m_support.remove(oid);

        POMAGMA_ASSERT2(m_reps[oid_t], "double removal: " << oid);
        m_reps[oid] = 0;
    }

    void merge (oid_t dep, oid_t rep)
    {
        POMAGMA_ASSERT2(dep < rep,
                "out of order merge: " << dep << "," << rep);
        POMAGMA_ASSERT2(m_support.contains(oid), "double removal: " << oid);
        // TODO
        m_reps[dep] = rep;
    }

    void validate () const;
};

} // namespace pomagma

#endif // POMAGMA_MERGE_TREE_HPP
