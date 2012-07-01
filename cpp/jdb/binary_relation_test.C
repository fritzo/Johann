
#include <definitions.h>
#include <jdb/dense_bin_rel.h>
#include <utility>

Logging::Logger logger("test");

unsigned g_num_moved(0);
void move_to (int i, int j)
{
    //cout << i << '-' << j << ' ' << std::flush; //DEBUG
    ++g_num_moved;
}

void test_dense_set (Int N)
{
    logger.info() << "Testing dense_set" |0;
    Logging::IndentBlock block;

    //========================================================================
    logger.info() << "creating dense_set of size " << N |0;
    typedef nonstd::dense_set Set;
    Set S(N);
    Assert (S.size() == 0, "set had nonzero size upon creation");

    //========================================================================
    logger.info() << "testing position insertion" |0;
    for (Int i=1; i<=N; ++i) S.insert(i);
    Assert (S.size() == N, "set is not full after inserting all items");

    //========================================================================
    logger.info() << "testing position removal" |0;
    for (Int i=1; i<=N; ++i) S.remove(i);
    Assert (S.size() == 0, "set is not empty after removing all items");

    //========================================================================
    logger.info() << "testing iteration" |0;
    for (Int i=1; i<=N/2; ++i) S.insert(i);
    Assert (S.size() == N/2, "set is not half-full after inserting N/2 items");
    unsigned num_items = 0;
    for (Set::iterator iter=S.begin(); iter; iter.next()) {
        Assert(S.contains(*iter), "iterated over uncontained item");
        ++num_items;
    }
    logger.info() << "found " << num_items << " / " << N/2 << " items" |0;
    Assert (num_items <= N/2, "iterated over too many items");
    Assert (num_items == N/2, "iterated over too few items");
}

bool br_test1 (int i, int j) { return i and j and i%61 <= j%31; }
bool br_test2 (int i, int j) { return i and j and i%61 == j%31; }

typedef nonstd::dense_bin_rel Rel;
enum Direction { LHS_FIXED=true, RHS_FIXED=false };

void test_dense_bin_rel (Int N, bool test1(int,int), bool test2(int,int))
{
    logger.info() << "Testing dense_bin_rel" |0;
    Logging::IndentBlock block;

    logger.info() << "creating dense_bin_rel of size " << N |0;
    Rel R(N);

    //========================================================================
    logger.info() << "testing position insertion" |0;
    unsigned num_items=0;
    for (Int i=1; i<=N; ++i) {
        R.insert(i);
        ++num_items;
    }
    Assert (num_items == R.sup_size(), "incorrect support size");

    //========================================================================
    logger.info() << "testing pair insertion" |0;
    unsigned num_pairs = 0;
    for (Int i=1; i<=N; ++i) {
    for (Int j=1; j<=N; ++j) {
        if (test1(i,j)) {
            R.insert(i,j);
            ++num_pairs;
        }
    } }
    logger.info() << "  " << num_pairs << " pairs inserted" |0;
    R.validate();
    Assert (num_pairs == R.size(),
            "dense_bin_rel contained incorrect number of pairs");

    //========================================================================
    logger.info() << "testing pair removal" |0;
    for (Int i=1; i<=N; ++i) {
    for (Int j=1; j<=N; ++j) {
        if (test1(i,j) and test2(i,j)) {
            R.remove(i,j);
            --num_pairs;
        }
    } }
    logger.info() << "  " << num_pairs << " pairs remain" |0;
    R.validate();
    Assert (num_pairs == R.size(),
            "dense_bin_rel contained incorrect number of pairs");

    //========================================================================
    logger.info() << "testing table iterator" |0;
    unsigned num_pairs_seen = 0;
    for (Rel::iterator iter(&R); iter; iter.next()) {
        ++num_pairs_seen;
    }
    logger.info() << "  iterated over "
        << num_pairs_seen << " / " << num_pairs << " pairs" |0;
    R.validate();
    Assert (num_pairs_seen == num_pairs,
            "dense_bin_rel iterated over incorrect number of pairs");

    //========================================================================
    logger.info() << "testing pair containment" |0;
    num_pairs = 0;
    for (Int i=1; i<=N; ++i) {
    for (Int j=1; j<=N; ++j) {
        if (test1(i,j) and not test2(i,j)) {
            Assert (R.contains_Lx(i,j),
                    "Lx relation doesn't contain what it should");
            Assert (R.contains_Rx(i,j),
                    "Rx relation doesn't contain what it should");
            ++num_pairs;
        } else {
            Assert (not R.contains_Lx(i,j),
                    "Lx relation contains what it shouldn't");
            Assert (not R.contains_Rx(i,j),
                    "Rx relation contains what it shouldn't");
        }
    } }
    logger.info() << "  " << num_pairs << " pairs found" |0;
    R.validate();
    Assert (num_pairs == R.size(),
            "dense_bin_rel contained incorrect number of pairs");

    //========================================================================
    logger.info() << "testing position merging" |0;
    for (Int i=1; i<=N/3; ++i) {
        Int m=(2*i)%N, n=(2*(N-i-1)+1)%N;
        if (not (R.supports(m,n) and R.contains(m,n))) continue;
        if (m == n) continue;
        if (m < n) swap(m,n);
        R.merge(m,n, move_to);
        --num_items;
    }
    logger.info() << "  " << g_num_moved << " pairs moved in merging" |0;
    R.validate();
    Assert (num_items == R.sup_size(), "incorrect support size");

    //========================================================================
    logger.info() << "testing table iterator again" |0;
    num_pairs_seen = 0;
    for (Rel::iterator iter(&R); iter; iter.next()) {
        ++num_pairs_seen;
    }
    num_pairs = R.size();
    logger.info() << "  iterated over "
        << num_pairs_seen << " / " << num_pairs << " pairs" |0;
    R.validate();
    Assert (num_pairs_seen == num_pairs,
            "dense_bin_rel iterated over incorrect number of pairs");

    //========================================================================
    logger.info() << "testing line Iterator<LHS_FIXED>" |0;
    num_pairs = 0;
    unsigned num_items_seen = 0;
    num_items = R.sup_size();
    for (Int i=1; i<=N; ++i) {
        if (not R.supports(i)) continue;
        ++num_items_seen;
        Rel::Iterator<LHS_FIXED> iter(i, &R);
        for (iter.begin(); iter; iter.next()) {
            ++num_pairs;
        }
    }
    logger.info() << "  Iterated over " << num_items_seen << " items" |0;
    logger.info() << "  Iterated over " << num_pairs << " pairs" |0;
    R.validate();
    Assert (num_items_seen == num_items, "Iterator had incorrect support");
    unsigned true_size = R.size();
    Assert (num_pairs == true_size, //each pair is seen twice
            "dense_bin_rel Iterated over incorrect number of pairs"
            << ": " << num_pairs << " vs " << true_size);

    //========================================================================
    logger.info() << "testing line Iterator<RHS_FIXED>" |0;
    num_pairs = 0;
    num_items_seen = 0;
    for (Int i=1; i<=N; ++i) {
        if (not R.supports(i)) continue;
        ++num_items_seen;
        Rel::Iterator<RHS_FIXED> iter(i, &R);
        for (iter.begin(); iter; iter.next()) {
            ++num_pairs;
        }
    }
    logger.info() << "  Iterated over " << num_items_seen << " items" |0;
    logger.info() << "  Iterated over " << num_pairs << " pairs" |0;
    R.validate();
    Assert (num_items_seen == num_items, "Iterator had incorrect support");
    Assert (num_pairs == true_size, //each pair is seen twice
            "dense_bin_rel Iterated over incorrect number of pairs"
            << ": " << num_pairs << " vs " << true_size);
}

int main ()
{
    Logging::switch_to_log("test.log");
    Logging::title("Running Binary Relation Test");

    test_dense_set(3 + (1<<16));
    test_dense_bin_rel(3 + (1<<9), br_test1, br_test2);

    return 0;
}

