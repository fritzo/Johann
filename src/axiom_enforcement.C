
#include "axiom_tools.h"

//log levels
#define LOG_DEBUG1(mess);
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << mess |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

#ifdef PROFILE
#define inline
#endif

//TODO add axioms for various finite domains a:V
//  f=f*a   g=g*a   /\x:a. f x[=g x      f=f*div   g=g*div   f _[=g _   f T[=g T
//  -------------------------------  eg  ---------------------------------------
//             f [= g                                   f [= g
//TODO add axioms for maybe, sum, and sset
//TODO avoid iterating over K x when possible.
//  TODO define is_const(x) <==> x = K(x T)
//    is it guarangeed that f _=f T ==> f=K(f T) ?  (it empirically seems so)
//TODO add axioms for subtyping schemata
//  a<:b   f*b[=g*b
//  ---------------
//     f*a[=g*a
//TODO Add axioms for: S x y z = x*z y z, Y f = f*Y f
//TODO Experiment with axiom PhiB x y z = (x z)*(y z)
//TODO close axiom schemata under permutation
//  (see notes/reflection.text (2008:11:07-13) (Q3.N1))

namespace LambdaTheories
{

//axiom scheme declarations, see below for naming conventions

//lattice theory
inline void enforce_A_mu_L (App eqn);
inline void enforce_A_nu_L (App eqn);
inline void enforce_A_mu_N (App eqn);
inline void enforce_A_nu_N (App eqn);
inline void enforce_C_mu_L (Comp eqn);
inline void enforce_C_nu_L (Comp eqn);
inline void enforce_C_mu_N (Comp eqn);
inline void enforce_C_nu_N (Comp eqn);
inline void enforce_L_mu_L (Ord ord);
inline void enforce_L_nu_L (Ord ord);
inline void enforce_N_mono_N (Ord ord);

inline void enforce_O_ord_L   (Ob ob);
inline void enforce_L_ord_pos (Ord ord);
inline void enforce_L_ord_neg (Ord ord);
inline void enforce_N_ord_neg (Ord ord);

inline void enforce_O_Bot_L (Ob ob);
inline void enforce_A_Bot_N (App eqn);
inline void enforce_N_Bot_N (Ord ord);

inline void enforce_O_Top_L (Ob ob);
inline void enforce_A_Top_N (App eqn);
inline void enforce_N_Top_N (Ord ord);

inline void enforce_L_join_pos (Ord ord);
inline void enforce_A_join_pos (App eqn);
inline void enforce_C_join_pos (Comp eqn);
inline void enforce_J_join_pos (Join eqn);
inline void enforce_J_Join_pos (Join eqn);
inline void enforce_A_Join_pos (App eqn);

inline void enforce_C_comp_assoc_C (Comp eqn);
inline void enforce_C_comp_app_A (Comp eqn);
inline void enforce_A_comp_app_A (App eqn);

inline void enforce_A_Rand_pos (App eqn);
inline void enforce_L_Rand_pos (Ord ord);

//lambda theory
inline void enforce_O_I_pos (Ob ob);
inline void enforce_O_K_pos (Ob ob);
inline void enforce_A_K_pos (App eqn);
inline void enforce_C_K_pos (Comp eqn);
inline void enforce_A_C_A (App eqn);
inline void enforce_A_W_A (App eqn);
inline void enforce_A_S_A (App eqn);

//extension theory
inline void enforce_A_compose_pos (App eqn);
inline void enforce_C_compose_pos (Comp eqn);

inline void enforce_A_Y_A   (App eqn);
inline void enforce_A_Y_L   (App eqn);
inline void enforce_L_Y_pos (Ord ord);

inline void enforce_A_U_pos (App eqn);
inline void enforce_C_U_pos (Comp eqn);
inline void enforce_L_U_pos (Ord ord);
inline void enforce_A_V_pos (App eqn);
inline void enforce_C_V_pos (Comp eqn);
inline void enforce_L_V_pos (Ord ord);
inline void enforce_A_P_A   (App eqn);

//typed theory, disjunctive axioms
inline void enforce_A_Div_pos (App eqn);
inline void enforce_C_Div_pos (Comp eqn);
inline void enforce_L_Div_pos (Ord ord);
inline void enforce_N_Div_pos (Ord ord);

inline void enforce_A_Unit_pos (App eqn);
inline void enforce_C_Unit_pos (Comp eqn);
inline void enforce_L_Unit_pos (Ord ord);
inline void enforce_N_Unit_pos (Ord ord);

inline void enforce_A_Semi_pos (App eqn);
inline void enforce_C_Semi_pos (Comp eqn);
inline void enforce_L_Semi_pos (Ord ord);
inline void enforce_N_Semi_pos (Ord ord);

inline void enforce_A_Bool_pos (App eqn);
inline void enforce_C_Bool_pos (Comp eqn);
inline void enforce_L_Bool_pos (Ord ord);
inline void enforce_N_Bool_pos (Ord ord);

//================ enforcement organized by theory ================

//order-theoretic lattice theory
//  simple reduction axioms
void LatticeTheory::enforce_O (Ob ob) const
{
    LOG_DEBUG1( "enforcing O" );
    LOG_INDENT_DEBUG1

    enforce_O_ord_L(ob);
    enforce_O_Bot_L(ob);
    enforce_O_Top_L(ob);
}
void LatticeTheory::enforce_A (App eqn) const
{
    LOG_DEBUG1( "enforcing A" );
    LOG_INDENT_DEBUG1

    enforce_A_Bot_N(eqn);
    enforce_A_Top_N(eqn);

    enforce_A_mu_L(eqn);
    enforce_A_nu_L(eqn);
    enforce_A_mu_N(eqn);
    enforce_A_nu_N(eqn);

    //DEBUG
    enforce_A_join_pos(eqn);
    enforce_A_Join_pos(eqn);

    if (R) {
        enforce_A_Rand_pos(eqn);
        //distributivity...
    }

    enforce_A_comp_app_A (eqn);
}
void LatticeTheory::enforce_C (Comp eqn) const
{
    LOG_DEBUG1( "enforcing C" );
    LOG_INDENT_DEBUG1

    enforce_C_mu_L(eqn);
    enforce_C_nu_L(eqn);
    enforce_C_mu_N(eqn);
    enforce_C_nu_N(eqn);

    //DEBUG
    enforce_C_join_pos(eqn);
    enforce_C_comp_assoc_C(eqn);
    enforce_C_comp_app_A(eqn);
}
void LatticeTheory::enforce_J (Join eqn) const
{
    LOG_DEBUG1( "enforcing J" );
    LOG_INDENT_DEBUG1

    //DEBUG
    enforce_J_join_pos(eqn);
    enforce_J_Join_pos(eqn);
}
//  order axioms
void LatticeTheory::enforce_L (Ord ord) const
{
    LOG_DEBUG1( "enforcing L" );
    LOG_INDENT_DEBUG1

    Assert3(OR::contains_pos(ord),
            "tried to enforce order axiom on non-ordered obs");
    enforce_L_ord_pos(ord);
    enforce_L_ord_neg(ord);
    enforce_L_mu_L(ord);
    enforce_L_nu_L(ord);

    //DEBUG
    enforce_L_join_pos(ord);

    if (R) enforce_L_Rand_pos(ord);
}
void LatticeTheory::enforce_N (Ord ord) const
{
    LOG_DEBUG1( "enforcing N_neg" );
    LOG_INDENT_DEBUG1

    Assert3(OR::contains_neg(ord),
            "tried to enforce order axiom on non-ordered obs");

    enforce_N_mono_N(ord);
    enforce_N_ord_neg(ord);
    enforce_N_Bot_N(ord);
    enforce_N_Top_N(ord);
}

//lambda-theory with variables & abstraction
void LambdaTheory::enforce_O (Ob ob) const
{
    LatticeTheory::enforce_O(ob);
    LOG_INDENT_DEBUG1

    enforce_O_I_pos(ob);
    enforce_O_K_pos(ob);
}
void LambdaTheory::enforce_A (App eqn) const
{
    LatticeTheory::enforce_A(eqn);
    LOG_INDENT_DEBUG1

    enforce_A_K_pos(eqn);
    enforce_A_C_A(eqn);
    enforce_A_W_A(eqn);
    enforce_A_S_A(eqn);
}
void LambdaTheory::enforce_C (Comp eqn) const
{
    LatticeTheory::enforce_C(eqn);
    LOG_INDENT_DEBUG1

    enforce_C_K_pos(eqn);
}

//extension-theory
void ExtnTheory::enforce_A (App eqn) const
{//additional bot axiom
    LambdaTheory::enforce_A(eqn);
    LOG_INDENT_DEBUG1

    enforce_A_compose_pos(eqn);

    enforce_A_Y_A(eqn);
    enforce_A_Y_L(eqn);

    if (U) enforce_A_U_pos(eqn);
    if (V) enforce_A_V_pos(eqn);
    if (P) enforce_A_P_A(eqn);
}
void ExtnTheory::enforce_C (Comp eqn) const
{
    LambdaTheory::enforce_C(eqn);
    LOG_INDENT_DEBUG1

    enforce_C_compose_pos(eqn);

    if (U) enforce_C_U_pos(eqn);
    if (V) enforce_C_V_pos(eqn);
}
void ExtnTheory::enforce_L (Ord ord) const
{
    LatticeTheory::enforce_L(ord);
    LOG_INDENT_DEBUG1

    enforce_L_Y_pos(ord);
    if (U) enforce_L_U_pos(ord);
    if (V) enforce_L_V_pos(ord);
}

//typed theory
void TypedTheory::enforce_A (App eqn) const
{
    ExtnTheory::enforce_A(eqn);
    LOG_INDENT_DEBUG1

    if (Div)  enforce_A_Div_pos(eqn);
    if (Unit) enforce_A_Unit_pos(eqn);
    if (Semi) enforce_A_Semi_pos(eqn);
    if (Bool) enforce_A_Bool_pos(eqn);
}
void TypedTheory::enforce_C (Comp eqn) const
{
    ExtnTheory::enforce_C(eqn);
    LOG_INDENT_DEBUG1

    if (Div)  enforce_C_Div_pos(eqn);
    if (Unit) enforce_C_Unit_pos(eqn);
    if (Semi) enforce_C_Semi_pos(eqn);
    if (Bool) enforce_C_Bool_pos(eqn);
}
void TypedTheory::enforce_L (Ord ord) const
{
    ExtnTheory::enforce_L(ord);
    LOG_INDENT_DEBUG1

    if (Div)  enforce_L_Div_pos(ord);
    if (Unit) enforce_L_Unit_pos(ord);
    if (Semi) enforce_L_Semi_pos(ord);
    if (Bool) enforce_L_Bool_pos(ord);
}
void TypedTheory::enforce_N (Ord ord) const
{
    LatticeTheory::enforce_N(ord);
    LOG_INDENT_DEBUG1

    if (Div)  enforce_N_Div_pos(ord);
    if (Unit) enforce_N_Unit_pos(ord);
    if (Semi) enforce_N_Semi_pos(ord);
    if (Bool) enforce_N_Bool_pos(ord);
}

/** Axiom Schemata
 * Naming convention:
 *      enforce_<type of node>_<description>_<conclusion type><number>
 *  where
 *      type of node:
 *          A -- apply equation
 *          C -- compose equation
 *          J -- join equation
 *          L -- order
 *          N -- negated order
 *      description: often just required structure
 *          app -- an application
 *          compose -- a composition
 *          join -- a join equation
 *          S, K, I, B, C, Y, Bot,... -- a particular atom
 *      conclusion type:
 *          E -- an equation "x==y"
 *          L -- an ordering "x[=y"
 *          N -- a negated ordering "x![=y"
 *      number:
 *          0-9 -- for schemes requiring compound structure
 *
 * Title comment conventions:
 *      <new structure> | <existing structure> , ... , <existing structure>
 *  where a structure is one of
 *       x[=y -- an ordering
 *      x![=y -- a negated ordering
 *         xy -- an application
 *        x*y -- a composition
 *        x|y -- a join
 */

//================ general axioms ================

//axiom A_idempotence_A: Axx=x
inline void enforce_A_idempotence_A (Ob A, App eqn)
{// Ax=A*x
    if (get_lhs(eqn) != A) return;
    Ob x  = get_rhs(eqn);
    Ob Ax = get_app(eqn);
    LOG_DEBUG1( "enforcing A_idempotence_A" );
    ensure_app (x, Ax,x);
}
//axiom A_idempotence_A: Axx=Bx (relative)
void enforce_A_idempotence_A1 (Ob A, Ob B, App eqn)
{// Ax=A*x | Axx=Ax*x or Bx=B*x
    if (get_lhs(eqn) != A) return;
    Ob x  = get_rhs(eqn);
    Ob Ax = get_app(eqn);
    LOG_DEBUG1( "enforcing A_idempotence_A1 (relative)" );
    ensure_apps (B,x, Ax,x);
}
void enforce_A_idempotence_A2 (Ob A, Ob B, App eqn)
{// Bx=B*x | Ax=A*x
    if (get_lhs(eqn) != B) return;
    Ob x  = get_rhs(eqn);
    Ob Ax = find_app(A,x); if (!Ax) return;
    Ob Bx = get_app(eqn);
    LOG_DEBUG1( "enforcing A_idempotence_A2 (relative)" );
    ensure_app (Bx, Ax,x);
}
void enforce_A_idempotence_A3 (Ob A, Ob B, App eqn)
{// Axx=Ax*x | Ax=A*x
    Ob x   = get_rhs(eqn);
    Ob Ax  = get_lhs(eqn); if (find_app(A,x) != Ax) return;
    Ob Axx = get_app(eqn);
    LOG_DEBUG1( "enforcing A_idempotence_A3 (relative)" );
    ensure_app (Axx, B,x);
}
void enforce_A_idempotence_A (Ob A, Ob B, App eqn)
{
    enforce_A_idempotence_A1(A,B,eqn);
    enforce_A_idempotence_A2(A,B,eqn);
    enforce_A_idempotence_A3(A,B,eqn);
}

//axiom A_commutativity_A: Axy=Ayx  (i.e., symmetry)
inline void enforce_A_commutativity_A1 (Ob A, App eqn)
{// Axy=Ax*y | Ax=A*x, Ay=A*y
    Ob Ax  = get_lhs(eqn);
    //here we assume that A injects
    Ob x   = find_ldiv(A,Ax); if (!x) return;
    Ob y   = get_rhs(eqn);
    Ob Ay  = find_app(A,y); if (!Ay) return;
    Ob Axy = get_app(eqn);
    LOG_DEBUG1( "enforcing A_commutativity_A1" );
    ensure_app(Axy, Ay,x);
}
inline void enforce_A_commutativity_A2 (Ob A, App eqn)
{// Ax=A*x | Ay=A*y, Axy=Ax*y
    if (get_lhs(eqn) != A) return;
    Ob x  = get_rhs(eqn);
    Ob Ax = get_app(eqn);

    for (Lra_Iterator Axy_iter(Ax); Axy_iter.ok(); Axy_iter.next()) {
        Ob y   = Axy_iter.rhs();
        Ob Ay  = find_app(A,y); if (!Ay) continue;
        Ob Axy = Axy_iter.app();
        LOG_DEBUG1( "enforcing A_commutativity_A2" );
        ensure_app(Axy, Ay,x);
    }
}
inline void enforce_A_commutativity_A3 (Ob A, App eqn)
{// Ay=A*y | Ax=A*x, Axy=Ax*y,
    if (get_lhs(eqn) != A) return;
    Ob Ay = get_app(eqn);
    Ob y  = get_rhs(eqn);
    for (Lra_Iterator Ax_iter(A); Ax_iter.ok(); Ax_iter.next()) {
        Ob Ax  = Ax_iter.app();
        Ob Axy = find_app(Ax,y); if (!Axy) continue;
        Ob x   = Ax_iter.rhs();
        LOG_DEBUG1( "enforcing A_commutativity_A3" );
        ensure_app(Axy, Ay,x);
    }
}
void enforce_A_commutativity_A (Ob A, App eqn)
{
    //Assert5(O::injects(A), "commutativity axiom assumes injectivity");
    enforce_A_commutativity_A1(A, eqn);
    enforce_A_commutativity_A2(A, eqn);
    enforce_A_commutativity_A3(A, eqn);
}

//axiom A_associativity_A: Ax(Ayz)=A(Axy)z
inline void enforce_A_associativity_A1 (Ob A, App eqn)
{//Ax=A*x | Ax_Ayz=Ax*Ayz,A_Axy_z=A_Axy*z,A_Axy=A*Axy,Ayz=Ay*z,Ay=A*y,Axy=Ax*y
    if (get_lhs(eqn) != A) return;
    Ob Ax = get_app(eqn);

    Lra_Iterator Ayz_iter; //for below

    for (Lra_Iterator Axy_iter(Ax); Axy_iter.ok(); Axy_iter.next()) {
        Ob Axy   = Axy_iter.app();
        Ob A_Axy = find_app(A, Axy); if (!A_Axy) continue;
        Ob y     = Axy_iter.rhs();
        Ob Ay    = find_app(A, y); if (!Ay) continue;

        for (Ayz_iter.begin(Ay); Ayz_iter.ok(); Ayz_iter.next()) {
            Ob z      = Ayz_iter.rhs();
            Ob Ayz    = Ayz_iter.app();
            Ob Ax_Ayz = find_app(Ax, Ayz);

            if (Ax_Ayz) {
                LOG_DEBUG1( "enforcing A_associtivity_E1a" );
                ensure_app(Ax_Ayz, A_Axy,z);
            } else {
                Ob A_Axy_z = find_app(A_Axy,z); if (!A_Axy_z) continue;
                LOG_DEBUG1( "enforcing A_associtivity_E1b" );
                ensure_app(A_Axy_z, Ax,Ayz);
            }
        }
    }
}
inline void enforce_A_associativity_A2 (Ob A, App eqn)
{//Axy=Ax*y | Ax_Ayz=Ax*Ayz,A_Axy_z=A_Axy*z,A_Axy=A*Axy,Ayz=Ay*z,Ay=A*y,Ax=A*x
    Ob Ax    = get_lhs(eqn);
    //here we assume that A injects
    Ob x     = find_ldiv(A, Ax); if (!x) return;
    Ob Axy   = get_app(eqn);
    Ob A_Axy = find_app(A, Axy); if (!A_Axy) return;
    Ob y     = get_rhs(eqn);
    Ob Ay    = find_app(A, y); if (!Ay) return;

    for (Lra_Iterator Ayz_iter(Ay); Ayz_iter.ok(); Ayz_iter.next()) {
        Ob z      = Ayz_iter.rhs();
        Ob Ayz    = Ayz_iter.app();
        Ob Ax_Ayz = find_app(Ax, Ayz);

        if (Ax_Ayz) {
            LOG_DEBUG1( "enforcing A_associtivity_E2a" );
            ensure_app(Ax_Ayz, A_Axy,z);
        } else {
            Ob A_Axy_z = find_app(A_Axy,z); if (!A_Axy_z) continue;
            LOG_DEBUG1( "enforcing A_associtivity_E2b" );
            ensure_app(A_Axy_z, Ax,Ayz);
        }
    }
}
inline void enforce_A_associativity_A3 (Ob A, App eqn)
{//Ay=A*y | Ax_Ayz=Ax*Ayz,A_Axy_z=A_Axy*z,A_Axy=A*Axy,Ayz=Ay*z,Axy=Ax*y,Ax=A*x
    if (get_lhs(eqn) != A) return;
    Ob y  = get_rhs(eqn);
    Ob Ay = get_app(eqn);

    Lra_Iterator Ayz_iter(Ay); if (!Ayz_iter.ok()) return; //for below

    for (Rla_Iterator Axy_iter(y); Axy_iter.ok(); Axy_iter.next()) {
        Ob Axy   = Axy_iter.app();
        Ob A_Axy = find_app(A,Axy); if (!A_Axy) continue;
        Ob Ax    = Axy_iter.lhs();
        //here we assume that A injects
        Ob x     = find_ldiv(A,Ax); if (!x) continue;

        for (Ayz_iter.begin(); Ayz_iter.ok(); Ayz_iter.next()) {
            Ob z      = Ayz_iter.rhs();
            Ob Ayz    = Ayz_iter.app();
            Ob Ax_Ayz = find_app(Ax,Ayz);

            if (Ax_Ayz) {
                LOG_DEBUG1( "enforcing A_associtivity_E3a" );
                ensure_app(Ax_Ayz, A_Axy,z);
            } else {
                Ob A_Axy_z = find_app(A_Axy,z); if (!A_Axy_z) continue;
                LOG_DEBUG1( "enforcing A_associtivity_E3b" );
                ensure_app(A_Axy_z, Ax,Ayz);
            }
        }
    }
}
inline void enforce_A_associativity_A4 (Ob A, App eqn)
{//Ayz=Ay*z | Ax_Ayz=Ax*Ayz,A_Axy_z=A_Axy*z,A_Axy=A*Axy,Ay=A*y,Axy=Ax*y,Ax=A*x
    Ob Ay  = get_lhs(eqn);
    //here we assume that A injects
    Ob y   = find_ldiv(A, Ay); if (!y) return;
    Ob z   = get_rhs(eqn);
    Ob Ayz = get_app(eqn);

    for (Rla_Iterator Axy_iter(y); Axy_iter.ok(); Axy_iter.next()) {
        Ob Axy    = Axy_iter.app();
        Ob A_Axy  = find_app(A,Axy); if (!A_Axy) continue;
        Ob Ax     = Axy_iter.lhs();
        //here we assume that A injects
        Ob x      = find_ldiv(A,Ax); if (!x) continue;
        Ob Ax_Ayz = find_app(Ax,Ayz);

        if (Ax_Ayz) {
            LOG_DEBUG1( "enforcing A_associtivity_E4a" );
            ensure_app(Ax_Ayz, A_Axy,z);
        } else {
            Ob A_Axy_z = find_app(A_Axy,z); if (!A_Axy_z) continue;
            LOG_DEBUG1( "enforcing A_associtivity_E4b" );
            ensure_app(A_Axy_z, Ax,Ayz);
        }
    }
}
inline void enforce_A_associativity_A5 (Ob A, App eqn)
{//A_Axy=A*Axy | Ax_Ayz=Ax*Ayz,A_Axy_z=A_Axy*z,Ayz=Ay*z,Ay=A*y,Axy=Ax*y,Ax=A*x
    if (get_lhs(eqn) != A) return;
    Ob Axy   = get_rhs(eqn);
    Ob A_Axy = get_app(eqn);

    Lra_Iterator Ayz_iter; //for below

    for (Alr_Iterator Axy_iter(Axy); Axy_iter.ok(); Axy_iter.next()) {
        Ob y  = Axy_iter.rhs();
        Ob Ay = find_app(A,y); if (!Ay) continue;
        Ob Ax = Axy_iter.lhs();
        //here we assume that A injects
        Ob x  = find_ldiv(A, Ax); if (!x) continue;

        for (Ayz_iter.begin(Ay); Ayz_iter.ok(); Ayz_iter.next()) {
            Ob z      = Ayz_iter.rhs();
            Ob Ayz    = Ayz_iter.app();
            Ob Ax_Ayz = find_app(Ax, Ayz);

            if (Ax_Ayz) {
                LOG_DEBUG1( "enforcing A_associtivity_E5a" );
                ensure_app(Ax_Ayz, A_Axy,z);
            } else {
                Ob A_Axy_z = find_app(A_Axy,z); if (!A_Axy_z) continue;
                LOG_DEBUG1( "enforcing A_associtivity_E5b" );
                ensure_app(A_Axy_z, Ax,Ayz);
            }
        }
    }
}
inline void enforce_A_associativity_A6 (Ob A, App eqn)
{//A_Axy_z=A_Axy*z | A_Axy=A*Axy,Ayz=Ay*z,Ay=A*y,Axy=Ax*y,Ax=A*x
    Ob A_Axy   = get_lhs(eqn);
    //here we assume that A injects
    Ob Axy     = find_ldiv(A,A_Axy); if (!Axy) return;
    Ob z       = get_rhs(eqn);
    Ob A_Axy_z = get_app(eqn);

    for (Alr_Iterator Axy_iter(Axy); Axy_iter.ok(); Axy_iter.next()) {
        Ob y   = Axy_iter.rhs();
        Ob Ay  = find_app(A, y); if (!Ay) continue;
        Ob Ayz = find_app(Ay, z); if (!Ayz) continue;
        Ob Ax  = Axy_iter.lhs();
        //here we assume that A injects
        Ob x   = find_ldiv(A, Ax); if (!x) continue;
        LOG_DEBUG1( "enforcing A_associtivity_A6" );
        ensure_app(A_Axy_z, Ax,Ayz);
    }
}
inline void enforce_A_associativity_A7 (Ob A, App eqn)
{//Ax_Ayz=Ax*Ayz | A_Axy=A*Axy,Ayz=Ay*z,Ay=A*y,Axy=Ax*y,Ax=A*x
    Ob Ax     = get_lhs(eqn);
    //here we assume that A injects
    Ob x      = find_ldiv(A, Ax); if (!x) return;
    Ob Ayz    = get_rhs(eqn);
    Ob Ax_Ayz = get_app(eqn);

    for (Alr_Iterator Ayz_iter(Ayz); Ayz_iter.ok(); Ayz_iter.next()) {
        Ob Ay    = Ayz_iter.lhs();
        //here we assume that A injects
        Ob y     = find_ldiv(A, Ay); if (!y) continue;
        Ob Axy   = find_app(Ax, y);  if (!Axy) continue;
        Ob A_Axy = find_app(A, Axy); if (!A_Axy) continue;
        Ob z     = Ayz_iter.rhs();
        LOG_DEBUG1( "enforcing A_associtivity_A7" );
        ensure_app(Ax_Ayz, A_Axy,z);
    }
}
void enforce_A_associativity_A (Ob A, App eqn)
{
    enforce_A_associativity_A1(A, eqn);
    enforce_A_associativity_A2(A, eqn);
    enforce_A_associativity_A3(A, eqn);
    enforce_A_associativity_A4(A, eqn);
    enforce_A_associativity_A5(A, eqn);
    enforce_A_associativity_A6(A, eqn);
    enforce_A_associativity_A7(A, eqn);
}

//axiom A_distrib_rapply_A: Axyz=A(xz)(yz)
inline void enforce_A_distrib_rapply_A1 (Ob A, App eqn)
{// Axyz=Axy*z | Axy=Ax*y, Ax=A*x, A_xz_yz=A_xz*yz, A_xz=A*xz, xz=x*z, yz=y*z
    Ob z    = get_rhs(eqn);
    Ob Axy  = get_lhs(eqn);
    Ob Axyz = get_app(eqn);

    for (Alr_Iterator Axy_iter(Axy); Axy_iter.ok(); Axy_iter.next()) {
        Ob Ax   = Axy_iter.lhs();
        //here we assume that A injects
        Ob x    = find_ldiv(A,Ax); if (!x) continue;
        Ob xz   = find_app(x,z);   if (!xz) continue;
        Ob y    = Axy_iter.rhs();
        Ob yz   = find_app(y,z);   if (!yz) continue;
        Ob A_xz = find_app(A,xz);  if (!A_xz) continue;
        LOG_DEBUG1( "enforcing A_distrib_rapply_A1" );
        ensure_app(Axyz, A_xz,yz);
    }
}
inline void enforce_A_distrib_rapply_A2 (Ob A, App eqn)
{// Axy=Ax*y | Axyz=Axy*z, Ax=A*x, A_xz_yz=A_xz*yz, A_xz=A*xz, xz=x*z, yz=y*z
    Ob Ax  = get_lhs(eqn);
    //here we assume that A injects
    Ob x   = find_ldiv(A, Ax); if (!x) return;
    Ob y   = get_rhs(eqn);
    Ob Axy = get_app(eqn);

    for (Lra_Iterator xz_iter(x); xz_iter.ok(); xz_iter.next()) {
        Ob xz   = xz_iter.app();
        Ob A_xz = find_app(A,xz); if (!A_xz) continue;
        Ob z    = xz_iter.rhs();
        Ob yz   = find_app(y,z);  if (!yz) continue;
        Ob Axyz = find_app(Axy,z);

        if (Axyz) {
            LOG_DEBUG1( "enforcing A_distrib_rapply_E2a" );
            ensure_app(Axyz, A_xz,yz);
        } else {
            Ob A_xz_yz = find_app(A_xz,yz); if (!A_xz_yz) continue;
            LOG_DEBUG1( "enforcing A_distrib_rapply_E2b" );
            ensure_app(A_xz_yz, Axy,z);
        }
    }
}
inline void enforce_A_distrib_rapply_A3 (Ob A, App eqn)
{// Ax=A*x | Axyz=Axy*z, Axy=Ax*y, A_xz_yz=A_xz*yz, A_xz=A*xz, xz=x*z, yz=y*z
    if (get_lhs(eqn) != A) return;
    Ob x  = get_rhs(eqn);
    Ob Ax = get_app(eqn);

    Lra_Iterator xz_iter(x); if (!xz_iter.ok()) return; //for below

    for (Lra_Iterator Axy_iter(Ax); Axy_iter.ok(); Axy_iter.next()) {
        Ob y   = Axy_iter.rhs();
        Ob Axy = Axy_iter.app();

        for (xz_iter.begin(); xz_iter.ok(); xz_iter.next()) {
            Ob xz   = xz_iter.app();
            Ob A_xz = find_app(A,xz); if (!A_xz) continue;
            Ob z    = xz_iter.rhs();
            Ob yz   = find_app(y,z);  if (!yz) continue;
            Ob Axyz = find_app(Axy,z);

            if (Axyz) {
                LOG_DEBUG1( "enforcing A_distrib_rapply_E3a" );
                ensure_app(Axyz, A_xz,yz);
            } else {
                Ob A_xz_yz = find_app(A_xz,yz); if (!A_xz_yz) continue;
                LOG_DEBUG1( "enforcing A_distrib_rapply_E3b" );
                ensure_app(A_xz_yz, Axy,z);
            }
        }
    }
}
inline void enforce_A_distrib_rapply_A4 (Ob A, App eqn)
{// A_xz_yz=A_xz*yz | Axyz=Axy*z, Axy=Ax*y, Ax=A*x, A_xz=A*xz, xz=x*z, yz=y*z
    Ob A_xz    = get_lhs(eqn);
    //here we assume that A injects
    Ob xz      = find_ldiv(A,A_xz); if (!xz) return;
    Ob yz      = get_rhs(eqn);
    Ob A_xz_yz = get_app(eqn);

    ARl_Iterator yz_iter; //for below

    for (Alr_Iterator xz_iter(xz); xz_iter.ok(); xz_iter.next()) {
        Ob x  = xz_iter.lhs();
        Ob Ax = find_app(A,x); if (!Ax) continue;
        Ob z  = xz_iter.rhs();

        for (yz_iter.begin(yz,z); yz_iter.ok(); yz_iter.next()) {
            Ob y   = yz_iter.lhs();
            Ob Axy = find_app(Ax,y); if (!Axy) continue;
            LOG_DEBUG1( "enforcing A_distrib_rapply_A4" );
            ensure_app(A_xz_yz, Axy,z);
        }
    }
}
inline void enforce_A_distrib_rapply_A5 (Ob A, App eqn)
{// A_xz=A*xz | Axyz=Axy*z, Axy=Ax*y, Ax=A*x, A_xz_yz=A_xz*yz, xz=x*z, yz=y*z
    if (get_lhs(eqn) != A) return;
    Ob xz   = get_rhs(eqn);
    Ob A_xz = get_app(eqn);

    Lra_Iterator Axy_iter; //for below

    for (Alr_Iterator xz_iter(xz); xz_iter.ok(); xz_iter.next()) {
        Ob x  = xz_iter.lhs();
        Ob Ax = find_app(A,x); if (!Ax) continue;
        Ob z  = xz_iter.rhs();

        for (Axy_iter.begin(Ax); Axy_iter.ok(); Axy_iter.next()) {
            Ob y    = Axy_iter.rhs();
            Ob yz   = find_app(y,z); if (!yz) continue;
            Ob Axy  = Axy_iter.app();
            Ob Axyz = find_app(Axy,z);

            if (Axyz) {
                LOG_DEBUG1( "enforcing A_distrib_rapply_E5a" );
                ensure_app(Axyz, A_xz,yz);
            } else {
                Ob A_xz_yz = find_app(A_xz,yz); if (!A_xz_yz) continue;
                LOG_DEBUG1( "enforcing A_distrib_rapply_E5b" );
                ensure_app(A_xz_yz, Axy,z);
            }
        }
    }
}
inline void enforce_A_distrib_rapply_A6 (Ob A, App eqn)
{// xz=x*z | Axyz=Axy*z, Axy=Ax*y, Ax=A*x, A_xz_yz=A_xz*yz, A_xz=A*xz, yz=y*z
    Ob x    = get_lhs(eqn);
    Ob Ax   = find_app(A,x); if (!Ax) return;
    Ob xz   = get_app(eqn);
    Ob A_xz = find_app(A,xz); if (!A_xz) return;
    Ob z    = get_rhs(eqn);

    for (Rla_Iterator yz_iter(z); yz_iter.ok(); yz_iter.next()) {
        Ob y    = yz_iter.lhs();
        Ob Axy  = find_app(Ax,y); if (!Axy) continue;
        Ob yz   = yz_iter.app();
        Ob Axyz = find_app(Axy,z);

        if (Axyz) {
            LOG_DEBUG1( "enforcing A_distrib_rapply_E6a" );
            ensure_app(Axyz, A_xz,yz);
        } else {
            Ob A_xz_yz = find_app(A_xz,yz); if (!A_xz_yz) continue;
            LOG_DEBUG1( "enforcing A_distrib_rapply_E6b" );
            ensure_app(A_xz_yz, Axy,z);
        }
    }
}
inline void enforce_A_distrib_rapply_A7 (Ob A, App eqn)
{// yz=y*z | Axyz=Axy*z, Axy=Ax*y, Ax=A*x, A_xz_yz=A_xz*yz, A_xz=A*xz, xz=x*z
    Ob y  = get_lhs(eqn);
    Ob z  = get_rhs(eqn);
    Ob yz = get_app(eqn);

    for (Rla_Iterator xz_iter(z); xz_iter.ok(); xz_iter.next()) {
        Ob xz   = xz_iter.app();
        Ob A_xz = find_app(A,xz); if (!A_xz) continue;
        Ob x    = xz_iter.lhs();
        Ob Ax   = find_app(A,x);  if (!Ax) continue;
        Ob Axy  = find_app(Ax,y); if (!Axy) continue;
        Ob Axyz = find_app(Axy,z);

        if (Axyz) {
            LOG_DEBUG1( "enforcing A_distrib_rapply_E7a" );
            ensure_app(Axyz, A_xz,yz);
        } else {
            Ob A_xz_yz = find_app(A_xz,yz); if (!A_xz_yz) continue;
            LOG_DEBUG1( "enforcing A_distrib_rapply_E7b" );
            ensure_app(A_xz_yz, Axy,z);
        }
    }
}
void enforce_A_distrib_rapply_A (Ob A, App eqn)
{
    //Assert5(O::injects(A), "distrib_rapply axiom assumes injectivity");
    enforce_A_distrib_rapply_A1(A,eqn);
    enforce_A_distrib_rapply_A2(A,eqn);
    enforce_A_distrib_rapply_A3(A,eqn);
    enforce_A_distrib_rapply_A4(A,eqn);
    enforce_A_distrib_rapply_A5(A,eqn);
    enforce_A_distrib_rapply_A6(A,eqn);
    enforce_A_distrib_rapply_A7(A,eqn);
}

//axiom supconvex: z[=x, z[=y |- z[=Axy
inline void enforce_A_supconvexity_L1 (Ob A, App eqn)
{// Axy=Ax*y | z[=x, z[=y, Ax=A*x
    Ob Ax  = get_lhs(eqn);
    //here we assume that A injects
    Ob x   = find_ldiv(A, Ax); if (!x) return;
    Ob y   = get_rhs(eqn);
    Ob Axy = get_app(eqn);
    Set& zs = OR::temp_set();
    zs.set_insn(OR::below(x), OR::below(y));

    for (Set::iterator z_iter(zs); z_iter.ok(); z_iter.next()) {
        Ob z = Ob(*z_iter);
        LOG_DEBUG1( "enforcing A_supconvexity_L1" );
        ensure_less(z,Axy);
    }
}
inline void enforce_A_supconvexity_L2 (Ob A, App eqn)
{// Ax=A*x | z[=x, z[=y, Axy=Ax*y
    if (get_lhs(eqn) != A) return;
    Ob x  = get_rhs(eqn);
    Ob Ax = get_app(eqn);
    Set& ys = OR::temp_set(); //for below

    for (OR::Iterator<RLpos> xz_iter(x); xz_iter.ok(); xz_iter.next()) {
        Ob z = xz_iter.rhs();
        ys.set_insn(OR::above(z), AE::Lx_support(Ax));
        for (Set::iterator y_iter(ys); y_iter.ok(); y_iter.next()) {
            Ob y = Ob(*y_iter);
            LOG_DEBUG1( "enforcing A_supconvexity_L2" );
            ensure_less(z,find_app(Ax,y));
        }
    }
}
inline void enforce_L_supconvexity_L1 (Ob A, Ord ord)
{// z[=x | z[=y, Axy=Ax*y, Ax=A*x
    Ob x  = ord.rhs();
    Ob Ax = find_app(A, x); if (!Ax) return;
    Ob z  = ord.lhs();
    Set& ys = OR::temp_set();
    ys.set_insn(OR::above(z), AE::Lx_support(Ax));

    for (Set::iterator y_iter(ys); y_iter.ok(); y_iter.next()) {
        Ob y = Ob(*y_iter);
        LOG_DEBUG1( "enforcing L_supconvexity_L1" );
        ensure_less(z,find_app(Ax,y));
    }
}
inline void enforce_L_supconvexity_L2 (Ob A, Ord ord)
{// z[=y | z[=x, Axy=Ax*y, Ax=A*x
    Ob y  = ord.rhs();
    Ob z  = ord.lhs();
    Set& xs = OR::temp_set();
    xs.set_insn(OR::above(z), AE::Lx_support(A));

    for (Set::iterator x_iter(xs); x_iter.ok(); x_iter.next()) {
        Ob x   = Ob(*x_iter);
        Ob Ax  = find_app(A,x);
        Ob Axy = find_app(Ax,y); if (!Axy) continue;
        LOG_DEBUG1( "enforcing L_supconvexity_L2" );
        ensure_less(z,Axy);
    }
}
inline void enforce_A_supconvexity_L (Ob A, App eqn)
{
    //Assert5(O::injects(A), "supconvex axiom assumes injectivity");
    enforce_A_supconvexity_L1(A,eqn);
    enforce_A_supconvexity_L2(A,eqn);
}
void enforce_L_supconvexity_L (Ob A, Ord ord)
{
    //Assert5(O::injects(A), "supconvex axiom assumes injectivity");
    enforce_L_supconvexity_L1(A,ord);
    enforce_L_supconvexity_L2(A,ord);
}

//axiom subconvex: z=]x, z=]y |- z=]Axy
inline void enforce_A_subconvexity_L1 (Ob A, App eqn)
{// Axy=Ax*y | z=]x, z=]y, Ax=A*x
    Ob Ax  = get_lhs(eqn);
    //here we assume that A injects
    Ob x   = find_ldiv(A, Ax); if (!x) return;
    Ob y   = get_rhs(eqn);
    Ob Axy = get_app(eqn);
    Set& zs = OR::temp_set();
    zs.set_insn(OR::above(x), OR::above(y));

    for (Set::iterator z_iter(zs); z_iter.ok(); z_iter.next()) {
        Ob z = Ob(*z_iter);
        LOG_DEBUG1( "enforcing A_subconvexity_L1" );
        ensure_less(Axy,z);
    }
}
inline void enforce_A_subconvexity_L2 (Ob A, App eqn)
{// Ax=A*x | z=]x, z=]y, Axy=Ax*y
    if (get_lhs(eqn) != A) return;
    Ob x  = get_rhs(eqn);
    Ob Ax = get_app(eqn);
    Set& ys = OR::temp_set(); //for below

    for (OR::Iterator<LRpos> xz_iter(x); xz_iter.ok(); xz_iter.next()) {
        Ob z = xz_iter.rhs();
        ys.set_insn(OR::below(z), AE::Lx_support(Ax));

        for (Set::iterator y_iter(ys); y_iter.ok(); y_iter.next()) {
            Ob y = Ob(*y_iter);
            LOG_DEBUG1( "enforcing A_subconvexity_L2" );
            ensure_less(find_app(Ax,y),z);
        }
    }
}
inline void enforce_L_subconvexity_L1 (Ob A, Ord ord)
{// z=]x | z=]y, Axy=Ax*y, Ax=A*x
    Ob x  = ord.lhs();
    Ob Ax = find_app(A, x); if (!Ax) return;
    Ob z  = ord.rhs();
    Set& ys = OR::temp_set();
    ys.set_insn(OR::below(z), AE::Lx_support(Ax));

    for (Set::iterator y_iter(ys); y_iter.ok(); y_iter.next()) {
        Ob y = Ob(*y_iter);
        LOG_DEBUG1( "enforcing L_subconvexity_L1" );
        ensure_less(find_app(Ax,y),z);
    }
}
inline void enforce_L_subconvexity_L2 (Ob A, Ord ord)
{// z=]y | z=]x, Axy=Ax*y, Ax=A*x
    Ob y  = ord.lhs();
    Ob z  = ord.rhs();
    Set& xs = OR::temp_set();
    xs.set_insn(OR::below(z), AE::Lx_support(A));

    for (Set::iterator x_iter(xs); x_iter.ok(); x_iter.next()) {
        Ob x   = Ob(*x_iter);
        Ob Ax  = find_app(A,x);
        Ob Axy = find_app(Ax,y); if (!Axy) continue;
        LOG_DEBUG1( "enforcing L_subconvexity_L2" );
        ensure_less(Axy,z);
    }
}
inline void enforce_A_subconvexity_L (Ob A, App eqn)
{
    //Assert5(O::injects(A), "subconvex axiom assumes injectivity");
    enforce_A_subconvexity_L1(A,eqn);
    enforce_A_subconvexity_L2(A,eqn);
}
void enforce_L_subconvexity_L (Ob A, Ord ord)
{
    //Assert5(O::injects(A), "subconvex axiom assumes injectivity");
    enforce_L_subconvexity_L1(A,ord);
    enforce_L_subconvexity_L2(A,ord);
}

//axiom A_duals_A: Ax_Bxy = x
inline void enforce_A_duals_A1 (Ob A, Ob B, App eqn)
{//Ax=A*x | Ax_Bxy=Ax*Bxy, Bxy=Bx*y, Bx=B*x
    if (get_lhs(eqn) != A) return;
    Ob x = get_rhs(eqn);
    Ob Bx = find_app(B,x); if (!Bx) return;
    Ob Ax = get_app(eqn);

    for (Lra_Iterator Bxy_iter(Bx); Bxy_iter.ok(); Bxy_iter.next()) {
        Ob Bxy = Bxy_iter.app();
        LOG_DEBUG1( "enforcing A_duals_A1" );
        ensure_app(x, Ax,Bxy);
    }
}
inline void enforce_A_duals_A2 (Ob A, Ob B, App eqn)
{//Bx=B*x | Ax_Bxy=Ax*Bxy, Bxy=Bx*y, Ax=A*x
    if (get_lhs(eqn) != B) return;
    Ob x = get_rhs(eqn);
    Ob Ax = find_app(A,x); if (!Ax) return;
    Ob Bx = get_app(eqn);

    for (Lra_Iterator Bxy_iter(Bx); Bxy_iter.ok(); Bxy_iter.next()) {
        Ob Bxy = Bxy_iter.app();
        LOG_DEBUG1( "enforcing A_duals_A2" );
        ensure_app(x, Ax,Bxy);
    }
}
inline void enforce_A_duals_A3 (Ob A, Ob B, App eqn)
{//Bxy=Bx*y | Ax_Bxy=Ax*Bxy, Ax=A*x, Bx=B*x
    Ob Bx  = get_lhs(eqn);
    //here we assume that B injects
    Ob x   = find_ldiv(B,Bx); if (!x) return;
    Ob Ax  = find_app(A,x); if (!Ax) return;
    Ob Bxy = get_app(eqn);
    LOG_DEBUG1( "enforcing A_duals_A3" );
    ensure_app(x, Ax,Bxy);
}
inline void enforce_A_duals_A4 (Ob A, Ob B, App eqn)
{//Ax_Bxy=Ax*Bxy | Bxy=Bx*y, Ax=A*x, Bx=B*x
    Ob Ax  = get_lhs(eqn);
    //here we assume that A injects
    Ob x   = find_ldiv(A,Ax); if (!x) return;
    Ob Bxy = get_rhs(eqn);

    for (Alr_Iterator Bxy_iter(Bxy); Bxy_iter.ok(); Bxy_iter.next()) {
        Ob Bx = Bxy_iter.lhs();
        //here we assume that B injects
        Ob x_ = find_ldiv(B,Bx); if (x_!=x) continue; //x_ may be 0
        LOG_DEBUG1( "enforcing A_duals_A4" );
        ensure_app(x, Ax,Bxy);
    }
}
void enforce_A_duals_A (Ob A, Ob B, App eqn)
{
    enforce_A_duals_A1(A,B,eqn);
    enforce_A_duals_A2(A,B,eqn);
    enforce_A_duals_A3(A,B,eqn);
    enforce_A_duals_A4(A,B,eqn);
}

//================ lattice theory axioms ================

//reflexivity
inline void enforce_O_ord_L (Ob ob)
{
    LOG_DEBUG1( "enforcing O_ord_L" );
    ensure_less(ob, ob);
}

//======== cubic-time monotony ========
//Note: mu & nu are Curry's names for right- and left- monotony, resp.
//axioms mu_L:  x[=y |- fx[=fy
inline void enforce_A_mu_L (App eqn)
{// fx=f*x | fy=f*y, x[=y or y[=x
    Ob f  = get_lhs(eqn);
    Ob x  = get_rhs(eqn);
    Ob fx = get_app(eqn);
    Set& ys = OR::temp_set();

    ys.set_insn(OR::above(x), AE::Lx_support(f));
    for (Set::iterator y_iter(ys); y_iter.ok(); y_iter.next()) {
        Ob y = Ob(*y_iter);
        LOG_DEBUG1( "enforcing A_mu_La" );
        ensure_less(fx, find_app(f,y));
    }

    ys.set_insn(OR::below(x), AE::Lx_support(f));
    for (Set::iterator y_iter(ys); y_iter.ok(); y_iter.next()) {
        Ob y = Ob(*y_iter);
        LOG_DEBUG1( "enforcing A_mu_Lb" );
        ensure_less(find_app(f,y), fx);
    }
}
inline void enforce_C_mu_L (Comp eqn)
{// fx=f*x | fy=f*y, x[=y or y[=x
    Ob f  = get_lhs(eqn);
    Ob x  = get_rhs(eqn);
    Ob fx = get_comp(eqn);
    Set& ys = OR::temp_set();

    ys.set_insn(OR::above(x), CE::Lx_support(f));
    for (Set::iterator y_iter(ys); y_iter.ok(); y_iter.next()) {
        Ob y = Ob(*y_iter);
        LOG_DEBUG1( "enforcing C_mu_La" );
        ensure_less(fx, find_comp(f,y));
    }

    ys.set_insn(OR::below(x), CE::Lx_support(f));
    for (Set::iterator y_iter(ys); y_iter.ok(); y_iter.next()) {
        Ob y = Ob(*y_iter);
        LOG_DEBUG1( "enforcing C_mu_Lb" );
        ensure_less(find_comp(f,y), fx);
    }
}
inline void enforce_L_mu_L (Ord ord)
{// x[=y | fx=f*x, gy=g*y
    Ob x = ord.lhs();
    Ob y = ord.rhs();

    for (RRla_Iterator iter(x,y); iter.ok(); iter.next()) {
        Ob fx = iter.app1();
        Ob fy = iter.app2();
        LOG_DEBUG1( "enforcing L_mu_La" );
        ensure_less(fx,fy);
    }

    for (RRlc_Iterator iter(x,y); iter.ok(); iter.next()) {
        Ob fx = iter.comp1();
        Ob fy = iter.comp2();
        LOG_DEBUG1( "enforcing L_mu_Lb" );
        ensure_less(fx,fy);
    }
}
//axioms nu_L:  x[=g |- fx[=gx
inline void enforce_A_nu_L (App eqn)
{// fx=f*x | gx=g*x, f[=g or g[=f
    Ob f  = get_lhs(eqn);
    Ob x  = get_rhs(eqn);
    Ob fx = get_app(eqn);
    Set& gs = OR::temp_set();

    gs.set_insn(OR::above(f), AE::Rx_support(x));
    for (Set::iterator g_iter(gs); g_iter.ok(); g_iter.next()) {
        Ob g = Ob(*g_iter);
        LOG_DEBUG1( "enforcing A_nu_La" );
        ensure_less(fx, find_app(g,x));
    }

    gs.set_insn(OR::below(f), AE::Rx_support(x));
    for (Set::iterator g_iter(gs); g_iter.ok(); g_iter.next()) {
        Ob g = Ob(*g_iter);
        LOG_DEBUG1( "enforcing A_nu_Lb" );
        ensure_less(find_app(g,x), fx);
    }
}
inline void enforce_C_nu_L (Comp eqn)
{// fx=f*x | gx=g*x, f[=g or g[=f
    Ob f  = get_lhs(eqn);
    Ob x  = get_rhs(eqn);
    Ob fx = get_comp(eqn);
    Set& gs = OR::temp_set();

    gs.set_insn(OR::above(f), CE::Rx_support(x));
    for (Set::iterator g_iter(gs); g_iter.ok(); g_iter.next()) {
        Ob g = Ob(*g_iter);
        LOG_DEBUG1( "enforcing A_nu_La" );
        ensure_less(fx, find_comp(g,x));
    }

    gs.set_insn(OR::below(f), CE::Rx_support(x));
    for (Set::iterator g_iter(gs); g_iter.ok(); g_iter.next()) {
        Ob g = Ob(*g_iter);
        LOG_DEBUG1( "enforcing A_nu_Lb" );
        ensure_less(find_comp(g,x), fx);
    }
}
inline void enforce_L_nu_L (Ord ord)
{// f[=g | fx=x*f, gy=g*g
    Ob f = ord.lhs();
    Ob g = ord.rhs();

    for (LLra_Iterator iter(f,g); iter.ok(); iter.next()) {
        Ob fx = iter.app1();
        Ob gx = iter.app2();
        LOG_DEBUG1( "enforcing L_nu_La" );
        ensure_less(fx,gx);
    }

    for (LLrc_Iterator iter(f,g); iter.ok(); iter.next()) {
        Ob fx = iter.comp1();
        Ob gx = iter.comp2();
        LOG_DEBUG1( "enforcing L_nu_Lb" );
        ensure_less(fx,gx);
    }
}
//axiom mu_N: fx![=fy |- x![=y
inline void enforce_A_mu_N (App eqn)
{// fx=f*x | fy=f*y, fx![=fy or fy![=fx
    Ob f  = get_lhs(eqn);
    Ob x  = get_rhs(eqn);
    Ob fx = get_app(eqn);

    for (Lra_Iterator fy_iter(f); fy_iter.ok(); fy_iter.next()) {
        Ob fy = fy_iter.app();
        Ob y  = fy_iter.rhs();

        if (is_nless(fx,fy)) {
            LOG_DEBUG1( "enforcing A_mu_Na" );
            ensure_nless(x,y);
        }
        if (is_nless(fy,fx)) {
            LOG_DEBUG1( "enforcing A_mu_Nb" );
            ensure_nless(y,x);
        }
    }
}
inline void enforce_C_mu_N (Comp eqn)
{// fx=f*x | fy=f*y, fx![=fy or fy![=fx
    Ob f  = get_lhs(eqn);
    Ob x  = get_rhs(eqn);
    Ob fx = get_comp(eqn);

    for (Lrc_Iterator fy_iter(f); fy_iter.ok(); fy_iter.next()) {
        Ob fy = fy_iter.comp();
        Ob y  = fy_iter.rhs();

        if (is_nless(fx,fy)) {
            LOG_DEBUG1( "enforcing A_mu_Na" );
            ensure_nless(x,y);
        }
        if (is_nless(fy,fx)) {
            LOG_DEBUG1( "enforcing A_mu_Nb" );
            ensure_nless(y,x);
        }
    }
}
//axiom nu_N: fx![=gx |- f![=g
inline void enforce_A_nu_N (App eqn)
{// fx=f*x | gx=g*x, fx![=gx or gx![=fx
    Ob f  = get_lhs(eqn);
    Ob x  = get_rhs(eqn);
    Ob fx = get_app(eqn);

    for (Rla_Iterator gx_iter(x); gx_iter.ok(); gx_iter.next()) {
        Ob gx = gx_iter.app();
        Ob g  = gx_iter.lhs();

        if (is_nless(fx,gx)) {
            LOG_DEBUG1( "enforcing A_nu_Na" );
            ensure_nless(f,g);
        }
        if (is_nless(gx,fx)) {
            LOG_DEBUG1( "enforcing A_nu_Nb" );
            ensure_nless(g,f);
        }
    }
}
inline void enforce_C_nu_N (Comp eqn)
{// fx=f*x | gx=g*x, fx![=gx or gx![=fx
    Ob f  = get_lhs(eqn);
    Ob x  = get_rhs(eqn);
    Ob fx = get_comp(eqn);

    for (Rlc_Iterator gx_iter(x); gx_iter.ok(); gx_iter.next()) {
        Ob gx = gx_iter.comp();
        Ob g  = gx_iter.lhs();

        if (is_nless(fx,gx)) {
            LOG_DEBUG1( "enforcing A_nu_Na" );
            ensure_nless(f,g);
        }
        if (is_nless(gx,fx)) {
            LOG_DEBUG1( "enforcing A_nu_Nb" );
            ensure_nless(g,f);
        }
    }
}
//combined monotonicity axioms -- very expensive
inline void enforce_N_mono_N1 (Ord ord)
{// fx![=fy | fx=f*x, fy=f*y   ---and---   fx![=gx | fx=f*x, gx=g*x

    //PROFILE: enforce_N_mono_N is very expensive: 16% of total time
    Ob fx = ord.lhs();
    Ob fy = ord.rhs();
    Ob gx = ord.rhs();

    ALr_Iterator fy_iter; //for below
    ARl_Iterator gx_iter; //for below
    for (Alr_Iterator fx_iter(fx); fx_iter.ok(); fx_iter.next()) {
        Ob f = fx_iter.lhs();
        Ob x = fx_iter.rhs();

        for (fy_iter.begin(fy,f); fy_iter.ok(); fy_iter.next()) {
            Ob y = fy_iter.rhs();

            LOG_DEBUG1( "enforcing N_mu_N1" );
            ensure_nless(x,y);
        }

        for (gx_iter.begin(gx,x); gx_iter.ok(); gx_iter.next()) {
            Ob g = gx_iter.lhs();

            LOG_DEBUG1( "enforcing N_nu_N1" );
            ensure_nless(f,g);
        }
    }
}
inline void enforce_N_mono_N2 (Ord ord)
{// fx![=fy | fx=f*x, fy=f*y   ---and---   fx![=gx | fx=f*x, gx=g*x

    Ob fx = ord.lhs();
    Ob fy = ord.rhs();
    Ob gx = ord.rhs();

    CLr_Iterator fy_iter; //for below
    CRl_Iterator gx_iter; //for below
    for (Clr_Iterator fx_iter(fx); fx_iter.ok(); fx_iter.next()) {
        Ob f = fx_iter.lhs();
        Ob x = fx_iter.rhs();

        for (fy_iter.begin(fy,f); fy_iter.ok(); fy_iter.next()) {
            Ob y = fy_iter.rhs();

            LOG_DEBUG1( "enforcing N_mu_N2" );
            ensure_nless(x,y);
        }

        for (gx_iter.begin(gx,x); gx_iter.ok(); gx_iter.next()) {
            Ob g = gx_iter.lhs();

            LOG_DEBUG1( "enforcing N_nu_N2" );
            ensure_nless(f,g);
        }
    }
}
inline void enforce_N_mono_N (Ord ord)
{
    enforce_N_mono_N1 (ord);
    enforce_N_mono_N2 (ord);
}

//axiom asymmetry:  x[=y, y[=x |- x=y
inline void enforce_L_asymmetry_L (Ord ord)
{// x[=y and y[=x | x=y
    Ob x = ord.lhs();
    Ob y = ord.rhs();
    if (is_less(y,x)) {
        LOG_DEBUG1( "enforcing L_asymetry_L1" );
        ensure_equiv(x,y);
    }
}
//axiom transitivity_L:  x[=y[=z |- x[=z
inline void enforce_L_transitivity_L1 (Ord ord)
{// x[=y | y[=z
    Ob x = ord.lhs();
    Ob y = ord.rhs(); if (x == y) return;
    ensure_less(x, OR::above(y));
}
inline void enforce_L_transitivity_L2 (Ord ord)
{// y[=z | x[=y
    Ob y = ord.lhs();
    Ob z = ord.rhs(); if (y == z) return;
    ensure_less(OR::below(y), z);
}
//axiom transitivity_N1:  x[=y, x![=z |- y![=z
inline void enforce_L_transitivity_N1 (Ord ord)
{// x[=y | x![=z
    Ob x = ord.lhs();
    Ob y = ord.rhs(); if (x == y) return;
    ensure_nless(y, OR::nabove(x));
}
inline void enforce_N_transitivity_N1 (Ord ord)
{// x![=z | x[=y
    Ob x = ord.lhs();
    Ob z = ord.rhs();
    ensure_nless(OR::above(x), z);
}
//axiom transitivity_N2:  y[=z, x![=z |- x![=y
inline void enforce_L_transitivity_N2 (Ord ord)
{// y[=z | x![=z
    Ob y = ord.lhs();
    Ob z = ord.rhs(); if (y == z) return;
    ensure_nless(OR::nbelow(z), y);
}
inline void enforce_N_transitivity_N2 (Ord ord)
{// x![=z | x[=y
    Ob x = ord.lhs();
    Ob z = ord.rhs();
    ensure_nless(OR::above(x), z);
}
//sorted into contexts
inline void enforce_L_ord_pos (Ord ord)
{
    enforce_L_asymmetry_L(ord);
    enforce_L_transitivity_L1(ord);
    enforce_L_transitivity_L2(ord);
}
inline void enforce_L_ord_neg (Ord ord)
{
    enforce_L_transitivity_N1(ord);
    enforce_L_transitivity_N2(ord);
}
inline void enforce_N_ord_neg (Ord ord)
{
    enforce_N_transitivity_N1(ord);
    enforce_N_transitivity_N2(ord);
}

//axioms Bot: Bot x = Bot
inline void enforce_O_Bot_L (Ob ob)
{
    LOG_DEBUG1( "enforcing O_Bot_L" );
    ensure_less(*Atoms::Bot, ob);
}
inline void enforce_A_Bot_N (App eqn)
{// fx=f*x | fx![=Bot
    Ob fx = get_app(eqn); if (not converges(fx)) return;
    Ob f = get_lhs(eqn);

    LOG_DEBUG1( "enforcing A_Bot_N" );
    ensure_conv(f);
}
inline void enforce_N_Bot_N (Ord ord)
{// fx![=Bot | fx=f*x
    Ob Bot = *Atoms::Bot; if (Bot != ord.rhs()) return;
    Ob fx  = ord.lhs();
    for (Alr_Iterator fx_iter(fx); fx_iter.ok(); fx_iter.next()) {
        Ob f = fx_iter.lhs();
        LOG_DEBUG1( "enforcing N_Bot_N" );
        ensure_nless(f, Bot);
    }
}

//axioms Top: Top x = Top
inline void enforce_O_Top_L (Ob ob)
{
    LOG_DEBUG1( "enforcing O_Top_L" );
    ensure_less(ob, *Atoms::Top);
}
inline void enforce_A_Top_N (App eqn)
{// fx=f*x | fx=!]Top
    Ob Top = *Atoms::Top;
    Ob fx = get_app(eqn); if (not is_nless(Top,fx)) return;
    Ob f = get_lhs(eqn);
    LOG_DEBUG1( "enforcing A_Top_N" );
    ensure_nless(Top, f);
}
inline void enforce_N_Top_N (Ord ord)
{// fx=!]Top | fx=f*x
    Ob Top = *Atoms::Top; if (Top != ord.lhs()) return;
    Ob fx  = ord.rhs();
    for (Alr_Iterator fx_iter(fx); fx_iter.ok(); fx_iter.next()) {
        Ob f = fx_iter.lhs();
        LOG_DEBUG1( "enforcing N_Top_N" );
        ensure_nless(Top, f);
    }
}

//join-order: x[=y ==> x|y=y
inline void enforce_L_join_pos1 (Ord ord)
{
    Ob x = ord.lhs();
    Ob y = ord.rhs();
    LOG_DEBUG1( "enforcing L_join_pos1" );
    ensure_join(y, x,y);
}
//join: x[=x|y, y[=x|y, x[=z & y[=z ==> x|y[=z
inline void enforce_J_join_pos1 (Join eqn)
{
    Ob xy = get_join(eqn);
    Ob x  = get_lhs(eqn);
    Ob y  = get_rhs(eqn);

    LOG_DEBUG1( "enforcing J_join_pos1" );
    ensure_less(x, xy);
    ensure_less(y, xy);
}
inline void enforce_L_join_pos2 (Ord ord)
{// x[=z | y[=z, x|y
    Ob x = ord.lhs();
    Ob z = ord.rhs();

    Set& ys = OR::temp_set();
    ys.set_insn(JE::Lx_support(x), OR::below(z));
    for (Set::iterator y_iter(ys); y_iter.ok(); y_iter.next()) {
        Ob y  = Ob(*y_iter);

        LOG_DEBUG1( "enforcing L_join_pos2" );
        ensure_less(find_join(x,y), z);
    }
}
inline void enforce_J_join_pos2 (Join eqn)
{// x|y | x[=z, y[=z
    Ob x  = get_lhs(eqn);
    Ob y  = get_rhs(eqn);
    Ob xy = get_join(eqn);

    Set& zs = OR::temp_set();
    zs.set_insn(OR::above(x), OR::above(y));

    LOG_DEBUG1( "enforcing J_join_pos2" );
    ensure_less(xy, zs);
}

//join-join (associativity): x|(y|z) = (x|y)|z
inline void enforce_J_join_pos3 (Join eqn, bool parity)
{// x|y | ...
    Ob x  = parity ? get_lhs(eqn) : get_rhs(eqn);
    Ob y  = parity ? get_rhs(eqn) : get_lhs(eqn);
    Ob xy = get_join(eqn);

    for (Lrj_Iterator xz_iter(x); xz_iter.ok(); xz_iter.next()) {
        Ob z  = xz_iter.moving();
        Ob xz = xz_iter.join();

        LOG_DEBUG1( "enforcing J_join_pos3" );
        ensure_joins(xz,y, xy,z);
    }
}
inline void enforce_J_join_pos4 (Join eqn, bool parity)
{// x|(y|z) | ...
    Ob x   = parity ? get_lhs(eqn) : get_rhs(eqn);
    Ob yz  = parity ? get_rhs(eqn) : get_lhs(eqn);
    Ob xyz = get_join(eqn);

    for (Jlr_Iterator yz_iter(yz); yz_iter.ok(); yz_iter.next()) {
        Ob y  = yz_iter.lhs();
        Ob z  = yz_iter.rhs();

        if (Ob xz = find_join(x,z)) {
            LOG_DEBUG1( "enforcing J_join_pos4a" );
            ensure_join(xyz, xz,y);
        }

        if (Ob xy = find_join(x,y)) {
            LOG_DEBUG1( "enforcing J_join_pos4b" );
            ensure_join(xyz, xy,z);
        }
    }
}

//join-apply: (x|y)z = x z|y z
inline void enforce_J_join_pos5 (Join eqn)
{// x|y | x z, y z, (x|y)z or xz|yz
    Ob x  = get_lhs(eqn);
    Ob y  = get_rhs(eqn);
    Ob xy = get_join(eqn);

    for (LLra_Iterator z_iter(x,y); z_iter.ok(); z_iter.next()) {
        Ob z  = z_iter.rhs();
        Ob xz = z_iter.app1();
        Ob yz = z_iter.app2();

        LOG_DEBUG1( "enforcing J_join_pos5" );
        ensure_app_join(xy,z, xz,yz);
    }
}
inline void enforce_J_join_pos6 (Join eqn)
{// xz|yz | x|y, xz, yz
    Ob xz  = get_lhs(eqn);
    Ob yz  = get_rhs(eqn);
    Ob xyz = get_join(eqn);

    ARl_Iterator xz_iter; //for below

    for (Alr_Iterator yz_iter(yz); yz_iter.ok(); yz_iter.next()) {
        Ob y = yz_iter.lhs();
        Ob z = yz_iter.rhs();

        for (xz_iter.begin(xz,z); xz_iter.ok(); xz_iter.next()) {
            Ob x  = xz_iter.lhs();
            Ob xy = find_join(x,y); if (!xy) continue;

            LOG_DEBUG1( "enforcing J_join_pos6" );
            ensure_app(xyz, xy,z);
        }
    }
}
inline void enforce_A_join_pos1 (App eqn)
{// xz | x|y, yz, xz|yz or (x|y)z
    Ob x  = get_lhs(eqn);
    Ob z  = get_rhs(eqn);
    Ob xz = get_app(eqn);

    Set& ys = OR::temp_set();
    ys.set_insn(JE::Lx_support(x), AE::Rx_support(z));
    for (Set::iterator y_iter(ys); y_iter.ok(); y_iter.next()) {
        Ob y  = Ob(*y_iter);
        Ob xy = find_join(x,y);
        Ob yz = find_app(y,z);

        LOG_DEBUG1( "enforcing A_join_pos1" );
        ensure_app_join(xy,z, xz,yz);
    }
}
inline void enforce_A_join_pos2 (App eqn)
{// (x|y)z | x|y, xz, yz
    Ob xy  = get_lhs(eqn);
    Ob z   = get_rhs(eqn);
    Ob xyz = get_app(eqn);

    for (Jlr_Iterator xy_iter(xy); xy_iter.ok(); xy_iter.next()) {
        Ob x  = xy_iter.lhs();
        Ob xz = find_app(x,z); if (!xz) continue;
        Ob y  = xy_iter.rhs();
        Ob yz = find_app(y,z); if (!yz) continue;

        LOG_DEBUG1( "enforcing A_join_pos2" );
        ensure_join(xyz, xz,yz);
    }
}

//join-compose: (x|y)*z = x*z|y*z
inline void enforce_J_join_pos7 (Join eqn)
{// x|y | x*z, y*z, (x|y)*z or x*z|y*z
    Ob x  = get_lhs(eqn);
    Ob y  = get_rhs(eqn);
    Ob xy = get_join(eqn);

    for (LLrc_Iterator z_iter(x,y); z_iter.ok(); z_iter.next()) {
        Ob z  = z_iter.rhs();
        Ob xz = z_iter.comp1();
        Ob yz = z_iter.comp2();

        LOG_DEBUG1( "enforcing J_join_pos7" );
        ensure_join_comp(xz,yz, xy,z);
    }
}
inline void enforce_J_join_pos8 (Join eqn)
{// x*z|y*z | x|y, x*z, y*z
    Ob xz  = get_lhs(eqn);
    Ob yz  = get_rhs(eqn);
    Ob xyz = get_join(eqn);

    CRl_Iterator xz_iter; //for below

    for (Clr_Iterator yz_iter(yz); yz_iter.ok(); yz_iter.next()) {
        Ob y = yz_iter.lhs();
        Ob z = yz_iter.rhs();

        for (xz_iter.begin(xz,z); xz_iter.ok(); xz_iter.next()) {
            Ob x  = xz_iter.lhs();
            Ob xy = find_join(x,y); if (!xy) continue;

            LOG_DEBUG1( "enforcing J_join_pos8" );
            ensure_comp(xyz, xy,z);
        }
    }
}
inline void enforce_C_join_pos1 (Comp eqn)
{// x*z | x|y, y*z, x*z|y*z or (x|y)*z
    Ob x  = get_lhs(eqn);
    Ob z  = get_rhs(eqn);
    Ob xz = get_comp(eqn);

    Set& ys = OR::temp_set();
    ys.set_insn(JE::Lx_support(x), CE::Rx_support(z));
    for (Set::iterator y_iter(ys); y_iter.ok(); y_iter.next()) {
        Ob y  = Ob(*y_iter);
        Ob xy = find_join(x,y);
        Ob yz = find_comp(y,z);

        LOG_DEBUG1( "enforcing C_join_pos1" );
        ensure_join_comp(xz,yz, xy,z);
    }
}
inline void enforce_C_join_pos2 (Comp eqn)
{// (x|y)*z | x|y, x*z, y*z
    Ob xy  = get_lhs(eqn);
    Ob z   = get_rhs(eqn);
    Ob xyz = get_comp(eqn);

    for (Jlr_Iterator xy_iter(xy); xy_iter.ok(); xy_iter.next()) {
        Ob x  = xy_iter.lhs();
        Ob xz = find_comp(x,z); if (!xz) continue;
        Ob y  = xy_iter.rhs();
        Ob yz = find_comp(y,z); if (!yz) continue;

        LOG_DEBUG1( "enforcing C_join_pos2" );
        ensure_join(xyz, xz,yz);
    }
}

//collected join axioms
inline void enforce_L_join_pos (Ord ord)
{
    enforce_L_join_pos1(ord);
    enforce_L_join_pos2(ord);
}
inline void enforce_A_join_pos (App eqn)
{
    enforce_A_join_pos1(eqn);
    enforce_A_join_pos2(eqn);
}
inline void enforce_C_join_pos (Comp eqn)
{
    enforce_C_join_pos1(eqn);
    enforce_C_join_pos2(eqn);
}
inline void enforce_J_join_pos (Join eqn)
{
    enforce_J_join_pos1(eqn);
    enforce_J_join_pos2(eqn);
    enforce_J_join_pos3(eqn, true);
    enforce_J_join_pos3(eqn, false);
    enforce_J_join_pos4(eqn, true);
    enforce_J_join_pos4(eqn, false);
    enforce_J_join_pos5(eqn);
    enforce_J_join_pos6(eqn);
    enforce_J_join_pos7(eqn);
    enforce_J_join_pos8(eqn);
}

//theorems A_Join_pos, J_Join_pos: Jxy = x|y
inline void enforce_A_Join_pos1 (App eqn)
{//Jx | x|y or Jxy
    Ob J  = get_lhs(eqn); if (J != *Atoms::J) return;
    Ob x  = get_rhs(eqn);
    Ob Jx = get_app(eqn);

    for (Lrj_Iterator xy_iter(x); xy_iter.ok(); xy_iter.next()) {
        Ob y  = xy_iter.moving();
        Ob xy = xy_iter.join();

        LOG_DEBUG1( "enforcing A_Join_pos1a" );
        ensure_app(xy, Jx,y);
    }

    for (Lra_Iterator Jxy_iter(Jx); Jxy_iter.ok(); Jxy_iter.next()) {
        Ob y   = Jxy_iter.rhs();
        Ob Jxy = Jxy_iter.app();

        LOG_DEBUG1( "enforcing A_Join_pos1b" );
        ensure_join(Jxy, x,y);
    }
}
inline void enforce_A_Join_pos2 (App eqn)
{//Jxy | Jx
    Ob Jx  = get_lhs(eqn);
    Ob x   = find_ldiv(*Atoms::J,Jx); if (!x) return;
    Ob y   = get_rhs(eqn);
    Ob Jxy = get_app(eqn);

    LOG_DEBUG1( "enforcing A_Join_pos2" );
    ensure_join(Jxy, x,y);
}
inline void enforce_J_Join_pos (Join eqn)
{// x|y | Jx or Jy
    Ob J  = *Atoms::J;
    Ob x  = get_lhs(eqn);
    Ob y  = get_rhs(eqn);
    Ob xy = get_join(eqn);

    if (Ob Jx = find_app(J,x)) {
        LOG_DEBUG1( "enforcing J_Join_pos1" );
        ensure_app(xy, Jx,y);
    }

    if (Ob Jy = find_app(J,y)) {
        LOG_DEBUG1( "enforcing J_Join_pos2" );
        ensure_app(xy, Jy,x);
    }
}
inline void enforce_A_Join_pos (App eqn)
{
    enforce_A_Join_pos1(eqn);
    enforce_A_Join_pos2(eqn);
}

//compose-compose (associativity): x*(y*z) = (x*y)*z
inline void enforce_C_comp_assoc_C1 (Comp eqn)
{// xy | yz, x_yz or xy_z
    Ob x  = get_lhs(eqn);
    Ob y  = get_rhs(eqn);
    Ob xy = get_comp(eqn);

    for (Lrc_Iterator yz_iter(y); yz_iter.ok(); yz_iter.next()) {
        Ob z  = yz_iter.rhs();
        Ob yz = yz_iter.comp();

        LOG_DEBUG1( "enforcing C_comp_assoc_C1" );
        ensure_comps(xy,z, x,yz);
    }
}
inline void enforce_C_comp_assoc_C2 (Comp eqn)
{// yz | xy, x_yz or xy_z
    Ob y  = get_lhs(eqn);
    Ob z  = get_rhs(eqn);
    Ob yz = get_comp(eqn);

    for (Rlc_Iterator xy_iter(y); xy_iter.ok(); xy_iter.next()) {
        Ob x  = xy_iter.lhs();
        Ob xy = xy_iter.comp();

        LOG_DEBUG1( "enforcing C_comp_assoc_C2" );
        ensure_comps(xy,z, x,yz);
    }
}
inline void enforce_C_comp_assoc_C3 (Comp eqn)
{// x_yz | xy, yz
    Ob x    = get_lhs(eqn);
    Ob yz   = get_rhs(eqn);
    Ob x_yz = get_comp(eqn);

    for (Clr_Iterator yz_iter(yz); yz_iter.ok(); yz_iter.next()) {
        Ob y  = yz_iter.lhs();
        Ob xy = find_comp(x,y); if (!xy) continue;
        Ob z  = yz_iter.rhs();

        LOG_DEBUG1( "enforcing C_comp_assoc_C3" );
        ensure_comp(x_yz, xy,z);
    }
}
inline void enforce_C_comp_assoc_C4 (Comp eqn)
{// xy_z | xy, yz
    Ob xy   = get_lhs(eqn);
    Ob z    = get_rhs(eqn);
    Ob xy_z = get_comp(eqn);

    for (Clr_Iterator xy_iter(xy); xy_iter.ok(); xy_iter.next()) {
        Ob y  = xy_iter.rhs();
        Ob yz = find_comp(y,z); if (!yz) continue;
        Ob x  = xy_iter.lhs();

        LOG_DEBUG1( "enforcing C_comp_assoc_C4" );
        ensure_comp(xy_z, x,yz);
    }

}
inline void enforce_C_comp_assoc_C (Comp eqn)
{
    enforce_C_comp_assoc_C1(eqn);
    enforce_C_comp_assoc_C2(eqn);
    enforce_C_comp_assoc_C3(eqn);
    enforce_C_comp_assoc_C4(eqn);
}

//compose-apply: (x*y)z = x(y z)
inline void enforce_C_comp_app_A (Comp eqn)
{// x*y | y z, (x*y)z or x(y z)
    Ob x  = get_lhs(eqn);
    Ob y  = get_rhs(eqn);
    Ob xy = get_comp(eqn);

    for (Lra_Iterator z_iter(y); z_iter.ok(); z_iter.next()) {
        Ob yz = z_iter.app();
        Ob z  = z_iter.rhs();

        LOG_DEBUG1( "enforcing C_comp_app_A" );
        ensure_apps(xy,z, x,yz);
    }
}
inline void enforce_A_comp_app_A1 (App eqn)
{// y z | x*y, (x*y)z or x(y z)
    Ob y  = get_lhs(eqn);
    Ob z  = get_rhs(eqn);
    Ob yz = get_app(eqn);

    for (Rlc_Iterator x_iter(y); x_iter.ok(); x_iter.next()) {
        Ob xy = x_iter.comp();
        Ob x  = x_iter.lhs();

        LOG_DEBUG1( "enforcing A_comp_app_A1" );
        ensure_apps(xy,z, x,yz);
    }
}
inline void enforce_A_comp_app_A2 (App eqn)
{// (x*y)z | y z, x*y
    Ob xy   = get_lhs(eqn);
    Ob z    = get_rhs(eqn);
    Ob xy_z = get_app(eqn);

    for (Clr_Iterator xy_iter(xy); xy_iter.ok(); xy_iter.next()) {
        Ob y  = xy_iter.rhs();
        Ob yz = find_app(y,z); if (!yz) continue;
        Ob x  = xy_iter.lhs();

        LOG_DEBUG1( "enforcing A_comp_app_A2" );
        ensure_app(xy_z, x,yz);
    }
}
inline void enforce_A_comp_app_A3 (App eqn)
{// x(y z) | y z, x*y
    Ob x    = get_lhs(eqn);
    Ob yz   = get_rhs(eqn);
    Ob x_yz = get_app(eqn);

    for (Alr_Iterator yz_iter(yz); yz_iter.ok(); yz_iter.next()) {
        Ob y  = yz_iter.lhs();
        Ob xy = find_comp(x,y); if (!xy) continue;
        Ob z  = yz_iter.rhs();

        LOG_DEBUG1( "enforcing A_comp_app_A3" );
        ensure_app(x_yz, xy,z);
    }
}
inline void enforce_A_comp_app_A (App eqn)
{
    enforce_A_comp_app_A1(eqn);
    enforce_A_comp_app_A2(eqn);
    enforce_A_comp_app_A3(eqn);
}

//axioms for R
inline void enforce_A_Rand_pos (App eqn)
{
    Ob R = *Atoms::R;
    enforce_A_idempotence_A   (R, eqn);
    enforce_A_commutativity_A (R, eqn);
    enforce_A_distrib_rapply_A(R, eqn);
    enforce_A_supconvexity_L  (R, eqn);
    enforce_A_subconvexity_L  (R, eqn);
}
inline void enforce_L_Rand_pos (Ord ord)
{
    Ob R = *Atoms::R;
    enforce_L_supconvexity_L (R, ord);
    enforce_L_subconvexity_L (R, ord);
}

//================ lambda theory axioms ================

//theorem O_I_pos: Ix = x
inline void enforce_O_I_pos (Ob ob)
{// I_x=x
    LOG_DEBUG1( "enforcing O_I_pos" );
    Ob I = *Atoms::I;
    ensure_app(ob, I,ob);
    ensure_comp(ob, I,ob);
    ensure_comp(ob, ob,I);
}

//axiom O_K_pos:  K x y = x, (K x)*y = K x
inline void enforce_O_K_pos (Ob ob)
{// y | K x
    LOG_DEBUG1( "enforcing O_K_pos" );
    Ob K = *Atoms::K;

    for (Lra_Iterator Kx_iter(K); Kx_iter.ok(); Kx_iter.next()) {
        Ob x  = Kx_iter.rhs();
        Ob Kx = Kx_iter.app();

        ensure_app(x, Kx,ob);

        ensure_comp(Kx, Kx,ob);
    }
}
//axiom A_K_pos:  K x y = x, (K x)*y = K x
inline void enforce_A_K_pos1 (App eqn)
{// K x | y
    if (get_lhs(eqn) != *Atoms::K) return;
    Ob x  = get_rhs(eqn);
    Ob Kx = get_app(eqn);

    LOG_DEBUG1( "enforcing A_K_pos1" );
    Ob::sparse_iterator end = Ob::send();
    for (Ob::sparse_iterator y_iter=Ob::sbegin(); y_iter!=end; ++y_iter) {
        Ob y = *y_iter;
        if (O::isDepricated(y)) continue;

        ensure_app(x, Kx,y);

        ensure_comp(Kx, Kx,y);
    }
}
//axiom [AC]_K_pos: f*(K x) = K(f x)
inline void enforce_A_K_pos2 (App eqn)
{// K x | f x, f*(K x) or K(f x)
    Ob x  = get_rhs(eqn);
    Ob K  = get_lhs(eqn); if (K != *Atoms::K) return;
    Ob Kx = get_app(eqn);

    for (Rla_Iterator fx_iter(x); fx_iter.ok(); fx_iter.next()) {
        Ob f  = fx_iter.lhs();
        Ob fx = fx_iter.app();

        LOG_DEBUG1( "enforcing A_K_pos2" );
        ensure_app_comp(K,fx, f,Kx);
    }
}
inline void enforce_A_K_pos3 (App eqn)
{// f x | K x, f*(K x) or K(f x)
    Ob x  = get_rhs(eqn);
    Ob K  = *Atoms::K;
    Ob Kx = find_app(K,x); if (!Kx) return;
    Ob f  = get_lhs(eqn);
    Ob fx = get_app(eqn);

    LOG_DEBUG1( "enforcing A_K_pos3" );
    ensure_app_comp(K,fx, f,Kx);
}
inline void enforce_A_K_pos4 (App eqn)
{// K(f x) | K x, f x
    Ob K   = get_lhs(eqn); if (K != *Atoms::K) return;
    Ob Kfx = get_app(eqn);
    Ob fx  = get_rhs(eqn);

    for (Alr_Iterator fx_iter(fx); fx_iter.ok(); fx_iter.next()) {
        Ob x  = fx_iter.rhs();
        Ob Kx = find_app(K,x); if (!Kx) continue;
        Ob f  = fx_iter.lhs();

        LOG_DEBUG1( "enforcing A_K_pos4" );
        ensure_comp(Kfx, f,Kx);
    }
}
inline void enforce_C_K_pos (Comp eqn)
{// f*(K x) | f x, K x
    Ob K   = *Atoms::K;
    Ob Kx  = get_rhs(eqn);
    Ob x   = find_ldiv(K,Kx); if (!x) return;
    Ob f   = get_lhs(eqn);
    Ob fx  = find_app(f,x); if (!fx) return;
    Ob fKx = get_comp(eqn);

    LOG_DEBUG1( "enforcing C_K_pos1" );
    ensure_app(fKx, K,fx);
}
inline void enforce_A_K_pos (App eqn)
{
    enforce_A_K_pos1 (eqn);
    enforce_A_K_pos2 (eqn);
    enforce_A_K_pos3 (eqn);
    enforce_A_K_pos4 (eqn);
}

//axiom A_S_A:  Sxyz = xz(yz)
inline void enforce_A_S_A1 (App eqn)
{// Sx=S*x | Sxy=Sx*y, Sxyz=Sxy*z, xz=x*z, yz=y*z, xz_yz=xz*yz
    Ob S  = *Atoms::S; if (get_lhs(eqn) != S) return;
    Ob x  = get_rhs(eqn);
    Ob Sx = get_app(eqn);

    Lra_Iterator xz_iter(x); if (!xz_iter.ok()) return; //for below

    for (Lra_Iterator Sxy_iter(Sx); Sxy_iter.ok(); Sxy_iter.next()) {
        Ob Sxy = Sxy_iter.app();
        Ob y   = Sxy_iter.rhs();

        for (xz_iter.begin(); xz_iter.ok(); xz_iter.next()) {
            Ob z     = xz_iter.rhs();
            Ob yz    = find_app(y,z); if (!yz) continue;
            Ob xz    = xz_iter.app();

            LOG_DEBUG1( "enforcing A_S_A1" );
            ensure_apps(Sxy,z, xz,yz);
        }
    }
}
inline void enforce_A_S_A2 (App eqn)
{// Sxy=Sx*y | S*x=Sx, Sxy*z=Sxyz, x*z=xz, y*z=yz, xz*yz=xz_yz
    Ob S   = *Atoms::S;
    Ob Sx  = get_lhs(eqn);
    Ob x   = find_ldiv(S,Sx); if (!x) return;
    Ob y   = get_rhs(eqn);
    Ob Sxy = get_app(eqn);

    for (Lra_Iterator xz_iter(x); xz_iter.ok(); xz_iter.next()) {
        Ob z     = xz_iter.rhs();
        Ob yz    = find_app(y,z); if (!yz) continue;
        Ob xz    = xz_iter.app();

        LOG_DEBUG1( "enforcing A_S_A2" );
        ensure_apps(Sxy,z, xz,yz);
    }
}
inline void enforce_A_S_A3 (App eqn)
{// Sxyz=Sxy*z | Sx=S*x, Sx*y=Sxy, x*z=xz, y*z=yz, xz*yz=xz_yz
    Ob S    = *Atoms::S;
    Ob Sxy  = get_lhs(eqn);
    Ob z    = get_rhs(eqn);
    Ob Sxyz = get_app(eqn);

    for (Alr_Iterator Sxy_iter(Sxy); Sxy_iter.ok(); Sxy_iter.next()) {
        Ob Sx = Sxy_iter.lhs();
        Ob x  = find_ldiv(S,Sx); if (!x) continue;
        Ob xz = find_app(x,z);   if (!xz) continue;
        Ob y  = Sxy_iter.rhs();
        Ob yz = find_app(y,z);   if (!yz) continue;

        LOG_DEBUG1( "enforcing A_S_A3" );
        ensure_app(Sxyz, xz,yz);
    }
}
inline void enforce_A_S_A4 (App eqn)
{// xz=x*z | S*x=Sx, Sx*y=Sxy, Sxy*z=Sxyz, y*z=yz, xz*yz=xz_yz
    Ob S  = *Atoms::S;
    Ob x  = get_lhs(eqn);
    Ob Sx = find_app(S,x); if (!Sx) return;
    Ob z  = get_rhs(eqn);
    Ob xz = get_app(eqn);

    for (Lra_Iterator Sxy_iter(Sx); Sxy_iter.ok(); Sxy_iter.next()) {
        Ob y     = Sxy_iter.rhs();
        Ob yz    = find_app(y,z); if (!yz) continue;
        Ob Sxy   = Sxy_iter.app();

        LOG_DEBUG1( "enforcing A_S_A4" );
        ensure_apps(Sxy,z, xz,yz);
    }
}
inline void enforce_A_S_A5 (App eqn)
{// yz=y*z | S*x=Sx, Sx*y=Sxy, Sxy*z=Sxyz, x*z=xz, xz*yz=xz_yz
    Ob S  = *Atoms::S;
    Ob y  = get_lhs(eqn);
    Ob z  = get_rhs(eqn);
    Ob yz = get_app(eqn);

    for (Rla_Iterator Sxy_iter(y); Sxy_iter.ok(); Sxy_iter.next()) {
        Ob Sx    = Sxy_iter.lhs();
        Ob x     = find_ldiv(S,Sx); if (!x) continue;
        Ob xz    = find_app(x,z);   if (!xz) continue;
        Ob Sxy   = Sxy_iter.app();

        LOG_DEBUG1( "enforcing A_S_A5" );
        ensure_apps(Sxy,z, xz,yz);
    }
}
inline void enforce_A_S_A6 (App eqn)
{// xz_yz=xz*yz | Sx=S*x, Sxy=Sx*y, Sxy*z=Sxyz, x*z=xz, y*z=yz
    Ob S     = *Atoms::S;
    Ob xz    = get_lhs(eqn);
    Ob yz    = get_rhs(eqn);
    Ob xz_yz = get_app(eqn);

    ARl_Iterator yz_iter; //for below

    for (Alr_Iterator xz_iter(xz); xz_iter.ok(); xz_iter.next()) {
        Ob x  = xz_iter.lhs();
        Ob Sx = find_app(S,x);   if (!Sx) continue;
        Ob z  = xz_iter.rhs();

        for (yz_iter.begin(yz,z); yz_iter.ok(); yz_iter.next()) {
            Ob y    = yz_iter.lhs();
            Ob Sxy  = find_app(Sx,y);  if (!Sxy)  continue;

            LOG_DEBUG1( "enforcing A_S_A6" );
            ensure_app(xz_yz, Sxy,z);
        }
    }
}
inline void enforce_A_S_A (App eqn)
{
    enforce_A_S_A1(eqn);
    enforce_A_S_A2(eqn);
    enforce_A_S_A3(eqn);
    enforce_A_S_A4(eqn);
    enforce_A_S_A5(eqn);
    enforce_A_S_A6(eqn);
}

//axiom A_S2_A: S'xyz = xy(xz)
inline void enforce_A_S2_A1 (App eqn)
{// S'x=S'*x | S'xy=S'x*y, S'xyz=S'xy*z, xy=x*y, xz=x*z, xy_xz=xy*xz
    Ob S  = *Atoms::S2; if (get_lhs(eqn) != S) return;
    Ob x  = get_rhs(eqn);
    Ob Sx = get_app(eqn);

    Lra_Iterator xz_iter(x); if (!xz_iter.ok()) return; //for below

    for (Lra_Iterator Sxy_iter(Sx); Sxy_iter.ok(); Sxy_iter.next()) {
        Ob y   = Sxy_iter.rhs();
        Ob xy = find_app(x,y); if (!xy) continue;
        Ob Sxy = Sxy_iter.app();

        for (xz_iter.begin(); xz_iter.ok(); xz_iter.next()) {
            Ob z     = xz_iter.rhs();
            Ob xz    = xz_iter.app();

            LOG_DEBUG1( "enforcing A_S2_A1" );
            ensure_apps(Sxy,z, xy,xz);
        }
    }
}
inline void enforce_A_S2_A2 (App eqn)
{// S'xy=S'x*y | S'*x=S'x, S'xy*z=S'xyz, x*y=xy, x*z=xz, xy*xz=xy_xz
    TODO();
}
inline void enforce_A_S2_A3 (App eqn)
{// S'xyz=S'xy*z | S'x=S'*x, S'x*y=S'xy, x*z=xz, x*z=xz, xy*xz=xy_xz
    TODO();
}
inline void enforce_A_S2_A4 (App eqn)
{// xy=x*y | S'*x=S'x, S'x*y=S'xy, S'xy*z=S'xyz, x*z=xz, xy*xz=xy_xz
    TODO();
}
inline void enforce_A_S2_A5 (App eqn)
{// xz=yxz | S'*x=S'x, S'x*y=S'xy, S'xy*z=S'xyz, x*z=xz, xy*xz=xy_xz
    TODO();
}
inline void enforce_A_S2_A6 (App eqn)
{// xy*xz=xy_xz | S'x=S'*x, S'xy=S'x*y, S'xy*z=S'xyz, x*y=xy, x*z=xz
    TODO();
}
inline void enforce_A_S2_A (App eqn)
{
    enforce_A_S2_A1(eqn);
    enforce_A_S2_A2(eqn);
    enforce_A_S2_A3(eqn);
    enforce_A_S2_A4(eqn);
    enforce_A_S2_A5(eqn);
    enforce_A_S2_A6(eqn);
}

//theorems compose_pos, compose_pos: Bxy = x*y, CBxy=y*x
inline void enforce_A_compose_pos1 (App eqn)
{// Bx=B*x | Bxy=Bx*y or x*y
    Ob B  = *Atoms::B; if (get_lhs(eqn) != B) return;
    Ob x  = get_rhs(eqn);
    Ob Bx = get_app(eqn);

    for (Lra_Iterator Bxy_iter(Bx); Bxy_iter.ok(); Bxy_iter.next()) {
        Ob y   = Bxy_iter.rhs();
        Ob Bxy = Bxy_iter.app();

        LOG_DEBUG1( "enforcing A_compose_pos1a" );
        ensure_comp(Bxy, x,y);
    }

    for (Lrc_Iterator xy_iter(x); xy_iter.ok(); xy_iter.next()) {
        Ob y  = xy_iter.rhs();
        Ob xy = xy_iter.comp();

        LOG_DEBUG1( "enforcing A_compose_pos1a" );
        ensure_app(xy, Bx,y);
    }
}
inline void enforce_A_compose_pos2 (App eqn)
{// CBx=CB*x | CBxy=CBx*y or y*x
    Ob CB  = *Atoms::CB; if (get_lhs(eqn) != CB) return;
    Ob x   = get_rhs(eqn);
    Ob CBx = get_app(eqn);

    for (Lra_Iterator CBxy_iter(CBx); CBxy_iter.ok(); CBxy_iter.next()) {
        Ob y   = CBxy_iter.rhs();
        Ob CBxy = CBxy_iter.app();

        LOG_DEBUG1( "enforcing A_compose_pos2a" );
        ensure_comp(CBxy, y,x);
    }

    for (Rlc_Iterator yx_iter(x); yx_iter.ok(); yx_iter.next()) {
        Ob y  = yx_iter.lhs();
        Ob yx = yx_iter.comp();

        LOG_DEBUG1( "enforcing A_compose_pos2a" );
        ensure_app(yx, CBx,y);
    }
}
inline void enforce_A_compose_pos3 (App eqn)
{// Bxy=Bx*y | Bx=B*x
    Ob Bx  = get_lhs(eqn);
    Ob B   = *Atoms::B;
    Ob x   = find_ldiv(B,Bx); if(!x) return;
    Ob y   = get_rhs(eqn);
    Ob Bxy = get_app(eqn);

    LOG_DEBUG1( "enforcing A_compose_pos3" );
    ensure_comp(Bxy, x,y);
}
inline void enforce_A_compose_pos4 (App eqn)
{// CBxy=CBx*y | CBx=CB*x
    Ob CBx  = get_lhs(eqn);
    Ob CB   = *Atoms::CB;
    Ob x    = find_ldiv(CB,CBx); if(!x) return;
    Ob y    = get_rhs(eqn);
    Ob CBxy = get_app(eqn);

    LOG_DEBUG1( "enforcing A_compose_pos4" );
    ensure_comp(CBxy, y,x);
}
inline void enforce_C_compose_pos (Comp eqn)
{
    Ob B  = *Atoms::B;
    Ob CB = *Atoms::CB;
    Ob x  = get_lhs(eqn);
    Ob y  = get_rhs(eqn);
    Ob xy = get_comp(eqn);

    if (Ob Bx = find_app(B,x)) {
        LOG_DEBUG1( "enforcing C_compose_pos1" );
        ensure_app(xy, Bx,y);
    }

    if (Ob CBy = find_app(CB,y)) {
        LOG_DEBUG1( "enforcing C_compose_pos2" );
        ensure_app(xy, CBy,x);
    }
}
inline void enforce_A_compose_pos (App eqn)
{
    enforce_A_compose_pos1(eqn);
    enforce_A_compose_pos2(eqn);
    enforce_A_compose_pos3(eqn);
    enforce_A_compose_pos4(eqn);
}

//theorem A_C_A: Cxyz = (xz)y
inline void enforce_A_C_A1 (App eqn)
{// Cx=C*x | Cxzy=Cxy*z, Cxy=Cx*y, xz=x*z, xzy=xz*y
    Ob C  = *Atoms::C; if (get_lhs(eqn) != C) return;
    Ob x  = get_rhs(eqn);
    Ob Cx = get_app(eqn);

    Lra_Iterator xz_iter(x); if (!xz_iter.ok()) return; //for below

    for (Lra_Iterator Cxy_iter(Cx); Cxy_iter.ok(); Cxy_iter.next()) {
        Ob Cxy = Cxy_iter.app();
        Ob y   = Cxy_iter.rhs();

        for (xz_iter.begin(); xz_iter.ok(); xz_iter.next()) {
            Ob z   = xz_iter.rhs();
            Ob xz  = xz_iter.app();
            Ob xzy = find_app(xz,y);

            if (xzy) {
                LOG_DEBUG1( "enforcing A_C_E1a" );
                ensure_app(xzy, Cxy,z);
            } else {
                Ob Cxyz = find_app(Cxy,z); if (!Cxyz) continue;
                LOG_DEBUG1( "enforcing A_C_E1b" );
                ensure_app(Cxyz, xz,y);
            }
        }
    }
}
inline void enforce_A_C_A2 (App eqn)
{// Cxy=Cx*y | Cxzy=Cxy*z, Cx=C*x, xz=x*z, xzy=xz*y
    Ob C   = *Atoms::C;
    Ob Cx  = get_lhs(eqn);
    Ob x   = find_ldiv(C,Cx); if (!x) return;
    Ob y   = get_rhs(eqn);
    Ob Cxy = get_app(eqn);

    for (Lra_Iterator xz_iter(x); xz_iter.ok(); xz_iter.next()) {
        Ob z   = xz_iter.rhs();
        Ob xz  = xz_iter.app();
        Ob xzy = find_app(xz,y);

        if (xzy) {
            LOG_DEBUG1( "enforcing A_C_E2a" );
            ensure_app(xzy, Cxy,z);
        } else {
            Ob Cxyz = find_app(Cxy,z); if (!Cxyz) continue;
            LOG_DEBUG1( "enforcing A_C_E2b" );
            ensure_app(Cxyz, xz,y);
        }
    }
}
inline void enforce_A_C_A3 (App eqn)
{// Cxyz=Cxy*z | Cxy=Cx*y, Cx=C*x, xz=x*z, xzy=xz*y
    Ob C    = *Atoms::C;
    Ob Cxy  = get_lhs(eqn);
    Ob z    = get_rhs(eqn);
    Ob Cxyz = get_app(eqn);

    for (Alr_Iterator Cxy_iter(Cxy); Cxy_iter.ok(); Cxy_iter.next()) {
        Ob Cx = Cxy_iter.lhs();
        Ob x  = find_ldiv(C,Cx); if (!x) continue;
        Ob xz = find_app(x,z);   if (!xz) continue;
        Ob y  = Cxy_iter.rhs();
        LOG_DEBUG1( "enforcing A_C_A3" );
        ensure_app(Cxyz, xz,y);
    }
}
inline void enforce_A_C_A4 (App eqn)
{// xz=x*z | Cxzy=Cxy*z, Cxy=Cx*y, Cx=C*x, xzy=xz*y
    Ob C  = *Atoms::C;
    Ob x  = get_lhs(eqn);
    Ob Cx = find_app(C,x); if (!Cx) return;
    Ob z  = get_rhs(eqn);
    Ob xz = get_app(eqn);

    for (Lra_Iterator Cxy_iter(Cx); Cxy_iter.ok(); Cxy_iter.next()) {
        Ob y   = Cxy_iter.rhs();
        Ob Cxy = Cxy_iter.app();
        Ob xzy = find_app(xz,y);

        if (xzy) {
            LOG_DEBUG1( "enforcing A_C_E4a" );
            ensure_app(xzy, Cxy,z);
        } else {
            Ob Cxyz = find_app(Cxy,z); if (!Cxyz) continue;
            LOG_DEBUG1( "enforcing A_C_E4b" );
            ensure_app(Cxyz, xz,y);
        }
    }
}
inline void enforce_A_C_A5 (App eqn)
{// xzy=xz*y | Cxzy=Cxy*z, Cxy=Cx*y, Cx=C*x, xz=x*z
    Ob C   = *Atoms::C;
    Ob xz  = get_lhs(eqn);
    Ob y   = get_rhs(eqn);
    Ob xzy = get_app(eqn);

    for (Alr_Iterator xz_iter(xz); xz_iter.ok(); xz_iter.next()) {
        Ob x    = xz_iter.lhs();
        Ob Cx   = find_app(C,x);   if (!Cx)   continue;
        Ob Cxy  = find_app(Cx,y);  if (!Cxy)  continue;
        Ob z    = xz_iter.rhs();
        LOG_DEBUG1( "enforcing A_C_A5" );
        ensure_app(xzy, Cxy,z);
    }
}
inline void enforce_A_C_A (App eqn)
{
    enforce_A_C_A1(eqn);
    enforce_A_C_A2(eqn);
    enforce_A_C_A3(eqn);
    enforce_A_C_A4(eqn);
    enforce_A_C_A5(eqn);
}

//================ extension theory axioms ================

//theorem A_Y_A1: Yf = f(Yf)
inline void enforce_A_Y_A1 (App eqn)
{// Yf=Y*f
    Ob Y = *Atoms::Y; if (get_lhs(eqn) != Y) return;
    Ob f  = get_rhs(eqn);
    Ob Yf = get_app(eqn);

    LOG_DEBUG1( "enforcing A_Y_A1" );
    ensure_app(Yf, f,Yf);
}
//axiom A_Y_A2: SIy=y |- Y=y  (i.e. Y is a least fixed point)
inline void enforce_A_Y_A2 (App eqn)
{// SIy=y
    if (get_lhs(eqn) != *Atoms::SI) return;
    Ob y   = get_rhs(eqn);
    Ob SIy = get_app(eqn); if (SIy != y) return;
    LOG_DEBUG1( "enforcing A_Y_L2" );
    Ob Y = *Atoms::Y;
    ensure_equiv(Y,y);
}
//axiom Y_L: fx[=x |- Yf[=x
inline void enforce_A_Y_L (App eqn)
{// fx=f*x | fx[=x
    Ob fx = get_app(eqn);
    Ob x  = get_rhs(eqn); if (not is_less(fx,x)) return;
    Ob Y  = *Atoms::Y;
    Ob f  = get_lhs(eqn);
    Ob Yf = find_app(Y,f); if (!Yf) return;
    if (Yf == x) return;
    LOG_DEBUG1( "enforcing A_Y_L" );
    ensure_less(Yf,x);
}
inline void enforce_L_Y_L (Ord ord)
{// fx[=x | fx=f*x
    Ob Y = *Atoms::Y;
    Ob fx = ord.lhs();
    Ob x  = ord.rhs();
    for (ARl_Iterator fx_iter(fx,x); fx_iter.ok(); fx_iter.next()) {

        Ob f = fx_iter.lhs();
        Ob Yf = find_app(Y,f); if (!Yf) continue;
        if (Yf == x) return;
        LOG_DEBUG1( "enforcing A_Y_L" );
        ensure_less(Yf,x);
    }
}
inline void enforce_A_Y_A (App eqn)
{
    enforce_A_Y_A1(eqn);
    enforce_A_Y_A2(eqn);
}
inline void enforce_L_Y_pos (Ord ord)
{
    enforce_L_Y_L(ord);
}

//theorem A_W_A: Wxy=xyy
inline void enforce_A_W_A1 (App eqn)
{// Wx=W*x | xy=x*y, Wxy=Wx*y or xyy=xy*y
    if (get_lhs(eqn) != *Atoms::W) return;
    Ob x  = get_rhs(eqn);
    Ob Wx = get_app(eqn);

    for (Lra_Iterator xy_iter(x); xy_iter.ok(); xy_iter.next()) {
        Ob y   = xy_iter.rhs();
        Ob xy  = xy_iter.app();
        LOG_DEBUG1( "enforcing A_W_A1" );
        ensure_apps(Wx,y, xy,y);
    }
}
inline void enforce_A_W_A2 (App eqn)
{// xy=x*y | Wx=W*x, Wxy=Wx*y or xyy=xy*y
    Ob x  = get_lhs(eqn);
    Ob Wx = find_app(*Atoms::W, x); if (!Wx) return;
    Ob y  = get_rhs(eqn);
    Ob xy = get_app(eqn);
    LOG_DEBUG1( "enforcing A_W_A2" );
    ensure_apps(Wx,y, xy,y);
}
inline void enforce_A_W_A3 (App eqn)
{// Wxy=Wx*y | Wx=W*x, xy=x*y
    Ob W   = *Atoms::W;
    Ob Wx  = get_lhs(eqn);
    Ob y   = get_rhs(eqn);
    Ob Wxy = get_app(eqn);
    for (ALr_Iterator Wx_iter(Wx,W); Wx_iter.ok(); Wx_iter.next()) {
        Ob x  = Wx_iter.rhs();
        Ob xy = find_app(x,y); if (!xy) continue;
        LOG_DEBUG1( "enforcing A_W_A3" );
        ensure_app(Wxy, xy,y);
    }
}
inline void enforce_A_W_A4 (App eqn)
{// xyy=xy*y | Wx=W*x, xy=x*y
    Ob W   = *Atoms::W;
    Ob xy  = get_lhs(eqn);
    Ob y   = get_rhs(eqn);
    Ob xyy = get_app(eqn);

    for (ARl_Iterator xy_iter(xy,y); xy_iter.ok(); xy_iter.next()) {
        Ob x  = xy_iter.lhs();
        Ob Wx = find_app(W,x); if (!Wx) continue;
        LOG_DEBUG1( "enforcing A_W_A4" );
        ensure_app(xyy, Wx,y);
    }
}
inline void enforce_A_W_A (App eqn)
{
    enforce_A_W_A1(eqn);
    enforce_A_W_A2(eqn);
    enforce_A_W_A3(eqn);
    enforce_A_W_A4(eqn);
}

//================ retract/closure axioms ================

//retraction axioms (def: f is a _retract_ iff f*f[=f)
//axiom retract:  Ux=x |- x*x[=x  (i.e., x:U ==> x a retract)
inline void enforce_A_U_pos1 (App eqn)
{// Uy=x | x*x
    Ob U = *Atoms::U; if (get_lhs(eqn) != U) return;
    Ob x = get_app(eqn);

    LOG_DEBUG1( "enforcing A_U_pos1a" );
    ensure_app(x, U,x);

    Ob xx = find_comp(x,x); if (!xx) return;

    LOG_DEBUG1( "enforcing A_U_pos1b" );
    ensure_less(xx, x);
}
inline void enforce_C_U_pos1 (Comp eqn)
{// x*x | Uy=x
    Ob x  = get_rhs(eqn); if (x != get_lhs(eqn)) return;
    Ob U  = *Atoms::U; if (find_app(U,x) != x) return;
    Ob xx = get_comp(eqn);

    LOG_DEBUG1( "enforcing C_U_pos1" );
    ensure_less(xx, x);
}
//axiom retracts: x*x[=x |- Ux=x  (i.e., x a retract ==> x:U)
inline void enforce_C_U_pos2 (Comp eqn)
{// x*x | x*x[=x
    Ob x  = get_lhs(eqn); if (x != get_rhs(eqn)) return;
    Ob xx = get_comp(eqn); if (not is_less(xx,x)) return;
    Ob U   = *Atoms::U;

    LOG_DEBUG1( "enforcing C_U_pos2" );
    ensure_app(x, U,x);
}
inline void enforce_L_U_pos1 (Ord ord)
{// x*x[=x | x*x
    Ob B   = *Atoms::B;
    Ob x   = ord.rhs();
    Ob Bx  = find_app(B,x); if (!Bx) return;
    Ob Bxx = find_app(Bx,x); if (ord.lhs() != Bxx) return; //Bxx may be 0
    Ob U   = *Atoms::U;

    LOG_DEBUG1( "enforcing L_U_pos" );
    ensure_app(x, U,x);
}
//axiom fix: fx[=x |- (Uf)x=fx
inline void enforce_L_U_pos2 (Ord ord)
{// fx[=x | Uf=U*f, fx=f*x
    Ob U  = *Atoms::U;
    Ob fx = ord.lhs();
    Ob x  = ord.rhs();
    for (ARl_Iterator fx_iter(fx,x); fx_iter.ok(); fx_iter.next()) {
        Ob f  = fx_iter.lhs();
        Ob Uf = find_app(U,f); if (!Uf) continue;

        LOG_DEBUG1( "enforcing L_U_pos2" );
        ensure_app(fx, Uf,x);
    }
}
inline void enforce_A_U_pos2 (App eqn)
{// Uf=U*f | fx[=x, fx=f*x
    if (get_lhs(eqn) != *Atoms::U) return;
    Ob Uf = get_app(eqn);
    Ob f  = get_rhs(eqn); //if (f == Uf) return; //would be unnecessary
    for (Lra_Iterator fx_iter(f); fx_iter.ok(); fx_iter.next()) {
        Ob x  = fx_iter.rhs();
        Ob fx = fx_iter.app(); if (not is_less(fx, x)) continue;

        LOG_DEBUG1( "enforcing A_U_pos2" );
        ensure_app(fx, Uf,x);
    }
}
inline void enforce_A_U_pos3 (App eqn)
{// fx=f*x | Uf=U*f, fx[=x
    Ob x  = get_rhs(eqn);
    Ob fx = get_app(eqn); if (not is_less(fx,x)) return;
    Ob f  = get_lhs(eqn);
    Ob U  = *Atoms::U;
    Ob Uf = find_app(U,f); if (!Uf) return;

    LOG_DEBUG1( "enforcing A_U_pos3" );
    ensure_app(fx, Uf,x);
}
inline void enforce_A_U_pos (App eqn)
{
    enforce_A_U_pos1(eqn);
    enforce_A_U_pos2(eqn);
    enforce_A_U_pos3(eqn);
}
inline void enforce_C_U_pos (Comp eqn)
{
    enforce_C_U_pos1(eqn);
    enforce_C_U_pos2(eqn);
}
inline void enforce_L_U_pos (Ord ord)
{
    enforce_L_U_pos1(ord);
    enforce_L_U_pos2(ord);
}

//closure axioms (def: f is a _closure_ iff f*f=f=]I)
//axiom: Vx=x |- x*x=x,  Vx=x |- I[=x  (i.e., x:V ==> x a closure)
inline void enforce_A_V_pos1(App eqn)
{// Vx=x
    if (get_lhs(eqn) != *Atoms::V) return;
    Ob x = get_app(eqn); if (x != get_rhs(eqn)) return;

    LOG_DEBUG1( "enforcing A_V_pos1a" );
    ensure_less(*Atoms::I, x);

    LOG_DEBUG1( "enforcing A_V_pos1b" );
    ensure_comp(x, x,x);
}
//axiom: I[=x=x*x |- Vx=x  (i.e., x a closure ==> x:V)
inline void enforce_C_V_pos (Comp eqn)
{// x=x*x | I[=x
    Ob x   = get_comp(eqn);
    if (x != get_lhs(eqn)) return;
    if (x != get_rhs(eqn)) return;
    Ob I   = *Atoms::I; if (not is_less(I,x)) return;

    LOG_DEBUG1( "enforcing C_V_pos" );
    ensure_app(x, *Atoms::V,x);
}
inline void enforce_L_V_pos1 (Ord ord)
{//I[=x | x=x*x
    Ob I  = ord.lhs(); if (I!=*Atoms::I) return;
    Ob x  = ord.rhs();
    Ob xx = find_comp(x,x); if (xx != x) return;

    LOG_DEBUG1( "enforcing L_V_pos1" );
    ensure_app(x, *Atoms::V,x);
}
//axiom: fx[=x |- Vfx=x
inline void enforce_L_V_pos2 (Ord ord)
{// f x[=x | V f, f x
    Ob V  = *Atoms::V;
    Ob fx = ord.lhs();
    Ob x  = ord.rhs();
    for (ARl_Iterator fx_iter(fx,x); fx_iter.ok(); fx_iter.next()) {
        Ob f  = fx_iter.lhs();
        Ob Vf = find_app(V,f); if (!Vf) continue;

        LOG_DEBUG1( "enforcing L_V_pos2" );
        ensure_app(x, Vf,x);
    }
}
inline void enforce_A_V_pos2 (App eqn)
{// V f | f x[=x
    if (get_lhs(eqn) != *Atoms::V) return;
    Ob Vf = get_app(eqn);
    Ob f  = get_rhs(eqn); //if (f == Vf) return; //would be unnecessary
    for (Lra_Iterator fx_iter(f); fx_iter.ok(); fx_iter.next()) {
        Ob x  = fx_iter.rhs();
        Ob fx = fx_iter.app(); if (not is_less(fx, x)) continue;

        LOG_DEBUG1( "enforcing A_V_pos2" );
        ensure_app(x, Vf,x);
    }
}
inline void enforce_A_V_pos3 (App eqn)
{// f x | V f, f x[=x
    Ob x  = get_rhs(eqn);
    Ob fx = get_app(eqn); if (not is_less(fx,x)) return;
    Ob f  = get_lhs(eqn);
    Ob V  = *Atoms::V;
    Ob Vf = find_app(V,f); if (!Vf) return;
    LOG_DEBUG1( "enforcing A_V_A6" );
    ensure_app(x, Vf,x);
}
inline void enforce_A_V_pos (App eqn)
{
    enforce_A_V_pos1(eqn);
    enforce_A_V_pos2(eqn);
    enforce_A_V_pos3(eqn);
}
inline void enforce_L_V_pos (Ord ord)
{
    enforce_L_V_pos1(ord);
    enforce_L_V_pos2(ord);
}

//intersection axioms
//axiom A_P_A: a,b:V, x:a,b |- x:Pab
/* TODO();
inline enforce_A_P_A1 (App eqn)
{// a:V | b:V, x:a, x:b
    Ob V  = *Atoms::V; if (get_lhs(eqn) != V) return;
    Ob a  = get_rhs(eqn);  if (get_app(eqn) != a) return;
    Ob P  = *Atoms::P;
    Ob Pa = find_app(P,a); if (!Pa) return;
    for (Lra_Iterator Pab_iter(Pa); Pab_iter.ok(); Pab_iter.next()) {
        Ob b   = Pab_iter.rhs(); if (b == a) continue; //unnecessary
        Ob Vb  = find_app(V,b); if (Vb != b) continue; //Vb may be 0
        Ob Pab = Pab_iter.app();
        for (LLra_Iterator ab_iter(a,b); ab_iter.ok(); ab_iter.next()) {
            Ob x = ab_iter.rhs();
            if (x != ab_iter.app1()) continue;
            if (x != ab_iter.app2()) continue;
            LOG_DEBUG1( "enforcing A_P_A1" );
            ensure_app(x, Pab,x);
        }
    }
}
*/
inline void enforce_A_P_A (App eqn)
{
    Ob P = *Atoms::P;
    Ob V = *Atoms::V;
    enforce_A_idempotence_A(P, V, eqn);
    enforce_A_commutativity_A(P, eqn);
    enforce_A_associativity_A(P, eqn);
}

//================ individual types ================

//Note: all conditions of is_const(...)'s are unnecessary below

//HACK there's a lot of copy & pasting in the quantification axioms

//-------- axioms Div --------
//axiom: xBot=Bot |- x[=Div, i.e., Div is the largest strict function
inline void enforce_A_Div_pos1 (App eqn)
{//Bot=x*Bot
    Ob Bot = *Atoms::Bot;
    if (get_app(eqn) != Bot) return;
    if (get_rhs(eqn) != Bot) return;
    LOG_DEBUG1( "enforcing A_Div_L" );
    ensure_less(get_lhs(eqn), *Atoms::Div);
}
//axiom: x![=Bot |- Divx=Top
inline void enforce_N_Div_pos (Ord ord)
{//x![=Bot
    if (ord.rhs() != *Atoms::Bot) return;
    LOG_DEBUG1( "enforcing N_Div_A" );
    ensure_app(*Atoms::Top, *Atoms::Div,ord.lhs());
}
//axiom: f=f*div, g=g*div, f _[=g _, f T[=g T |- f[=g
inline void enforce_L_Div_pos1 (Ord ord, Ob x, Ob y)
{// f _[=g _ | f T[=g T, f*div, g*div, and permutations
    Ob fx  = ord.lhs();
    Ob gx  = ord.rhs();
    Ob Div = *Atoms::Div;

    ARl_Iterator gx_iter; //for below

    for (ARl_Iterator fx_iter(fx,x); fx_iter.ok(); fx_iter.next()) {
        Ob f  = fx_iter.lhs(); if (f != find_comp(f,Div)) continue;
        Ob fy = find_app(f,y); if (!fy) continue;
        if (is_const(f)) continue;

        for (gx_iter.begin(gx,x); gx_iter.ok(); gx_iter.next()) {
            Ob g  = gx_iter.lhs(); if (g != find_comp(g,Div)) continue;
            Ob gy = find_app(g,y); if (!gy) continue;
            if (is_const(g)) continue;

            if (not is_less(fy,gy)) continue;

            LOG_DEBUG1( "enforcing L_Div_pos1" );
            ensure_less(f,g);
        }
    }
}
inline void enforce_C_Div_pos1 (Comp eqn, const Set& gs)
{// f=f*div | g=g*div, f _[=g _, f T[=g T
    Ob Div = *Atoms::Div; if (get_rhs(eqn) != Div) return;
    Ob f   = get_comp(eqn); if (get_lhs(eqn) != f) return;
    if (is_const(f)) return;
    Ob x   = *Atoms::Bot;  Ob fx  = find_app(f,x); if (!fx) return;
    Ob y   = *Atoms::Top;  Ob fy  = find_app(f,y); if (!fy) return;

    for (Set::iterator g_iter(gs); g_iter.ok(); g_iter.next()) {
        Ob g  = Ob(*g_iter); if (g != find_comp(g,Div)) continue;
        if (is_const(g)) continue;
        Ob gx = find_app(g,x);
        Ob gy = find_app(g,y);

        if (is_less(fx,gx) and is_less(fy,gy)) {
            LOG_DEBUG1( "enforcing C_Div_pos1a" );
            ensure_less(f,g);
        }

        if (is_less(gx,fx) and is_less(gy,fy)) {
            LOG_DEBUG1( "enforcing C_Div_pos1b" );
            ensure_less(g,f);
        }
    }
}
inline void enforce_A_Div_pos2 (App eqn, Ob x, Ob y, const Set& gs)
{// f _ | f _[=g _, f T[=g T, f*div, g*div, and permutations
    if (x != get_rhs(eqn)) return;
    Ob f   = get_lhs(eqn);
    Ob fy  = find_app(f,y); if (!fy) return;
    Ob Div = *Atoms::Div; if (f != find_comp(f,Div)) return;
    if (is_const(f)) return;
    Ob fx  = get_app(eqn);

    for (Set::iterator g_iter(gs); g_iter.ok(); g_iter.next()) {
        Ob g  = Ob(*g_iter); if (g != find_comp(g,Div)) continue;
        if (is_const(g)) continue;
        Ob gx = find_app(g,x);
        Ob gy = find_app(g,y);

        if (is_less(fx,gx) and is_less(fy,gy)) {
            LOG_DEBUG1( "enforcing A_Div_pos2a" );
            ensure_less(f,g);
        }

        if (is_less(gx,fx) and is_less(gy,fy)) {
            LOG_DEBUG1( "enforcing A_Div_pos2b" );
            ensure_less(g,f);
        }
    }
}
//sorted into contexts
inline void enforce_A_Div_pos (App eqn)
{
    enforce_A_Div_pos1(eqn);

    Ob Bot = *Atoms::Bot;
    Ob Top = *Atoms::Top;
    const Set& gs = get_funs_of_div();

    enforce_A_Div_pos2(eqn, Bot, Top, gs);
    enforce_A_Div_pos2(eqn, Top, Bot, gs);
}
inline void enforce_C_Div_pos (Comp eqn)
{
    const Set& gs = get_funs_of_div();

    enforce_C_Div_pos1(eqn, gs);
}
inline void enforce_L_Div_pos (Ord ord)
{
    Ob Bot = *Atoms::Bot;
    Ob Top = *Atoms::Top;

    enforce_L_Div_pos1(ord, Bot, Top);
    enforce_L_Div_pos1(ord, Top, Bot);
}

//-------- axioms Unit --------
//axiom: x I [= I |- x[=Unit, i.e., Unit is the largest function : I |--> I
inline void enforce_A_Unit_pos1 (App eqn)
{//xI=x*I | xI[=I
    Ob I = *Atoms::I; if (get_rhs(eqn) != I) return;
    Ob xI = get_app(eqn); if (not is_less(xI,I)) return;
    LOG_DEBUG1( "enforcing A_Unit_pos1" );
    ensure_less(get_lhs(eqn), *Atoms::Unit);
}
inline void enforce_L_Unit_pos1 (Ord ord)
{//xI[=I | xI=x*I
    Ob I    = *Atoms::I; if (ord.rhs() != I) return;
    Ob xI   = ord.lhs();
    Ob Unit = *Atoms::Unit;
    for (ARl_Iterator xI_iter(xI,I); xI_iter.ok(); xI_iter.next()) {

        LOG_DEBUG1( "enforcing L_Unit_pos1" );
        ensure_less(xI_iter.lhs(), Unit);
    }
}
//axiom: x![=I |- Unitx=Top
inline void enforce_N_Unit_pos (Ord ord)
{//x![=I
    if (ord.rhs() != *Atoms::I) return;
    LOG_DEBUG1( "enforcing N_Unit_pos" );
    ensure_app(*Atoms::Top, *Atoms::Unit,ord.lhs());
}
//axiom: f=f*unit, g=g*unit, f I[=g I, f T[=g T |- f[=g
inline void enforce_L_Unit_pos2 (Ord ord, Ob x, Ob y)
{// f I[=g I | f T[=g T, f*div, g*div, and permutations
    Ob fx  = ord.lhs();
    Ob gx  = ord.rhs();
    Ob Unit = *Atoms::Unit;

    ARl_Iterator gx_iter; //for below

    for (ARl_Iterator fx_iter(fx,x); fx_iter.ok(); fx_iter.next()) {
        Ob f  = fx_iter.lhs(); if (f != find_comp(f,Unit)) continue;
        Ob fy = find_app(f,y); if (!fy) continue;
        if (is_const(f)) continue;

        for (gx_iter.begin(gx,x); gx_iter.ok(); gx_iter.next()) {
            Ob g  = gx_iter.lhs(); if (g != find_comp(g,Unit)) continue;
            Ob gy = find_app(g,y); if (!gy) continue;
            if (is_const(g)) continue;

            if (not is_less(fy,gy)) continue;

            LOG_DEBUG1( "enforcing L_Unit_pos2" );
            ensure_less(f,g);
        }
    }
}
inline void enforce_C_Unit_pos1 (Comp eqn, const Set& gs)
{// f=f*div | g=g*div, f I[=g I, f T[=g T
    Ob Unit = *Atoms::Unit; if (get_rhs(eqn) != Unit) return;
    Ob f   = get_comp(eqn); if (get_lhs(eqn) != f) return;
    if (is_const(f)) return;
    Ob x   = *Atoms::Bot;  Ob fx  = find_app(f,x); if (!fx) return;
    Ob y   = *Atoms::Top; Ob fy  = find_app(f,y); if (!fy) return;

    for (Set::iterator g_iter(gs); g_iter.ok(); g_iter.next()) {
        Ob g  = Ob(*g_iter); if (g != find_comp(g,Unit)) continue;
        if (is_const(g)) continue;
        Ob gx = find_app(g,x);
        Ob gy = find_app(g,y);

        if (is_less(fx,gx) and is_less(fy,gy)) {
            LOG_DEBUG1( "enforcing C_Unit_pos1a" );
            ensure_less(f,g);
        }

        if (is_less(gx,fx) and is_less(gy,fy)) {
            LOG_DEBUG1( "enforcing C_Unit_pos1b" );
            ensure_less(g,f);
        }
    }
}
inline void enforce_A_Unit_pos2 (App eqn, Ob x, Ob y, const Set& gs)
{// f I | f I[=g I, f T[=g T, f*div, g*div, and permutations
    if (x != get_rhs(eqn)) return;
    Ob f   = get_lhs(eqn);
    Ob fy  = find_app(f,y); if (!fy) return;
    Ob Unit = *Atoms::Unit; if (f != find_comp(f,Unit)) return;
    if (is_const(f)) return;
    Ob fx  = get_app(eqn);

    for (Set::iterator g_iter(gs); g_iter.ok(); g_iter.next()) {
        Ob g  = Ob(*g_iter); if (g != find_comp(g,Unit)) continue;
        if (is_const(g)) continue;
        Ob gx = find_app(g,x);
        Ob gy = find_app(g,y);

        if (is_less(fx,gx) and is_less(fy,gy)) {
            LOG_DEBUG1( "enforcing A_Unit_pos2a" );
            ensure_less(f,g);
        }

        if (is_less(gx,fx) and is_less(gy,fy)) {
            LOG_DEBUG1( "enforcing A_Unit_pos2b" );
            ensure_less(g,f);
        }
    }
}
//sorted into contexts
inline void enforce_A_Unit_pos (App eqn)
{
    enforce_A_Unit_pos1(eqn);

    Ob I = *Atoms::I;
    Ob Top = *Atoms::Top;
    const Set& gs = get_funs_of_unit();

    enforce_A_Unit_pos2(eqn, I, Top, gs);
    enforce_A_Unit_pos2(eqn, Top, I, gs);
}
inline void enforce_C_Unit_pos (Comp eqn)
{
    const Set& gs = get_funs_of_unit();

    enforce_C_Unit_pos1(eqn, gs);
}
inline void enforce_L_Unit_pos (Ord ord)
{
    enforce_L_Unit_pos1(ord);

    Ob I   = *Atoms::I;
    Ob Top = *Atoms::Top;

    enforce_L_Unit_pos2(ord, I, Top);
    enforce_L_Unit_pos2(ord, Top, I);
}

//--------  axioms Semi --------
//axiom: xBot=Bot, xI[=I |- x[=Semi
inline void enforce_A_Semi_pos1 (App eqn)
{//Bot=x*Bot | xI=x*I, xI[=I
    Ob Bot = *Atoms::Bot; if (get_rhs(eqn)!=Bot or get_app(eqn)!=Bot) return;
    Ob x   = get_lhs(eqn);
    Ob I   = *Atoms::I;
    Ob xI  = find_app(x,I); if (!xI) return;
    if (not is_less(xI,I)) return;

    LOG_DEBUG1( "enforcing A_Semi_pos1" );
    ensure_less(x, *Atoms::Semi);
}
inline void enforce_A_Semi_pos2 (App eqn)
{//xI=x*I | Bot=x*Bot, xI[=I
    Ob I    = *Atoms::I; if (get_rhs(eqn) != I) return;
    Ob xI   = get_app(eqn); if (not is_less(xI,I)) return;
    Ob x    = get_lhs(eqn);
    Ob Bot  = *Atoms::Bot;
    Ob xBot = find_app(x,Bot); if (xBot!=Bot) return;

    LOG_DEBUG1( "enforcing A_Semi_pos" );
    ensure_less(x, *Atoms::Semi);
}
inline void enforce_L_Semi_pos1 (Ord ord)
{//xI[=I | Bot=x*Bot, xI=x*I
    Ob I    = *Atoms::I; if (ord.rhs() != I) return;
    Ob xI   = ord.lhs();
    Ob Bot  = *Atoms::Bot;
    Ob Semi = *Atoms::Semi;

    for (ARl_Iterator xI_iter(xI,I); xI_iter.ok(); xI_iter.next()) {
        Ob x    = xI_iter.lhs();
        Ob xBot = find_app(x,Bot); if (xBot!=Bot) continue;

        LOG_DEBUG1( "enforcing L_Semi_pos1" );
        ensure_less(x, Semi);
    }
}
//axiom: x![=I |- Semix=Top
inline void enforce_N_Semi_pos1 (Ord ord)
{//x![=I
    if (ord.rhs() != *Atoms::I) return;
    LOG_DEBUG1( "enforcing N_Semi_pos1" );
    ensure_app(*Atoms::Top, *Atoms::Semi,ord.lhs());
}
//axiom: x![=Bot |- Semix=]I
inline void enforce_N_Semi_pos2 (Ord ord)
{//x![=Bot | Semix=Semi*x
    if (ord.rhs() != *Atoms::Bot) return;
    Ob x     = ord.lhs();
    Ob Semi  = *Atoms::Semi;
    Ob Semix = find_app(Semi,x); if (!Semix) return;
    LOG_DEBUG1( "enforcing N_Semi_pos2" );
    ensure_less(*Atoms::I, Semix);
}
inline void enforce_A_Semi_pos3 (App eqn)
{//Semix=Semi*x | x![=Bot
    Ob Semi  = *Atoms::Semi; if (get_lhs(eqn) != Semi) return;
    Ob x     = get_rhs(eqn); if (not converges(x)) return;
    Ob Semix = get_app(eqn);
    LOG_DEBUG1( "enforcing A_Semi_pos3" );
    ensure_less(*Atoms::I, Semix);
}
//axiom: f=f*semi, g=g*semi, f _[=g _, f I[=g I, f T[=g T |- f[=g
inline void enforce_L_Semi_pos2 (Ord ord, Ob x, Ob y, Ob z)
{// f _[=g _ | f I[=g I, f T[=g T, f*semi, g*semi, and permutations
    Ob fx  = ord.lhs();
    Ob gx  = ord.rhs();
    Ob Semi = *Atoms::Semi;

    ARl_Iterator gx_iter; //for below

    for (ARl_Iterator fx_iter(fx,x); fx_iter.ok(); fx_iter.next()) {
        Ob f  = fx_iter.lhs(); if (f != find_comp(f,Semi)) continue;
        Ob fy = find_app(f,y); if (!fy) continue;
        Ob fz = find_app(f,z); if (!fz) continue;
        if (is_const(f)) continue;

        for (gx_iter.begin(gx,x); gx_iter.ok(); gx_iter.next()) {
            Ob g  = gx_iter.lhs(); if (g != find_comp(g,Semi)) continue;
            Ob gy = find_app(g,y); if (!gy) continue;
            Ob gz = find_app(g,z); if (!gz) continue;
            if (is_const(g)) continue;

            if (is_less(fy,gy) and is_less(fz,gz)) {
                LOG_DEBUG1( "enforcing L_Semi_pos1" );
                ensure_less(f,g);
            }
        }
    }
}
inline void enforce_C_Semi_pos1 (Comp eqn, const Set& gs)
{// f=f*semi | g=g*semi, f _[=g _, f I[=g I, f T[=g T
    Ob Semi = *Atoms::Semi; if (get_rhs(eqn) != Semi) return;
    Ob f   = get_comp(eqn); if (get_lhs(eqn) != f) return;
    if (is_const(f)) return;
    Ob x   = *Atoms::Bot;  Ob fx  = find_app(f,x); if (!fx) return;
    Ob y   = *Atoms::Top;  Ob fy  = find_app(f,y); if (!fy) return;
    Ob z   = *Atoms::I;    Ob fz  = find_app(f,z); if (!fz) return;

    for (Set::iterator g_iter(gs); g_iter.ok(); g_iter.next()) {
        Ob g  = Ob(*g_iter); if (g != find_comp(g,Semi)) continue;
        if (is_const(g)) continue;
        Ob gx = find_app(g,x);
        Ob gy = find_app(g,y);
        Ob gz = find_app(g,z);

        if (is_less(fx,gx) and is_less(fy,gy) and is_less(fz,gz)) {
            LOG_DEBUG1( "enforcing C_Semi_pos1a" );
            ensure_less(f,g);
        }

        if (is_less(gx,fx) and is_less(gy,fy) and is_less(gz,fz)) {
            LOG_DEBUG1( "enforcing C_Semi_pos1b" );
            ensure_less(g,f);
        }
    }
}
inline void enforce_A_Semi_pos4 (App eqn, Ob x, Ob y, Ob z, const Set& gs)
{// f _ | f _[=g _, f I[=g I, f T[=g T, f*semi, g*semi, and permutations
    if (x != get_rhs(eqn)) return;
    Ob f   = get_lhs(eqn);
    Ob fy  = find_app(f,y); if (!fy) return;
    Ob fz  = find_app(f,z); if (!fz) return;
    Ob Semi = *Atoms::Semi; if (f != find_comp(f,Semi)) return;
    if (is_const(f)) return;
    Ob fx  = get_app(eqn);

    for (Set::iterator g_iter(gs); g_iter.ok(); g_iter.next()) {
        Ob g  = Ob(*g_iter); if (g != find_comp(g,Semi)) continue;
        if (is_const(g)) continue;
        Ob gx = find_app(g,x);
        Ob gy = find_app(g,y);
        Ob gz = find_app(g,z);

        if (is_less(fx,gx) and is_less(fy,gy) and is_less(fz,gz)) {
            LOG_DEBUG1( "enforcing A_Semi_pos4a" );
            ensure_less(f,g);
        }

        if (is_less(gx,fx) and is_less(gy,fy) and is_less(gz,fz)) {
            LOG_DEBUG1( "enforcing A_Semi_pos4b" );
            ensure_less(g,f);
        }
    }
}
//sorted into contexts
inline void enforce_A_Semi_pos (App eqn)
{
    enforce_A_Semi_pos1(eqn);
    enforce_A_Semi_pos2(eqn);
    enforce_A_Semi_pos3(eqn);

    Ob x = *Atoms::Top;
    Ob y = *Atoms::Bot;
    Ob z = *Atoms::I;

    const Set& gs = get_funs_of_semi();

    enforce_A_Semi_pos4(eqn, x, y, z, gs);
    enforce_A_Semi_pos4(eqn, z, x, y, gs);
    enforce_A_Semi_pos4(eqn, y, z, x, gs);
}
inline void enforce_C_Semi_pos (Comp eqn)
{
    const Set& gs = get_funs_of_semi();

    enforce_C_Semi_pos1(eqn, gs);
}
inline void enforce_L_Semi_pos (Ord ord)
{
    enforce_L_Semi_pos1(ord);

    Ob x = *Atoms::Top;
    Ob y = *Atoms::Bot;
    Ob z = *Atoms::I;

    enforce_L_Semi_pos2(ord, x, y, z);
    enforce_L_Semi_pos2(ord, z, x, y);
    enforce_L_Semi_pos2(ord, y, z, x);
}
inline void enforce_N_Semi_pos (Ord ord)
{
    enforce_N_Semi_pos1(ord);
    enforce_N_Semi_pos2(ord);
}

//--------  axioms Bool --------
//axiom: x![=Bot, x[=K |- Bool x=K
//axiom: x![=Bot, x[=F |- Bool x=F
inline void enforce_L_Bool_pos1 (Ord ord)
{// x[=K | x![=Bot
    Ob K    = ord.rhs(); if (K != *Atoms::K) return;
    Ob x    = ord.lhs(); if (not converges(x)) return;
    Ob Bool = *Atoms::Bool;

    LOG_DEBUG1( "enforcing L_Bool_pos1" );
    ensure_app(K, Bool,x);
}
inline void enforce_L_Bool_pos2 (Ord ord)
{// x[=KI | x![=Bot
    Ob KI   = ord.rhs(); if (KI != *Atoms::KI) return;
    Ob x    = ord.lhs(); if (not converges(x)) return;
    Ob Bool = *Atoms::Bool;

    LOG_DEBUG1( "enforcing L_Bool_pos2" );
    ensure_app(KI, Bool,x);
}
inline void enforce_N_Bool_pos1 (Ord ord)
{// x![=Bot | x[=K or x[=F
    Ob Bot  = ord.rhs(); if (Bot != *Atoms::Bot) return;
    Ob x    = ord.lhs();
    Ob K    = *Atoms::K;
    Ob KI   = *Atoms::KI;
    Ob Bool = *Atoms::Bool;

    if (is_less(x,K)) {
        LOG_DEBUG1( "enforcing N_Bool_pos1a" );
        ensure_app(K, Bool,x);
    }

    if (is_less(x,KI)) {
        LOG_DEBUG1( "enforcing N_Bool_pos1b" );
        ensure_app(KI, Bool,x);
    }
}
//axiom: x![=F |- K[=Bool x
//axiom: x![=K |- F[=Bool x
inline void enforce_A_Bool_pos1 (App eqn)
{// Bool x | x![=K or x![=F
    Ob Bool = get_lhs(eqn); if (Bool != *Atoms::Bool) return;
    Ob x    = get_rhs(eqn);
    Ob bx   = get_app(eqn);
    Ob K    = *Atoms::K;
    Ob KI   = *Atoms::KI;

    if (is_nless(x,K)) {
        LOG_DEBUG1( "enforcing A_Bool_pos1a" );
        ensure_less(KI, bx);
    }

    if (is_nless(x,KI)) {
        LOG_DEBUG1( "enforcing A_Bool_pos1b" );
        ensure_less(K, bx);
    }
}
inline void enforce_N_Bool_pos2 (Ord ord)
{// x![=F | Bool x
    Ob KI = ord.rhs(); if (KI != *Atoms::KI) return;
    Ob x  = ord.lhs();
    Ob bx = find_app(*Atoms::Bool,x); if (!bx) return;
    Ob K  = *Atoms::K;

    LOG_DEBUG1( "enforcing N_Bool_pos2" );
    ensure_less(K, bx);
}
inline void enforce_N_Bool_pos3 (Ord ord)
{// x![=K | Bool x
    Ob K  = ord.rhs(); if (K != *Atoms::K) return;
    Ob x  = ord.lhs();
    Ob bx = find_app(*Atoms::Bool,x); if (!bx) return;
    Ob KI = *Atoms::KI;

    LOG_DEBUG1( "enforcing N_Bool_pos3" );
    ensure_less(KI, bx);
}
//axiom: f=f*bool, g=g*bool, f _[=g _, f K[=g K, f F[=g F, f T[=g T |- f[=g
inline void enforce_L_Bool_pos3 (Ord ord, Ob w, Ob x, Ob y, Ob z)
{// f _[=g _ | f K[=g K, f KI[=g KI, f T[=g T, f*bool, g*bool, and permutations
    Ob fw  = ord.lhs();
    Ob gw  = ord.rhs();
    Ob Bool = *Atoms::Bool;

    ARl_Iterator gw_iter; //for below

    for (ARl_Iterator fw_iter(fw,w); fw_iter.ok(); fw_iter.next()) {
        Ob f  = fw_iter.lhs(); if (f != find_comp(f,Bool)) continue;
        Ob fx = find_app(f,x); if (!fx) continue;
        Ob fy = find_app(f,y); if (!fy) continue;
        Ob fz = find_app(f,z); if (!fz) continue;
        if (is_const(f)) continue;

        for (gw_iter.begin(gw,w); gw_iter.ok(); gw_iter.next()) {
            Ob g  = gw_iter.lhs(); if (g != find_comp(g,Bool)) continue;
            Ob gx = find_app(g,x); if (!gx) continue;
            Ob gy = find_app(g,y); if (!gy) continue;
            Ob gz = find_app(g,z); if (!gz) continue;
            if (is_const(g)) continue;

            if (is_less(fx,gx) and is_less(fy,gy) and is_less(fz,gz)) {
                LOG_DEBUG1( "enforcing L_Bool_pos3" );
                ensure_less(f,g);
            }
        }
    }
}
inline void enforce_C_Bool_pos1 (Comp eqn, const Set& gs)
{// f=f*bool | g=g*bool, f _[=g _, f K[=g K, f KI[=g KI, f T[=g T
    Ob Bool = *Atoms::Bool; if (get_rhs(eqn) != Bool) return;
    Ob f   = get_comp(eqn); if (get_lhs(eqn) != f) return;
    if (is_const(f)) return;
    Ob w   = *Atoms::Top;  Ob fw  = find_app(f,w); if (!fw) return;
    Ob x   = *Atoms::Bot;  Ob fx  = find_app(f,x); if (!fx) return;
    Ob y   = *Atoms::K;    Ob fy  = find_app(f,y); if (!fy) return;
    Ob z   = *Atoms::KI;   Ob fz  = find_app(f,z); if (!fz) return;

    for (Set::iterator g_iter(gs); g_iter.ok(); g_iter.next()) {
        Ob g  = Ob(*g_iter); if (g != find_comp(g,Bool)) continue;
        if (is_const(g)) continue;
        Ob gw = find_app(g,w);
        Ob gx = find_app(g,x);
        Ob gy = find_app(g,y);
        Ob gz = find_app(g,z);

        if (is_less(fw,gw) and is_less(fx,gx)
        and is_less(fy,gy) and is_less(fz,gz)) {
            LOG_DEBUG1( "enforcing C_Bool_pos1a" );
            ensure_less(f,g);
        }

        if (is_less(gw,fw) and is_less(gx,fx)
        and is_less(gy,fy) and is_less(gz,fz)) {
            LOG_DEBUG1( "enforcing C_Bool_pos1b" );
            ensure_less(g,f);
        }
    }
}
inline void enforce_A_Bool_pos2 (App eqn, Ob w, Ob x, Ob y, Ob z, const Set& gs)
{// f _ | f _[=g _, f I[=g I, f T[=g T, f*bool, g*bool, and permutations
    if (w != get_rhs(eqn)) return;
    Ob f   = get_lhs(eqn);
    Ob fx  = find_app(f,x); if (!fx) return;
    Ob fy  = find_app(f,y); if (!fy) return;
    Ob fz  = find_app(f,z); if (!fz) return;
    Ob Bool = *Atoms::Bool; if (f != find_comp(f,Bool)) return;
    if (is_const(f)) return;
    Ob fw  = get_app(eqn);

    for (Set::iterator g_iter(gs); g_iter.ok(); g_iter.next()) {
        Ob g  = Ob(*g_iter); if (g != find_comp(g,Bool)) continue;
        if (is_const(g)) continue;
        Ob gw = find_app(g,w);
        Ob gx = find_app(g,x);
        Ob gy = find_app(g,y);
        Ob gz = find_app(g,z);

        if (is_less(fw,gw) and is_less(fx,gx)
        and is_less(fy,gy) and is_less(fz,gz)) {
            LOG_DEBUG1( "enforcing A_Bool_pos2a" );
            ensure_less(f,g);
        }

        if (is_less(gw,fw) and is_less(gx,fx)
        and is_less(gy,fy) and is_less(gz,fz)) {
            LOG_DEBUG1( "enforcing A_Bool_pos2b" );
            ensure_less(g,f);
        }
    }
}
//sorted into contexts
inline void enforce_A_Bool_pos (App eqn)
{
    enforce_A_Bool_pos1(eqn);

    Ob w = *Atoms::Top;
    Ob x = *Atoms::Bot;
    Ob y = *Atoms::K;
    Ob z = *Atoms::KI;

    const Set& gs = get_funs_of_semi();

    enforce_A_Bool_pos2(eqn, w, x, y, z, gs);
    enforce_A_Bool_pos2(eqn, z, w, x, y, gs);
    enforce_A_Bool_pos2(eqn, y, z, w, x, gs);
    enforce_A_Bool_pos2(eqn, x, y, z, w, gs);
}
inline void enforce_C_Bool_pos (Comp eqn)
{
    const Set& gs = get_funs_of_semi();

    enforce_C_Bool_pos1(eqn, gs);
}
inline void enforce_L_Bool_pos (Ord ord)
{
    enforce_L_Bool_pos1(ord);
    enforce_L_Bool_pos2(ord);

    Ob w = *Atoms::Top;
    Ob x = *Atoms::Bot;
    Ob y = *Atoms::K;
    Ob z = *Atoms::KI;

    enforce_L_Bool_pos3(ord, w, x, y, z);
    enforce_L_Bool_pos3(ord, z, w, x, y);
    enforce_L_Bool_pos3(ord, y, z, w, x);
    enforce_L_Bool_pos3(ord, x, y, z, w);
}
inline void enforce_N_Bool_pos (Ord ord)
{
    enforce_N_Bool_pos1(ord);
    enforce_N_Bool_pos2(ord);
    enforce_N_Bool_pos3(ord);
}

}

