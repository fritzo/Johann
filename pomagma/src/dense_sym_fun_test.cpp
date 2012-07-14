#include "dense_sym_fun.hpp"
#include <vector>

using pomagma::Log;
using pomagma::dense_sym_fun;

unsigned gcd (unsigned n, unsigned m) { return m ? gcd(m, n % m) : n; }

void test_dense_sym_fun (unsigned size)
{
    POMAGMA_INFO("Defining function");
    dense_sym_fun fun(size);
    for (unsigned i = 1; i <= size; ++i) {
    for (unsigned j = i; j <= size; ++j) {
        unsigned k = gcd(i,j);
        if (k > 1) {
            fun.insert(i, j, k);
        }
    } }

    POMAGMA_INFO("Checking function values");
    std::vector<unsigned> line_size(size + 1, 0);
    for (unsigned i = 1; i <= size; ++i) {
    for (unsigned j = 1; j <= size; ++j) {
        int k = gcd(i,j);
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
    for (unsigned i = 1; i <= size; ++i) {
        unsigned line_size_i = 0;
        for (iter.begin(i); iter.ok(); iter.next()) {
            unsigned j = iter.moving();
            unsigned k = iter.value();
            POMAGMA_ASSERT(k, "null item at " << i << ',' << j);
            POMAGMA_ASSERT(gcd(i, j) == k, "bad value at " << i << ',' << j);
            ++line_size_i;
        }
        POMAGMA_ASSERT_EQUAL(line_size[i], line_size_i);
    }

    POMAGMA_INFO("Validating");
    fun.validate();
}

int main ()
{
    Log::title("Dense Symmetric Function Test");

    for (size_t i = 0; i < 4; ++i) {
        test_dense_sym_fun(i + (1 << 9));
    }

    for (size_t exponent = 0; exponent < 10; ++exponent) {
        test_dense_sym_fun(1 << exponent);
    }

    return 0;
}
