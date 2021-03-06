#include "dense_sym_fun.hpp"
#include <vector>

using namespace pomagma;

oid_t gcd (oid_t n, oid_t m) { return m ? gcd(m, n % m) : n; }

void test_basic (oid_t size)
{
    POMAGMA_INFO("Defining function");
    dense_sym_fun fun(size);
    for (oid_t i = 1; i <= size; ++i) {
    for (oid_t j = i; j <= size; ++j) {
        oid_t k = gcd(i,j);
        if (k > 1) {
            fun.insert(i, j, k);
        }
    } }

    POMAGMA_INFO("Checking function values");
    std::vector<size_t> line_size(size + 1, 0);
    for (oid_t i = 1; i <= size; ++i) {
    for (oid_t j = 1; j <= size; ++j) {
        oid_t k = gcd(i,j);
        if (k > 1) {
            POMAGMA_ASSERT(fun.contains(i, j),
                    "function does not contain good pair " << i << ',' << j);
            POMAGMA_ASSERT(fun.get_value(i, j) == k,
                    "bad value at " << i << ',' << j);
            ++line_size[i];
        } else {
            POMAGMA_ASSERT(not fun.contains(i, j),
                    "function contains bad pair " << i << ',' << j);
        }
    } }

    POMAGMA_INFO("Checking line iterators");
    dense_sym_fun::Iterator iter(&fun);
    for (oid_t i = 1; i <= size; ++i) {
        size_t line_size_i = 0;
        for (iter.begin(i); iter.ok(); iter.next()) {
            oid_t j = iter.moving();
            oid_t k = iter.value();
            POMAGMA_ASSERT(k, "null item at " << i << ',' << j);
            POMAGMA_ASSERT(gcd(i, j) == k, "bad value at " << i << ',' << j);
            ++line_size_i;
        }
        POMAGMA_ASSERT_EQ(line_size[i], line_size_i);
    }

    POMAGMA_INFO("Validating");
    fun.validate();
}

int main ()
{
    Log::title("Dense Symmetric Function Test");

    for (size_t i = 0; i < 4; ++i) {
        test_basic(i + (1 << 9));
    }

    for (size_t exponent = 0; exponent < 10; ++exponent) {
        test_basic(1 << exponent);
    }

    return 0;
}
