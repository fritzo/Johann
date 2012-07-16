
#include "dense_sym_fun.hpp"
#include "aligned_alloc.hpp"
#include <cstring>

namespace pomagma
{

dense_sym_fun::dense_sym_fun (size_t item_dim)
    : m_item_dim(item_dim),
      m_block_dim((m_item_dim + ITEMS_PER_BLOCK) / ITEMS_PER_BLOCK),
      m_word_dim(dense_set::word_count(m_item_dim)),
      m_blocks(pomagma::alloc_blocks<Block4x4>(
                  unordered_pair_count(m_block_dim))),
      m_Lx_lines(pomagma::alloc_blocks<Word>((m_item_dim + 1) * m_word_dim)),
      m_temp_line(pomagma::alloc_blocks<Word>(m_word_dim))
{
    POMAGMA_DEBUG("creating dense_sym_fun with "
            << unordered_pair_count(m_block_dim) << " blocks");

    // FIXME allow larger
    POMAGMA_ASSERT(m_item_dim < (1 << 15), "dense_sym_fun is too large");

    // initialize to zero
    bzero(m_blocks, unordered_pair_count(m_block_dim) * sizeof(Block4x4));
    bzero(m_Lx_lines, (m_item_dim + 1) * m_word_dim * sizeof(Word));
}

dense_sym_fun::~dense_sym_fun ()
{
    pomagma::free_blocks(m_blocks);
    pomagma::free_blocks(m_Lx_lines);
    pomagma::free_blocks(m_temp_line);
}

// for growing
void dense_sym_fun::move_from (const dense_sym_fun & other)
{
    POMAGMA_DEBUG("Copying dense_sym_fun");

    // copy data
    unsigned minM = min(m_block_dim, other.m_block_dim);
    for (unsigned j_ = 0; j_ < minM; ++j_) {
        oid_t * destin = _block(0, j_);
        const oid_t * source = other._block(0, j_);
        memcpy(destin, source, sizeof(Block4x4) * (1 + j_));
    }

    // copy sets
    unsigned minN = min(m_item_dim, other.m_item_dim);
    unsigned minL = min(m_word_dim, other.m_word_dim);
    for (unsigned i = 1; i <= minN; ++i) {
        memcpy(get_Lx_line(i), other.get_Lx_line(i), sizeof(Word) * minL);
    }
}

//----------------------------------------------------------------------------
// Diagnostics

unsigned dense_sym_fun::count_pairs () const
{
    dense_set set(m_item_dim, NULL);

    unsigned result = 0;
    for (unsigned i = 1; i <= m_item_dim; ++i) {
        set.init(get_Lx_line(i));
        result += set.count_items();
    }
    return result;
}

void dense_sym_fun::validate () const
{
    POMAGMA_DEBUG("Validating dense_sym_fun");

    POMAGMA_DEBUG("validating line-block consistency");
    for (unsigned i_ = 0; i_ < m_block_dim; ++i_) {
    for (unsigned j_ = i_; j_ < m_block_dim; ++j_) {
        const oid_t * block = _block(i_, j_);

        for (unsigned _i = 0; _i < ITEMS_PER_BLOCK; ++_i) {
        for (unsigned _j = 0; _j < ITEMS_PER_BLOCK; ++_j) {
            unsigned i = i_ * ITEMS_PER_BLOCK + _i;
            unsigned j = j_ * ITEMS_PER_BLOCK + _j;
            if (i == 0 or m_item_dim < i) continue;
            if (j < i or m_item_dim < j) continue;
            oid_t val = _block2value(block, _i, _j);

            if (val) {
                POMAGMA_ASSERT(contains(i,j),
                        "invalid: found unsupported value: "<<i<<','<<j);
            } else {
                POMAGMA_ASSERT(not contains(i,j),
                        "invalid: found supported null value: "<<i<<','<<j);
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
    POMAGMA_ASSERT_RANGE_(4, dep, m_item_dim);

    dense_set set(m_item_dim, NULL);

    for (Iterator iter(this, dep); iter.ok(); iter.next()) {
        oid_t rhs = iter.moving();
        oid_t & dep_val = value(rhs, dep);
        remove_value(dep_val);
        set.init(get_Lx_line(rhs));
        set.remove(dep);
        dep_val = 0;
    }
    set.init(get_Lx_line(dep));
    set.zero();
}

void dense_sym_fun::merge(
        const oid_t dep, // dep_val
        const oid_t rep, // rep_val
        void merge_values(oid_t, oid_t), // dep_val, rep_val
        void move_value(oid_t, oid_t, oid_t)) // moved, lhs, rhs
{
    POMAGMA_ASSERT4(rep != dep, "self merge: " << dep << "," << rep);
    POMAGMA_ASSERT_RANGE_(4, dep, m_item_dim);
    POMAGMA_ASSERT_RANGE_(4, rep, m_item_dim);

    dense_set set(m_item_dim, NULL);

    // (dep, dep) -> (dep, rep)
    if (contains(dep, dep)) {
        oid_t & dep_val = value(dep, dep);
        oid_t & rep_val = value(rep, rep);
        set.init(get_Lx_line(dep));
        set.remove(dep);
        if (rep_val) {
            merge_values(dep_val, rep_val);
        } else {
            move_value(dep_val, rep, rep);
            set.init(get_Lx_line(rep));
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
        set.init(get_Lx_line(rhs));
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
    dense_set rep_set(m_item_dim, get_Lx_line(rep));
    dense_set dep_set(m_item_dim, get_Lx_line(dep));
    rep_set.merge(dep_set);
}

//----------------------------------------------------------------------------
// Intersection iteration

Word * dense_sym_fun::_get_LLx_line (oid_t i, oid_t j) const
{
    Word * i_line = get_Lx_line(i);
    Word * j_line = get_Lx_line(j);
    for (oid_t k_ = 0; k_ < m_word_dim; ++k_) {
        m_temp_line[k_] = i_line[k_] & j_line[k_];
    }
    return m_temp_line;
}

}


