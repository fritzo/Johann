#include "merge_tree.hpp"

namespace pomagma
{

oid_t merge_tree::_find (oid_t & oid) const
{
    return oid = find(m_reps[oid]); // ATOMIC
}

void merge_tree::validate () const
{
    //TODO
}

} // namespace pomagma
