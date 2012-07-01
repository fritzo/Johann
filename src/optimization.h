#ifndef JOHANN_OPTIMIZATION_H
#define JOHANN_OPTIMIZATION_H

#include "definitions.h"
#include "moments.h"
#include "linalg.h"
#include <vector>

namespace Optimization
{

const Logging::Logger logger("optim", Logging::DEBUG);

namespace LA = LinAlg;
namespace M = Moments;
using LA::Vect;
using LA::Mat;
using LA::Nbhd1;

//================ objective functions ================
//abstract classes
class FunOf___
{
protected:
    Float m_f, m_df;
    Int m_calls;
public:
    FunOf___ () : m_calls(0) {}
    virtual ~FunOf___ () {}

    //access
    bool isfinite () const { return std::isfinite(m_f); }
    Float value () const { return m_f; }
    Float deriv () const { return m_df; }
    void reset () { m_calls = 0; }
    Int calls () const { return m_calls; }
};
class FunOfReal : public FunOf___
{
public:
    virtual ~FunOfReal () {}

    //calling
protected:
    virtual void eval (Float t) = 0;
    virtual void eval_in_dir (Float t, Float dt) = 0; //value, directional deriv
public:
    Float operator() (Float t) { eval(t); ++m_calls; return m_f; }
    Nbhd1 operator() (Float t, Float dt)
    { eval_in_dir(t,dt); ++m_calls; return Nbhd1(m_f,m_df); }

};
class FunOfVect : public FunOf___
{
protected:
    const Int m_dim;
public:
    FunOfVect (Int dim) : m_dim(dim) {}
    virtual ~FunOfVect () {}

    //calling
protected:
    virtual void eval (const Vect& v) = 0;
    virtual void eval_in_dir (const Vect& v, const Vect& dv) = 0;
public:
    Float operator() (const Vect& v) { eval(v); ++m_calls; return m_f; }
    Nbhd1 operator() (const Vect& v, const Vect& dv)
    { eval_in_dir(v,dv); ++m_calls; return Nbhd1(m_f,m_df); }

    //access
    Int dim () const { return m_dim; }
};
class FunAndGradOfVect : public FunOfVect
{
protected:
    Int m_calls1;
    Vect m_grad;
public:
    FunAndGradOfVect (Int dim)
        : FunOfVect(dim), m_calls1(0), m_grad(dim) {}
    virtual ~FunAndGradOfVect () {}

    //calling
protected:
    virtual void eval_grad (const Vect& v) = 0;
public:
    Float nbhd (const Vect& v) { eval_grad(v); ++m_calls1; return m_f; }

    //access
    Int calls1 () const { return m_calls1; }
    const Vect& grad () const { return m_grad; }
    Float grad (Int i) const
    {
        Assert2(i < m_dim, "index out of bounds");
        return m_grad(i);
    }
};
class FunGradMetricOfVect : public FunAndGradOfVect
{
protected:
    Int m_calls2;
    Mat m_metric;
public:
    FunGradMetricOfVect (Int dim)
        : FunAndGradOfVect(dim), m_calls2(0), m_metric(LA::identity(dim)) {}
    virtual ~FunGradMetricOfVect () {}

    //calling
protected:
    virtual void eval_metric (const Vect& v) = 0;
public:
    Float nbhd2 (const Vect& v) { eval_metric(v); ++m_calls2; return m_f; }

    //access
    Int calls2 () const { return m_calls2; }
    const Vect& metric () const { return m_metric; }
    Float metric (Int i, Int j) const
    {
        Assert2(i < m_dim, "index 1 out of bounds");
        Assert2(j < m_dim, "index 2 out of bounds");
        return m_metric(i,j);
    }
};

//wrapper classes
class CStyle_FunOfReal : public FunOfReal
{
protected:
    Float (*m_fun)(Float);
    Float (*m_deriv)(Float);
public:
    CStyle_FunOfReal (Float (*fun)(Float), Float (*deriv)(Float))
        : m_fun(fun), m_deriv(deriv) {}
    virtual ~CStyle_FunOfReal () {}
protected:
    virtual void eval (Float t) { m_f = m_fun(t); }
    virtual void eval_in_dir (Float t, Float dt)
    { m_f = m_fun(t); m_df = m_deriv(t) * dt; }
};
class FunOnLine : public FunOfReal
{
    FunOfVect& m_fun;
    Vect m_x;
    const Vect m_x0, m_dx;
public:
    FunOnLine (FunOfVect& fun, const Vect& x0, const Vect& dx)
        : m_fun(fun), m_x(x0), m_x0(x0), m_dx(dx)
    {}
    virtual ~FunOnLine () {}

    //access
protected:
    void eval (Float t);
    void eval_in_dir (Float t, Float dt);
};

//================ 1D methods ================
//one-dimensional root-finding
Float bisect_root (FunOfReal& fun, Float LB, Float UB, Float x_tol);
Float bisect_min  (FunOfReal& fun, Float LB, Float UB, Float x_tol);

//polynomial optimization
inline Float extreme_value(M::Poly2 poly)
{ return -poly.getDeriv1() / poly.getDeriv2(); }

//================ multidimensional methods ================

//constrained pmf
//transform equality-constrained N-D simplex to unconstrained (N-1)-D simplex
class FunOfPMF : public FunGradMetricOfVect
{
    FunGradMetricOfVect& m_fun;
public:
    FunOfPMF (FunGradMetricOfVect* fun)
        : FunGradMetricOfVect(fun->dim() - 1),
          m_fun(*fun)
    {}
    virtual ~FunOfPMF () { delete &m_fun; }

    //lifting and projection functions
    Vect lift_val    (const Vect&  x) const;
    Vect lift_dir    (const Vect& dx) const;
    Vect proj_val    (const Vect&  p) const;
    Vect proj_grad   (const Vect& dp) const;
    Mat  proj_metric (const Mat& ddp) const;

protected:
    virtual void eval (const Vect& x);
    virtual void eval_in_dir (const Vect& x, const Vect& dx);
    virtual void eval_grad (const Vect& x);
    virtual void eval_metric (const Vect& x);
};

/** Nonlinear optimization
 *
 * Arguments:
 *   x on input  - initial guess
 *   x on output - final solution
 *
 */
enum Status { CONVERGE, DIVERGE, BARRIER, NOT_FINITE };
Status cg_over_simplex (
        Vect& x, FunAndGradOfVect& fun,
        Float x_tol=1e-6f, Float f_tol=1e-6f, Int max_steps=16);
Status newton_over_simplex (
        Vect& x, FunGradMetricOfVect& fun,
        Float x_tol=1e-6f, Float f_tol=1e-6f, Int max_steps=16);

}

#endif

