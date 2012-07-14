
#include "util.hpp"
#include "dense_bin_rel.hpp"
#include <utility>

using pomagma::Log;
using pomagma::dense_set;
using pomagma::dense_bin_rel;

unsigned g_num_moved(0);
void move_to (int i __attribute__((unused)), int j __attribute__((unused)))
{
    //std::cout << i << '-' << j << ' ' << std::flush; //DEBUG
    ++g_num_moved;
}

bool br_test1 (int i, int j) { return i and j and i % 61 <= j % 31; }
bool br_test2 (int i, int j) { return i and j and i % 61 == j % 31; }

typedef pomagma::dense_bin_rel Rel;
enum Direction { LHS_FIXED=true, RHS_FIXED=false };

void test_dense_bin_rel (
        size_t N,
        bool test1(int, int),
        bool test2(int, int))
{
    POMAGMA_INFO("Testing dense_bin_rel");

    POMAGMA_INFO("creating dense_bin_rel of size " << N);
    Rel R(N);

    //========================================================================
    POMAGMA_INFO("testing position insertion");
    unsigned item_count=0;
    for (size_t i = 1; i <= N; ++i) {
        R.insert(i);
        ++item_count;
    }
    POMAGMA_ASSERT(item_count == R.count_items_support(),
            "incorrect support size");

    //========================================================================
    POMAGMA_INFO("testing pair insertion");
    unsigned num_pairs = 0;
    for (size_t i = 1; i <= N; ++i) {
    for (size_t j = 1; j <= N; ++j) {
        if (test1(i, j)) {
            R.insert(i, j);
            ++num_pairs;
        }
    } }
    POMAGMA_INFO("  " << num_pairs << " pairs inserted");
    R.validate();
    POMAGMA_ASSERT(num_pairs == R.count_items(),
            "dense_bin_rel contained incorrect number of pairs");

    //========================================================================
    POMAGMA_INFO("testing pair removal");
    for (size_t i = 1; i <= N; ++i) {
    for (size_t j = 1; j <= N; ++j) {
        if (test1(i, j) and test2(i, j)) {
            R.remove(i, j);
            --num_pairs;
        }
    } }
    POMAGMA_INFO("  " << num_pairs << " pairs remain");
    R.validate();
    POMAGMA_ASSERT(num_pairs == R.count_items(),
            "dense_bin_rel contained incorrect number of pairs");

    //========================================================================
    POMAGMA_INFO("testing table iterator");
    unsigned num_pairs_seen = 0;
    for (Rel::iterator iter(&R); iter.ok(); iter.next()) {
        ++num_pairs_seen;
    }
    POMAGMA_INFO("  iterated over "
        << num_pairs_seen << " / " << num_pairs << " pairs");
    R.validate();
    POMAGMA_ASSERT(num_pairs_seen == num_pairs,
            "dense_bin_rel iterated over incorrect number of pairs");

    //========================================================================
    POMAGMA_INFO("testing pair containment");
    num_pairs = 0;
    for (size_t i = 1; i <= N; ++i) {
    for (size_t j = 1; j <= N; ++j) {
        if (test1(i, j) and not test2(i, j)) {
            POMAGMA_ASSERT(R.contains_Lx(i, j),
                    "Lx relation doesn't contain what it should");
            POMAGMA_ASSERT(R.contains_Rx(i, j),
                    "Rx relation doesn't contain what it should");
            ++num_pairs;
        } else {
            POMAGMA_ASSERT(not R.contains_Lx(i, j),
                    "Lx relation contains what it shouldn't");
            POMAGMA_ASSERT(not R.contains_Rx(i, j),
                    "Rx relation contains what it shouldn't");
        }
    } }
    POMAGMA_INFO("  " << num_pairs << " pairs found");
    R.validate();
    POMAGMA_ASSERT(num_pairs == R.count_items(),
            "dense_bin_rel contained incorrect number of pairs");

    //========================================================================
    POMAGMA_INFO("testing position merging");
    for (size_t i = 1; i <= N/3; ++i) {
        size_t m=(2*i)%N, n=(2*(N-i-1)+1)%N;
        if (not (R.supports(m,n) and R.contains(m,n))) continue;
        if (m == n) continue;
        if (m < n) std::swap(m,n);
        R.merge(m,n, move_to);
        --item_count;
    }
    POMAGMA_INFO("  " << g_num_moved << " pairs moved in merging");
    R.validate();
    POMAGMA_ASSERT(item_count == R.count_items_support(), "incorrect support size");

    //========================================================================
    POMAGMA_INFO("testing table iterator again");
    num_pairs_seen = 0;
    for (Rel::iterator iter(&R); iter.ok(); iter.next()) {
        ++num_pairs_seen;
    }
    num_pairs = R.count_items();
    POMAGMA_INFO("  iterated over "
        << num_pairs_seen << " / " << num_pairs << " pairs");
    R.validate();
    POMAGMA_ASSERT(num_pairs_seen == num_pairs,
            "dense_bin_rel iterated over incorrect number of pairs");

    //========================================================================
    POMAGMA_INFO("testing line Iterator<LHS_FIXED>");
    num_pairs = 0;
    unsigned seen_item_count = 0;
    item_count = R.count_items_support();
    for (size_t i = 1; i <= N; ++i) {
        if (not R.supports(i)) continue;
        ++seen_item_count;
        Rel::Iterator<LHS_FIXED> iter(i, &R);
        for (iter.begin(); iter.ok(); iter.next()) {
            ++num_pairs;
        }
    }
    POMAGMA_INFO("  Iterated over " << seen_item_count << " items");
    POMAGMA_INFO("  Iterated over " << num_pairs << " pairs");
    R.validate();
    POMAGMA_ASSERT(seen_item_count == item_count,
            "Iterator had incorrect support");
    unsigned true_size = R.count_items();
    POMAGMA_ASSERT(num_pairs == true_size, //each pair is seen twice
            "dense_bin_rel Iterated over incorrect number of pairs"
            << ": " << num_pairs << " vs " << true_size);

    //========================================================================
    POMAGMA_INFO("testing line Iterator<RHS_FIXED>");
    num_pairs = 0;
    seen_item_count = 0;
    for (size_t i = 1; i <= N; ++i) {
        if (not R.supports(i)) continue;
        ++seen_item_count;
        Rel::Iterator<RHS_FIXED> iter(i, &R);
        for (iter.begin(); iter.ok(); iter.next()) {
            ++num_pairs;
        }
    }
    POMAGMA_INFO("  Iterated over " << seen_item_count << " items");
    POMAGMA_INFO("  Iterated over " << num_pairs << " pairs");
    R.validate();
    POMAGMA_ASSERT(seen_item_count == item_count,
            "Iterator had incorrect support");
    POMAGMA_ASSERT(num_pairs == true_size, //each pair is seen twice
            "dense_bin_rel Iterated over incorrect number of pairs"
            << ": " << num_pairs << " vs " << true_size);
}

int main ()
{
    Log::title("Running Binary Relation Test");

    test_dense_bin_rel(3 + (1 << 9), br_test1, br_test2);

    return 0;
}

