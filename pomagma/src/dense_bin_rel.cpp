
#include "dense_bin_rel.hpp"
#include "aligned_alloc.hpp"
#include <cstring>

namespace pomagma
{

dense_bin_rel::dense_bin_rel (size_t item_dim, bool is_full)
    : m_lines(item_dim, false)
{
    POMAGMA_DEBUG("creating dense_bin_rel with "
            << round_word_dim() << " words");
    POMAGMA_ASSERT(round_item_dim() <= MAX_ITEM_DIM,
            "dense_bin_rel is too large");

    // fill if necessary
    if (is_full) support().insert_all();
}

dense_bin_rel::~dense_bin_rel ()
{
}

void dense_bin_rel::move_from (
        const dense_bin_rel & other,
        const oid_t * new2old)
{
    POMAGMA_DEBUG("Copying dense_bin_rel");

    if (POMAGMA_DEBUG_LEVEL >= 1) other.validate();

    // WARNING: assumes this has been done
    //bzero(m_lines.Lx(), sizeof(Word) * data_size_words());
    //bzero(m_lines.Rx(), sizeof(Word) * data_size_words());

    if (new2old == NULL) {
        POMAGMA_DEBUG("copying by column and by row");
        m_lines.move_from(other.m_lines);
    } else {
        POMAGMA_DEBUG("copying and reordering");
        support().move_from(other.support(), new2old);
        // copy & reorder WIKKIT SLOW
        for (oid_t i_new = 1; i_new <= item_dim(); ++i_new) {
            if (not supports(i_new)) continue;
            oid_t i_old = new2old[i_new];

            for (oid_t j_new = 1; j_new <= item_dim(); ++j_new) {
                if (not supports(j_new)) continue;
                oid_t j_old = new2old[j_new];

                if (other.contains(i_old, j_old)) insert(i_new, j_new);
            }
        }
    }
    if (POMAGMA_DEBUG_LEVEL >= 1) validate();
}

//----------------------------------------------------------------------------
// Diagnostics

// supa-slow, try not to use
size_t dense_bin_rel::count_pairs () const
{
    size_t result = 0;
    for (dense_set::iterator i(support()); i.ok(); i.next()) {
        result += get_Lx_set(*i).count_items();
    }
    return result;
}

void dense_bin_rel::validate () const
{
    POMAGMA_DEBUG("Validating dense_bin_rel");

    support().validate();
    m_lines.validate();

    size_t num_pairs = 0;

    dense_set Lx(round_item_dim(), NULL);
    dense_set Rx(round_item_dim(), NULL);
    for (oid_t i = 1; i <= item_dim(); ++i) {
        bool sup_i = supports(i);
        Lx.init(m_lines.Lx(i));

        for (oid_t j = 1; j <= item_dim(); ++j) {
            bool sup_ij = sup_i and supports(j);
            Rx.init(m_lines.Rx(j));

            bool Lx_ij = Lx.contains(j);
            bool Rx_ij = Rx.contains(i);
            num_pairs += Rx_ij;

            POMAGMA_ASSERT(Lx_ij == Rx_ij,
                    "Lx,Rx disagree at " << i << "," << j
                    << ", Lx is " << Lx_ij << ", Rx is " << Rx_ij  );

            POMAGMA_ASSERT(sup_ij or not Lx_ij,
                    "Lx unsupported at " << i << "," << j );

            POMAGMA_ASSERT(sup_ij or not Rx_ij,
                    "Rx unsupported at " << i << "," << j );
        }
    }

    size_t true_size = count_pairs();
    POMAGMA_ASSERT(num_pairs == true_size,
            "incorrect number of pairs: "
            << num_pairs << " should be " << true_size);
}

void dense_bin_rel::validate_disjoint (const dense_bin_rel & other) const
{
    POMAGMA_DEBUG("Validating disjoint pair of dense_bin_rels");

    // validate supports agree
    POMAGMA_ASSERT_EQ(support().item_dim(), other.support().item_dim());
    POMAGMA_ASSERT_EQ(
            support().count_items(),
            other.support().count_items());
    POMAGMA_ASSERT(support() == other.support(),
            "dense_bin_rel supports differ");

    // validate disjointness
    dense_set this_set(item_dim(), NULL);
    dense_set other_set(item_dim(), NULL);
    for (dense_set::iterator i(support()); i.ok(); i.next()) {
        this_set.init(m_lines.Lx(*i));
        other_set.init(other.m_lines.Lx(*i));
        POMAGMA_ASSERT(this_set.disjoint(other_set),
                "dense_bin_rels intersect at row " << *i);
    }
}

void dense_bin_rel::print_table (size_t n) const
{
    if (n == 0) n = item_dim();
    for (oid_t i = 1; i <= n; ++i) {
        std::cout << '\n';
        for (oid_t j = 1; j <= n; ++j) {
            std::cout << (contains(i, j) ? 'O' : '.');
        }
    }
    std::cout << std::endl;
}

//----------------------------------------------------------------------------
// Operations

void dense_bin_rel::remove_Lx (const dense_set & is, oid_t j)
{
    // slower version
    //for (dense_set::iterator i(is); i.ok(); i.next()) {
    //    remove_Lx(*i, j);
    //}

    // faster version
    Word mask = ~(1u << (j % BITS_PER_WORD));
    size_t offset = j / BITS_PER_WORD;
    Word * lines = m_lines.Lx() + offset;
    for (dense_set::iterator i(is); i.ok(); i.next()) {
         lines[*i * round_word_dim()] &= mask; // ATOMIC
    }
}

void dense_bin_rel::remove_Rx (oid_t i, const dense_set& js)
{
    // slower version
    //for (dense_set::iterator j(js); j.ok(); j.next()) {
    //    remove_Rx(i, *j);
    //}

    // faster version
    Word mask = ~(1u << (i % BITS_PER_WORD));
    size_t offset = i / BITS_PER_WORD;
    Word * lines = m_lines.Rx() + offset;
    for (dense_set::iterator j(js); j.ok(); j.next()) {
         lines[*j * round_word_dim()] &= mask; // ATOMIC
    }
}

void dense_bin_rel::remove (oid_t i)
{
    POMAGMA_ASSERT4(supports(i), "tried to remove unsupported element " << i);

    dense_set set(item_dim(), NULL);

    // remove column
    set.init(m_lines.Lx(i));
    remove_Rx(i, set);
    set.zero();

    // remove row
    set.init(m_lines.Rx(i));
    remove_Lx(set, i);
    set.zero();

    support().remove(i);
}

void dense_bin_rel::ensure_inserted (
        oid_t i,
        const dense_set & js,
        void (*change)(oid_t, oid_t))
{
    dense_set diff(item_dim());
    dense_set dest(item_dim(), m_lines.Lx(i));
    if (dest.ensure(js, diff)) {
        for (dense_set::iterator k(diff); k.ok(); k.next()) {
            insert_Rx(i, *k);
            change(i, *k);
        }
    }
}

void dense_bin_rel::ensure_inserted (
        const dense_set & is,
        oid_t j,
        void (*change)(oid_t, oid_t))
{
    dense_set diff(item_dim());
    dense_set dest(item_dim(), m_lines.Rx(j));
    if (dest.ensure(is, diff)) {
        for (dense_set::iterator k(diff); k.ok(); k.next()) {
            insert_Lx(*k, j);
            change(*k, j);
        }
    }
}

// policy: call move_to if i~k but not j~k
void dense_bin_rel::merge (
        oid_t i, // dep
        oid_t j, // rep
        void (*move_to)(oid_t, oid_t)) // typically enforce_
{
    POMAGMA_ASSERT4(j != i, "dense_bin_rel tried to merge item with self");
    POMAGMA_ASSERT4(supports(i) and supports(j),
            "dense_bin_rel tried to merge unsupported items");

    dense_set diff(item_dim());
    dense_set rep(item_dim(), NULL);
    dense_set dep(item_dim(), NULL);

    // merge rows (i, _) into (j, _)
    dep.init(m_lines.Lx(i));
    remove_Rx(i, dep);
    rep.init(m_lines.Lx(j));
    if (rep.merge(dep, diff)) {
        for (dense_set::iterator k(diff); k.ok(); k.next()) {
            insert_Rx(j, *k);
            move_to(j, *k);
        }
    }

    // merge cols (_, i) into (_, j)
    dep.init(m_lines.Rx(i));
    remove_Lx(dep, i);
    rep.init(m_lines.Rx(j));
    if (rep.merge(dep, diff)) {
        for (dense_set::iterator k(diff); k.ok(); k.next()) {
            insert_Lx(*k, j);
            move_to(*k, j);
        }
    }

    support().merge(i, j);
}

// saving/loading, quicker rather than smaller
inline void safe_fread (void * ptr, size_t size, size_t count, FILE * file)
{
    size_t read = fread(ptr, size, count, file);
    POMAGMA_ASSERT(read == count, "fread failed");
}
inline void safe_fwrite (const void * ptr, size_t size, size_t count, FILE * file)
{
    size_t written = fwrite(ptr, size, count, file);
    POMAGMA_ASSERT(written == count, "fwrite failed");
}

oid_t dense_bin_rel::data_size () const
{
    return 2 * sizeof(Word) * data_size_words();
}
void dense_bin_rel::write_to_file (FILE * file)
{
    safe_fwrite(m_lines.Lx(), sizeof(Word), data_size_words(), file);
    safe_fwrite(m_lines.Rx(), sizeof(Word), data_size_words(), file);
}
void dense_bin_rel::read_from_file (FILE * file)
{
    // WARNING assumes support is full
    safe_fread(m_lines.Lx(), sizeof(Word), data_size_words(), file);
    safe_fread(m_lines.Rx(), sizeof(Word), data_size_words(), file);
}

// iteration
void dense_bin_rel::iterator::_find_rhs ()
{
    while (m_lhs.ok()) {
        m_rhs_set.init(m_rel.m_lines.Lx(*m_lhs));
        m_rhs.begin();
        if (m_rhs.ok()) {
            _update_rhs();
            _update_lhs();
            POMAGMA_ASSERT5(m_rel.contains(m_pos),
                    "dense_bin_rel::iterator landed outside of relation: "
                    << m_pos.lhs << "," << m_pos.rhs);
            return;
        }
        m_lhs.next();
    }
    _finish();
}

} // namespace pomagma
