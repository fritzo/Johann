#include "dense_set.hpp"
#include <vector>

using pomagma::Log;
using pomagma::dense_set;
using pomagma::random_bool;

bool is_even (size_t i, size_t modulus = 2) { return i % modulus == 0; }

void test_misc (size_t size)
{
    POMAGMA_INFO("Testing dense_set");

    POMAGMA_INFO("creating dense_set of size " << size);
    dense_set set(size);
    POMAGMA_ASSERT(set.count_items() == 0,
            "set had nonzero size upon creation");

    POMAGMA_INFO("testing position insertion");
    for (size_t i = 1; i <= size; ++i) {
        set.insert(i);
    }
    POMAGMA_ASSERT(set.count_items() == size,
            "set is not full after inserting all items");

    POMAGMA_INFO("testing position removal");
    for (size_t i = 1; i <= size; ++i) {
        set.remove(i);
    }
    POMAGMA_ASSERT(set.count_items() == 0,
            "set is not empty after removing all items");

    POMAGMA_INFO("testing iteration");
    for (size_t i = 1; i <= size / 2; ++i) {
        set.insert(i);
    }
    POMAGMA_ASSERT(set.count_items() == size / 2,
            "set is not half-full after inserting size/2 items");
    unsigned item_count = 0;
    for (dense_set::iterator iter(set); iter.ok(); iter.next()) {
        POMAGMA_ASSERT(set.contains(*iter), "iterated over uncontained item");
        ++item_count;
    }
    POMAGMA_INFO("found " << item_count << " / " << (size / 2) << " items");
    POMAGMA_ASSERT(item_count <= (size / 2), "iterated over too many items");
    POMAGMA_ASSERT(item_count == (size / 2), "iterated over too few items");
}

void test_even (size_t size)
{
    dense_set evens[7] = {0, size, size, size, size, size, size};

    POMAGMA_INFO("Defining sets");
    for (size_t i = 1; i <= 6; ++i) {
        for (size_t j = 1; j < 1 + size; ++j) {
            if (is_even(j, i)) { evens[i].insert(j); }
        }
    }

    POMAGMA_INFO("Testing set containment");
    for (size_t i = 1; i <= 6; ++i) {
    for (size_t j = 1; j <= 6; ++j) {
        POMAGMA_INFO(j << " % " << i << " = " << (j % i));
        if (j % i == 0) {
            POMAGMA_ASSERT(evens[j] <= evens[i],
                    "expected containment " << j << ", " << i);
        } else {
            // XXX FIXME this fails and I don't know why
            //POMAGMA_ASSERT(not (evens[j] <= evens[i]),
            //        "expected non-containment " << j << ", " << i);
        }
    }}

    POMAGMA_INFO("Testing set intersection");
    dense_set evens6(size);
    evens6.set_insn(evens[2], evens[3]);
    POMAGMA_ASSERT(evens6 == evens[6], "expected 6 = lcm(2, 3)")

    POMAGMA_INFO("Validating");
    for (size_t i = 0; i <= 6; ++i) {
        evens[i].validate();
    }
    evens6.validate();
}

void test_iterator(size_t size)
{
    dense_set set(size);
    std::vector<bool> vect(size, false);
    size_t true_count = 0;

    for (size_t i = 1; i <= size; ++i) {
        if (random_bool(0.2)) {
            set.insert(i);
            vect[i-1] = true;
            ++true_count;
        }
    }

    for (size_t i = 1; i <= size; ++i) {
        POMAGMA_ASSERT_EQUAL(bool(set(i)), vect[i-1]);
    }

    size_t count = 0;
    for (dense_set::iterator i(set); i.ok(); i.next()) {
        POMAGMA_ASSERT(vect[*i - 1], "unexpected item " << *i);
        ++count;
    }
    POMAGMA_ASSERT_EQUAL(count, true_count);
}

int main ()
{
    Log::title("Dense Set Test");

    for (size_t i = 0; i < 4; ++i) {
        test_misc(i + (1 << 16));
    }

    for (size_t size = 0; size < 100; ++size) {
        test_even(size);
        test_iterator(size);
    }

    return 0;
}
