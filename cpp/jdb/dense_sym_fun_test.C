
#include <definitions.h>
#include "dense_sym_fun.h"
#include <vector>

Logging::Logger logger("test");

typedef nonstd::dense_sym_fun Fun;
typedef Fun::Iterator Iter;

//eg: f(x,y) = gcd(x,y)
unsigned gcd (unsigned n,unsigned m) { return m==0 ? n : gcd(m,n%m); }

void test_dense_sym_fun (unsigned N)
{
    logger.info() << "Defining function" |0;
    Fun fun(N);
    for (unsigned i=1; i<=N; ++i) {
    for (unsigned j=i; j<=N; ++j) {
        unsigned k = gcd(i,j);
        if (k <= 1) continue;
        fun.insert(i,j,k);
    } }

    logger.info() << "Checking function values" |0;
    std::vector<unsigned> line_size(N+1);
    for (unsigned i=1; i<=N; ++i) { line_size[i] = 0;
    for (unsigned j=1; j<=N; ++j) {
        int k = gcd(i,j);
        if (k <= 1) {
            Assert (not fun.contains(i,j),
                    "function contains bad pair " << i << ',' << j);
        } else {
            Assert (fun.contains(i,j),
                    "function does not contain good pair " << i << ',' << j);
            Assert (fun.get_value(i,j) == k,
                    "function contains wrong value for " << i << ',' << j);
            ++line_size[i];
        }
    } }

    logger.info() << "Checking line iterators" |0;
    Iter iter(&fun);
    for (unsigned i=1; i<=N; ++i) {
        unsigned line_size_i = 0;
        for (iter.begin(i); iter; iter.next()) {
            unsigned j = iter.moving();
            unsigned k = iter.value();
            Assert (gcd(i,j) == k and k >= 1,
                    "iterator gave wrong function value for " << i << ',' << j);
            ++line_size_i;
        }
        Assert (line_size[i] == line_size_i,
                "line sizes disagree: "
                << line_size[i] << " != " << line_size_i);
    }

    logger.info() << "Validating" |0;
    fun.validate();
}

int main ()
{
    Logging::switch_to_log("test.log");
    Logging::title("Running Dense Symmetric Function Test");

    test_dense_sym_fun(3 + (1<<9));

    return 0;
}

