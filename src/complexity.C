
#include "complexity.h"
#include "optimization.h"
#include "obs.h"
#include "apply.h"
#include "compose.h"
#include "join.h"
#include "order.h"
#include "combinatory_structure.h"
#include "lambda_theories.h"
#include "moments.h"
#include <cmath>
#include <utility>

namespace Complexity
{

//TODO update language optimization for compositions and joins

//namespace O = Obs;
namespace M  = Measures;
namespace LA = LinAlg;
namespace OP = Optimization;
namespace AE = Apply;
namespace OR = Order;
namespace CS = CombinatoryStructure;
namespace LT = LambdaTheories;

using M::comp; using M::komp; using M::rel;

typedef const ObPMF& PMF;

//================ complexity interface ================

//global parameters
Float g_tol = 4e-7;
void set_tolerance (Float tol) { g_tol = tol; }
Float get_tolerance () { return g_tol; }

//global data
//  Z = total, H = entropy, D = divergence (relentropy), R = relevance
//  A_ is portion of _ in enumerated obs
Moments::Z_H g_ZHA(1);
Moments::Z_Z_D g_ZZD(1,1);
Double g_R_term(0);
// geomerty
std::vector<Float> g_metric(0);
std::vector<Float> g_christoffel(0);
std::vector<Float> g_skewness(0);

//access
Float get_ZA() { return g_ZHA.getTotal(); };
Float get_HA() { return g_ZHA.getEntropy(); }
Float get_D () { return g_ZZD.getRelentropy(); }
Float get_R_term  () { return g_R_term; }

//logging
void write_stats_to (ostream& os)
{
    os << "\tP(A) = 1 - " << (1.0 - g_ZHA.total());
    os << ", N(A) = " << exp(g_ZHA.entropy());
    //os << "\n\tP(Bot) = " << comp(*LT::Atoms::Bot);
    //os << "\n\tP(Top) = " << comp(*LT::Atoms::Top);
    os << std::endl;
}

//================ floating-point operations ================

//safety functions
inline void bound_in_01 (Float& t)
{
    if (t <= 0.0) { t=0.0; } else
    if (1.0 <= t) { t=1.0; }
}
inline void bound_in_interval (Double LB, Double UB, Double& t)
{
    if      (t <= LB) { t = LB; }
    else if (UB <= t) { t = UB; }
}
inline Float bounded_in_interval (Float LB, Float UB, Float t)
{
    if (t <= LB) return LB;
    if (t >= UB) return UB;
    return t;
}
inline Float bounded_in_01_smooth (Float t)
{
    Float theta = M_PI * (t - 0.5);
    theta = atan(theta);
    return (theta / M_PI) + 0.5;
}
inline Float portion (Float num, Float denom)
{//safely returns a portion in [0,1]
    if (denom <= 0) return 0.0;
    Float result = num / denom;
    return result > 1 ? 1.0 : result;
}

//normality checking
const Int MAX_ERRORS = 64;
Float check_real (Float x)
{
    const Float DEFAULT_REAL = 0.0;
    static Int errors_remaining = MAX_ERRORS;

    if (not std::isfinite(x)) {
        logger.warning() << "invalid real: " << x |0;
        Assert(--errors_remaining, "too many bad reals");
        return DEFAULT_REAL;
    }
    return x;
}
Float check_nonneg (Float x)
{
    const Float DEFAULT_NONEG = 0.0;
    static Int errors_remaining = MAX_ERRORS;

    if (not std::isfinite(x)) {
        logger.warning() << "bad nonneg: " << x |0;
        Assert(--errors_remaining, "too many bad nonneg's");
        return DEFAULT_NONEG;
    }
    if (x < 0.0) {
        if (x < -g_tol) {
            logger.warning() << "nonneg < 0: " << x |0;
            Assert(--errors_remaining, "too many bad nonneg's");
        }
        return 0.0; //otherwise don't complain
    }
    return x;
}
Float check_prob (Float x)
{
    const Float DEFAULT_PROB = 0.0;
    static Int errors_remaining = MAX_ERRORS;

    if (not std::isfinite(x)) {
        logger.warning() << "bad probability: " << x |0;
        Assert(--errors_remaining, "too many bad x's");
        return DEFAULT_PROB; //default value
    }
    if (x < 0.0) {
        if (x < -g_tol) {
            logger.warning() << "probability < 0: " << x |0;
            Assert(--errors_remaining, "too many bad probabilitites");
        }
        return 0.0; //otherwise don't complain
    }
    if (x > 1.0) {
        if (1.0 + g_tol < x) {
            logger.warning() << "probability > 1: " << x |0;
            Assert(--errors_remaining, "too many bad probabilitites");
        }
        return 1.0; //otherwise don't complain
    }
    return x;
}

//entropy functions
inline Double entropy_term (Double x)
{//entropy
    Assert3(x>=0.0, "negative prob enocuntered");
    if (x == 0.0) return 0.0;
    return - x * std::log(x);
}
inline Double nn_entropy_term (Double x)
{//non-normalized entropy
    Assert3(x>=0.0, "negative probability enocuntered");
    if (x == 0.0) return 1.0;
    return 1.0 - x * std::log(x);
}
inline Double relentropy_term (Double x, Double y)
{//relentropy
    Assert3(x >= 0.0, "negative relentropy term (x) enocuntered");
    Assert3(y >= 0.0, "negative relentropy term (y) enocuntered");
    if (x == 0.0) return 0;
    if (y == 0.0) return INFINITY;
    return x * std::log(x/y);
}
inline Double nn_relentropy_term (Double x, Double y)
{//non-normalized relentropy
    Assert3(x >= 0.0, "negative relentropy term (x) enocuntered");
    Assert3(y >= 0.0, "negative relentropy term (y) enocuntered");
    if (x == 0.0) return y;
    if (y == 0.0) return INFINITY;
    return y + x * (std::log(x/y) - 1.0);
}

//norms
class OneNorm
{
    Double m_sum;
public:
    OneNorm () : m_sum(0) {}
    void add (Float x, Float y) { m_sum += fabs(x-y); }
    void add (Float x) { m_sum += fabs(x); }
    Float total () { return m_sum; }
};
class TwoNorm
{
    Double m_sum;
public:
    TwoNorm () : m_sum(0) {}
    void add (Float x, Float y) { m_sum += sqr(x-y); }
    void sub (Float x, Float y) { m_sum -= sqr(x-y); }
    void add (Float x) { m_sum += sqr(x); }
    void sub (Float x) { m_sum -= sqr(x); }
    Float total () { return sqrt(m_sum); }
};
class MeanSquareNorm
{
    Double m_sum;
    unsigned m_num;
public:
    MeanSquareNorm () : m_sum(0.0), m_num(0) {}
    void add (Float x, Float y) { m_sum += sqr(x-y);  ++m_num; }
    void add (Float x) { m_sum += sqr(x);  ++m_num; }
    Float total () { return sqrt(m_sum / m_num); }
};
class InftyNorm
{
    Float m_max;
public:
    InftyNorm () : m_max(0) {}
    void add (Float x, Float y) { m_max = max(m_max, fabs(x-y)); }
    void add (Float x) { m_max = max(m_max, fabs(x)); }
    Float total () { return m_max; }
};
class InftyScaleNorm
{
    InftyNorm m_diff;
    MeanSquareNorm m_scale;
public:
    void add (Float x, Float y) { m_diff.add(x,y); m_scale.add(x); }
    Float total () { return m_diff.total() / (1.0 + m_scale.total()); }
    Float scale () { return m_scale.total(); }

};
class ProbNorm
{
    Float m_max;
public:
    ProbNorm () : m_max(0) {}
    void add (const Symbols::Prob n, const Symbols::Prob o)
    {
        if (n > 0) imax(m_max, fabs(n-o) / min(n,o));
    }
    Float total () { return m_max; }
};
class IProbNorm
{
    Float m_max;
public:
    IProbNorm () : m_max(0.0) {}
    void add (const Symbols::IProb& n, const Symbols::IProb& o)
    {
        if (n.pos > 0) imax(m_max, fabs(n.pos-o.pos) / min(n.pos, o.pos));
        if (n.neg > 0) imax(m_max, fabs(n.neg-o.neg) / min(n.neg, o.neg));
    }
    Float total () { return m_max; }
};
class RelentropyNorm
{
    Double m_sum;
public:
    RelentropyNorm () : m_sum(0.0) {}
    void infinity () { m_sum = INFINITY; }
    void add (Float x, Float y) { m_sum += nn_relentropy_term(x,y); }
    void sub (Float x, Float y) { m_sum -= nn_relentropy_term(x,y); }
    Float total () { return m_sum; }
};

//logical operations
inline void soft_disjoin (Float& p1, Float p2)
{//disjunction assuming independence: p1 v p2 := 1 - (1-p1)(1-p2)
    p1 = 1.0 - (1.0 - p1) * (1.0 - p2);
}
inline void hard_disjoin (Float& p1, Float p2)
{//disjunction assuming dependence: p1 v p2 := max(p1, p2)
    p1 = max(p1, p2);
}

//======================== iterator shorthand ========================

using AE::Lra_Iterator;
using AE::Rla_Iterator;
using AE::ALr_Iterator;
using OR::LRpos; using OR::RLpos;
using OR::LRneg; using OR::RLneg;

#define iter_over_Obs (Ob::sparse_iterator iter=Ob::sbegin(), end=Ob::send(); iter!=end; ++iter)
#define iter_over_Apps (App::rev_sps_iterator iter=App::rsbegin(), end=App::rsend(); iter!=end; ++iter)
#define iter_over_Comps (Comp::rev_sps_iterator iter=Comp::rsbegin(), end=Comp::rsend(); iter!=end; ++iter)
#define iter_over_Joins (Join::rev_sps_iterator iter=Join::rsbegin(), end=Join::rsend(); iter!=end; ++iter)
#define iter_over_Basis (ObPMF::const_iterator iter=L.get_basis().begin(), end=L.get_basis().end(); iter!=end; ++iter)

#define unsafe_stepsize_loop (Double stepsize = 2.0*g_tol; !(stepsize < g_tol);)
#define stepsize_loop (Double stepsize = 2.0*g_tol, steps=0;\
        (steps < max_steps) and not (stepsize < g_tol);\
        NEXT_STEP(stepsize, steps, __PRETTY_FUNCTION__))
const Int max_steps = 128;
inline void NEXT_STEP (Double stepsize, Double& steps,
                       const char* function_name)
{
    steps += 1;
    if (steps >= max_steps and not stepsize < g_tol) {
        logger.warning() << "iteration only converged to within " << stepsize
            << ", in \n\t" << function_name |0;
    }
}

//================ term complexity ================

void calc_Z (Lang& L)
{//policy: calculates individual (z) and total (Z) complexities
// x in basis           comp(x)   comp(y)
// ---------- P_basis   ----------------- P_app
//   comp(x)                comp(x y)
//
// comp(x)   comp(y)          comp(x)   comp(y)
// ----------------- P_comp   ----------------- P_join
//     comp(x*y)                  comp(x|y)

    const Float P_app = L.get_P_app();
    const Float P_comp = L.get_P_comp();
    const Float P_join = L.get_P_join();
    const Float P_basis = L.P_basis();

    logger.debug() << "Propagating term complexity"
                   << ", P_app = " << P_app
                   << ", P_comp = " << P_comp
                   << ", P_join = " << P_join |0;
    Logging::IndentBlock block(logger.at_debug());

    comp.ensure_normal();
    komp.ensure_nonneg();
    Meas c_temp = Meas::alloc();    c_temp.set(0);
    Meas k_temp = Meas::alloc();    k_temp.set(INFINITY);

    const std::vector<ObHdl> atoms = CS::get_atoms(); //for below
    Int A = atoms.size();

    for stepsize_loop {
        //lexical basis (semantics)
        for iter_over_Basis { Ob ob = *(iter->first);
            Float mass = iter->second;
            c_temp(ob) += P_basis * mass;
        }

        //structural basis (syntax)
        for (Int a=0; a<A; ++a) { Ob ob = *(atoms[a]);
            k_temp(ob) = -log(P_basis * comp(ob));
        }

        //app part
        for iter_over_Apps { App eqn = *iter;
            Ob lhs = get_lhs(eqn);
            Ob rhs = get_rhs(eqn);
            Ob app = get_app(eqn);
                 c_temp(app) +=     P_app  * comp(lhs) * comp(rhs);
            imin(k_temp(app) , -log(P_app) + komp(lhs) + komp(rhs));
        }

        //comp part
        for iter_over_Comps { Comp eqn = *iter;
            Ob lhs = get_lhs(eqn);
            Ob rhs = get_rhs(eqn);
            Ob cmp = get_comp(eqn);
                 c_temp(cmp) +=     P_comp  * comp(lhs) * comp(rhs);
            imin(k_temp(cmp) , -log(P_comp) + komp(lhs) + komp(rhs));
        }

        //join part
        for iter_over_Joins { Join eqn = *iter;
            Ob lhs = get_lhs(eqn);
            Ob rhs = get_rhs(eqn);
            Ob join = get_join(eqn);
                 c_temp(join) +=     P_join  * comp(lhs) * comp(rhs);
            imin(k_temp(join) , -log(P_join) + komp(lhs) + komp(rhs));
        }

        //move mass over
        RelentropyNorm diff;
        for iter_over_Obs { Ob ob = *iter;

            Float _new = check_prob(c_temp(ob));
            diff.add(_new, comp(ob));
            comp(ob) = _new;
            c_temp(ob) = 0.0;

            if (    std::isfinite(k_temp(ob))
            and not std::isfinite(  komp(ob))) {
                diff.infinity();
            }
            komp(ob) = k_temp(ob);
            k_temp(ob) = INFINITY;
        }

        stepsize = diff.total();
        logger.debug() << "stepsize = " << stepsize |0;
    }
    k_temp.free();
    c_temp.free();
}
void calc_dZ (Lang& L, VMeas dcomp, bool initialized)
{//calculates individual (z,dz/dL) and total (Z,dZ/dL) complexities,
//the derivative is WRT the non-normalized coords
//    a[i] = L.prob(i) = L.mass(i) * (1 - L.P_app() - L.P_comp())
//see (thesis/info_geometry.text:Evolution of Language) for details

    using namespace Languages;
    Int dim = L.vect_size(); Assert (dim == dcomp.size(), "dZ has wrong size");
    Int num_atoms = dim - i_basis;

    const Float P_app  = L.get_P_app();
    const Float P_comp = L.get_P_comp();
    const Float P_join = L.get_P_join();

    //calculate values first, so as not to waste derivative calculation;
    //value calculation is continued (to higher-accuracy) in deriv calculation.
    //WARNING: value calculation must match algorithm in calc_Z
    calc_Z(L);
    if (not initialized) {
        dcomp.set(0.0);
        for (Int a=0; a<num_atoms; ++a) {
            dcomp(L.atom(a))[i_basis+a] = 1.0;
        }
    }

    logger.debug() << "finding gradient dZ, dim = " << dim |0;
    Logging::IndentBlock block(logger.at_debug());

    //temporaries
    Meas  temp  = Meas::alloc();
    VMeas dtemp = VMeas::alloc(dim);

    //p and q are comp and temp, resp
    Tensor dp   (dim, 1, NULL);
    Tensor dq   (dim, 1, NULL);
    Tensor dp_L (dim, 1, NULL);
    Tensor dp_R (dim, 1, NULL);

    for stepsize_loop {
        //add atom part
        temp.set(0);
        dtemp.set(0);
        for (Int a=0; a<num_atoms; ++a) { Ob ob = L.atom(a);
            temp(ob) = L.prob(a);
            dtemp(ob)[i_basis+a] = 1.0;
        }

        //add app part
        for iter_over_Apps { App eqn = *iter;

            Ob L = get_lhs(eqn);   Ob R = get_rhs(eqn);   Ob A = get_app(eqn);
            Float p_L = comp(L);   Float p_R = comp(R);   Float& q = temp(A);
            dp_L.set(dcomp(L));    dp_R.set(dcomp(R));    dq.set(dtemp(A));

            //value
            q += P_app * p_L * p_R;

            //1st deriv
            dq(i_app) += p_L * p_R;
            for (Int i=0; i<dim; ++i) {
                dq(i) += P_app * ( p_L * dp_R(i)
                                 + dp_L(i) * p_R );
            }
        }

        //add comp part
        for iter_over_Comps { Comp eqn = *iter;

            Ob L = get_lhs(eqn);   Ob R = get_rhs(eqn);   Ob C = get_comp(eqn);
            Float p_L = comp(L);   Float p_R = comp(R);   Float& q = temp(C);
            dp_L.set(dcomp(L));    dp_R.set(dcomp(R));    dq.set(dtemp(C));

            //value
            q += P_comp * p_L * p_R;

            //1st deriv
            dq(i_comp) += p_L * p_R;
            for (Int i=0; i<dim; ++i) {
                dq(i) += P_comp * ( p_L * dp_R(i)
                                  + dp_L(i) * p_R );
            }
        }

        //add join part
        for iter_over_Joins { Join eqn = *iter;

            Ob L = get_lhs(eqn);   Ob R = get_rhs(eqn);   Ob J = get_join(eqn);
            Float p_L = comp(L);   Float p_R = comp(R);   Float& q = temp(J);
            dp_L.set(dcomp(L));    dp_R.set(dcomp(R));    dq.set(dtemp(J));

            //value
            q += P_join * p_L * p_R;

            //1st deriv
            dq(i_join) += p_L * p_R;
            for (Int i=0; i<dim; ++i) {
                dq(i) += P_join * ( p_L * dp_R(i)
                                  + dp_L(i) * p_R );
            }
        }

        //update
        InftyScaleNorm diff;
        for iter_over_Obs { Ob ob = *iter;

            dp.set(dcomp(ob));
            dq.set(dtemp(ob));

            //value
            comp(ob) = check_prob(temp(ob));
            for (Int i=0; i<dim; ++i) {

                //1st deriv
                const Float _new = check_real(dq(i));
                diff.add(_new, dp(i));
                dp(i) = _new;
            }
        }
        temp.set(0);
        dtemp.set(0);

        stepsize = diff.total();
        logger.debug() << "stepsize = " << stepsize
            << ", scale = " << diff.scale()|0;
    }
    temp.free();
    dtemp.free();
}
void calc_ddZ (Lang& L, VMeas dcomp, VMeas ddcomp, bool initialized)
{
    //the derivatives are WRT the non-normalized coords
    //    a[i] = L.prob(i)  = L.mass(i) * (1 - L.P_app())
    //see (thesis/info_geometry.text:Evolution of Language) for details

    using namespace Languages;
    Int dim = L.vect_size();
    Assert (dim == dcomp.size(), "dZ has wrong size");
    Assert (dim == ddcomp.size(), "ddZ has wrong size");
    Int num_atoms = dim - i_basis;

    const Float P_app = L.get_P_app();
    const Float P_comp = L.get_P_comp();
    const Float P_join = L.get_P_join();

    //calculate 1st deriv first, so as not to waste derivative calculation;
    //value calculation is continued (to higher-accuracy) in deriv calculation.
    //WARNING: value calculation must match algorithm in calc_dZ
    calc_dZ(L, dcomp, initialized);
    if (not initialized) ddcomp.set(0.0);

    logger.info() << "finding Hessian ddZ, dim = " << dim |0;
    Logging::IndentBlock block;

    //temporaries
    Meas  temp   = Meas::alloc();
    VMeas dtemp  = VMeas::alloc(dim);
    VMeas ddtemp = VMeas::alloc(dim,2);

    //p and q are comp and temp, resp
    Tensor dp   (dim, 1, NULL);     Tensor ddp   (dim, 2, NULL);
    Tensor dq   (dim, 1, NULL);     Tensor ddq   (dim, 2, NULL);
    Tensor dp_L (dim, 1, NULL);     Tensor ddp_L (dim, 2, NULL);
    Tensor dp_R (dim, 1, NULL);     Tensor ddp_R (dim, 2, NULL);
    Float p_LR;
    Tensor dp_LR (dim,1);

    for stepsize_loop {
        //add atom part
          temp.set(0);
         dtemp.set(0);
        ddtemp.set(0);
        for (Int a=0; a<num_atoms; ++a) { Ob ob = L.atom(a);
            temp(ob) = L.prob(a);
            dtemp(ob)[i_basis+a] = 1.0;
        }

        //add app part
        for iter_over_Apps { App eqn = *iter;

            Ob L = get_lhs(eqn);   Ob R = get_rhs(eqn);   Ob A = get_app(eqn);
            Float p_L = comp(L);   Float p_R = comp(R);   Float& q = temp(A);
            dp_L.set(dcomp(L));    dp_R.set(dcomp(R));    dq.set(dtemp(A));
            ddp_L.set(ddcomp(L));  ddp_R.set(ddcomp(R));  ddq.set(ddtemp(A));

            //value
            q += P_app * (p_LR = p_L * p_R);

            //1st deriv
            dq(i_app) += p_LR;
            for (Int i=0; i<dim; ++i) {
                dq(i) += P_app * (dp_LR(i) = dp_L(i) * p_R
                                            + p_L * dp_R(i) );
            }

            //2nd deriv
            for (Int i=0; i<dim; ++i) {
                Int j = min<int>(i,i_app);
                Int k = max<int>(i,i_app);
                ddq(j,k) += dp_LR(i);
            }
            for (Int i=0; i<dim; ++i) {
            for (Int j=0; j<=i; ++j) {
                ddq(i,j) += P_app * ( ddp_L(i,j) * p_R
                                    + dp_L(i) * dp_R(j)
                                    + dp_L(j) * dp_R(i)
                                    + p_L * ddp_R(i,j)  );
            } }
        }

        //add comp part
        for iter_over_Comps { Comp eqn = *iter;

            Ob L = get_lhs(eqn);   Ob R = get_rhs(eqn);   Ob C = get_comp(eqn);
            Float p_L = comp(L);   Float p_R = comp(R);   Float& q = temp(C);
            dp_L.set(dcomp(L));    dp_R.set(dcomp(R));    dq.set(dtemp(C));
            ddp_L.set(ddcomp(L));  ddp_R.set(ddcomp(R));  ddq.set(ddtemp(C));

            //value
            q += P_comp * (p_LR = p_L * p_R);

            //1st deriv
            dq(i_comp) += p_LR;
            for (Int i=0; i<dim; ++i) {
                dq(i) += P_comp * (dp_LR(i) = dp_L(i) * p_R
                                            + p_L * dp_R(i) );
            }

            //2nd deriv
            for (Int i=0; i<dim; ++i) {
                Int j = min<int>(i,i_comp);
                Int k = max<int>(i,i_comp);
                ddq(j,k) += dp_LR(i);
            }
            for (Int i=0; i<dim; ++i) {
            for (Int j=0; j<=i; ++j) {
                ddq(i,j) += P_comp * ( ddp_L(i,j) * p_R
                                     + dp_L(i) * dp_R(j)
                                     + dp_L(j) * dp_R(i)
                                     + p_L * ddp_R(i,j)  );
            } }
        }

        //add join part
        for iter_over_Joins { Join eqn = *iter;

            Ob L = get_lhs(eqn);   Ob R = get_rhs(eqn);   Ob J = get_join(eqn);
            Float p_L = comp(L);   Float p_R = comp(R);   Float& q = temp(J);
            dp_L.set(dcomp(L));    dp_R.set(dcomp(R));    dq.set(dtemp(J));
            ddp_L.set(ddcomp(L));  ddp_R.set(ddcomp(R));  ddq.set(ddtemp(J));

            //value
            q += P_join * (p_LR = p_L * p_R);

            //1st deriv
            dq(i_join) += p_LR;
            for (Int i=0; i<dim; ++i) {
                dq(i) += P_join * (dp_LR(i) = dp_L(i) * p_R
                                            + p_L * dp_R(i) );
            }

            //2nd deriv
            for (Int i=0; i<dim; ++i) {
                Int j = min<int>(i,i_join);
                Int k = max<int>(i,i_join);
                ddq(j,k) += dp_LR(i);
            }
            for (Int i=0; i<dim; ++i) {
            for (Int j=0; j<=i; ++j) {
                ddq(i,j) += P_join * ( ddp_L(i,j) * p_R
                                     + dp_L(i) * dp_R(j)
                                     + dp_L(j) * dp_R(i)
                                     + p_L * ddp_R(i,j)  );
            } }
        }

        //update
        InftyScaleNorm diff;
        for iter_over_Obs { Ob ob = *iter;

            dp.set(dcomp(ob));      ddp.set(ddcomp(ob));
            dq.set(dtemp(ob));      ddq.set(ddtemp(ob));

            //value
            comp(ob) = check_prob(temp(ob));
            for (Int i=0; i<dim; ++i) {

                //1st deriv
                dp(i) = check_real(dq(i));
                for (Int j=0; j<=i; ++j) {

                    //2nd deriv
                    const Float _new = check_real(ddq(i,j));
                    diff.add(_new, ddp(i,j));
                    ddp(i,j) = _new;
                }
            }
        }

        stepsize = diff.total();
        logger.debug() << "stepsize = " << stepsize
            << ", scale = " << diff.scale()|0;
    }

    //symmetrize p_,ij = p_,ji
    for iter_over_Obs { Ob ob = *iter;
        ddp.set(dcomp(ob));

        for (Int i=0; i<dim; ++i) {
        for (Int j=0; j<i; ++j) {
            ddp(j,i) = ddp(i,j);
        } }
    }
    temp.free();
    dtemp.free();
    ddtemp.free();
}
void calc_dZ (Lang& L, Meas dcomp, const Vect& dL, bool initialized)
{//calculates individual (z,dz) and total (Z,dZ) complexities,
    //the derivative is WRT the non-normalized coords
    //    a[i] = L.prob(i) = L.mass(i) * (1 - L.P_app())
    //see (thesis/info_geometry.text:Evolution of Language) for details

    using namespace Languages;
    Int dim = L.vect_size(); Assert (dim == dL.size(), "dL has wrong size");
    Int num_atoms = dim - i_basis;

    const Float P_app  = L.get_P_app(),  dP_app  = dL(i_app);
    const Float P_comp = L.get_P_comp(), dP_comp = dL(i_comp);
    const Float P_join = L.get_P_join(), dP_join = dL(i_join);

    //calculate values first, so as not to waste derivative calculation;
    //value calculation is continued (to higher-accuracy) in deriv calculation.
    //WARNING: value calculation must match algorithm in calc_Z
    calc_Z(L);
    if (not initialized) {
        dcomp.set(0.0);
        for (Int a=0; a<num_atoms; ++a) {
            dcomp(L.atom(a)) = dL(i_basis+a);
        }
    }

    logger.debug() << "finding directional dZ, dim = " << dim |0;
    Logging::IndentBlock block(logger.at_debug());

    //temporaries
    Meas  temp = Meas::alloc();
    Meas dtemp = Meas::alloc();

    for stepsize_loop {
        //add atom part
        temp.set(0);
        dtemp.set(0);
        for (Int a=0; a<num_atoms; ++a) { Ob ob = L.atom(a);
            temp(ob)  = L.prob(a);
            dtemp(ob) = dL(i_basis+a);
        }

        //add app part
        for iter_over_Apps { App eqn = *iter;

            //p and q are comp and temp, resp
            Ob L = get_lhs(eqn);   Ob R = get_rhs(eqn);   Ob A = get_app(eqn);
            Float  p_L =  comp(L); Float  p_R =  comp(R); Float&  q =  temp(A);
            Float dp_L = dcomp(L); Float dp_R = dcomp(R); Float& dq = dtemp(A);

            //value and derivative
             q +=  P_app *    p_L *  p_R;
            dq += dP_app *    p_L *  p_R
                +  P_app * ( dp_L *  p_R
                         +    p_L * dp_R );
        }

        //add comp part
        for iter_over_Comps { Comp eqn = *iter;

            //p and q are comp and temp, resp
            Ob L = get_lhs(eqn);   Ob R = get_rhs(eqn);   Ob C = get_comp(eqn);
            Float  p_L =  comp(L); Float  p_R =  comp(R); Float&  q =  temp(C);
            Float dp_L = dcomp(L); Float dp_R = dcomp(R); Float& dq = dtemp(C);

            //value and derivative
             q +=  P_comp *    p_L *  p_R;
            dq += dP_comp *    p_L *  p_R
                +  P_comp * ( dp_L *  p_R
                          +    p_L * dp_R );
        }

        //add join part
        for iter_over_Joins { Join eqn = *iter;

            //p and q are comp and temp, resp
            Ob L = get_lhs(eqn);   Ob R = get_rhs(eqn);   Ob J = get_join(eqn);
            Float  p_L =  comp(L); Float  p_R =  comp(R); Float&  q =  temp(J);
            Float dp_L = dcomp(L); Float dp_R = dcomp(R); Float& dq = dtemp(J);

            //value and derivative
             q +=  P_join *    p_L *  p_R;
            dq += dP_join *    p_L *  p_R
                +  P_join * ( dp_L *  p_R
                          +    p_L * dp_R );
        }

        //update
        InftyScaleNorm diff;
        for iter_over_Obs { Ob ob = *iter;

            //value
            comp(ob) = check_prob(temp(ob));

            //1st deriv
            Float& dp = dcomp(ob);
            Float  dq = dtemp(ob);
            const Float _new = check_real(dq);
            diff.add(_new, dp);
            dp = _new;
        }

        stepsize = diff.total();
        logger.debug() << "stepsize = " << stepsize
            << ", scale = " << diff.scale()|0;
    }
    temp.free();
    dtemp.free();
}
void calc_ZHA (Lang& L)
{//policy: sums over database
    g_ZHA = 0.0;
    for iter_over_Obs { Ob ob = *iter;
        g_ZHA += Moments::Z_H(comp(ob));
    }
    logger.debug() << "H(A) = " << get_HA() |0;
}
void _calc_app_part (Lang& L, AMeas& app_part)
{//calculates app_part(a=lr) := comp(l)*comp(r)/comp(a)
    const Float P_app = L.get_P_app();

    for iter_over_Apps { App eqn = *iter;
        Float numer = P_app * comp(get_lhs(eqn)) * comp(get_rhs(eqn));
        Float denom = comp(get_app(eqn));
        Float part = (numer <= g_tol * denom) ? 0.0 : numer/denom;
        Assert4(0.0 <= part + g_tol, "comp part is less than zero: " << part);
        //Assert4(part <= 1.0+g_tol,
        //        "comp part is greater than one: " << part);
        //Note: very recent complex terms may have ratios greater than one;
        //  these tend to have little effect anyway, so bounding doesn't hurt
        bound_in_01(part);
        app_part(eqn) = part;
    }
}
void _calc_comp_part (Lang& L, CMeas& comp_part)
{//calculates comp_part(c=l*r) := comp(l)*comp(r)/comp(c)
    const Float P_comp = L.get_P_comp();

    for iter_over_Comps { Comp eqn = *iter;
        Float numer = P_comp * comp(get_lhs(eqn)) * comp(get_rhs(eqn));
        Float denom = comp(get_comp(eqn));
        Float part = (numer <= g_tol * denom) ? 0.0 : numer/denom;
        Assert4(0.0 <= part + g_tol, "comp part is less than zero: " << part);
        //Assert4(part <= 1.0+g_tol,
        //        "comp part is greater than one: " << part);
        //Note: very recent complex terms may have ratios greater than one;
        //  these tend to have little effect anyway, so bounding doesn't hurt
        bound_in_01(part);
        comp_part(eqn) = part;
    }
}
void _calc_join_part (Lang& L, JMeas& join_part)
{//calculates comp_part(j=l|r) := comp(l)*comp(r)/comp(j)
    const Float P_join = L.get_P_join();

    for iter_over_Joins { Join eqn = *iter;
        Float numer = P_join * comp(get_lhs(eqn)) * comp(get_rhs(eqn));
        Float denom = comp(get_join(eqn));
        Float part = (numer <= g_tol * denom) ? 0.0 : numer/denom;
        Assert4(0.0 <= part + g_tol, "join part is less than zero: " << part);
        //Assert4(part <= 1.0+g_tol,
        //        "join part is greater than one: " << part);
        //Note: very recent complex terms may have ratios greater than one;
        //  these tend to have little effect anyway, so bounding doesn't hurt
        bound_in_01(part);
        join_part(eqn) = part;
    }
}
void calc_R (Lang& L, Interest& interest)
{//policy:
//  r_0(x) = u(x)   #initial relevance
//                           r(w)z(x)z(y)               r(w)z(y)z(x)
//  r(x) = u(x) + sum_{xy=w} ------------  + sum_{yx=w} ------------
//                               z(w)                       z(w)
//                            r(w)z(x)z(y)                r(w)z(y)z(x)
//              + sum_{x*y=w} ------------  + sum_{y*x=w} ------------
//                                z(w)                        z(w)
    logger.debug() << "Propagating term relevance" |0;
    Logging::IndentBlock block(logger.at_debug());

    //compute complexity part beforehand
    AMeas app_part;     _calc_app_part (L, app_part);
    CMeas comp_part;    _calc_comp_part(L, comp_part);
    JMeas join_part;    _calc_join_part(L, join_part);
    Meas temp = Meas::alloc();  temp.set(0);
    rel.ensure_normal();

    Double total = 0.0;
    for stepsize_loop {
        //initialize relevance
        interest.init_R_term(temp);

        //propagate relevance from app --> lhs, app --> rhs
        for iter_over_Apps { App eqn = *iter;
            const Float _rel = app_part(eqn) * rel(get_app(eqn));
            temp(get_lhs(eqn)) += _rel;
            temp(get_rhs(eqn)) += _rel;
        }

        //propagate relevance from comp --> lhs, comp --> rhs
        for iter_over_Comps { Comp eqn = *iter;
            const Float _rel = comp_part(eqn) * rel(get_comp(eqn));
            temp(get_lhs(eqn)) += _rel;
            temp(get_rhs(eqn)) += _rel;
        }

        //propagate relevance from join --> lhs, join --> rhs
        for iter_over_Joins { Join eqn = *iter;
            const Float _rel = join_part(eqn) * rel(get_join(eqn));
            temp(get_lhs(eqn)) += _rel;
            temp(get_rhs(eqn)) += _rel;
        }

        //update and calculate stepsize
        RelentropyNorm diff;
        total = 0.0;
        for iter_over_Obs { Ob ob = *iter;
            const Float _new = check_nonneg(temp(ob));
            Float& _old = rel(ob);
            diff.add(_new, _old);
            total += _old = _new;
            temp(ob) = 0.0;
        }

        stepsize = diff.total();
        logger.debug() << "total = " << total << ", stepsize = " << stepsize |0;
    }
    temp.free();

    g_R_term = total;
}
void calc_rho (Lang& L, const ObPoly& C, Meas rho)
{//derivative of corpus cost WRT basis weights
//           C(x)
//  rho(x) = ----  +  P_app [ sum_{xy=w} rho(w) z(y)
//           z(x)           + sum_{yx=w} rho(w) z(y) ]
//                 + P_comp [ ... ]
//                 + P_join [ ... ]
    logger.debug() << "Propagating relevance of terms to corpus" |0;
    Logging::IndentBlock block(logger.at_debug());

    Meas temp = Meas::alloc();  temp.set(0);
    rho.set(0);
    for (Int i=0; i<C.size(); ++i) { Ob ob = C.ob(i);
        rho(C.ob(i)) = C.num(i) / comp(ob);
        AssertW(comp(ob) > 0, "ob in corpus has infinite complexity: " << ob);
    }

    const Float P_app  = L.get_P_app();
    const Float P_comp = L.get_P_comp();
    const Float P_join = L.get_P_join();
    for stepsize_loop {
        //initialize relevance
        for (Int i=0; i<C.size(); ++i) { Ob ob = C.ob(i);
            temp(C.ob(i)) = C.num(i) / comp(ob);
        }

        //propagate relevance from app --> lhs, app --> rhs
        for iter_over_Apps { App eqn = *iter;
            Ob A = get_app(eqn), L = get_lhs(eqn), R = get_rhs(eqn);
            Float rho_eqn = rho(A) * P_app;
            temp(L) += rho_eqn * comp(R);
            temp(R) += rho_eqn * comp(L);
        }

        //propagate relevance from comp --> lhs, comp --> rhs
        for iter_over_Comps { Comp eqn = *iter;
            Ob C = get_comp(eqn), L = get_lhs(eqn), R = get_rhs(eqn);
            Float rho_eqn = rho(C) * P_comp;
            temp(L) += rho_eqn * comp(R);
            temp(R) += rho_eqn * comp(L);
        }

        //propagate relevance from join --> lhs, join --> rhs
        for iter_over_Joins { Join eqn = *iter;
            Ob J = get_join(eqn), L = get_lhs(eqn), R = get_rhs(eqn);
            Float rho_eqn = rho(J) * P_join;
            temp(L) += rho_eqn * comp(R);
            temp(R) += rho_eqn * comp(L);
        }

        //update and calculate stepsize
        InftyScaleNorm diff;
        for iter_over_Obs { Ob ob = *iter;
            const Float _new = check_nonneg(temp(ob));
            Float& _old = rho(ob);
            diff.add(_new, _old);
            _old = _new;
            temp(ob) = 0.0;
        }

        stepsize = diff.total();
        logger.debug() << "stepsize = " << stepsize
            << ", scale = " << diff.scale() |0;
    }
    temp.free();
}
Vect calc_coords (Lang& L, VMeas coords)
{//calculates a few principle components of perturbation
    Int dim = coords.size();
    Assert (dim > 0, "too few perturbation directions: " << dim);
    logger.info() << "calculating " << dim << " perturbation directions" |0;
    Logging::IndentBlock block;

    //temporaries
    Tensor temp(dim,1,NULL);
    Meas mu = Meas::alloc(), len = Meas::alloc();
    Vect evals(dim);
    Float shift = 1.0; //so dominant eigenvalues are positive

    for iter_over_Obs { Ob ob = *iter;
        len(ob) = -log(comp(ob));
    }

    //calculate coordinates by power method
    coords.set(0);
    for (Int i=0; i<dim; ++i) {
        Meas nu = coords[i];

        logger.info() << "computing coordinate " << i |0;
        Logging::IndentBlock block;

        if (i == 0) {
            //initialize uniformly
            mu.set(sqrt(1.0 / Ob::size()));
        } else {
            //initialize randomly
            for iter_over_Obs { Ob ob = *iter;
                mu(ob) = LA::random_normal();
            }
            normalize2(mu,mu);
        }

        //iteratively diffused orthogonal to earlier eivenvectors
        for stepsize_loop { //updating nu <= mu

            //transform under diffusion matrix
            for iter_over_Obs { Ob ob = *iter;
                if (not (comp(ob) > 0)) continue;
                nu(ob) = shift * mu(ob);
            }
            for iter_over_Apps { App eqn = *iter;
                Ob A = get_app(eqn); if (not (comp(A) > 0)) continue;
                Ob L = get_lhs(eqn); if (not (comp(L) > 0)) continue;
                Ob R = get_rhs(eqn); if (not (comp(R) > 0)) continue;

                /* Version 1.
                Float len_L = len(L), len_R = len(R);
                Float part = comp(L) * comp(R) / (comp(A) * (len_L + len_R));
                A
                Float diff_LA = part * len_L * (mu(L) - mu(A));
                Float diff_RA = part * len_R * (mu(R) - mu(A));

                nu(A) += diff_LA + diff_RA;
                nu(L) -= diff_LA;
                nu(R) -= diff_RA;
                */

                // Version 2.
                Float len_L = len(L), len_R = len(R);
                Float part = comp(L) * comp(R) / (len_L + len_R);
                Float diff_LA = part * len_L * (mu(L) - mu(A));
                Float diff_RA = part * len_R * (mu(R) - mu(A));

                nu(A) += (diff_LA + diff_RA) / comp(A);
                nu(L) -= diff_LA / comp(L);
                nu(R) -= diff_RA / comp(R);
            }
            //fix weightless obs
            for iter_over_Obs { Ob ob = *iter;
                if (not (comp(ob) > 0)) nu(ob) = 0;
            }

            //orthonormalize via Gram-Schmidt
            for (Int j=0; j<i; ++j) {
                Meas coord = coords[j];
                Float ip = dot(nu, coord);
                for iter_over_Obs { Ob ob = *iter;
                    nu(ob) -= ip * coord(ob);
                }
            }
            Float ip = dot(mu,nu);
            evals(i) = ip - shift;
            if (ip < 0) {           //possibly shrink diffusion stepsize
                shift *= 1.25f;
                logger.info() << "shifting eigenvalues, center = " << shift |0;
            }
            normalize2(nu,nu);

            //calculate stepsize
            stepsize = 0;
            for iter_over_Obs { Ob ob = *iter;
                stepsize += sqr(mu(ob)- nu(ob));
                mu(ob) = nu(ob);
            }
            stepsize = sqrt(stepsize);
            logger.info() << "eval = " << evals(i)
                          << ", step = " << stepsize |0;
        }
    }

    //clean up
    len.free();
    mu.free();

    logger.info() << "eigenvalues = " << evals |0;
    return evals;
}
Vect calc_coords_alt (Lang& L, VMeas coords)
{//calculates eigenvectors of discriminability metric, returns eigenvalues
//(see notes/ideas/statistical_models.text (2007:11:15))
//
// cor(x,y) = Sum f (if not not (fx conv <==> fy conv)). P(f)
//
// diff(x,y) = -log( 1 - cor(x,y) ) P(x) P(y)
//
    Int dim = coords.size();
    Assert (dim > 0, "too few perturbation directions: " << dim);
    logger.info() << "calculating " << dim << " perturbation directions" |0;
    Logging::IndentBlock block;

    //temporaries
    Tensor temp(dim,1,NULL);
    Meas mu = Meas::alloc();
    Vect evals(dim);
    Float shift = 1.0; //so dominant eigenvalues are positive

    //precompute diffusion matrix
    logger.info() << "computing diffusion matrix" |0;
    Mat diff(1+Ob::size(),2);
    Ob Bot = *LT::Atoms::Bot;
    Int O = Ob::size();
    Float cor_min = INFINITY, cor_max = -INFINITY;
    for (Int xi=1; xi<=O;  ++xi) { Ob x(xi);  Float P_x = comp(x);
    for (Int yi=1; yi<=xi; ++yi) { Ob y(yi);  Float P_y = comp(y);
#define conv(ob) CS::isNLessThan(ob,Bot)
        Float cor = 0.0;
        for (AE::RRla_Iterator fx_fy(x,y); fx_fy.ok(); fx_fy.next()) {
            if (conv(fx_fy.app1()) == conv(fx_fy.app2())) {
                cor += comp(fx_fy.lhs());
            }
        }
        if (cor < cor_min) cor_min = cor;
        if (cor > cor_max) cor_max = cor;
        diff(x,y) = diff(y,x) = -log(1.0 - cor) * P_x * P_y;
#undef conv
    } }
    logger.info() << "correlation range: " << cor_min << " < " << cor_max |0;

    //calculate coordinates by power method
    coords.set(0);
    for (Int i=0; i<dim; ++i) {
        Meas nu = coords[i];

        logger.info() << "computing coordinate " << i |0;
        Logging::IndentBlock block;

        //initialize randomly
        for iter_over_Obs { Ob ob = *iter;
            mu(ob) = LA::random_normal();
        }
        normalize2(mu,mu);

        //iteratively diffused orthogonal to larger vectors
        for stepsize_loop { //updating nu <= mu

            //transform under similarity matrix
            for iter_over_Obs { Ob ob = *iter;
                if (not (comp(ob) > 0)) continue;
                nu(ob) = shift * mu(ob);
            }
            for iter_over_Obs { Ob x = *iter;
            for iter_over_Obs { Ob y = *iter;
                nu(x) += diff(x,y) * nu(y);
            } }
            //fix weightless obs
            for iter_over_Obs { Ob ob = *iter;
                if (not (comp(ob) > 0)) nu(ob) = 0;
            }

            //orthonormalize via Gram-Schmidt
            for (Int j=0; j<i; ++j) {
                Meas coord = coords[j];
                Float ip = dot(nu, coord);
                for iter_over_Obs { Ob ob = *iter;
                    nu(ob) -= ip * coord(ob);
                }
            }
            Float ip = dot(mu,nu);
            evals(i) = ip - shift;
            if (ip < 0) {           //possibly shrink diffusion stepsize
                shift *= 1.25f;
                logger.info() << "shifting eigenvalues, center = " << shift |0;
            }
            normalize2(nu,nu);

            //calculate stepsize
            stepsize = 0;
            for iter_over_Obs { Ob ob = *iter;
                stepsize += sqr(mu(ob)- nu(ob));
                mu(ob) = nu(ob);
            }
            stepsize = sqrt(stepsize);
            logger.info() << "eval = " << evals(i)
                          << ", step = " << stepsize |0;
        }
    }

    //clean up
    mu.free();

    logger.info() << "eigenvalues = " << evals |0;
    return evals;
}

//================ ordering complexity ================

//falure propagation
void prop_known (FOrder& less2)
{//proven theorems have 0-1 probability
    using namespace Symbols;

    for (Ord::iterator<OR::POS> iter; iter.ok(); iter.next()) {
        Ob lhs = iter.lhs();
        Ob rhs = iter.rhs();
        less2(lhs, rhs) = P_TRUE;
    }

    //unnecessary
    //for (Ord::iterator<OR::NEG> iter; iter; iter.next()) {
    //    Ob lhs = iter.lhs();
    //    Ob rhs = iter.rhs();
    //    less2(lhs, rhs) = P_FALSE;
    //}
}
void prop_mu (Float P_mu, const FOrder& less, FOrder& less2)
{//right-monotonicity, failures of
// comp(f)   nless(fx,fy)
// ----------------------
//        nless(x,y)

    AE::RRla_Iterator fx_fy;
    Int N = Ob::numUsed();
    for (Int _x=1; _x<=N; ++_x) { Ob x(_x);
    for (Int _y=1; _y<=N; ++_y) { Ob y(_y);
    if (not OR::contains(x,y)) {

        Float less_xy = 0.0;
        for (fx_fy.begin(x,y); fx_fy.ok(); fx_fy.next()) {
            Ob f  = fx_fy.lhs();
            Ob fx = fx_fy.app1();
            Ob fy = fx_fy.app2();

            less_xy += comp(f) * less(fx,fy);
        }
        less2(x,y) += P_mu * less_xy;
    }}}
}
void prop_nu (Float P_nu, const FOrder& less, FOrder& less2)
{//left-monotonicity
// nless(fx,gx)   comp(f)
// ----------------------
//       nless(f,g)

    AE::LLra_Iterator fx_gx;
    Int N = Ob::numUsed();
    for (Int _f=1; _f<=N; ++_f) { Ob f(_f);
    for (Int _g=1; _g<=N; ++_g) { Ob g(_g);
    if (not OR::contains(f,g)) {
        Float less_fg = 0.0;
        for (fx_gx.begin(f,g); fx_gx.ok(); fx_gx.next()) {
            Ob x  = fx_gx.rhs();
            Ob fx = fx_gx.app1();
            Ob gx = fx_gx.app2();

            less_fg += less(fx,gx) * comp(x);
        }
        less2(f,g) += P_nu * less_fg;
    }}}
}
void prop_tau (Float P_tau, const FOrder& less, FOrder& less2)
{//transitivity, failures of
// nless(x,y)  less(z,y)   less(y,x)   nless(y,z)
// ---------------------   ----------------------
//       nless(x,z)              nless(x,z)
//XXX this is no longer monotone!

    using namespace Symbols;
    Float P_tau_LR = 0.5f * P_tau;

    Int N = Ob::numUsed();
    for (Int _x=1; _x<=N; ++_x) { Ob x(_x);
    for (Int _z=1; _z<=N; ++_z) { Ob z(_z);
    if (not OR::contains(x,z)) {
        Float less_xz = 0.0;
        for (Int _y=1; _y<=N; ++_y) { Ob y(_y);
            less_xz += comp(y) * ( Not(And(     less(x,y)  , Not(less(z,y)) ))
                                 + Not(And( Not(less(y,x)) ,     less(y,z)  )));
        }
        less2(x,z) += P_tau_LR * less_xz;
    }}}
}

//propagation interface
#define CASE(rule,mess,fun)\
    case rule: log << mess |1; fun(weight,less,less2); break;
void calc_P (Lang& L, FOrder& less)  //may contain prior knowledge
{//policy: (see notes/proof/main.text for older version)
// P(x[=y) = if |- x[=y  then 1 else
//           if |- x[!=y then 0 else
//         ( mu E w. P(wx[=wy)                      #see prop_mu
//         + nu E z. P(xz[=yx)                      #see prop_nu
//         + tau_L E z. 1 - P(x[=z) (1-P(y[=z))     #see prop_tau
//         + tau_R E z. 1 - (1-P(z[=x)) P(z[=y) )   #see prop_tau
// where 1 = mu + nu + tau_L + tau_R    (typically tau_L = tau_R = tau)
// and expectation (E z) is wrt complexity measure

    logger.info() << "Propagating failure complexity" |0;
    Logging::IndentBlock block;
    L.log_rules();
    const RulePMF& rules = L.rules();

    using namespace Symbols;
    Int N = Ob::numUsed();
    FOrder less2(N);

    //propagate evidence (contractively, with the correct fixed point)
    for stepsize_loop {
        const Logging::fake_ostream& log = logger.info();

        //set known theorems to 0-1 probability
        less2.clear();
        prop_known (less2);

        //propagate all rules
        for (RulePMF::const_iterator r=rules.begin(); r!=rules.end(); ++r) {
            PropRule name = r->first;
            Float weight = r->second;
            if (weight <= 0) continue;
            switch (name) {
                CASE( RULE_MU,   "-mu",   prop_mu );
                CASE( RULE_NU,   "-nu",   prop_nu );
                CASE( RULE_TAU,  "-tau",  prop_tau );
            }
        }
        log << "- " |1;

        //calculate stepsize
        Prob knowledge = P_FALSE;
        ProbNorm diff;
        for (Int _x=1; _x<=N; ++_x) { Ob x(_x); //Float comp_x = comp(x);
        for (Int _y=1; _y<=N; ++_y) { Ob y(_y); //Float comp_y = comp(y);
            Prob& old_xy = less(x,y);
            Prob& new_xy = less2(x,y);
            bound_in_01(new_xy);
            diff.add(new_xy, old_xy);
            old_xy = new_xy;
            knowledge += new_xy;
        }}
        stepsize = diff.total();

        log << "info = " << knowledge << ", step = " << stepsize |0;
    }
}
#undef CASE

void calc_Q (Meas q)
{//policy: q(x) = "info unknown about x"
//               { 0          |- x[=y  }   { 0          |- x=]y  }
//  q(x) = sum_y { 0          |- x[!=y } + { 0          |- x=!]y }
//               { p(x) p(y)  o/wise   }   { p(x) p(y)  o/wise   }

    q.set(0);
    typedef Ob::sparse_iterator Iter;
    Iter beg = Ob::sbegin(), end = Ob::send();
    for (Iter i=beg; i!=end; ++i) { Ob x = *i; Float P_x = comp(x);
    for (Iter j=beg; j!=end; ++j) { Ob y = *j; Float P_y = comp(y);
        if (OR::contains(x,y)) continue;
        Float p = P_x * P_y;
        q(x) += p;
        q(y) += p;
    }}
}

//================ information geometry ================

/** Info Geometry [not used for anything yet]
 * cost:
 *   c   = -log p
 *   dc  = -dp/p
 *   ddc = dp^2/p^2 - ddp/p
 *
 * metric tensor
 *   g_ij = E[ dc_i dc_j ]
 *        = Sum[ dc_i dc_j  p ]
 *        = Sum[ dp_i dp_j / p ]
 *   g_ij,k = Sum[ (ddp dp + dp ddp - dp dp dp / p ) / p  ]
 *   g^ij = "matrix inverse of g_ij"
 *
 * skewness (negative of Amari's sign convention)
 *   T_ijk = E[ dc_i dc_j dc_k ]
 *         = -Sum[ dp_i dp_j dp_k / p^2 ]
 *
 * christoffel symbols, defining the riemannian connection
 *   G_ijk = E[ 1/2 (g_ik,j + g_ij,k - g_jk,i) ]
 *   G^i_jg = g^il G_ljk
 *
 * alpha-connection
 *   G(a)_ijk = G_ijk + a/2 T_ijk
 */

Tensor calc_g (VMeas dcomp)
{//computes metric; sets global value g_metric
    Int dim = dcomp.dim();
    logger.info() << "calculating metric, dim = " << dim |0;
    Logging::IndentBlock block;

    Tensor dp(dim, 1, NULL);
    Tensor g (dim, 2);  //g_ij

    //calculate fisher metric at obs: g_ij(ob)
    for iter_over_Obs { Ob ob = *iter;
        Float p = comp(ob); if (p <= 0) continue;
        dp.set(dcomp(ob));
        for (Int i=0; i<dim; ++i) {
        for (Int j=0; j<=i; ++j) {
            g(i,j) += dp(i) * dp(j) / p;
        } }
    }

    //symmetrize g_ij = g_ji
    for (Int i=0; i<dim; ++i) {
    for (Int j=0; j<i; ++j) {
        g(j,i) = g(i,j);
    } }

    g.copy_to(g_metric);
    return g;
}
Tensor calc_G (VMeas dcomp, VMeas ddcomp, const Tensor& g)
{//computes Christoffel symbols; sets global value g_christoffel
    Int dim = dcomp.dim();
    logger.info() << "calculating Christoffel's, dim = " << dim |0;
    Logging::IndentBlock block;

    Tensor dp  (dim, 1, NULL);
    Tensor ddp (dim, 2, NULL);
    Tensor dg  (dim, 3);  //g_ij,k
    Tensor gG  (dim, 3);  //G_ijk
    Tensor G   (dim, 3);  //G^i_jk

    //calculate at obs x
    for iter_over_Obs { Ob ob = *iter;
        Float p = comp(ob);
        dp.set(dcomp(ob));
        ddp.set(ddcomp(ob));

        //calculate derivs of fisher metric: g_ij,k(ob)
        for (Int i=0; i<dim; ++i) {
        for (Int j=0; j<=i; ++j) {
        for (Int k=0; k<dim; ++k) {
            dg(i,j,k) = dg(j,i,k)
                      = ( ddp(i,k) * dp(j)
                        + dp(i) * ddp(j,k)
                        - dp(i) * dp(j) * dp(k) / p ) / p;
        } } }

        //add component to lower-triangle of Christoffel symbol: 2 G_ijk
        for (Int i=0; i<dim; ++i) {
        for (Int j=0; j<dim; ++j) {
        for (Int k=0; k<=j; ++k) {
            //the factor of 1/2 is outside this loop
            gG(i,j,k) += dg(i,j,k) + dg(i,k,j) - dg(j,k,i);
        } } }
    }
    gG *= 0.5; //the missing factor of 1/2

    //symmetrize G_ijk = G_ikj
    for (Int i=0; i<dim; ++i) {
    for (Int j=0; j<dim; ++j) {
    for (Int k=0; k<j; ++k) {
        gG(i,k,j) = gG(i,j,k);
    } } }

    //compute G^i_jk by raising first index
    Tensor g_inv = g.inverse(); //g^ij
    for (Int i1=0; i1<dim; ++i1) {
    for (Int j=0; j<dim; ++j) {
    for (Int k=0; k<=j; ++k) {
        for (Int i2=0; i2<dim; ++i2) {
            G(i1,j,k) += g_inv(i1,i2) * gG(i2,j,k);
        }
        G(i1,k,j) = G(i1,j,k);
    } } }

    G.copy_to(g_christoffel);
    return G;
}
Tensor calc_T  (VMeas dcomp, const Tensor& g)
{//computes skewness; sets global value g_skewness
    Int dim = dcomp.dim();
    logger.info() << "calculating skewness, dim = " << dim |0;
    Logging::IndentBlock block;

    Tensor dp (dim, 1, NULL);
    Tensor gT (dim, 3);  //T_ijk
    Tensor T  (dim, 3);  //T^i_jk

    //calculate at obs x
    for iter_over_Obs { Ob ob = *iter;
        Float p = comp(ob); if (not (0 <= p)) continue;
        Float pp_inv = -1.0 / sqr(p);
        dp.set(dcomp(ob));

        //add component to lower-triangle of skewness tensor: 2 G_ijk
        for (Int i=0; i<dim; ++i) {
        for (Int j=0; j<=i; ++j) {
        for (Int k=0; k<=j; ++k) {
            //the factor of 1/2 is outside this loop
            gT(i,j,k) += dp(i) * dp(j) * dp(k) * pp_inv;
        } } }
    }

    //symmetrize T_ijk over {i,j,k}
    for (Int i=0; i<dim; ++i) {
    for (Int j=0; j<i; ++j) {
    for (Int k=0; k<j; ++k) {
         gT(k,j,i) = gT(k,i,j) = gT(j,k,i) = gT(j,i,k) = gT(i,k,j) = gT(i,j,k);
    } } }

    //compute T^i_jk by raising first index
    Tensor g_inv = g.inverse(); //g^ij
    for (Int i1=0; i1<dim; ++i1) {
    for (Int j=0; j<dim; ++j) {
    for (Int k=0; k<=j; ++k) {
        for (Int i2=0; i2<dim; ++i2) {
            T(i1,j,k) += g_inv(i1,i2) * gT(i2,j,k);
        }
        T(i1,k,j) = T(i1,j,k);
    } } }

    T.copy_to(g_skewness);
    return T;
}
Tensor calc_G_alpha (const Tensor& G, const Tensor& T, Float alpha)
{
    Tensor Ga = T;
    Ga *= alpha / 2.0;
    Ga += G;
    return Ga;
}

//================ language optimization ================

/** Language Optimization
 *
 * Local Optimization.
 *   The function fit_lang_to_obs(obs) fits a language to a training set C.
 *   The objective function is
 *
 *     "L is fit" <==> "basis is small and L covers the corpus C"
 *
 *     cost(L)  =  exp(-e) H(C|L) + H(L0) + const
 *
 *   where
 *    * C may or may not be normalized, according to NORMALIZE_CONTEXT,
 *    * the entropy H(L0) may or may not be added, according to SHRINK_BASIS
 *    * e is the elegance.
 *   Note that H(L0) does not depend on P_app.
 *
 *   If NEWTONS_METHOD is defined,
 *   optimizer will descend WRT the information metric;
 *   otherwise it uses a conjugate gradient direction.
 *
 * Basis modification.
 *   When state vector approaches a constraint,
 *   the optimizer will "retract" an atom from the basis.
 *
 *   If EXTEND_BASIS is defined,
 *   the optimizer will look for good atoms to "extend" the basis.
 *   see notes/ideas/optimization_formulations.text (2007:05:07-10) (N1.N3).
 */
//#define SHRINK_BASIS
#define NORMALIZE_CONTEXT
//#define EXTEND_BASIS
//#define NEWTONS_METHOD

//unconstrained objective function
class CorpusCost : public OP::FunGradMetricOfVect
{
protected:
    Lang &L;
    const ObPoly& C;
    const Float corpus_weight;
    Meas  dcomp0; bool initialized0;
    VMeas dcomp1; bool initialized1;
public:
    CorpusCost (Lang& L_, const ObPoly& C_, Float elegance)
        : FunGradMetricOfVect(L_.vect_size()),
          L(L_), C(C_),
#ifdef NORMALIZE_CONTEXT
          corpus_weight(exp(-elegance) / C.total()),
#else
          corpus_weight(exp(-elegance)),
#endif
          dcomp0(Meas::alloc()),       initialized0(false),
          dcomp1(VMeas::alloc(m_dim)), initialized1(false)
    {}
    virtual ~CorpusCost () { dcomp0.free(); dcomp1.free(); }

protected:
    virtual void eval (const Vect& p);
    virtual void eval_in_dir (const Vect& p, const Vect& dp);
    virtual void eval_grad (const Vect& p);
    virtual void eval_metric (const Vect& p);
};
void CorpusCost::eval (const Vect& p)
{
    //return values
    Float& cost = m_f;

    //update complexity
    L.from_vect(p);
    calc_Z(L);

    //evaluate (-log of) polynomial
    cost = 0.0;

    //  portion due to apps, comps, and joins
    cost += -log(L.P_app())  * C.apps();
    cost += -log(L.P_comp()) * C.comps();
    cost += -log(L.P_join()) * C.joins();

    //  portion due to terms in corpus
    for (Int c=0; c<C.size(); ++c) {
        Ob ob = C.ob(c);
        Float num = C.num(c);
        cost += num * (-log(comp(ob)));
    }

    //rescale
    cost *= corpus_weight;

#ifdef SHRINK_BASIS
    //add basis entropy
    cost += L.calc_H();
#endif

    logger.debug() << "P_app = "    << L.P_app()
                   << ", P_comp = " << L.P_comp()
                   << ", P_join = " << L.P_join()
                   << ", cost = "   << cost |0;
}
void CorpusCost::eval_in_dir (const Vect& p, const Vect& dp)
{
    logger.debug() << "evaluating directional deriv of language cost" |0;
    Logging::IndentBlock block(logger.at_debug());

    using namespace Languages;
    Assert2(m_dim == L.vect_size(), "LangFitness's Lang has wrong dimension");

    //return values
    Float& cost = m_f;
    Float& dcost = m_df;

    //update complexity
    L.from_vect(p);
    calc_dZ(L, dcomp0, dp, initialized0);
    initialized0 = true;

    //evaluate (-log of) polynomial
     cost = 0.0;
    dcost = 0.0;

    //  portion due to apps, comps, and joins
    Float P_app  = L.get_P_app(),  dP_app  = dp(i_app);
    Float P_comp = L.get_P_comp(), dP_comp = dp(i_comp);
    Float P_join = L.get_P_join(), dP_join = dp(i_join);

    cost += -log(P_app)  * C.apps();   dcost += -(dP_app  / P_app)  * C.apps();
    cost += -log(P_comp) * C.comps();  dcost += -(dP_comp / P_comp) * C.comps();
    cost += -log(P_join) * C.joins();  dcost += -(dP_join / P_join) * C.joins();

    //  portion due to terms in corpus
    for (Int c=0; c<C.size(); ++c) {
        Ob ob = C.ob(c);
        Float num = C.num(c);

        Float p = comp(ob); Assert (p>0, "nonpositive mass in corpus: " << p);
        Float dp = dcomp0(ob);

         cost += num * check_nonneg(-log(p));
        dcost += num * check_real(-dp / p);
    }

    //rescale
     cost *= corpus_weight;
    dcost *= corpus_weight;

#ifdef SHRINK_BASIS
    //add basis entropy
    LA::Nbhd1 nbhd = L.calc_dH(dp);
     cost += nbhd.val;
    dcost += nbhd.deriv;
#endif

    logger.debug() << "P_app = "    << L.P_app()
                   << ", P_comp = " << L.P_comp()
                   << ", P_join = " << L.P_join()
                   << ", cost = "   << cost
                   << ", dcost = "  << m_df |0;
}
void CorpusCost::eval_grad (const Vect& p)
{
    logger.debug() << "evaluating gradient of language cost" |0;
    Logging::IndentBlock block(logger.at_debug());

    using namespace Languages;
    Assert2(m_dim == L.vect_size(), "LangFitness's Lang has wrong dimension");

    //return values
    Float& cost = m_f;
    Vect& dcost = m_grad;

    //update complexity
    L.from_vect(p);
    calc_dZ(L, dcomp1, initialized1);
    initialized1 = true;

    //evaluate (-log of) polynomial
     cost = 0.0;
    dcost = 0.0;

    //  portion due to apps, comps, and joins
    Float P_app  = L.get_P_app();
    Float P_comp = L.get_P_comp();
    Float P_join = L.get_P_join();

    cost += -log(P_app)  * C.apps();
    cost += -log(P_comp) * C.comps();
    cost += -log(P_join) * C.joins();

    dcost(i_app)  += -(1.0 / P_app)  * C.apps();
    dcost(i_comp) += -(1.0 / P_comp) * C.comps();
    dcost(i_join) += -(1.0 / P_join) * C.joins();

    //  portion due to terms in corpus
    for (Int c=0; c<C.size(); ++c) {
        Ob ob = C.ob(c);
        Float num = C.num(c);

        Float p = comp(ob); Assert (p>0, "nonpositive mass in corpus: " << p);
        Float* dp = dcomp1(ob);

        cost += num * check_nonneg(-log(p));
        for (Int i=0; i<m_dim; ++i) {
            dcost(i) += check_real(-dp[i] / p);
        }
    }

    //rescale
     cost *= corpus_weight;
    dcost *= corpus_weight;

#ifdef SHRINK_BASIS
    //add basis entropy
    LA::VNbhd1 nbhd = L.calc_dH();
     cost += nbhd.val;
    dcost += nbhd.grad;
#endif

    logger.debug() << "P_app = "    << L.P_app()
                   << ", P_comp = " << L.P_comp()
                   << ", P_join = " << L.P_join()
                   << ", cost = "   << cost |0;
}
void CorpusCost::eval_metric (const Vect& p)
{
    logger.debug() << "evaluating metric of language cost" |0;
    Logging::IndentBlock block(logger.at_debug());

    using namespace Languages;
    Assert2(m_dim == L.vect_size(), "LangFitness's Lang has wrong dimension");

    //return values
    Float& cost = m_f;
    Vect& dcost = m_grad;
    Mat& ddcost = m_metric;

    //update complexity
    L.from_vect(p);
    calc_dZ(L, dcomp1, initialized1);
    initialized1 = true;

    //evaluate (-log of) polynomial
      cost = 0.0;
     dcost = 0.0;
    ddcost = 0.0;

    //  portion due to apps, comps, and joins
    Float P_app  = L.get_P_app();
    Float P_comp = L.get_P_comp();
    Float P_join = L.get_P_join();

    cost += -log(P_app)  * C.apps();
    cost += -log(P_comp) * C.comps();
    cost += -log(P_join) * C.joins();

    dcost(i_app)  += -(1.0 / P_app)  * C.apps();
    dcost(i_comp) += -(1.0 / P_comp) * C.comps();
    dcost(i_join) += -(1.0 / P_join) * C.joins();

    ddcost(i_app, i_app)  += sqr(1.0 / P_app)  * C.apps();
    ddcost(i_comp,i_comp) += sqr(1.0 / P_comp) * C.comps();
    ddcost(i_join,i_join) += sqr(1.0 / P_join) * C.joins();

    //  portion due to terms in corpus
    Vect dE(m_dim);
    for (Int c=0; c<C.size(); ++c) {
        Ob ob = C.ob(c);
        Float num = C.num(c);

        Float p = comp(ob); Assert (p>0, "nonpositive mass in corpus: " << p);
        Float E = check_nonneg(-log(p));
        dE = dcomp1(ob); dE /= -p;   //dE = d(-log p) = -dp / p

        cost += num * E;
        for (Int i=0; i<m_dim; ++i) {
            dcost(i) += num * dE(i);
            for (Int j=0; j<m_dim; ++j) {
                ddcost(i,j) += num * dE(i) * dE(j);
                //Note: we ignore ddE(i,j) terms
            }
        }
    }

    //rescale
      cost *= corpus_weight;
     dcost *= corpus_weight;
    ddcost *= corpus_weight;

#ifdef SHRINK_BASIS
    //add basis entropy
    //Note: The Hessian is negative definite, and entropy is lowest
    //  when the gradient is _large_ (not small, as in Newton's method).
    //  Thus we use -Hessian for the metric.
    LA::VNbhd2 nbhd = L.calc_ddH();
      cost += nbhd.val;
     dcost += nbhd.grad;
    ddcost -= nbhd.hess;
#endif

    logger.debug() << "P_app = "    << L.P_app()
                   << ", P_comp = " << L.P_comp()
                   << ", P_join = " << L.P_join()
                   << ", cost = "   << cost |0;
}

//constrained objective function
class LangFitness
{
protected:
    Lang &L;
    const ObPoly& C;
    OP::FunOfPMF m_fun;
    Float x_tol;
public:
    LangFitness (Lang& L_, const ObPoly& C_, Float elegance, Float _x_tol=1e-5)
        : L(L_), C(C_),
          m_fun(new CorpusCost(L,C,elegance)),
          x_tol(_x_tol)
    {}

    //interface as optimization strategy
    //WARNING: objective function cannot be used after changing L's basis
public:
    OP::Status optimize_locally ();
    bool try_to_retract (Ob avoid); //true on success
    Ob try_to_extend ();            //extended on success
};
OP::Status LangFitness::optimize_locally ()
{
    Vect p = L.to_vect();
    Vect x = m_fun.proj_val(p);

#ifdef NEWTONS_METHOD
    OP::Status status = OP::newton_over_simplex(x, m_fun, x_tol);
#else
    OP::Status status = OP::cg_over_simplex(x, m_fun, x_tol);
#endif
    logger.debug() << "projector called "
        << m_fun.calls() << " fun0's, "
        << m_fun.calls1() << " fun1's, and "
        << m_fun.calls2() << " fun2's" |0;

    p = m_fun.lift_val(x);
    L.from_vect(p);
    L.update();
    return status;
}
bool LangFitness::try_to_retract (Ob avoid)
{
    logger.info() << "retracting language " |0;
    std::vector<Ob> pruned = L.prune_below(x_tol);
    if (pruned.empty() and L.prune_smallest() == avoid) return false;
    if (pruned.size() == 1 and pruned[0] == avoid)      return false;
    return true;
}
Ob LangFitness::try_to_extend ()
{
    //find best extension candidate
    Meas rho = Meas::alloc();
    calc_rho(L,C,rho); //ob relevance
    Ob best_ob(0);
    Float best_rho = -INFINITY;
    for iter_over_Obs { Ob ob = *iter;
        if (L.contains(ObHdl(ob))) continue;
        if (rho(ob) > best_rho) {
            best_ob = ob;
            best_rho = rho(ob);
        }
    }
    rho.free();
    if (not best_ob) {
        logger.warning() << "no ob was found" |0;
        return Ob(0);
    }

    //decide whether to extend
    TODO();
    //best_rho *= corpus_weight;
    Float Delta = m_fun.value();
    Float delta = best_rho - Delta;
    if (delta <= 0) {
        logger.info() << "nothing worth extending: best rho = " << best_rho |0;
        return Ob(0);
    }

    //decide how far to extend
    Float weight = exp(-delta -1.0);
    weight = max(L.to_vect().min(), weight);
    weight = min(exp(-L.entropy()), weight);
    logger.info() << "extending basis with [" << best_ob << "], delta = "
        << delta |0;
    L.insert(ObHdl(best_ob), weight);
    return best_ob;
}

//language optimization interface
Float get_cost_of_poly (Lang& L, const ObPoly& C, Float e)
{
    return CorpusCost(L,C,e)(L.to_vect());
}
const Int max_changes = 32;
void fit_lang_to_obs (Lang& L, const ObPoly& C, Float e)
{
    logger.info() << "fitting language to corpus, elegance = " << e |0;
    Logging::IndentBlock block;

    Ob extended(0); //to avoid extend-retract loops
    Int changes;    //to avoid cycling
    for (changes = 0; changes < max_changes; ++changes) {

        //optimize locally
        LangFitness optimizer(L,C,e);
        OP::Status status = optimizer.optimize_locally();

        if (status == OP::BARRIER) {
            //check for retraction
            if (optimizer.try_to_retract(extended)) continue;
            else break;
        }
        if (status != OP::CONVERGE) {
            logger.warning() << "language optimizer failed to converge" |0;
            break;
        }

#ifdef EXTEND_BASIS
        //try to extend basis
        extended = optimizer.try_to_extend();
        if (not extended) break;
#else
        break;
#endif
    }
    logger.info() << "final result after " << changes << " basis changes:\n\t"
        << L.to_vect() |0;
}

//================ general measure operations ================

Double calc_entropy (Meas mu)
{
    Double H = 0.0;
    for iter_over_Obs { Ob ob = *iter;
        H += entropy_term(mu(ob));
    }
    return H;
}
Double calc_relentropy (Meas mu0, Meas mu)
{
    Double D = 0.0;
    for iter_over_Obs { Ob ob = *iter;
        D += relentropy_term(mu0(ob), mu(ob));
    }
    return D;
}

void normalize (Meas mu, Meas result, Double total)
{
    Double my_total = 0.0;
    for iter_over_Obs { Ob ob = *iter;
        my_total += mu(ob);
    }
    if (my_total > 0) {
        Float factor = total / my_total;
        for iter_over_Obs { Ob ob = *iter;
            result(ob) = factor * mu(ob);
        }
    } else {
        logger.warning() << "total was zero; normalizing to uniform" |0;;
        Float mass(total / Ob::size());
        for iter_over_Obs { Ob ob = *iter;
            result(ob) = mass;
        }
    }
}
void normalize2 (Meas mu, Meas result, Double total)
{
    Double my_total = 0.0;
    for iter_over_Obs { Ob ob = *iter;
        my_total += sqr(mu(ob));
    }
    my_total = sqrt(my_total);

    if (my_total > 0) {
        Float factor = total / my_total;
        for iter_over_Obs { Ob ob = *iter;
            result(ob) = factor * mu(ob);
        }
    } else {
        logger.warning() << "total was zero; normalizing to uniform" |0;;
        Float mass = sqrt(total / Ob::size());
        for iter_over_Obs { Ob ob = *iter;
            result(ob) = mass;
        }
    }
}
Float dot (Meas mu, Meas nu)
{
    Double ip = 0.0;
    for iter_over_Obs { Ob ob = *iter;
        ip += mu(ob) * nu(ob);
    }
    return ip;
}
void calc_power (Meas mu, Float p, Meas mu_p)
{
    for iter_over_Obs { Ob ob = *iter;
        mu_p(ob) = powf(mu(ob), p);
    }
}
void calc_weighted_sum (Float c1, Meas mu1, Float c2, Meas mu2, Meas mu12)
{
    for iter_over_Obs { Ob ob = *iter;
        mu12(ob) = c1 * mu1(ob) + c2 * mu2(ob);
    }
}

#undef iter_over_Obs
#undef iter_over_Apps
#undef iter_over_Basis
#undef stepsize_loop

}



