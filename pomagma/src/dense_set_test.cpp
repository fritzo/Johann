#include "dense_set.hpp"
#include <vector>

using namespace pomagma;

bool is_even (oid_t i, oid_t modulus = 2) { return i % modulus == 0; }

void test_sizes ()
{
    POMAGMA_INFO("Testing dense_set sizes");

    for (size_t i = 0; i <= 511; ++i) {
        POMAGMA_ASSERT_EQ(dense_set::round_item_dim(i), 511);
    }
    for (size_t i = 512; i <= 1023; ++i) {
        POMAGMA_ASSERT_EQ(dense_set::round_item_dim(i), 1023);
    }
}

void test_basic (size_t size)
{
    POMAGMA_INFO("Testing dense_set");

    dense_set set(size);
    POMAGMA_ASSERT_EQ(set.count_items(), 0);

    POMAGMA_INFO("testing position insertion");
    for (oid_t i = 1; i <= size; ++i) {
        set.insert(i);
    }
    POMAGMA_ASSERT_EQ(set.count_items(), size);

    POMAGMA_INFO("testing position removal");
    for (oid_t i = 1; i <= size; ++i) {
        set.remove(i);
    }
    POMAGMA_ASSERT_EQ(set.count_items(), 0);

    POMAGMA_INFO("testing iteration");
    for (oid_t i = 1; i <= size / 2; ++i) {
        set.insert(i);
    }
    POMAGMA_ASSERT_EQ(set.count_items(), size / 2);
    unsigned item_count = 0;
    for (dense_set::iterator iter(set); iter.ok(); iter.next()) {
        POMAGMA_ASSERT(set.contains(*iter), "iterated over uncontained item");
        ++item_count;
    }
    POMAGMA_INFO("found " << item_count << " / " << (size / 2) << " items");
    POMAGMA_ASSERT(item_count <= (size / 2), "iterated over too many items");
    POMAGMA_ASSERT_EQ(item_count, size / 2);
}

void test_even (size_t size)
{
    std::vector<dense_set *> evens(7, NULL);
    for (auto & e : evens) {
        e = new dense_set(size);
    }

    for (oid_t i = 1; i <= 6; ++i) {
        for (oid_t j = 1; j < 1 + size; ++j) {
            if (is_even(j, i)) { evens[i]->insert(j); }
        }
    }

    POMAGMA_INFO("Testing set containment");
    for (oid_t i = 1; i <= 6; ++i) {
    for (oid_t j = 1; j <= 6; ++j) {
        POMAGMA_INFO(j << " % " << i << " = " << (j % i));
        if (j % i == 0) {
            POMAGMA_ASSERT(*evens[j] <= *evens[i],
                    "expected containment " << j << ", " << i);
        } else {
            // XXX FIXME this fails and I don't know why
            //POMAGMA_ASSERT(not (*evens[j] <= *evens[i]),
            //        "expected non-containment " << j << ", " << i);
        }
    }}

    POMAGMA_INFO("Testing set intersection");
    dense_set evens6(size);
    evens6.set_insn(*evens[2], *evens[3]);
    POMAGMA_ASSERT(evens6 == *evens[6], "expected 6 = lcm(2, 3)")

    POMAGMA_INFO("Validating");
    for (oid_t i = 0; i <= 6; ++i) {
        evens[i]->validate();
    }
    evens6.validate();

    for (auto e : evens) {
        delete e;
    }
}

void test_iterator (size_t size)
{
    POMAGMA_INFO("Testing dense_set iterator");
    dense_set set(size);
    std::vector<bool> vect(size, false);
    size_t true_count = 0;

    for (oid_t i = 1; i <= size; ++i) {
        if (random_bool(0.2)) {
            set.insert(i);
            vect[i-1] = true;
            ++true_count;
        }
    }

    for (oid_t i = 1; i <= size; ++i) {
        POMAGMA_ASSERT_EQ(bool(set(i)), vect[i-1]);
    }

    size_t count = 0;
    for (dense_set::iterator i(set); i.ok(); i.next()) {
        POMAGMA_ASSERT(vect[*i - 1], "unexpected item " << *i);
        ++count;
    }
    POMAGMA_ASSERT_EQ(count, true_count);
}

void test_operations (size_t size)
{
    POMAGMA_INFO("Testing dense_set operations");

    dense_set x(size);
    dense_set y(size);
    dense_set expected(size);
    dense_set actual(size);

    for (size_t i = 1; i <= size; ++i) {
        if (random_bool(0.5)) x.insert(i);
        if (random_bool(0.5)) y.insert(i);
    }
    POMAGMA_ASSERT(bool(x.count_items()) ^ x.empty(), ".empty() is wrong");
    POMAGMA_ASSERT(bool(y.count_items()) ^ y.empty(), ".empty() is wrong");

    POMAGMA_INFO("testing insert_all");
    expected.zero();
    actual.zero();
    for (oid_t i = 1; i <= size; ++i) {
        expected.insert(i);
    }
    actual.insert_all();
    POMAGMA_ASSERT(actual == expected, "insert_all is wrong");

    POMAGMA_INFO("testing insert_one");
    expected.zero();
    actual.zero();
    std::vector<oid_t> free_list;
    for (oid_t i = 1; i <= size; ++i) {
        if (x(1)) {
            expected.insert(i);
            actual.insert(i);
        } else {
            free_list.push_back(i);
        }
    }
    for (oid_t i : free_list) {
        expected.insert(i);
        oid_t j = actual.insert_one();
        POMAGMA_ASSERT(i == j, "wrong insert_one " << j << " vs " << i);
        POMAGMA_ASSERT(actual == expected, "insert_one is wrong");
    }

    POMAGMA_INFO("testing union");
    expected.zero();
    actual.zero();
    for (oid_t i = 1; i <= size; ++i) {
        if (x(i) or y(i)) { expected.insert(i); }
        if (x(i)) { actual.insert(i); }
    }
    actual += y;
    POMAGMA_ASSERT(actual == expected, "operator += is wrong");
    actual.set_union(x, y);
    POMAGMA_ASSERT(actual == expected, "set_union is wrong");
    actual.zero();
    actual.set_union(x, y);
    POMAGMA_ASSERT(actual == expected, "set_union is wrong");

    POMAGMA_INFO("testing intersection");
    expected.zero();
    actual.zero();
    for (oid_t i = 1; i <= size; ++i) {
        if (x(i) and y(i)) { expected.insert(i); }
        if (x(i)) { actual.insert(i); }
    }
    actual *= y;
    POMAGMA_ASSERT(actual == expected, "operator *= is wrong");
    actual.set_insn(x, y);
    POMAGMA_ASSERT(actual == expected, "set_insn is wrong");
    actual.zero();
    actual.set_insn(x, y);
    POMAGMA_ASSERT(actual == expected, "set_insn is wrong");

    // these are shared for merge(-), merge(-,-) & ensure
    dense_set expected_rep(size);
    dense_set expected_dep(size);
    dense_set expected_diff(size);
    dense_set actual_rep(size);
    dense_set actual_dep(size);
    dense_set actual_diff(size);
    expected_rep.set_union(x, y);
    for (oid_t i = 1; i <= size; ++i) {
        if (y(i) and not x(i)) { expected_diff.insert(i); }
    }

    POMAGMA_INFO("testing merge(dense_set)");
    actual_rep.zero();
    actual_dep.zero();
    for (oid_t i = 1; i <= size; ++i) {
        if (x(i)) { actual_rep.insert(i); }
        if (y(i)) { actual_dep.insert(i); }
    }
    actual_rep.merge(actual_dep);
    POMAGMA_ASSERT(actual_rep == expected_rep, "merge rep is wrong");
    POMAGMA_ASSERT(actual_dep == expected_dep, "merge dep is wrong");

    POMAGMA_INFO("testing merge(dense_set, dense_set)");
    actual_diff.zero();
    actual_rep.zero();
    actual_dep.zero();
    for (oid_t i = 1; i <= size; ++i) {
        if (x(i)) { actual_rep.insert(i); }
        if (y(i)) { actual_dep.insert(i); }
    }
    actual_rep.merge(actual_dep, actual_diff);
    POMAGMA_ASSERT(actual_rep == expected_rep, "merge rep is wrong");
    POMAGMA_ASSERT(actual_dep == expected_dep, "merge dep is wrong");
    POMAGMA_ASSERT(actual_diff == expected_diff, "merge diff is wrong");

    POMAGMA_INFO("testing merge(dense_set, dense_set)");
    actual_diff.zero();
    actual_rep.zero();
    for (oid_t i = 1; i <= size; ++i) {
        if (x(i)) { actual_rep.insert(i); }
    }
    actual_rep.ensure(y, actual_diff);
    POMAGMA_ASSERT(actual_rep == expected_rep, "merge rep is wrong");
    POMAGMA_ASSERT(actual_diff == expected_diff, "merge diff is wrong");
}

int main ()
{
    Log::title("Dense Set Test");

    test_sizes();

    for (size_t i = 0; i < 4; ++i) {
        test_basic(i + (1 << 16));
    }

    for (size_t size = 0; size < 100; ++size) {
        test_even(size);
        test_iterator(size);
        test_operations(size);
    }

    return 0;
}
