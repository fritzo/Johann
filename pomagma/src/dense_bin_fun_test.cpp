#include "dense_bin_fun.hpp"
#include <vector>

using namespace pomagma;

inline bool example_fun (oid_t i, oid_t j)
{
    return ((i % j) > (j % i)) * ((i * i + j + 1) % max(i, j));
}

void test_dense_bin_fun (unsigned size)
{
    POMAGMA_INFO("Defining function");
    dense_bin_fun fun(size);
    for (unsigned i = 1; i <= size; ++i) {
    for (unsigned j = 1; j <= size; ++j) {
        unsigned k = example_fun(i, j);
        if (k > 1) {
            fun.insert(i, j, k);
        }
    } }

    POMAGMA_INFO("Checking function values");
    std::vector<unsigned> Lx_line_size(size + 1, 0);
    std::vector<unsigned> Rx_line_size(size + 1, 0);
    for (unsigned i = 1; i <= size; ++i) {
    for (unsigned j = 1; j <= size; ++j) {
        int k = example_fun(i, j);
        if (k > 1) {
            POMAGMA_ASSERT(fun.contains(i, j),
                    "function does not contain good pair " << i << ',' << j);
            POMAGMA_ASSERT(fun.get_value(i, j) == k,
                    "function contains wrong value for " << i << ',' << j);
            ++Lx_line_size[i];
            ++Rx_line_size[j];
        } else {
            POMAGMA_ASSERT(not fun.contains(i, j),
                    "function contains bad pair " << i << ',' << j);
        }
    } }

    POMAGMA_INFO("Checking line Iterators<LHS_FIXED>");
    {
        dense_bin_fun::Iterator<dense_bin_fun::LHS_FIXED> iter(&fun);
        for (unsigned i = 1; i <= size; ++i) {
            unsigned line_size_i = 0;
            for (iter.begin(i); iter.ok(); iter.next()) {
                unsigned j = iter.rhs();
                unsigned k = iter.value();
                POMAGMA_ASSERT(example_fun(i, j) == k and k >= 1,
                        "iterator gave wrong value at " << i << ',' << j);
                ++line_size_i;
            }
            POMAGMA_ASSERT(Lx_line_size[i] == line_size_i,
                    "line sizes disagree: "
                    << Lx_line_size[i] << " != " << line_size_i);
        }
    }

    POMAGMA_INFO("Checking line Iterators<RHS_FIXED>");
    {
        dense_bin_fun::Iterator<dense_bin_fun::RHS_FIXED> iter(&fun);
        for (unsigned j = 1; j <= size; ++j) {
            unsigned line_size_j = 0;
            for (iter.begin(j); iter.ok(); iter.next()) {
                unsigned i = iter.lhs();
                unsigned k = iter.value();
                POMAGMA_ASSERT(example_fun(i, j) == k and k >= 1,
                        "iterator gave wrong value at " << i << ',' << j);
                ++line_size_j;
            }
            POMAGMA_ASSERT(Rx_line_size[j] == line_size_j,
                    "line sizes disagree: "
                    << Rx_line_size[j] << " != " << line_size_j);
        }
    }

    POMAGMA_INFO("Validating");
    fun.validate();
}

int main ()
{
    Log::title("Dense Binary Function Test");

    for (size_t i = 0; i < 4; ++i) {
        test_dense_bin_fun(i + (1 << 9));
    }

    for (size_t exponent = 0; exponent < 10; ++exponent) {
        test_dense_bin_fun(1 << exponent);
    }

    return 0;
}
