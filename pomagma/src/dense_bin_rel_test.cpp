
#include "util.hpp"
#include "dense_bin_rel.hpp"
#include <utility>

using namespace pomagma;

size_t g_num_moved(0);
void move_to (oid_t i __attribute__((unused)), oid_t j __attribute__((unused)))
{
    //std::cout << i << '-' << j << ' ' << std::flush; //DEBUG
    ++g_num_moved;
}

bool br_test1 (oid_t i, oid_t j) { return i and j and i % 61u <= j % 31u; }
bool br_test2 (oid_t i, oid_t j) { return i and j and i % 61u == j % 31u; }

typedef pomagma::dense_bin_rel dense_bin_rel;

void test_dense_bin_rel (
        size_t size,
        bool test1(oid_t, oid_t),
        bool test2(oid_t, oid_t))
{
    POMAGMA_INFO("Testing dense_bin_rel");

    POMAGMA_INFO("creating dense_bin_rel of size " << size);
    dense_bin_rel rel(size);


    POMAGMA_INFO("testing position insertion");
    size_t item_count = 0;
    for (oid_t i = 1; i <= size; ++i) {
        rel.insert(i);
        ++item_count;
    }
    POMAGMA_ASSERT_EQ(item_count, rel.count_items_support());


    POMAGMA_INFO("testing pair insertion");
    size_t num_pairs = 0;
    for (oid_t i = 1; i <= size; ++i) {
    for (oid_t j = 1; j <= size; ++j) {
        if (test1(i, j)) {
            rel.insert(i, j);
            ++num_pairs;
        }
    } }
    POMAGMA_INFO("  " << num_pairs << " pairs inserted");
    rel.validate();
    POMAGMA_ASSERT_EQ(num_pairs, rel.count_pairs());


    POMAGMA_INFO("testing pair removal");
    for (oid_t i = 1; i <= size; ++i) {
    for (oid_t j = 1; j <= size; ++j) {
        if (test1(i, j) and test2(i, j)) {
            rel.remove(i, j);
            --num_pairs;
        }
    } }
    POMAGMA_INFO("  " << num_pairs << " pairs remain");
    rel.validate();
    POMAGMA_ASSERT_EQ(num_pairs, rel.count_pairs());


    POMAGMA_INFO("testing table iterator");
    size_t num_pairs_seen = 0;
    for (dense_bin_rel::iterator iter(&rel); iter.ok(); iter.next()) {
        ++num_pairs_seen;
    }
    POMAGMA_INFO("  iterated over "
        << num_pairs_seen << " / " << num_pairs << " pairs");
    rel.validate();
    POMAGMA_ASSERT_EQ(num_pairs_seen, num_pairs);


    POMAGMA_INFO("testing pair containment");
    num_pairs = 0;
    for (oid_t i = 1; i <= size; ++i) {
    for (oid_t j = 1; j <= size; ++j) {
        if (test1(i, j) and not test2(i, j)) {
            POMAGMA_ASSERT(rel.contains_Lx(i, j),
                    "Lx relation missing " << i << ',' << j);
            POMAGMA_ASSERT(rel.contains_Rx(i, j),
                    "Rx relation missing " << i << ',' << j);
            ++num_pairs;
        } else {
            POMAGMA_ASSERT(not rel.contains_Lx(i, j),
                    "Lx relation has extra " << i << ',' << j);
            POMAGMA_ASSERT(not rel.contains_Rx(i, j),
                    "Rx relation has extra " << i << ',' << j);
        }
    } }
    POMAGMA_INFO("  " << num_pairs << " pairs found");
    rel.validate();
    POMAGMA_ASSERT_EQ(num_pairs, rel.count_pairs());


    POMAGMA_INFO("testing position merging");
    for (oid_t i = 1; i <= size / 3; ++i) {
        oid_t m = (2 * i) % size;
        oid_t n = (2 * (size - i - 1) + 1) % size;
        if (not (rel.supports(m, n) and rel.contains(m, n))) continue;
        if (m == n) continue;
        if (m < n) std::swap(m, n);
        rel.merge(m, n, move_to);
        --item_count;
    }
    POMAGMA_INFO("  " << g_num_moved << " pairs moved in merging");
    rel.validate();
    POMAGMA_ASSERT_EQ(item_count, rel.count_items_support());


    POMAGMA_INFO("testing table iterator again");
    num_pairs_seen = 0;
    for (dense_bin_rel::iterator iter(&rel); iter.ok(); iter.next()) {
        ++num_pairs_seen;
    }
    num_pairs = rel.count_pairs();
    POMAGMA_INFO("  iterated over "
        << num_pairs_seen << " / " << num_pairs << " pairs");
    rel.validate();
    POMAGMA_ASSERT_EQ(num_pairs_seen, num_pairs);


    POMAGMA_INFO("testing line Iterator<LHS_FIXED>");
    num_pairs = 0;
    size_t seen_item_count = 0;
    item_count = rel.count_items_support();
    for (oid_t i = 1; i <= size; ++i) {
        if (not rel.supports(i)) continue;
        ++seen_item_count;
        dense_bin_rel::Iterator<dense_bin_rel::LHS_FIXED> iter(i, &rel);
        for (iter.begin(); iter.ok(); iter.next()) {
            ++num_pairs;
        }
    }
    POMAGMA_INFO("  Iterated over " << seen_item_count << " items");
    POMAGMA_INFO("  Iterated over " << num_pairs << " pairs");
    rel.validate();
    POMAGMA_ASSERT_EQ(seen_item_count, item_count);
    size_t true_size = rel.count_pairs();
    POMAGMA_ASSERT_EQ(num_pairs, true_size);


    POMAGMA_INFO("testing line Iterator<RHS_FIXED>");
    num_pairs = 0;
    seen_item_count = 0;
    for (size_t i = 1; i <= size; ++i) {
        if (not rel.supports(i)) continue;
        ++seen_item_count;
        dense_bin_rel::Iterator<dense_bin_rel::RHS_FIXED> iter(i, &rel);
        for (iter.begin(); iter.ok(); iter.next()) {
            ++num_pairs;
        }
    }
    POMAGMA_INFO("  Iterated over " << seen_item_count << " items");
    POMAGMA_INFO("  Iterated over " << num_pairs << " pairs");
    rel.validate();
    POMAGMA_ASSERT_EQ(seen_item_count, item_count);
    POMAGMA_ASSERT_EQ(num_pairs, true_size);
}

int main ()
{
    Log::title("Running Binary Relation Test");

    for (size_t i = 0; i < 4; ++i) {
        test_dense_bin_rel(i + (1 << 9), br_test1, br_test2);
    }

    return 0;
}
