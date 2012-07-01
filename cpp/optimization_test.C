
#include "definitions.h"
#include "linalg.h"
#include "optimization.h"
#include <cmath>

const Logging::Logger logger("test");

namespace L = LinAlg;
namespace OP = Optimization;
using OP::Vect;

//================ tests for cg method ================

class RosenbrockFun0 : public OP::FunOfVect
{
public:
    Vect m_x0;
    Vect m_dx;
    Float m_a;
    RosenbrockFun0 (Float x0, Float y0, Float dx=1.0f, Float dy=1.0f,
            Float a=1.0f);
    virtual ~RosenbrockFun0 () {}
protected:
    virtual void eval (const Vect& x);
    virtual void eval_in_dir (const Vect& x, const Vect& dx) { LATER(); }
};
RosenbrockFun0::RosenbrockFun0 (Float x0, Float y0, Float dx, Float dy, Float a)
    : OP::FunOfVect(2), m_x0(2), m_dx(2), m_a(a)
{
    m_x0(0) = x0;
    m_x0(1) = y0;
    m_dx(0) = dx;
    m_dx(1) = dy;
}
void RosenbrockFun0::eval (const Vect& x)
{
    Float u = (x(0) - m_x0(0)) / m_dx(0);
    Float v = (x(1) - m_x0(1)) / m_dx(1);
    m_f = sqr(u) + m_a * sqr(v - sqr(u));
}
class RosenbrockFun1 : public OP::FunAndGradOfVect
{
public:
    const RosenbrockFun0& m_r;
    RosenbrockFun1 (const RosenbrockFun0& r)
        : OP::FunAndGradOfVect(2), m_r(r) {}
    virtual ~RosenbrockFun1 () {}
protected:
    virtual void eval (const Vect& x);
    virtual void eval_in_dir (const Vect& x, const Vect& dx);
    virtual void eval_grad (const Vect& x);
};
void RosenbrockFun1::eval (const Vect& x)
{
    Float u = (x(0) - m_r.m_x0(0)) / m_r.m_dx(0);
    Float v = (x(1) - m_r.m_x0(1)) / m_r.m_dx(1);
    m_f = sqr(u) + m_r.m_a * sqr(v - sqr(u));
}
void RosenbrockFun1::eval_in_dir (const Vect& x, const Vect& dx)
{
    Float u = (x(0) - m_r.m_x0(0)) / m_r.m_dx(0);
    Float v = (x(1) - m_r.m_x0(1)) / m_r.m_dx(1);
    m_f = sqr(u) + m_r.m_a * sqr(v - sqr(u));

    LATER();
}
void RosenbrockFun1::eval_grad (const Vect& x)
{
    Float u = (x(0) - m_r.m_x0(0)) / m_r.m_dx(0);
    Float v = (x(1) - m_r.m_x0(1)) / m_r.m_dx(1);
    m_f = sqr(u) + m_r.m_a * sqr(v - sqr(u));

    Float dudx = 1.0f / m_r.m_dx(0);
    Float dvdx = 1.0f / m_r.m_dx(1);
    m_grad(0) = (2.0f * u - m_r.m_a * 4.0f * u * (v-sqr(u))) * dudx;
    m_grad(1) = m_r.m_a * (2.0f * (v-sqr(u))) * dvdx;
}

void test_cg ()
{
    logger.info() << "Testing cg on Rosenbrock function" |0;
    Logging::IndentBlock block;

    Int dim = 2;
    Vect x_true(dim), x0(dim), x_est(dim);
    x_true(0) = 0.6f;
    x_true(1) = 0.2f;
    RosenbrockFun0 fun0(x_true(0),x_true(1), 0.4,0.2,5.0);
    RosenbrockFun1 fun1(fun0);
    Float x_tol = 1e-7;
    Float f_tol = 1e-6;

    //test a range of values in 2-d simplex
    int I = 10;
    Float dx = 1.0/I;
    for (int i=1; i<I;   ++i) { x0(0) = i * dx;
    for (int j=1; j<I-i; ++j) { x0(1) = j * dx;

        logger.info() << "x0 = " << x0(0) << ", y0 = " << x0(1) |0;
        x_est = x0;
        OP::cg_over_simplex(x_est, fun1, x_tol, f_tol);
        Float f_est = fun0(x_est);

        Float x_err = L::dist(x_est, x_true);
        Float f_err = fabs(f_est);
        logger.info() << "x_est = " << x_est(0)
                      << ", y_est = " << x_est(1)
                      << ", x_err = " << x_err
                      << ", f_err = " << f_err |0;
        Assert (x_err < 2.0*x_tol or f_err < 2.0*f_tol,
                "method did not converge");
    } }

}

int main ()
{
    Logging::switch_to_log("test.log");
    Logging::title("Running Optimization Test");

    test_cg();

    return 0;
}

