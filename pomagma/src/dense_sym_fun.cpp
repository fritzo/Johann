
#include "dense_sym_fun.hpp"
#include "aligned_alloc.hpp"
#include <cstring>

namespace pomagma
{

dense_sym_fun::dense_sym_fun (size_t item_dim)
    : m_lines(item_dim, true),
      m_block_dim((item_dim + ITEMS_PER_BLOCK) / ITEMS_PER_BLOCK),
      m_blocks(pomagma::alloc_blocks<Block4x4>(
                  unordered_pair_count(m_block_dim)))
{
    POMAGMA_DEBUG("creating dense_sym_fun with "
            << unordered_pair_count(m_block_dim) << " blocks");

    // FIXME allow larger
    POMAGMA_ASSERT(item_dim < (1 << 15), "dense_sym_fun is too large");

    // initialize to zero
    bzero(m_blocks, unordered_pair_count(m_block_dim) * sizeof(Block4x4));
}

dense_sym_fun::~dense_sym_fun ()
{
    pomagma::free_blocks(m_blocks);
}

// for growing
void dense_sym_fun::move_from (const dense_sym_fun & other)
{
    POMAGMA_DEBUG("Copying dense_sym_fun");

    size_t min_block_dim = min(m_block_dim, other.m_block_dim);
    for (size_t j_ = 0; j_ < min_block_dim; ++j_) {
        oid_t * destin = _block(0, j_);
        const oid_t * source = other._block(0, j_);
        memcpy(destin, source, sizeof(Block4x4) * (1 + j_));
    }

    m_lines.move_from(other.m_lines);
}

//----------------------------------------------------------------------------
// Diagnostics

size_t dense_sym_fun::count_pairs () const
{
    dense_set set(item_dim(), NULL);

    size_t result = 0;
    for (size_t i = 1; i <= item_dim(); ++i) {
        set.init(m_lines.Lx(i));
        result += set.count_items();
    }
    return result;
}

void dense_sym_fun::validate () const
{
    POMAGMA_DEBUG("Validating dense_sym_fun");

    m_lines.validate();

    POMAGMA_DEBUG("validating line-block consistency");
    for (size_t i_ = 0; i_ < m_block_dim; ++i_) {
    for (size_t j_ = i_; j_ < m_block_dim; ++j_) {
        const oid_t * block = _block(i_, j_);

        for (size_t _i = 0; _i < ITEMS_PER_BLOCK; ++_i) {
        for (size_t _j = 0; _j < ITEMS_PER_BLOCK; ++_j) {
            size_t i = i_ * ITEMS_PER_BLOCK + _i;
            size_t j = j_ * ITEMS_PER_BLOCK + _j;
            if (i == 0 or item_dim() < i) continue;
            if (j < i or item_dim() < j) continue;
            oid_t val = _block2value(block, _i, _j);

            if (val) {
                POMAGMA_ASSERT(contains(i,j),
                        "found unsupported value: "<<i<<','<<j);
            } else {
                POMAGMA_ASSERT(not contains(i,j),
                        "found supported null value: "<<i<<','<<j);
            }
        }}
    }}
}

//----------------------------------------------------------------------------
// Operations

void dense_sym_fun::remove(
        const oid_t dep,
        void remove_value(oid_t)) // rem
{
    POMAGMA_ASSERT_RANGE_(4, dep, item_dim());

    dense_set set(item_dim(), NULL);

    for (Iterator iter(this, dep); iter.ok(); iter.next()) {
        oid_t rhs = iter.moving();
        oid_t & dep_val = value(rhs, dep);
        remove_value(dep_val);
        set.init(m_lines.Lx(rhs));
        set.remove(dep);
        dep_val = 0;
    }
    set.init(m_lines.Lx(dep));
    set.zero();
}

void dense_sym_fun::merge(
        const oid_t dep, // dep_val
        const oid_t rep, // rep_val
        void merge_values(oid_t, oid_t), // dep_val, rep_val
        void move_value(oid_t, oid_t, oid_t)) // moved, lhs, rhs
{
    POMAGMA_ASSERT4(rep != dep, "self merge: " << dep << "," << rep);
    POMAGMA_ASSERT_RANGE_(4, dep, item_dim());
    POMAGMA_ASSERT_RANGE_(4, rep, item_dim());

    dense_set set(item_dim(), NULL);
    dense_set dep_set(item_dim(), NULL);
    dense_set rep_set(item_dim(), NULL);

    // (dep, dep) -> (dep, rep)
    if (contains(dep, dep)) {
        oid_t & dep_val = value(dep, dep);
        oid_t & rep_val = value(rep, rep);
        set.init(m_lines.Lx(dep));
        set.remove(dep);
        if (rep_val) {
            merge_values(dep_val, rep_val);
        } else {
            move_value(dep_val, rep, rep);
            set.init(m_lines.Lx(rep));
            set.insert(rep);
            rep_val = dep_val;
        }
        dep_val = 0;
    }

    // (dep, rhs) --> (rep, rep) for rhs != dep
    for (Iterator iter(this, dep); iter.ok(); iter.next()) {
        oid_t rhs = iter.moving();
        oid_t & dep_val = value(rhs, dep);
        oid_t & rep_val = value(rhs, rep);
        set.init(m_lines.Lx(rhs));
        set.remove(dep);
        if (rep_val) {
            merge_values(dep_val,rep_val);
        } else {
            move_value(dep_val, rhs, rep);
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
