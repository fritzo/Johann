#include "carrier.hpp"
#include "aligned_alloc.hpp"

namespace pomagma
{

Carrier::Carrier (size_t item_dim)
    : m_support(item_dim),
      m_unsupport(item_dim),
      m_item_count(0),
      m_reps(alloc_blocks<oid_t>(1 + item_dim))
{
    POMAGMA_DEBUG("creating Carrier with " << item_dim << " items");
    m_support.zero();
    m_unsupport.insert_all();
}

void Carrier::move_from (
        const Carrier & other __attribute__((unused)),
        const oid_t * new2old __attribute__((unused)))
{
    // TODO
}

oid_t Carrier::insert () // WARNING not thread safe
{
    POMAGMA_ASSERT1(item_count() < item_dim(),
            "tried to insert in full Carrier");

    oid_t oid = * dense_set::iterator(m_unsupport);
    m_support.insert(oid);
    m_unsupport.remove(oid);
    m_reps[oid] = oid;
    return oid;
}

void Carrier::remove (oid_t oid) // WARNING not thread safe
{
    POMAGMA_ASSERT2(m_support.contains(oid), "double removal: " << oid);
    POMAGMA_ASSERT2(m_reps[oid], "double removal: " << oid);

    oid_t rep = m_reps[oid];
    if (rep == oid) {
        for (oid_t other = oid + 1; other <= item_dim(); ++other) {
            POMAGMA_ASSERT2(m_reps[other] != oid, "removed a rep: " << oid);
        }
    } else {
        for (oid_t other = oid + 1; other <= item_dim(); ++other) {
            if (m_reps[other] == oid) {
                m_reps[other] = rep;
            }
        }
    }

    m_support.remove(oid);
    m_unsupport.insert(oid);
    m_reps[oid] = 0;
}

//----------------------------------------------------------------------------
// Merge trees

oid_t Carrier::_find (oid_t & oid) const
{
    return oid = find(m_reps[oid]); // ATOMIC
}

void Carrier::validate () const
{
    m_support.validate();
    m_unsupport.validate();
    dense_set all(item_dim());
    all.insert_all();
    dense_set diff(item_dim());
    diff.set_diff(m_support, m_unsupport);
    POMAGMA_ASSERT(diff == all, "support, unsupport are not complementary");

    for (oid_t i = 1; i <= item_dim(); ++i) {
        if (contains(i)) {
            POMAGMA_ASSERT(m_reps[i], "supported object has no rep: " << i);
            POMAGMA_ASSERT(m_reps[i] <= i,
                "rep out of order: " << m_reps[i] << "," << i);
        } else {
            POMAGMA_ASSERT(m_reps[i] == 0, "unsupported object has rep: " << i);
        }
    }
}

} // namespace pomagma
