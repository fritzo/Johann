#ifndef JOHANN_COMPLEXITY_H
#define JOHANN_COMPLEXITY_H

#include "definitions.h"
#include "symbols.h"
#include "measures.h"
#include "languages.h"
#include "linalg.h"
#include "moments.h"
#include "fuzzy.h"
#include "nodes.h"
#include "order.h"
#include "thought.h"
#include <vector>
#include <utility>

/** \namespace Complexity \brief
 * see
 *   <a href="../doc/algorithms/complexity.html">algorithms/complexity</a>
 * and
 *   complexity.h, complexity.C.
 */

namespace Complexity
{

const Logging::Logger logger("cplxty", Logging::DEBUG);

//gobal parameters
void set_tolerance (Float tol);
Float get_tolerance ();

//datatypes
using Thought::Interest;
using LinAlg::Tensor;
using LinAlg::Vect;
typedef VectMeas VMeas;
typedef Fuzzy::Order FOrder;
typedef std::vector<Order::Rel> Rels;

//================ complexity & relevance ================

//access
Float get_ZA();
Float get_HA();
Float get_D ();
Float get_R_term ();

//moment computations, some only partially implemented
void calc_Z   (Lang& L); //complexity Z
void calc_dZ  (Lang& L, VMeas dcomp, bool initialized=false); // dZ/dL
void calc_ddZ (Lang& L, VMeas dcomp, VMeas ddcomp, bool initialized=false);
void calc_dZ  (Lang& L, const Vect& dL, Meas dcomp, bool init=false); //dir'l
void calc_ZHA (Lang& L); //complexity & entropy of database
void calc_rho (Lang& L, const ObPoly& C, Meas rho); //relevance, sets rel
void calc_R   (Lang& L, Interest& inst); //very basic relevance, sets rel
void calc_P   (Lang& L, FOrder& fail); //rule propagation
void calc_Q   (Meas q);

//info geometry
Tensor calc_g  (VMeas dcomp);                                   //metric
Tensor calc_G  (VMeas dcomp, VMeas ddcomp, const Tensor& g);    //christoffel
Tensor calc_T  (VMeas dcomp, const Tensor& g);                  //skewness
Tensor calc_Ga (const Tensor& G, const Tensor& T, Float alpha); //alpha-con'n

//language calibration
Float get_cost_of_poly (Lang& L, const ObPoly& C, Float elegance=0.0);
void fit_lang_to_obs   (Lang& L, const ObPoly& C, Float elegance=0.0);

//perturbation
Vect calc_coords (Lang& L, VMeas coords); //returns eigenvalues

//logging
void write_stats_to (ostream& os);

//================ low-level calculations ================

//plausibility operations
void collect_mass_over (Ob x, Meas over_mu);
void collect_mass_over (Meas mu, Meas over_mu);
void remove (Meas mu, Ob rem, Meas result);
inline void remove (Meas mu, Ob rem) { remove(mu,rem,mu); }
void specify (Meas mu, Int max_steps = 10);

//general measure operations
Double calc_entropy (Meas mu);
Double calc_relentropy (Meas mu0, Meas mu);

void normalize (Meas mu, Meas result, Double total=1.0);
void normalize2 (Meas mu, Meas result, Double total=1.0);
Float dot (Meas mu, Meas nu);
void calc_power (Meas mu, Float p, Meas mu_p);
void calc_weighted_sum (Float c1, Meas mu1, Float c2, Meas mu2, Meas mu12);

void restrict_to_type (Meas mu, Ob type, Meas mu_type);

}

#endif
