#include "util.hpp"
#include "inverse_bin_fun.hpp"
#include "dense_bin_fun.hpp"

using namespace pomagma;

void test_random (size_t size, float fill = 0.3)
{
    POMAGMA_INFO("Buiding fun,inv of size " << size);
    dense_bin_fun fun(size);
    inverse_bin_fun inv(size);

    POMAGMA_INFO("testing insertion");
    size_t insert_count = size * size * fill;
    for (size_t n = 0; n < insert_count; ++n) {
        oid_t lhs;
        oid_t rhs;
        do {
            lhs = random_int(1, size);
            rhs = random_int(1, size);
        } while (fun.contains(lhs, rhs));
        oid_t val = random_int(1, size);

        fun.insert(lhs, rhs, val);
        inv.insert(lhs, rhs, val);
    }

    fun.validate();
    inv.validate(fun);
}

int main ()
{
    test_random(1 << 9);

    // TODO test with multiple threads

    return 0;
}
