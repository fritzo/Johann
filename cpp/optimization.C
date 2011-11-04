
#include "optimization.h"
#include <cmath>

//log levels
#define LOG_DEBUG1(mess)
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << mess |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

namespace Optimization
{

using LinAlg::dot;

//objective functions
void FunOnLine::eval (Float t)
{
    for (Int i=0; i<m_fun.dim(); ++i) {
        m_x(i) = m_x0(i) + t * m_dx(i);
    }
    m_f = m_fun(m_x);
}
void FunOnLine::eval_in_dir (Float t, Float dt)
{
    for (Int i=0; i<m_fun.dim(); ++i) {
        m_x(i) = m_x0(i) + t * m_dx(i);
    }
    Nbhd1 nbhd = m_fun(m_x, m_dx);
    m_f  = nbhd.val;
    m_df = nbhd.deriv * dt;
}

//line bisection optimizers
inline Float arith_mean (Float a, Float b) { return 0.5 * (a + b); }
inline Float geom_mean (Float a, Float b, Float tol)
{
    return sqrtf((a + tol) * (b + tol)) - tol;
}
Float arith_bisect_min (FunOfReal& fun, Float x_tol, Float LB=0, Float UB=1)
{//treat all time values equally: [   |   |   |   +   |   |   |   ]
    while (fabsf(UB - LB) > x_tol) {
        Float x = arith_mean(LB,UB);
        Float df = fun(x,1.0f).deriv;
        if (df == 0.0f) return x;
        if (df < 0) LB = x;
        else        UB = x;
    }
    return arith_mean(LB,UB);
}
Float geom_bisect_min (FunOfReal& fun, Float x_tol, Float LB=0, Float UB=1)
{//favor small time values: [|| |  +   |       |          |             ]
    while (fabsf(UB - LB) > x_tol) {
        Float x = geom_mean(LB,UB,x_tol);
        Float df = fun(x,1.0f).deriv;
        if (df == 0.0f) return x;
        if (df < 0) LB = x;
        else        UB = x;
    }
    return geom_mean(LB,UB,x_tol);
}

//================ nonlinear optimization ================

//constrained pmf
/** Translating between constrained and unconstrained simplices
 *
 * p = [1-x.total, x(0), x(1), ... , x(x.dim-1)]
 * x = [p(1), p(2), ... , p(x.dim)]
 *
 * dp = [1-dx.total, dx(0), dx(1), ... , dx(x.dim)]
 * dx =  [dp(1), dp(2), ... , dp(x.dim)]  -  dp(0)
 */
Vect FunOfPMF::lift_val (const Vect& x) const
{
    Assert (x.dim() == m_dim, "lifting value of wrong dimension");
    Vect p(x.dim() + 1);

    for (unsigned i=0; i<x.dim(); ++i) {
        p(0) -= (p(i+1) = x(i));
    }
    p(0) += 1.0;

    return p;
}
Vect FunOfPMF::lift_dir (const Vect& dx) const
{
    Assert (dx.dim() == m_dim, "lifting direction of wrong dimension");
    Vect dp(dx.dim() + 1);

    for (unsigned i=0; i<dx.dim(); ++i) {
        dp(0) -= (dp(i+1) = dx(i));
    }

    return dp;
}
Vect FunOfPMF::proj_val (const Vect& p) const
{
    Assert (p.dim() == m_dim + 1, "projecting value of wrong dimension");
    Vect x(p.dim() - 1);

    for (unsigned i=0; i<x.dim(); ++i) {
        x(i) = p(i+1);
    }

    return x;
}
Vect FunOfPMF::proj_grad (const Vect& dp) const
{
    Assert (dp.dim() == m_dim + 1, "projecting gradient of wrong dimension");
    Vect dx(dp.dim() - 1);

    for (unsigned i=0; i<dx.dim(); ++i) {
        dx(i) = dp(i+1) - dp(0);
    }

    return dx;
}
Mat FunOfPMF::proj_metric (const Mat& ddp) const
{
    Assert (ddp.dim() == m_dim + 1, "projecting metric of wrong dimension");
    Vect ddx(ddp.dim() - 1, 2);

    for (unsigned i=0; i<ddx.dim(); ++i) {
    for (unsigned j=0; j<ddx.dim(); ++j) {
        ddx(i,j) = ddp(i+1,j+1)
                 - ddp(i+1, 0 )
                 - ddp( 0 ,j+1)
                 + ddp( 0 , 0 );
    } }

    return ddx;
}
void FunOfPMF::eval (const Vect& x)
{
    //lift
    Vect p = lift_val(x);

    //call
     m_fun(p);

    //project
    m_f = m_fun.value();
}
void FunOfPMF::eval_in_dir (const Vect& x, const Vect& dx)
{
    //lift
    Vect  p = lift_val( x);
    Vect dp = lift_dir(dx);

    //call
    m_fun(p,dp);

    //project
    m_f  = m_fun.value();
    m_df = m_fun.deriv();
}
void FunOfPMF::eval_grad (const Vect& x)
{
    //lift
    Vect p = lift_val(x);

    //call
    m_fun.nbhd(p);

    //project
    m_f    = m_fun.value();
    m_grad = proj_grad(m_fun.grad());
}
void FunOfPMF::eval_metric (const Vect& x)
{
    //lift
    Vect p = lift_val(x);

    //call
    m_fun.nbhd2(p);

    //project
    m_f      = m_fun.value();
    m_grad   = proj_grad(m_fun.grad());
    m_metric = proj_metric(m_fun.metric());
}

/** Conjugators, for conjugate gradient descent
 * These can both be defined on arbitrary manifolds,
 *   when the inverse metric can be specified.
 */

class Conjugator
{
    typedef Conjugator MyType;
protected:
    const Int m_dim;
    Int m_initialized;
    Conjugator (const MyType&) : m_dim(0)
    { logger.error() << "invalid copy" |0; }
public:
    Conjugator (Int dim) : m_dim(dim), m_initialized(0) {}
    virtual ~Conjugator () {}

    void clear() { m_initialized = 0; }

    //conjugation with and without the inverse metric
    virtual void operator() (Vect& dx) = 0;
    virtual void operator() (Vect& dx, const Mat& Mi) = 0;
};
class GradientDescent : public Conjugator
{//trivial: negate the gradient
public:
    GradientDescent (Int dim) : Conjugator(dim) {}
    virtual ~GradientDescent () {}

    virtual void operator() (Vect& dx) { dx *= -1.0f; };
    virtual void operator() (Vect& dx, const Mat& Mi) { dx *= -1.0f; };
};
class FletcherReeves : public Conjugator
{//original CG technique
    typedef FletcherReeves MyType;

    Float m_old_norm2;
    Vect  m_old_ngrad; //negative gradient
    Vect  m_old_step;

    FletcherReeves (const MyType& other)
        : Conjugator (other), m_old_ngrad(0), m_old_step(0)
    { logger.error() << "invalid copy" |0; }
public:
    FletcherReeves (Int dim)
        : Conjugator(dim),
          m_old_ngrad(dim),
          m_old_step(dim)
    {}
    virtual ~FletcherReeves () {}

    virtual void operator() (Vect& dx);
    virtual void operator() (Vect& dx, const Mat& Mi);
};
class PolakRibiere : public Conjugator
{//improvement apon F-R
    typedef PolakRibiere MyType;

    Float m_old_norm2;
    Vect  m_old_ngrad; //negative gradient
    Vect  m_old_step;

    PolakRibiere (const MyType& other)
        : Conjugator(other), m_old_ngrad(0), m_old_step(0)
    { logger.error() << "invalid copy" |0; }
public:
    PolakRibiere (Int dim)
        : Conjugator(dim),
          m_old_ngrad(dim),
          m_old_step(dim)
    {}
    virtual ~PolakRibiere () {}

    virtual void operator() (Vect& dx);
    virtual void operator() (Vect& dx, const Mat& Mi);
};
//in Euclidean space
void FletcherReeves::operator() (Vect& dx)
{//policy:
//  g^t = -grad f(x^t)                  #gradient descent
//  h^(t+1) = g^(t+1) - gamma^t h^t     #corrected step,    "step"
//            < g^(t+1) | g^(t+1) >
//  gamma^t = ---------------------     #F-R update factor, "factor"
//                < g^t | g^t >
    dx *= -1.0f;
    if (m_initialized) {
        Float old_norm2 = m_old_ngrad.norm2();  // |g^t|^2
        m_old_ngrad = dx;
        Float new_norm2 = dx.norm2();           // |g^(t+1)|^2
        Float factor = new_norm2 / old_norm2;   // gamma = |...|^2 / |...|^2
        m_old_step *= factor;                   // gamma^t h^t
        dx -= m_old_step;                       // g^(t+1) - gamma^t h^t
    }
    else { //just reverse the gradient
        m_old_ngrad = dx;
        m_initialized = true;
    }
    m_old_step = dx;
}
void FletcherReeves::operator() (Vect& dx, const Mat& Mi)
{//policy:
//  g^t = -grad f(x^t)                    #gradient descent
//  h^(t+1) = g^(t+1) - gamma^t h^t       #corrected step,    "step"
//            < g^(t+1) | Mi | g^(t+1) >
//  gamma^t = --------------------------  #F-R update factor, "factor"
//                < g^t | Mi | g^t >
    dx *= -1.0f;
    if (m_initialized) {
        Float old_norm2 = m_old_ngrad.norm2(Mi);// |g^t|^2
        m_old_ngrad = dx;
        Float new_norm2 = dx.norm2(Mi);         // |g^(t+1)|^2
        Float factor = new_norm2 / old_norm2;   // gamma = |...|^2 / |...|^2
        m_old_step *= factor;                   // gamma^t h^t
        dx -= m_old_step;                       // g^(t+1) - gamma^t h^t
    }
    else { //just reverse the gradient
        m_old_ngrad = dx;
        m_initialized = true;
    }
    m_old_step = dx;
}
//in a non-isometric space
void PolakRibiere::operator() (Vect& dx)
{//policy:
//  g^t = -grad f(x^t)                  #gradient descent,  "ngrad"
//  h^(t+1) = g^(t+1) - gamma^t h^t     #corrected step,    "step"
//where
//            < g^(t+1)-g^t | g^(t+1) >
//  gamma^t = ------------------------- #P-R update factor, "factor"
//                    < g^t | g^t >
    dx *= -1.0f;                            // g^(t+1)
    if (m_initialized) {
        Float denom = m_old_ngrad.norm2();  // < g^t | g^t >
        m_old_ngrad -= dx;                  // g^t - g^(t+1)
        Float numer = -dot(m_old_ngrad,dx); // < g^(t+1)-g^t | g^(t+1) >
        m_old_ngrad = dx;
        Float factor = numer / denom;   // gamma = <...> / <...>
        m_old_step *= factor;           // gamma * h^t
        dx -= m_old_step;               // h^(t+1) = g^(t+1) - gamma * h^t
    }
    else { //just reverse the gradient
        m_old_ngrad = dx;
        m_initialized = true;
    }
    m_old_step = dx;
}
void PolakRibiere::operator() (Vect& dx, const Mat& Mi)
{//policy:
//  g^t = -grad f(x^t)                  #gradient descent,  "ngrad"
//  h^(t+1) = g^(t+1) - gamma^t h^t     #corrected step,    "step"
//where
//            < g^(t+1)-g^t | Mi | g^(t+1) >
//  gamma^t = ------------------------------ #P-R update factor, "factor"
//                    < g^t | Mi | g^t >
    dx *= -1.0f;
    if (m_initialized) {
        Float denom = m_old_ngrad.norm2(Mi);    // < g^t | Mi | g^t >
        m_old_ngrad -= dx;                      // g^t - g^(t+1)
        Float numer = -dot(m_old_ngrad,Mi,dx);  // <g^(t+1)-g^t | Mi | g^(t+1)>
        m_old_ngrad = dx;
        Float factor = numer / denom;   // gamma = <...> / <...>
        m_old_step *= factor;           // gamma * h^t
        dx -= m_old_step;               // h^(t+1) = g^(t+1) - gamma * h^t
    }
    else { //just reverse the gradient
        m_old_ngrad = dx;
        m_initialized = true;
    }
    m_old_step = dx;
}

//constraints
void max_step_simplex (const Vect& x, Vect& dx)
{//keep_away:(0,1]
    //rescale so that conartaints are violated at time t = 1.0
    Int dim = x.size();

    //compute the extra coordinate
    Float e = 1.0, de = 0;
    for (Int i=0; i<dim; ++i) {
        e  -=  x(i);
        de -= dx(i);
    }

    //compute scale factor t
    Float t = INFINITY;
    for (Int i=0; i<dim; ++i) {
        if (fabsf(t * dx(i)) > x(i)) { //otherwise dx(i) is small
            Float ti = -x(i) / dx(i);
            if (0 < ti and ti < t) t = ti;
        }
    }
    if (fabsf(t * de) > e) { //otherwise de is small
        Float te = -e / de;
        if (0 < te and te < t) t = te;
    }

    //rescale by t
    logger.debug() << "scaling by " << t << " to stay inside simplex" |0;
    for (Int i=0; i<dim; ++i) dx(i) *= t;
}
void assert_in_simplex (const Vect& x)
{
    Float eps = 0.0f;
    for (Int i=0; i<x.size(); ++i) {
        Assert(x(i) > 0, "out of simplex: x[" << i << "] = "
                << x(i) << " <= 0");
        eps += x(i);
    }
    Assert (eps < 1, "out of simplex: eps = " << eps << " >= 1");
}
Float dist_to_barrier_simplex (const Vect& x)
{
    Float t = INFINITY;
    Float eps = 1.0f;
    for (Int i=0; i<x.size(); ++i) {
        t = min(t,x(i));
        eps -= x(i);
    }
    t = min(t,eps);
    return t;
}

//minimizers
//XXX TODO: refactor these
Status line_search_in_simplex (Vect& x, Vect& dx, FunOfVect& fun,
                              Float x_tol, Float f_tol)
{
    logger.debug() << "Line searching interval to tolerance " << x_tol |0;
    Logging::IndentBlock block;

    //find interval size
    dx.normalize();
    max_step_simplex(x,dx);     //dx is now the max feasible step size
    dx *= 0.667f;               //let's shrink it a bit to be safe
    Float norm_dx = dx.norm();
    if (std::isfinite(norm_dx)) {
        logger.debug() << "interval size = " << norm_dx |0;
    } else {
        logger.error() << "invalid stepsize (" << norm_dx << "); giving up" |0;
        return NOT_FINITE;
    }

    //construct one-parameter function
    FunOnLine lineFun(fun, x, dx);
    Float df = lineFun(0.0,1.0).deriv;
    logger.debug() << "directional deriv = " << df |0;
    if (fabsf(df) < f_tol) {
        logger.debug() << "directional derivative is negligible; quitting" |0;
        return CONVERGE;
    }
    if (not (df < 0)) {
        logger.error() << "gradient and directional derivative disagree" |0;
        return DIVERGE;
    }

    //linesearch
    Float t_tol = 0.5f * x_tol / norm_dx;
    Float t = geom_bisect_min(lineFun, t_tol);
    dx *= t;  norm_dx *= t;
    x += dx;
    Float d = dist_to_barrier_simplex(x);
    logger.debug() << "t = "         << t
                   << ", |dx| = "    << norm_dx
                   << ", safety = "  << d |0;
    return (d >= x_tol) ? CONVERGE : BARRIER;
}
Status cg_over_simplex (
        Vect& x, FunAndGradOfVect& fun,
        Float x_tol, Float f_tol, Int max_steps)
{
    Int dim = fun.dim();
    logger.info() << "Optimizing via interior CG: dim = " << dim |0;
    Logging::IndentBlock block;

    //initialize
    Float old_f = fun(x), f;
    assert_in_simplex(x);

    //GradientDescent conjugate(dim);
    //FletcherReeves conjugate(dim);
    PolakRibiere conjugate(dim);

    //iterate
    for (unsigned step=0; step<max_steps; ++step) {
        //find conjugated natural gradient direction
        f = fun.nbhd(x); //for value and gradient
        if (not fun.isfinite()) {
            logger.warning() << "bad function value: " << fun.value() |0;
            return NOT_FINITE;
        }
        Vect grad = fun.grad(), dx = grad;
        conjugate(dx);
        AssertW(dot(dx,grad) <= 0, "conjugated in backwards direction");

        //linesearch
        Status status = line_search_in_simplex(x, dx, fun, x_tol, f_tol);
        switch (status) {
            case CONVERGE:  break;
            case BARRIER:   logger.info() << "close to boundary; stopping" |0;
            default:        return status;
        }

        //check for termination
        ++step;
        f = fun(x);
        AssertW(f <= old_f, "objective function did not decrease");
        Float df = f - old_f;  old_f = f;
        if (std::isfinite(df) and df > -f_tol) {
            logger.info() << "f-tolerance met: df = " << df
                          << ", cost = " << f |0;
            return CONVERGE;
        }
        if (dx.norm() < x_tol) {
            logger.info() << "x-tolerance met: |dx| = " << dx.norm()
                          << ", cost = " << f |0;
            return CONVERGE;
        }
    }
    logger.warning() << "too many steps: " << max_steps |0;
    return DIVERGE;
}
Status newton_over_simplex (
        Vect& x, FunGradMetricOfVect& fun,
        Float x_tol, Float f_tol, Int max_steps)
{
    Int dim = fun.dim();
    logger.info() << "Optimizing via Newton's method: dim = " << dim |0;
    Logging::IndentBlock block;

    //initialize
    Float old_f = fun(x), f;
    assert_in_simplex(x);

    //iterate
    for (unsigned step=0; step<max_steps; ++step) {
        //find conjugated natural gradient direction
        f = fun.nbhd2(x); //for value, gradient, and Hessian
        if (not fun.isfinite()) {
            logger.warning() << "bad function value: " << fun.value() |0;
            return NOT_FINITE;
        }
        logger.info() << "cost = " << f |0;
        Vect grad = fun.grad(), dx = grad;
        const Mat& g = fun.metric();
        dx = g.solve(dx);
        AssertW(dot(dx,grad) >= 0, "conjugated in backwards direction");
        if (not (fun(x,dx).deriv > 0)) {
            logger.warning() << "Newton direction is not descending"
                             << ", using -grad instead" |0;
            dx = fun.grad();
        }
        dx *= -1.0f;

        //linesearch
        Status status = line_search_in_simplex(x, dx, fun, x_tol, f_tol);
        switch (status) {
            case CONVERGE:  break;
            case BARRIER:   logger.info() << "close to boundary; stopping" |0;
            default:        return status;
        }

        //check for termination
        ++step;
        f = fun(x);
        AssertW(f < old_f, "objective function did not decrease");
        Float df = f - old_f;  old_f = f;
        if (std::isfinite(df) and df > -f_tol) {
            logger.info() << "f-tolerance met: df = " << df
                          << ", cost = " << f |0;
            return CONVERGE;
        }
        if (dx.norm() < x_tol) {
            logger.info() << "x-tolerance met: |dx| = " << dx.norm()
                          << ", cost = " << f |0;
            return CONVERGE;
        }
    }
    logger.warning() << "too many steps: " << max_steps |0;
    return DIVERGE;
}

}

