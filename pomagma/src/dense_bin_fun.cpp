#include "dense_bin_fun.hpp"
#include "aligned_alloc.hpp"
#include <cstring>

namespace pomagma
{

dense_bin_fun::dense_bin_fun (size_t item_dim)
    : m_lines(item_dim, false),
      m_block_dim((item_dim + ITEMS_PER_BLOCK) / ITEMS_PER_BLOCK),
      m_blocks(pomagma::alloc_blocks<Block4x4>(m_block_dim * m_block_dim))
{
    POMAGMA_DEBUG("creating dense_bin_fun with "
            << (m_block_dim * m_block_dim) << " blocks");

    // FIXME allow larger
    POMAGMA_ASSERT(item_dim < (1<<15), "dense_bin_fun is too large");

    // initialize to zero
    bzero(m_blocks, m_block_dim * m_block_dim * sizeof(Block4x4));
}

dense_bin_fun::~dense_bin_fun ()
{
    pomagma::free_blocks(m_blocks);
}

// for growing
void dense_bin_fun::move_from (const dense_bin_fun & other)
{
    POMAGMA_DEBUG("Copying dense_bin_fun");

    size_t min_block_dim = min(m_block_dim, other.m_block_dim);
    for (size_t j_ = 0; j_ < min_block_dim; ++j_) {
        oid_t * destin = _block(0, j_);
        const oid_t * source = other._block(0, j_);
        memcpy(destin, source, sizeof(Block4x4) * min_block_dim);
    }

    m_lines.move_from(other.m_lines);
}

//----------------------------------------------------------------------------
// Diagnostics

size_t dense_bin_fun::count_pairs () const
{
    dense_set Lx_set(item_dim(), NULL);
    size_t result = 0;
    for (size_t i = 1; i <= item_dim(); ++i) {
        Lx_set.init(m_lines.Lx(i));
        result += Lx_set.count_items();
    }
    return result;
}

void dense_bin_fun::validate () const
{
    POMAGMA_DEBUG("Validating dense_bin_fun");

    m_lines.validate();

    POMAGMA_DEBUG("validating line-block consistency");
    for (size_t i_ = 0; i_ < m_block_dim; ++i_) {
    for (size_t j_ = 0; j_ < m_block_dim; ++j_) {
        const oid_t * block = _block(i_,j_);

        for (size_t _i = 0; _i < ITEMS_PER_BLOCK; ++_i) {
        for (size_t _j = 0; _j < ITEMS_PER_BLOCK; ++_j) {
            size_t i = i_ * ITEMS_PER_BLOCK+_i;
            size_t j = j_ * ITEMS_PER_BLOCK+_j;
            if (i == 0 or item_dim() < i) continue;
            if (j == 0 or item_dim() < j) continue;
            oid_t val = _block2value(block, _i, _j);

            if (val) {
                POMAGMA_ASSERT(contains(i, j),
                        "found unsupported value: " << i << ',' << j);
            } else {
                POMAGMA_ASSERT(not contains(i, j),
                        "found supported null value: " << i << ',' << j);
            }
        }}
    }}
}

//----------------------------------------------------------------------------
// Operations

void dense_bin_fun::remove(
        const oid_t dep,
        void remove_value(oid_t)) // rem
{
    POMAGMA_ASSERT_RANGE_(4, dep, item_dim());

    dense_set set(item_dim(), NULL);

    // (lhs, dep)
    for (Iterator<RHS_FIXED> iter(this, dep); iter.ok(); iter.next()) {
        oid_t lhs = iter.lhs();
        oid_t & dep_val = value(lhs, dep);
        remove_value(dep_val);
        set.init(m_lines.Lx(lhs));
        set.remove(dep);
        dep_val = 0;
    }
    set.init(m_lines.Rx(dep));
    set.zero();

    // (dep, rhs)
    for (Iterator<LHS_FIXED> iter(this, dep); iter.ok(); iter.next()) {
        oid_t rhs = iter.rhs();
        oid_t & dep_val = value(dep, rhs);
        remove_value(dep_val);
        set.init(m_lines.Rx(rhs));
        set.remove(dep);
        dep_val = 0;
    }
    set.init(m_lines.Lx(dep));
    set.zero();
}

void dense_bin_fun::merge(
        const oid_t dep,
        const oid_t rep,
        void merge_values(oid_t, oid_t), // dep, rep
        void move_value(oid_t, oid_t, oid_t)) // moved, lhs, rhs
{
    POMAGMA_ASSERT4(rep != dep, "self merge: " << dep << "," << rep);
    POMAGMA_ASSERT_RANGE_(4, dep, item_dim());
    POMAGMA_ASSERT_RANGE_(4, rep, item_dim());

    dense_set set(item_dim(), NULL);
    dense_set dep_set(item_dim(), NULL);
    dense_set rep_set(item_dim(), NULL);

    // Note: the spacial case
    //   (dep, dep) --> (dep, rep) --> (rep, rep)
    // merges in two steps

    // (lhs, dep) --> (lhs, rep)
    for (Iterator<RHS_FIXED> iter(this, dep); iter.ok(); iter.next()) {
        oid_t lhs = iter.lhs();
        oid_t & dep_val = value(lhs,dep);
        oid_t & rep_val = value(lhs,rep);
        set.init(m_lines.Lx(lhs));
        set.remove(dep);
        if (rep_val) {
            merge_values(dep_val, rep_val);
        } else {
            move_value(dep_val, lhs, rep);
            set.insert(rep);
            rep_val = dep_val;
        }
        dep_val = 0;
    }
    rep_set.init(m_lines.Rx(rep));
    dep_set.init(m_lines.Rx(dep));
    rep_set.merge(dep_set);

    // (dep, rhs) --> (rep, rhs)
    for (Iterator<LHS_FIXED> iter(this, dep); iter.ok(); iter.next()) {
        oid_t rhs = iter.rhs();
        oid_t & dep_val = value(dep, rhs);
        oid_t & rep_val = value(rep, rhs);
        set.init(m_lines.Rx(rhs));
        set.remove(dep);
        if (rep_val) {
            merge_values(dep_val, rep_val);
        } else {
            move_value(dep_val, rep, rhs);
            set.insert(rep);
            rep_val = dep_val;
        }
        dep_val = 0;
    }
    rep_set.init(m_lines.Lx(rep));
    dep_set.init(m_lines.Lx(dep));
    rep_set.merge(dep_set);
}

} // namespace pomagma
