
#include "definitions.h"
#include "moments.h"
#include <cstdlib>

using namespace std;
using Moments::Poly2;
using Moments::FiniteDifference;

Logging::Logger logger("test");

//partially symbolic testing types
struct Quadratic
{
    Float m_x0, m_x1, m_x2;
    Quadratic () : m_x0(1.0f+drand48()),
                   m_x1(2.0f*drand48() + 1.0f),
                   m_x2(2.0f*drand48() + 1.0f)
    {}
    inline Float operator () (Float t) { return m_x0 + t*(m_x1 + t*m_x2); }
};
template<class LHS, class RHS>
struct Sum
{
    LHS m_lhs;
    RHS m_rhs;
    Sum (LHS lhs, RHS rhs) : m_lhs(lhs), m_rhs(rhs) {}
    inline Float operator () (Float t) { return m_lhs(t) + m_rhs(t); }
};
template<class LHS, class RHS>
struct Difference
{
    LHS m_lhs;
    RHS m_rhs;
    Difference (LHS lhs, RHS rhs) : m_lhs(lhs), m_rhs(rhs) {}
    inline Float operator () (Float t) { return m_lhs(t) - m_rhs(t); }
};
template<class LHS, class RHS>
struct Product
{
    LHS m_lhs;
    RHS m_rhs;
    Product (LHS lhs, RHS rhs) : m_lhs(lhs), m_rhs(rhs) {}
    inline Float operator () (Float t) { return m_lhs(t) * m_rhs(t); }
};
template<class LHS, class RHS>
struct Quotient
{
    LHS m_lhs;
    RHS m_rhs;
    Quotient (LHS lhs, RHS rhs) : m_lhs(lhs), m_rhs(rhs) {}
    inline Float operator () (Float t) { return m_lhs(t) / m_rhs(t); }
};
template<class RHS>
struct Composition
{
    Float (*m_lhs)(Float);
    RHS m_rhs;
    Composition (Float (*lhs)(Float), RHS rhs) : m_lhs(lhs), m_rhs(rhs) {}
    inline Float operator () (Float t) { return m_lhs(m_rhs(t)); }
};

template<class LHS, class RHS> Sum<LHS,RHS> sum(LHS lhs, RHS rhs)
{ return Sum<LHS,RHS>(lhs, rhs); }
template<class LHS, class RHS> Difference<LHS,RHS> difference(LHS lhs, RHS rhs)
{ return Difference<LHS,RHS>(lhs, rhs); }
template<class LHS, class RHS> Product<LHS,RHS> product(LHS lhs, RHS rhs)
{ return Product<LHS,RHS>(lhs, rhs); }
template<class LHS, class RHS> Quotient<LHS,RHS> quotient(LHS lhs, RHS rhs)
{ return Quotient<LHS,RHS>(lhs, rhs); }
template<class LHS, class RHS> Composition<RHS> compose(LHS lhs, RHS rhs)
{ return Composition<RHS>(lhs, rhs); }

const int test_cases = 100;

void bound_error(Float error, Float max_error=1e-2)
{
    if (error > max_error) {
      logger.warning() << "large error: MSE = " << error |0;
    }
}

//testing functions
void test_sum ()
{
    logger.info() << "Testing sum operation" |0;
    Logging::IndentBlock block;

    Float error2 = 0;
    for (int i=0; i<test_cases; ++i) {
        Quadratic x,y;
        error2 +=
        (
            FiniteDifference(x) + FiniteDifference(y)
            -FiniteDifference(sum(x,y))
        ).norm2();
    }
    bound_error(sqrt(error2/test_cases));
}
void test_difference ()
{
    logger.info() << "Testing difference operation" |0;
    Logging::IndentBlock block;

    Float error2 = 0;
    for (int i=0; i<test_cases; ++i) {
        Quadratic x,y;
        error2 +=
        (
            FiniteDifference(x) - FiniteDifference(y)
            -FiniteDifference(difference(x,y))
        ).norm2();
    }
    bound_error(sqrt(error2/test_cases));
}
void test_product ()
{
    logger.info() << "Testing product operation" |0;
    Logging::IndentBlock block;

    Float error2 = 0;
    for (int i=0; i<test_cases; ++i) {
        Quadratic x,y;
        error2 +=
        (
            FiniteDifference(x) * FiniteDifference(y)
            -FiniteDifference(product(x,y))
        ).norm2();
    }
    bound_error(sqrt(error2/test_cases));
}
void test_quotient ()
{
    logger.info() << "Testing quotient operation" |0;
    Logging::IndentBlock block;

    Float error2 = 0;
    for (int i=0; i<test_cases; ++i) {
        Quadratic x,y;
        error2 +=
        (
            FiniteDifference(x) / FiniteDifference(y)
            -FiniteDifference(quotient(x,y))
        ).norm2();
    }
    bound_error(sqrt(error2/test_cases));
}
void test_exp ()
{
    logger.info() << "Testing exp function" |0;
    Logging::IndentBlock block;

    Float error2 = 0;
    for (int i=0; i<test_cases; ++i) {
        Quadratic x;
        Float (*Exp)(Float) = exp;
        error2 +=
        (
            Moments::exp(FiniteDifference(x))
            -FiniteDifference(compose(Exp,x))
        ).norm2();
    }
    bound_error(sqrt(error2/test_cases));
}
void test_log ()
{
    logger.info() << "Testing log function" |0;
    Logging::IndentBlock block;

    Float error2 = 0;
    for (int i=0; i<test_cases; ++i) {
        Quadratic x;
        Float (*Log)(Float) = log;
        error2 +=
        (
            Moments::log(FiniteDifference(x))
            -FiniteDifference(compose(Log,x))
        ).norm2();
    }
    bound_error(sqrt(error2/test_cases));
}

int main ()
{
    Logging::switch_to_log("test.log");
    Logging::title("Running Moments Test");

    test_sum();
    test_difference();
    test_product();
    test_quotient();
    test_exp();
    test_log();

    return 0;
}

