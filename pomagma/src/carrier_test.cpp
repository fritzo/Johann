#include "carrier.hpp"

using namespace pomagma;

void test_random (size_t size)
{
    Carrier carrier(size);
    carrier.validate();

    for (size_t i = 1; i <= size; ++i) {
        POMAGMA_ASSERT_EQUAL(carrier.insert(), i);
    }
    carrier.validate();
}

int main ()
{
    Log::title("Carrier Test");

    POMAGMA_INFO("Testing random carrier");
    for (size_t size = 1; size < 200; ++size) {
        test_random(size);
    }

    return 0;
}
