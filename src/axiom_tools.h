#ifndef JOHANN_AXIOM_TOOLS_H
#define JOHANN_AXIOM_TOOLS_H

#include "lambda_theories.h"
#include "obs.h"
#include "apply.h"
#include "compose.h"
#include "join.h"
#include "combinatory_structure.h"
#include "expressions.h"

namespace LambdaTheories
{

namespace O  = Obs;
namespace AE = Apply;
namespace CE = Compose;
namespace JE = JoinEqn;
namespace OR = Order;
namespace CS = CombinatoryStructure;

//lookup
using AE::find_app;
using AE::find_ldiv;
using AE::find_rdiv;

using CE::find_comp;
using CE::find_linv_comp;
using CE::find_rinv_comp;

using JE::find_join;

//iterators
using AE::Lra_Iterator;
using AE::Rla_Iterator;
using AE::Alr_Iterator;
using AE::ALr_Iterator;
using AE::ARl_Iterator;
using AE::LLra_Iterator;
using AE::LRxa_Iterator;
using AE::RRla_Iterator;

using CE::Lrc_Iterator;
using CE::Rlc_Iterator;
using CE::Clr_Iterator;
using CE::CLr_Iterator;
using CE::CRl_Iterator;
using CE::LLrc_Iterator;
using CE::LRxc_Iterator;
using CE::RRlc_Iterator;

using JE::Lrj_Iterator;
using JE::Jlr_Iterator;
using JE::JLr_Iterator;
using JE::LLrj_Iterator;

using OR::LRpos; using OR::LRneg; using OR::RLpos; using OR::RLneg;
using OR::NEG; using OR::POS;

//assumption
using CS::ensure_equiv;
using CS::ensure_less;
using CS::ensure_nless;
using CS::ensure_app;
using CS::ensure_comp;
using CS::ensure_join;

//vector order
typedef nonstd::dense_set Set;
void ensure_less (const Set& xs, Ob y);
void ensure_less (Ob x, const Set& ys);
void ensure_nless (const Set& xs, Ob y);
void ensure_nless (Ob x, const Set& ys);

//multiple structure
bool ensure_apps      (Ob L1, Ob R1, Ob L2, Ob R2);
bool ensure_comps     (Ob L1, Ob R1, Ob L2, Ob R2);
bool ensure_joins     (Ob L1, Ob R1, Ob L2, Ob R2);
bool ensure_app_comp  (Ob L1, Ob R1, Ob L2, Ob R2);
bool ensure_app_join  (Ob L1, Ob R1, Ob L2, Ob R2);
bool ensure_join_comp (Ob L1, Ob R1, Ob L2, Ob R2);

//predicates
inline bool is_less (Ob x, Ob y) { return CS::isLessThan(x,y); }
inline bool is_nless (Ob x, Ob y) { return CS::isNLessThan(x,y); }
inline bool is_const (Ob f) { return f == find_comp(f,*Atoms::Top); }

//convergence
inline bool ensure_conv (Ob x) { return ensure_nless(x, *Atoms::Bot); }
inline bool converges   (Ob x) { return     is_nless(x, *Atoms::Bot); }

//funs_of
//example:
// funs_of(a).rapp(x).rcomp(y) returns all f such that the following exist:
//   f*a, f T, f x, f*y
struct funs_of
{
    Set& fs;
    funs_of (Ob a) : fs(OR::temp_set())
    {
        fs.set_insn(CE::Rx_support(a),              //composed with a type
                    AE::Rx_support(*Atoms::Top));   //applied to T
    }
    funs_of& rapp  (Ob rhs) { fs *= AE::Rx_support(rhs); return *this; }
    funs_of& lapp  (Ob lhs) { fs *= AE::Lx_support(lhs); return *this; }
    funs_of& rcomp (Ob rhs) { fs *= CE::Rx_support(rhs); return *this; }
    operator Set& () { return fs; }
};

//types with finite domains
inline const Set& get_funs_of_div ()
{
    return funs_of(*Atoms::Div).rapp(*Atoms::Bot);
}
inline const Set& get_funs_of_unit ()
{
    return funs_of(*Atoms::Unit).rapp(*Atoms::I);
}
inline const Set& get_funs_of_semi ()
{
    return funs_of(*Atoms::Semi).rapp(*Atoms::Bot)
                                .rapp(*Atoms::I);
}
inline const Set& get_funs_of_bool ()
{
    return funs_of(*Atoms::Bool).rapp(*Atoms::Bot)
                                .rapp(*Atoms::K)
                                .rapp(*Atoms::KI);
}
inline const Set& get_funs_of_maybe ()
{
    return funs_of(*Atoms::Maybe).rapp(*Atoms::Bot)
                                 .rapp(*Atoms::K)     // = None
                                 .rcomp(*Atoms::Inr); // = Some
}
inline const Set& get_funs_of_sum ()
{
    return funs_of(*Atoms::Sum).rapp(*Atoms::Bot)
                               .rcomp(*Atoms::Inl)
                               .rcomp(*Atoms::Inr);
}
inline const Set& get_funs_of_sset ()
{
    return funs_of(*Atoms::Sset).rcomp(*Atoms::CI)
                                .lapp(*Atoms::J2);
}

}

#endif
