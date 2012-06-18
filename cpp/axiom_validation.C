
#include "axiom_tools.h"
#include "syntax_semantics.h"

namespace LambdaTheories
{

//printing
namespace EX = Expressions;
inline EX::ExprHdl print (Ob ob) { return EX::parse_ob(ob); }

//axiom schemes, see below axiom_enforcement.C for naming schemes

//lattice theory
void validate_L_order ();
void validate_N_order ();
void validate_L_monotony ();
void validate_N_monotony ();
void validate_Bot_E ();
void validate_Bot_N ();
void validate_Top_E ();
void validate_Top_N ();
void validate_join ();
void validate_compose ();
void validate_Rand ();

//lambda theory
void validate_S ();
void validate_K ();
void validate_I ();
void validate_C ();

//extension theory
void validate_B ();
void validate_Y ();
void validate_W ();
void validate_U ();
void validate_V ();
void validate_P ();
void validate_Det ();

//typed theory
void validate_Div ();
void validate_Unit ();
void validate_Semi ();
void validate_Bool ();
void validate_Maybe ();
void validate_Sum ();
void validate_Sset ();

//======== validation organized by theory ========

void MagmaTheory::validate (Int level)
{
    logger.info() << "Validating Theory" |0;
    _validate(level);
    Logging::IndentBlock block;

    if (CS::saturation_pending()) {
        logger.info() << "Processing repairs" |0;
        Logging::IndentBlock block;
        CS::saturate();
    }
}
void LatticeTheory::_validate (Int level)
{
    logger.debug() << "Validating lattice-theory" |0;
    Logging::IndentBlock block;

    if (level >= 1) {
        validate_Bot_E();
        validate_Top_E();
    }
    if (level >= 2) {
        validate_Bot_N();
        validate_Top_N();
    }
    if (level >= 3) {
        validate_L_order();
        validate_L_monotony();
        validate_N_monotony();

        validate_join();
        validate_compose();
        if (R) validate_Rand();
    }
    if (level >= 4) {
        validate_N_order();
    }
}
void LambdaTheory::_validate (Int level)
{
    LatticeTheory::_validate(level);

    logger.debug() << "Validating lambda-theory" |0;
    Logging::IndentBlock block;

    if (level >= 1) validate_I();
    if (level >= 2) {
        validate_K();
        validate_W();
    }
    if (level >= 3) {
        validate_B();
        validate_C();
        validate_S();
    }
}
void ExtnTheory::_validate (Int level)
{
    LambdaTheory::_validate(level);

    logger.debug() << "Validating extension-theory" |0;
    Logging::IndentBlock block;

    if (level >= 1) validate_Y();
    if (level >= 3) {
        if (U) validate_U();
        if (V) validate_V();
        if (P) validate_P();
    }
}
void TypedTheory::_validate (Int level)
{
    ExtnTheory::_validate(level);
    if (not V) return;

    logger.debug() << "Validating typed theory" |0;
    Logging::IndentBlock block;

    if (level >= 1) {
        if (Div)  validate_Div();
        if (Unit) validate_Unit();
        if (Semi) validate_Semi();
        if (Bool) validate_Bool();
        if (Maybe) validate_Maybe();
        if (Sum) validate_Sum();
        if (Sset) validate_Sset();
    }
}

//================================ validation ================================

//general axiom schemes
void validate_idempotence (Ob A)
{// Axx=x
    logger.debug() << "Validating " << print(A) << "-idempotence" |0;
    Logging::IndentBlock block;

    for (Lra_Iterator Ax_iter(A); Ax_iter; Ax_iter.next()) {
        Ob x  = Ax_iter.rhs();
        Ob Ax = Ax_iter.app();
        AssertV(ensure_app(x, Ax,x),
                "repaired " << print(A) << "-idempotence instance: "
                << "(" << print(Ax) << ") (" << print(x)
                << ") == " << print(x));
    }
}
void validate_commutativity (Ob A)
{// Axy=Ayx
    logger.debug() << "Validating " << print(A) << "-commutativity" |0;
    Logging::IndentBlock block;

    for (Lra_Iterator Ax_iter(A); Ax_iter; Ax_iter.next()) {
        Ob x  = Ax_iter.rhs();
        Ob Ax = Ax_iter.app();

        for (Lra_Iterator Axy_iter(Ax); Axy_iter; Axy_iter.next()) {
            Ob Axy = Axy_iter.app();
            Ob y   = Axy_iter.rhs();
            Ob Ay  = find_app(A,y); if (!Ay) continue;
            AssertV(ensure_app(Axy, Ay,x),
                    "repaired " << print(A)
                    << "-commutativity instance: "
                    << print(A) << ", " << print(x) << ", " << print(y));
        }
    }
}
void validate_right_distrib (Ob A)
{// Axyz=A(xz)(yz)
    logger.debug() << "Validating " << print(A) << "-right_distrib" |0;
    Logging::IndentBlock block;

    for (Lra_Iterator Ax_iter(A); Ax_iter; Ax_iter.next()) {
        Ob x  = Ax_iter.rhs();
        Ob Ax = Ax_iter.app();

        Lra_Iterator xz_iter(x); if (!xz_iter) continue; //for below

        for (Lra_Iterator Axy_iter(Ax); Axy_iter; Axy_iter.next()) {
            Ob Axy = Axy_iter.app();
            Ob y   = Axy_iter.rhs();

            for (xz_iter.begin(); xz_iter; xz_iter.next()) {
                Ob xz      = xz_iter.app();
                Ob A_xz    = find_app(A,xz); if (!A_xz) continue;
                Ob z       = xz_iter.rhs();
                Ob yz      = find_app(y,z);  if (!yz) continue;
                AssertV(ensure_apps(Axy,z, A_xz,yz),
                        "repaired " << print(A)
                        << "-right_distrib instance");
            }
        }
    }
}
void validate_associativity (Ob A)
{// Ax(Ayz)=A(Axy)z
    logger.debug() << "Validating " << print(A) << "-associativity" |0;
    Logging::IndentBlock block;

    Lra_Iterator Axy_iter; //for below
    Lra_Iterator Ayz_iter; //for below

    for (Lra_Iterator Ax_iter(A); Ax_iter; Ax_iter.next()) {
        Ob Ax = Ax_iter.app();

        for (Axy_iter.begin(Ax); Axy_iter; Axy_iter.next()) {
            Ob y     = Axy_iter.rhs();
            Ob Ay    = find_app(A,y); if (!Ay) continue;
            Ob Axy   = Axy_iter.app();
            Ob A_Axy = find_app(A,Axy); if (!A_Axy) continue;

            for (Ayz_iter.begin(Ay); Ayz_iter; Ayz_iter.next()) {
                Ob z       = Ayz_iter.rhs();
                Ob Ayz     = Ayz_iter.app();
                AssertV(ensure_apps(Ax,Ayz, A_Axy,z),
                        "repaired " << print(A)
                        << "-associativity instance");
            }
        }
    }
}
void validate_supconvexity (Ob A)
{// z[=x, z[=y |- z[=Axy
    Lra_Iterator Axy_iter; //for below
    Set& zs = OR::temp_set(); //for below

    for (Lra_Iterator Ax_iter(A); Ax_iter; Ax_iter.next()) {
        Ob Ax = Ax_iter.app();
        Ob x  = Ax_iter.rhs();

        for (Axy_iter.begin(Ax); Axy_iter; Axy_iter.next()) {
            Ob Axy = Axy_iter.app();
            Ob y   = Axy_iter.rhs();
            zs.set_insn(OR::below(x), OR::below(y));

            for (Set::iterator z_iter(zs); z_iter; z_iter.next()) {
                Ob z = Ob(*z_iter);
                AssertV(ensure_less(z,Axy),
                        "repaired z[=x, z[=y |- z[=Axy instance: "
                        << "x = " << print(x)
                        << ", y = " << print(y)
                        << ", z = " << print(z));
            }
        }
    }
}
void validate_subconvexity (Ob A)
{// z=]x, z=]y |- z=]Axy
    Lra_Iterator Axy_iter; //for below
    Set& zs = OR::temp_set(); //for below

    for (Lra_Iterator Ax_iter(A); Ax_iter; Ax_iter.next()) {
        Ob Ax = Ax_iter.app();
        Ob x  = Ax_iter.rhs();

        for (Axy_iter.begin(Ax); Axy_iter; Axy_iter.next()) {
            Ob Axy = Axy_iter.app();
            Ob y   = Axy_iter.rhs();
            zs.set_insn(OR::above(x), OR::above(y));

            for (Set::iterator z_iter(zs); z_iter; z_iter.next()) {
                Ob z = Ob(*z_iter);
                AssertV(ensure_less(Axy,z),
                        "repaired z=]x, z=]y |- z=]Axy instance: "
                        << "x = " << print(x)
                        << ", y = " << print(y)
                        << ", z = " << print(z));
            }
        }
    }
}
void validate_subaffinity (Ob A)
{// z=]x, z=]y |- Azz=]Axy
    Lra_Iterator Axy_iter; //for below
    Set& zs = OR::temp_set(); //for below

    for (Lra_Iterator Ax_iter(A); Ax_iter; Ax_iter.next()) {
        Ob Ax = Ax_iter.app();
        Ob x  = Ax_iter.rhs();

        for (Axy_iter.begin(Ax); Axy_iter; Axy_iter.next()) {
            Ob Axy = Axy_iter.app();
            Ob y   = Axy_iter.rhs();
            zs.set_insn(OR::above(x), OR::above(y));

            for (Set::iterator z_iter(zs); z_iter; z_iter.next()) {
                Ob z   = Ob(*z_iter);
                Ob Az  = find_app(A,z);  if (!Az) continue;
                Ob Azz = find_app(Az,z); if (!Azz) continue;
                AssertV(ensure_less(Axy,Azz),
                        "repaired z=]x, z=]y |- Azz=]Axy instance: "
                        << "x = " << print(x)
                        << ", y = " << print(y)
                        << ", z = " << print(z));
            }
        }
    }
}
void validate_distributivity (Ob A, Ob B)
{
    logger.debug() << "Validating "
        << print(A) << ',' << print(B) << "-distributivity" |0;
    Logging::IndentBlock block;

    Lra_Iterator Axy_iter, Axz_iter; //for later

    //Ax(Byz) = B(Axy)(Axz)
    for (Lra_Iterator Ax_iter(A); Ax_iter; Ax_iter.next()) {
        Ob x  = Ax_iter.rhs();
        Ob Bx = find_app(B,x); if (!Bx) continue;
        Ob Ax = Ax_iter.app();

        for (Axy_iter.begin(Ax); Axy_iter; Axy_iter.next()) {
            Ob Axy = Axy_iter.app();
            Ob BA  = find_app(B,Axy); if (!BA) continue;
            Ob y   = Axy_iter.rhs();
            Ob By  = find_app(B,y); if (!By) continue;

            for (Axz_iter.begin(Ax); Axz_iter; Axz_iter.next()) {
                Ob z   = Axz_iter.rhs();
                Ob Byz = find_app(By,z); if (!Byz) continue;
                Ob Axz = find_app(Ax,z); if (!Axz) continue;
                AssertV(ensure_apps(Ax,Byz, BA,Axz),
                        "repaired instance of Ax(Byz)=B(Axy)(Axz), "
                        << "where A = " << print(A)
                        << ", B = " << print(B)
                        << ", x = " << print(x)
                        << ", y = " << print(y)
                        << ", z = " << print(z) );
            }
        }
    }
}
void validate_duals (Ob A, Ob B)
{//Ax_Bxy = x
    logger.debug() << "Validating "
        << print(A) << ',' << print(B) << "-duality" |0;
    Logging::IndentBlock block;

    Lra_Iterator Bxy_iter; //for later

    for (Lra_Iterator Ax_iter(A); Ax_iter; Ax_iter.next()) {
        Ob x  = Ax_iter.rhs();
        Ob Bx = find_app(B,x); if (!Bx) continue;
        Ob Ax = Ax_iter.app();

        for (Bxy_iter.begin(Bx); Bxy_iter; Bxy_iter.next()) {
            Ob y      = Bxy_iter.rhs();
            Ob Bxy    = Bxy_iter.app();
            AssertV(ensure_app(x, Ax,Bxy),
                    "repaired duals instance: Ax(Bxy)!=x, where"
                    << "A = " << print(A)
                    << ", B = " << print(B)
                    << ", x = " << print(x)
                    << ", y = " << print(y) );
        }
    }
}

//lattice theory
void validate_L_order ()
{
    logger.debug() << "Validating axiom scheme L-order" |0;
    Logging::IndentBlock block;

    for (Ord::iterator<POS> iter; iter; iter.next()) {

        Ob x = iter->lhs();
        Ob y = iter->rhs();

        //validate asymmetry
        if (is_less(y,x)) {
            AssertV(ensure_equiv(x,y),
                    "repaired asymmetry_L instance: "
                    << print(x) << " [=] " << print(y));
        }

        //validate transitivity
        for (OR::Iterator<LRpos> yz_iter(y); yz_iter; yz_iter.next()) {

            Ob z = yz_iter.rhs();
            AssertV(ensure_less(x,z),
                    "repaired transitivity_L instance: "
                    << print(x) << " [= " << print(y) << " [= " << print(z));
        }
    }
}
void validate_N_order ()
{
    logger.debug() << "Validating axiom scheme N-order (very slow)" |0;
    Logging::IndentBlock block;

    for (Ord::iterator<NEG> iter; iter; iter.next()) {

        Ob x = iter->lhs();
        Ob z = iter->rhs();

        //validate transitivity_N1:  x[=y, x![=z |- y![=z
        for (OR::Iterator<LRpos> xy_iter(x); xy_iter; xy_iter.next()) {

            Ob y = xy_iter.rhs();
            AssertV(ensure_nless(y,z),
                    "repaired transitivity_N1 instance: "
                    << print(x) << " [= " << print(y) << " ![= " << print(z));
        }

        //validate transitivity_N2:  y[=z, x![=z |- x![=y
        for (OR::Iterator<RLpos> yz_iter(z); yz_iter; yz_iter.next()) {

            Ob y = yz_iter.lhs();
            AssertV(ensure_nless(x,y),
                    "repaired transitivity_N2 instance: "
                    << print(x) << " ![= " << print(y) << " [= " << print(z));
        }
    }
}
void validate_L_monotony ()
{
    logger.debug() << "Validating axiom scheme L-monotony" |0;
    Logging::IndentBlock block;

    RRla_Iterator ff_iter; //for below
    LLra_Iterator xx_iter; //for below

    RRlc_Iterator f_f_iter; //for below
    LLrc_Iterator x_x_iter; //for below

    for (Ord::iterator<POS> iter; iter; iter.next()) {

        Ob x = iter->lhs();
        Ob y = iter->rhs();
        Ob f = x;
        Ob g = y;

        //validate mu-app
        for (ff_iter.begin(x,y); ff_iter; ff_iter.next()) {
            Ob f  = ff_iter.lhs();
            Ob fx = ff_iter.app1();
            Ob fy = ff_iter.app2();
            AssertV(ensure_less(fx,fy),
                    "repaired mu_L instance: "
                    << print(f) << "(" << print(x) << ") [= "
                    << print(f) << "(" << print(y) << ")");
        }

        //validate nu-app
        for (xx_iter.begin(f,g); xx_iter; xx_iter.next()) {
            Ob x  = xx_iter.rhs();
            Ob fx = xx_iter.app1();
            Ob gx = xx_iter.app2();
            AssertV(ensure_less(fx,gx),
                    "repaired nu_L instance: "
                    << print(f) << "(" << print(x) << ") [= "
                    << print(g) << "(" << print(x) << ")");
        }

        //validate mu-comp
        for (f_f_iter.begin(x,y); f_f_iter; f_f_iter.next()) {
            Ob f  = f_f_iter.lhs();
            Ob fx = f_f_iter.comp1();
            Ob fy = f_f_iter.comp2();
            AssertV(ensure_less(fx,fy),
                    "repaired mu_L instance: "
                    << "(" << print(f) << ")*(" << print(x) << ") [= "
                    << "(" << print(f) << ")*(" << print(y) << ")");
        }

        //validate nu-comp
        for (x_x_iter.begin(f,g); x_x_iter; x_x_iter.next()) {
            Ob x  = x_x_iter.rhs();
            Ob fx = x_x_iter.comp1();
            Ob gx = x_x_iter.comp2();
            AssertV(ensure_less(fx,gx),
                    "repaired nu_L instance: "
                    << "(" << print(f) << ")*(" << print(x) << ") [= "
                    << "(" << print(g) << ")*(" << print(x) << ")");
        }
    }
}
void validate_N_monotony ()
{
    logger.debug() << "Validating axiom scheme N-monotony" |0;
    Logging::IndentBlock block;

    RRla_Iterator aa_iter; //for below
    LLra_Iterator zz_iter; //for below
    RRlc_Iterator a_a_iter; //for below
    LLrc_Iterator z_z_iter; //for below

    Ob::sparse_iterator end = Ob::send();
    for (Ob::sparse_iterator x_iter=Ob::sbegin(); x_iter!=end; ++x_iter) {
    for (Ob::sparse_iterator y_iter=Ob::sbegin(); y_iter!=end; ++y_iter) {

        Ob x = *x_iter;
        Ob y = *y_iter;

        //validate mu-app
        if (is_nless(x,y)) continue;
        for (aa_iter.begin(x,y); aa_iter; aa_iter.next()) {
            Ob a  = aa_iter.lhs();
            Ob ax = aa_iter.app1();
            Ob ay = aa_iter.app2();
            if (not is_nless(ax,ay)) continue;
            AssertV(ensure_nless(x,y),
                    "repaired ax![=ay |- x![=y instance"
                    << ": x = " << print(x)
                    << ", y = " << print(y)
                    << ", a = " << print(a) );
            break;
        }

        //validate nu-app
        if (is_nless(x,y)) continue;
        for (zz_iter.begin(x,y); zz_iter; zz_iter.next()) {
            Ob z  = zz_iter.rhs();
            Ob xz = zz_iter.app1();
            Ob yz = zz_iter.app2();
            if (not is_nless(xz,yz)) continue;
            AssertV(ensure_nless(x,y),
                    "repaired xz![=yz |- x![=y instance"
                    << ": x = " << print(x)
                    << ", y = " << print(y)
                    << ", z = " << print(z) );
            break;
        }

        //validate mu-comp
        if (is_nless(x,y)) continue;
        for (a_a_iter.begin(x,y); a_a_iter; a_a_iter.next()) {
            Ob a  = a_a_iter.lhs();
            Ob ax = a_a_iter.comp1();
            Ob ay = a_a_iter.comp2();
            if (not is_nless(ax,ay)) continue;
            AssertV(ensure_nless(x,y),
                    "repaired a*x![=a*y |- x![=y instance"
                    << ": x = " << print(x)
                    << ", y = " << print(y)
                    << ", a = " << print(a) );
            break;
        }

        //validate nu-comp
        if (is_nless(x,y)) continue;
        for (z_z_iter.begin(x,y); z_z_iter; z_z_iter.next()) {
            Ob z  = z_z_iter.rhs();
            Ob xz = z_z_iter.comp1();
            Ob yz = z_z_iter.comp2();
            if (not is_nless(xz,yz)) continue;
            AssertV(ensure_nless(x,y),
                    "repaired x*z![=y*z |- x![=y instance"
                    << ": x = " << print(x)
                    << ", y = " << print(y)
                    << ", z = " << print(z) );
            break;
        }

    } }
}

void validate_Bot_E ()
{// Bot x = x
    logger.debug() << "Validating axiom scheme Bot-E" |0;
    Logging::IndentBlock block;

    Ob Bot = *Atoms::Bot;
    for (Lra_Iterator Bot_x_iter(Bot); Bot_x_iter; Bot_x_iter.next()) {
        Ob Bot_x = Bot_x_iter.app();
        AssertV(ensure_equiv(Bot_x, Bot),
                "repaired Bot-E instance: "
                << print(Bot_x) << " == " << print(Bot));
    }
}
void validate_Top_E ()
{// Top x = x
    logger.debug() << "Validating axiom scheme Top-E" |0;
    Logging::IndentBlock block;

    Ob Top = *Atoms::Top;
    for (Lra_Iterator Top_x_iter(Top); Top_x_iter; Top_x_iter.next()) {
        Ob Top_x = Top_x_iter.app();
        AssertV(ensure_equiv(Top_x, Top),
                "repaired Top-E instance: "
                << print(Top_x) << " == " << print(Top));
    }
}
void validate_Bot_N ()
{// fx![=Bot, fx=f*x |- f![=Bot
    logger.debug() << "Validating axiom scheme Bot-N" |0;
    Logging::IndentBlock block;

    Alr_Iterator fx_iter; //for below

    Ob Bot = *Atoms::Bot;
    for (OR::Iterator<RLneg> fx_Bot_iter(Bot); fx_Bot_iter; fx_Bot_iter.next()) {
        Ob fx = fx_Bot_iter.lhs();

        for (fx_iter.begin(fx); fx_iter; fx_iter.next()) {
            //Bot_N
            Ob f = fx_iter.lhs();
            AssertV(ensure_nless(f, Bot),
                    "repaired Bot-N instance: "
                    << print(f) << " ![= " << print(Bot));
        }
    }
}
void validate_Top_N ()
{// fx=!]Top, fx=f*x |- f=!]Top
    logger.debug() << "Validating axiom scheme Bot-N" |0;
    Logging::IndentBlock block;

    Alr_Iterator fx_iter; //for below

    Ob Top = *Atoms::Top;
    for (OR::Iterator<LRneg> fx_Top_iter(Top);
            fx_Top_iter; fx_Top_iter.next()) {
        Ob fx = fx_Top_iter.rhs();

        for (fx_iter.begin(fx); fx_iter; fx_iter.next()) {
            //Top_N
            Ob f = fx_iter.lhs();
            AssertV(ensure_nless(Top, f),
                    "repaired Top-N instance: "
                    << print(f) << " =!] " << print(Top));
        }
    }
}

void validate_join ()
{
    logger.debug() << "Validating join schemata" |0;
    Logging::IndentBlock block;

    //join-order: x[=y ==> x|y=y
    logger.debug() << "validating join-order axiom" |0;
    for (Ord::iterator<POS> iter; iter; iter.next()) {
        Ob x = iter->lhs();
        Ob y = iter->rhs();

        AssertV(ensure_join(y, x,y),
                "repaired x[=y ==> x|y=y instance: "
                << "x = " << print(x)
                << ", y = " << print(y));
    }

    //join: x[=x|y, y[=x|y,  x[=z & y[=z ==> x|y[=z
    logger.debug() << "validating join axiom" |0;
    for (Join::sparse_iterator xy_iter=Join::sbegin(), end=Join::send();
            xy_iter!=end; ++xy_iter) {
        Ob x  = get_lhs(*xy_iter);
        Ob y  = get_rhs(*xy_iter);
        Ob xy = get_join(*xy_iter);

        AssertV(ensure_less(x, xy),
                "repaired x[=x|y insance: "
                << "x = " << print(x)
                << ", y = " << print(y));

        AssertV(ensure_less(y, xy),
                "repaired y[=x|y insance: "
                << "x = " << print(x)
                << ", y = " << print(y));

        Set& zs = OR::temp_set();
        zs.set_insn(OR::above(x), OR::above(y));
        for (Set::iterator z_iter(zs); z_iter; z_iter.next()) {
            Ob z = Ob(*z_iter);

            AssertV(ensure_less(xy, z),
                    "repaired x,y[=z ==> x|y[=z instance: "
                    << "x = " << print(x)
                    << ", y = " << print(y)
                    << ", z = " << print(z));
        }
    }

    //join-J: Jxy=x|y
    logger.debug() << "validating join-J axiom" |0;
    Ob J = *Atoms::J;
    for (Lra_Iterator Jx_iter(J); Jx_iter; Jx_iter.next()) {
        Ob x  = Jx_iter.rhs();
        Ob Jx = Jx_iter.app();

        for (Lra_Iterator Jxy_iter(Jx); Jxy_iter; Jxy_iter.next()) {
            Ob y   = Jxy_iter.rhs();
            Ob Jxy = Jxy_iter.app();

            AssertV(ensure_join(Jxy, x,y),
                    "repaired Jxy = x|y instance: "
                    << "x = " << print(x)
                    << ", y = " << print(y));
        }

        for (Lrj_Iterator xy_iter(x); xy_iter; xy_iter.next()) {
            Ob y  = xy_iter.moving();
            Ob xy = xy_iter.join();

            AssertV(ensure_app(xy, Jx,y),
                    "repaired x|y = Jxy instance: "
                    << "x = " << print(x)
                    << ", y = " << print(y));
        }
    }

    //idempotence
    logger.debug() << "validating join idempotence" |0;
    Ob::sparse_iterator end = Ob::send();
    for (Ob::sparse_iterator x_iter=Ob::sbegin(); x_iter!=end; ++x_iter) {
        Ob x = *x_iter;

        AssertV(ensure_join(x, x,x),
                "repaired x=x|x instance: x = " << print(x));
    }

    //associativity and distributivity
    logger.debug() << "validating join-join, join-apply, and join-compose" |0;
    for (Join::sparse_iterator xy_iter=Join::sbegin(), end=Join::send();
            xy_iter!=end; ++xy_iter) {
        Ob x  = get_lhs(*xy_iter);
        Ob y  = get_rhs(*xy_iter);
        Ob xy = get_join(*xy_iter);

        //join-join: x|(y|z)=(x|y)|z
        for (LLrj_Iterator z_iter(x,xy); z_iter; z_iter.next()) {
            Ob z   = z_iter.moving();
            Ob xz  = z_iter.join1();
            Ob xyz = z_iter.join2();

            AssertV(ensure_join(xyz, xz,y),
                    "repaired (x|y)|z = (x|z)|y instance: "
                    << "x = " << print(x)
                    << ", y = " << print(y)
                    << ", z = " << print(z));
        }
        for (LLrj_Iterator z_iter(y,xy); z_iter; z_iter.next()) {
            Ob z   = z_iter.moving();
            Ob yz  = z_iter.join1();
            Ob xyz = z_iter.join2();

            AssertV(ensure_join(xyz, yz,x),
                    "repaired (x|y)|z = (y|z)|x instance: "
                    << "x = " << print(x)
                    << ", y = " << print(y)
                    << ", z = " << print(z));
        }

        //join-apply: (x|y)z=xz|yz
        for (LLra_Iterator z_iter(x,y); z_iter; z_iter.next()) {
            Ob z  = z_iter.rhs();
            Ob xz = z_iter.app1();
            Ob yz = z_iter.app2();

            AssertV(ensure_app_join(xy,z, xz,yz),
                    "repaired (x|y)z = xz|yz instance: "
                    << "x = " << print(x)
                    << ", y = " << print(y)
                    << ", z = " << print(z));
        }

        //join-compose: (x|y)*z=x*z|y*z
        for (LLrc_Iterator z_iter(x,y); z_iter; z_iter.next()) {
            Ob z  = z_iter.rhs();
            Ob xz = z_iter.comp1();
            Ob yz = z_iter.comp2();

            AssertV(ensure_join_comp(xz,yz, xy,z),
                    "repaired (x|y)*z = x*z|y*z instance: "
                    << "x = " << print(x)
                    << ", y = " << print(y)
                    << ", z = " << print(z));
        }
    }
}
void validate_compose ()
{
    logger.debug() << "Validating composition schemata" |0;
    Logging::IndentBlock block;

    //associtivity
    logger.debug() << "validating compose-compose" |0;
    for (Comp::sparse_iterator xy_iter=Comp::sbegin(), end=Comp::send();
            xy_iter!=end; ++xy_iter) {
        Ob x  = get_lhs(*xy_iter);
        Ob y  = get_rhs(*xy_iter);
        Ob xy = get_comp(*xy_iter);

        for (Lrc_Iterator yz_iter(y); yz_iter; yz_iter.next()) {
            Ob z  = yz_iter.rhs();
            Ob yz = yz_iter.comp();

            AssertV(ensure_comps(x,yz, xy,z),
                    "repaired comp-assoc instance: "
                    << "(" << print(x) << ")*("
                           << print(y) << ")*(" << print(z) << ")");
        }
    }

    //compose-apply
    logger.debug() << "validating compose-apply" |0;
    for (Comp::sparse_iterator xy_iter=Comp::sbegin(), end=Comp::send();
            xy_iter!=end; ++xy_iter) {
        Ob x  = get_lhs(*xy_iter);
        Ob y  = get_rhs(*xy_iter);
        Ob xy = get_comp(*xy_iter);

        for (Lra_Iterator yz_iter(y); yz_iter; yz_iter.next()) {
            Ob z  = yz_iter.rhs();
            Ob yz = yz_iter.app();

            AssertV(ensure_apps(x,yz, xy,z),
                    "repaired comp-app instance: "
                    << "(" << print(x) << ")*("
                           << print(y) << ")*(" << print(z) << ")");
        }
    }
}

void validate_Rand ()
{// Rxx=x, Rxy=Ryx, Rxyz=R(xz)(yz), x=]y ==> Rxy=]y
    logger.debug() << "Validating axiom schemata Rand" |0;
    Logging::IndentBlock block;

    Ob R = *Atoms::R;

    validate_idempotence(R);
    validate_commutativity(R);
    validate_right_distrib(R);
    validate_supconvexity(R);
    validate_subconvexity(R);
}

//lambda-theory
void validate_I ()
{// Ix = x
    logger.debug() << "Validating axiom scheme I" |0;
    Logging::IndentBlock block;

    Ob I = *Atoms::I;

    Ob::sparse_iterator end = Ob::send();
    for (Ob::sparse_iterator x_iter=Ob::sbegin(); x_iter!=end; ++x_iter) {
        Ob x = *x_iter;
        AssertV(ensure_app(x, I,x),
                "repaired I-instance:\n\t"
                << " x = " << print(x));
    }
}
void validate_K ()
{// Kxy = x
    logger.debug() << "Validating axiom schemata K" |0;
    Logging::IndentBlock block;

    Ob K = *Atoms::K;

    for (Lra_Iterator Kx_iter(K); Kx_iter; Kx_iter.next()) {
        Ob Kx = Kx_iter.app();
        Ob x  = Kx_iter.rhs();

        Ob::sparse_iterator end = Ob::send();
        for (Ob::sparse_iterator y_iter=Ob::sbegin(); y_iter!=end; ++y_iter) {
            Ob y = *y_iter;
            AssertV(ensure_app(x, Kx,y),
                    "repaired K x y = x instance:\n\t"
                    << " x = " << print(x)
                    << ", y = " << print(y));

            AssertV(ensure_comp(Kx, Kx,y),
                    "repaired (K x)*y = K x instance:\n\t"
                    << " x = " << print(x)
                    << ", y = " << print(y));

        }

        for (Rla_Iterator fx_iter(x); fx_iter; fx_iter.next()) {
            Ob f  = fx_iter.lhs();
            Ob fx = fx_iter.app();

            AssertV(ensure_app_comp(K,fx, f,Kx),
                    "repaired f*(K x) = K(f x) instance:\n\t"
                    << " x = " << print(x)
                    << ", f = " << print(f));
        }
    }
}
void validate_S ()
{// Sxyz = xz(yz)
    logger.debug() << "Validating axiom scheme S" |0;
    Logging::IndentBlock block;

    Ob S = *Atoms::S;

    Lra_Iterator xz_iter; //for below
    Lra_Iterator Sxy_iter; //for below

    for (Lra_Iterator Sx_iter(S); Sx_iter; Sx_iter.next()) {
        Ob Sx = Sx_iter.app();
        Ob x  = Sx_iter.rhs();

        xz_iter.begin(x); if (!xz_iter) continue; //for below

        for (Sxy_iter.begin(Sx); Sxy_iter; Sxy_iter.next()) {
            Ob Sxy = Sxy_iter.app();
            Ob y   = Sxy_iter.rhs();

            for (xz_iter.begin(); xz_iter; xz_iter.next()) {
                Ob z     = xz_iter.rhs();
                Ob yz    = find_app(y,z); if (!yz) continue;
                Ob xz    = xz_iter.app();
                AssertV(ensure_apps(Sxy,z, xz,yz),
                        "repaired S instance:\n\t"
                        << " x = " << print(x)
                        << ", y = " << print(y)
                        << ", z = " << print(z));
            }
        }
    }
}
void validate_C ()
{// Cxyz = xzy
    logger.debug() << "Validating axiom scheme C" |0;
    Logging::IndentBlock block;

    Ob C = *Atoms::C;

    Lra_Iterator xz_iter; //for below
    Lra_Iterator Cxy_iter; //for below

    for (Lra_Iterator Cx_iter(C); Cx_iter; Cx_iter.next()) {
        Ob Cx = Cx_iter.app();
        Ob x  = Cx_iter.rhs();

        xz_iter.begin(x); if (!xz_iter) continue; //for below

        for (Cxy_iter.begin(Cx); Cxy_iter; Cxy_iter.next()) {
            Ob Cxy = Cxy_iter.app();
            Ob y   = Cxy_iter.rhs();

            for (xz_iter.begin(); xz_iter; xz_iter.next()) {
                Ob z    = xz_iter.rhs();
                Ob yz   = find_app(y,z); if (!yz) continue;
                Ob xz   = xz_iter.app();
                AssertV(ensure_apps(Cxy,z, xz,y),
                        "repaired C instance:\n\t"
                        << " x = " << print(x)
                        << ", y = " << print(y)
                        << ", z = " << print(z));
            }
        }
    }
}

//extension theory
void validate_B ()
{
    logger.debug() << "Validating B-composition schemata" |0;
    Logging::IndentBlock block;

    Ob B = *Atoms::B;
    for (Lra_Iterator Bx_iter(B); Bx_iter; Bx_iter.next()) {
        Ob x  = Bx_iter.rhs();
        Ob Bx = Bx_iter.app();

        for (Lra_Iterator Bxy_iter(Bx); Bxy_iter; Bxy_iter.next()) {
            Ob y   = Bxy_iter.rhs();
            Ob Bxy = Bxy_iter.app();

            AssertV(ensure_comp(Bxy, x,y),
                    "repaired B-comp instance: " << print(Bxy)
                    << " = (" << print(x) << ")*(" << print(y) << ")");
        }
    }

    Ob CB = *Atoms::CB;
    for (Lra_Iterator CBy_iter(CB); CBy_iter; CBy_iter.next()) {
        Ob y   = CBy_iter.rhs();
        Ob CBy = CBy_iter.app();

        for (Lra_Iterator CByx_iter(CBy); CByx_iter; CByx_iter.next()) {
            Ob x    = CByx_iter.rhs();
            Ob CByx = CByx_iter.app();

            AssertV(ensure_comp(CByx, x,y),
                    "repaired CB-comp instance: " << print(CByx)
                    << " = (" << print(x) << ")*(" << print(y) << ")");
        }
    }

    for (Comp::sparse_iterator iter=Comp::sbegin(); iter!=Comp::send(); ++iter) {
        Comp eqn = *iter;
        Ob x  = get_lhs(eqn);
        Ob y  = get_rhs(eqn);
        Ob xy = get_comp(eqn);

        if (Ob Bx = find_app(B,x)) {
            AssertV(ensure_app(xy, Bx,y),
                    "repaired comp-B instance: " << print(xy)
                    << " = (" << print(Bx) << ") (" << print(y) << ")");
        }
        if (Ob CBy = find_app(CB,y)) {
            AssertV(ensure_app(xy, CBy,x),
                    "repaired comp-CB instance: " << print(xy)
                    << " = (" << print(CBy) << ") (" << print(x) << ")");
        }
    }
}
void validate_Y ()
{// Yf = f(Yf)
    logger.debug() << "Validating axiom scheme Y" |0;
    Logging::IndentBlock block;

    Ob Y = *Atoms::Y;
    Ob SI = *Atoms::SI;

    for (Lra_Iterator Yf_iter(Y); Yf_iter; Yf_iter.next()) {
        Ob f    = Yf_iter.rhs();
        Ob Yf   = Yf_iter.app();
        AssertV(ensure_app(Yf, f,Yf),
                "repaired Y1 instance: " << print(Yf) << " == ???");
    }

    for (Lra_Iterator SIy_iter(SI); SIy_iter; SIy_iter.next()) {
        Ob y   = SIy_iter.rhs();
        Ob SIy = SIy_iter.app(); if (SIy != y) continue;
        AssertV(ensure_equiv(y, Y),
                "repaired Y2 instance: " << print(y));
    }
}
void validate_W ()
{// Wxy = xyy
    logger.debug() << "Validating axiom scheme W" |0;
    Logging::IndentBlock block;

    Ob W = *Atoms::W;

    Lra_Iterator xy_iter; //for below

    for (Lra_Iterator Wx_iter(W); Wx_iter; Wx_iter.next()) {
        Ob Wx = Wx_iter.app();
        Ob x  = Wx_iter.rhs();

        for (xy_iter.begin(x); xy_iter; xy_iter.next()) {
            Ob y  = xy_iter.rhs();
            Ob xy = xy_iter.app();
            AssertV(ensure_apps(Wx,y, xy,y),
                    "repaired Wxy instance:\n\t"
                    << " x = " << print(x)
                    << ", y = " << print(y));
        }
    }
}
void validate_U ()
{// Ux=x  <==>  x*x[=x
    logger.debug() << "Validating axiom scheme U" |0;
    Logging::IndentBlock block;

    Ob U = *Atoms::U;

    //validate U(Ux)=Ux
    for (Lra_Iterator Ux_iter(U); not Ux_iter.done(); Ux_iter.next()) {
        Ob Ux = Ux_iter.app();
        AssertV(ensure_app(Ux, U,Ux),
                "repaired unenforced U(Ux)=Ux instance: "
                << print(Ux));
    }

    //validate Ux=x |- x*x[=x
    for (Lra_Iterator Ux_iter(U); not Ux_iter.done(); Ux_iter.next()) {
        Ob x  = Ux_iter.rhs();
        Ob Ux = Ux_iter.app(); if (x != Ux) continue;
        Ob xx = find_comp(x,x); if (!xx) continue;
        AssertV(ensure_less(xx, x),
                "repaired unenforced Ux=x |- x*x[=x instance: " << print(x));
    }

    //validate x*x[=x |- Ux=x
    Ob::sparse_iterator end = Ob::send();
    for (Ob::sparse_iterator x_iter=Ob::sbegin(); x_iter!=end; ++x_iter) {
        Ob x = *x_iter;
        Ob xx = find_comp(x,x); if (!xx) continue;
        if (not is_less(xx,x)) continue;
        AssertV(ensure_app(x, U,x),
                "repaired unenforced x*x[=x |- Ux=x instance: " << print(x));
    }

    //validate fx[=x |- (Uf)x=fx
    for (Lra_Iterator Uf_iter(U); not Uf_iter.done(); Uf_iter.next()) {
        Ob f  = Uf_iter.rhs();
        Ob Uf = Uf_iter.app();

        for (Lra_Iterator fx_iter(f); not fx_iter.done(); fx_iter.next()) {
            Ob x  = fx_iter.rhs();
            Ob fx = fx_iter.app(); if (not is_less(fx, x)) continue;
            AssertV(ensure_app(fx, Uf,x),
                    "repaired unenforced fx[=x |- (Uf)x=fx instance: "
                    << print(f) << "." << print(x));
        }
    }
}
void validate_V ()
{// Vx=x  <==>  I[=x=x*x
    logger.debug() << "Validating axiom scheme V" |0;
    Logging::IndentBlock block;

    Ob I = *Atoms::I;
    Ob V = *Atoms::V;

    //validate V(Vx)=Vx
    for (Lra_Iterator Vx_iter(V); Vx_iter; Vx_iter.next()) {
        Ob Vx  = Vx_iter.app();
        AssertV(ensure_app(Vx, V,Vx),
                "repaired V(V x)=V x instance:\n\t"
                << " V x = " << print(Vx));
    }

    //validate Vx=x |- x*x=x, I[=x
    for (Lra_Iterator Vx_iter(V); Vx_iter; Vx_iter.next()) {
        Ob x  = Vx_iter.rhs();
        Ob Vx = Vx_iter.app(); if (x != Vx) continue;

        //validate x*x=x
        AssertV(ensure_comp(x, x,x),
                "repaired V x=x |- x*x=x instance:\n\t"
                << " x = " << print(x));

        //validate I[=x
        AssertV(ensure_less(I, x),
                "repaired V x=x |- I[=x instance:\n\t"
                << " x = " << print(x));
    }

    //validate I[=x=x*x |- Vx=x
    for (OR::Iterator<LRpos> ix_iter(I); ix_iter; ix_iter.next()) {
        Ob x = ix_iter.rhs();
        Ob xx = find_comp(x,x); if (xx != x) continue;

        AssertV(ensure_app(x, V,x),
                "repaired I[=x=x*x |- V x=x instance:\n\t"
                << " x = " << print(x));
    }

    //validate fx[=x |- Vfx=x
    for (Lra_Iterator Vf_iter(V); Vf_iter; Vf_iter.next()) {
        Ob f  = Vf_iter.rhs();
        Ob Vf = Vf_iter.app();

        for (Lra_Iterator fx_iter(f); fx_iter; fx_iter.next()) {
            Ob x  = fx_iter.rhs();
            Ob fx = fx_iter.app(); if (not is_less(fx, x)) continue;
            AssertV(ensure_app(x, Vf,x),
                    "repaired fx[=x |- Vfx=x instance:\n\t"
                    << " f = " << print(f) << ", x = " << print(x));
        }
    }
}
void validate_P ()
{// Pxy=Pyx, Px(Pyz)=P(Pxy)z
    logger.debug() << "Validating axiom schemata P" |0;
    Logging::IndentBlock block;

    Ob P = *Atoms::P;

    validate_commutativity(P);
    validate_associativity(P);
}

//typed theory
void validate_Div ()
{
    logger.debug() << "Validating axiom scheme 'div'" |0;
    Logging::IndentBlock block;

    Ob Div = *Atoms::Div;
    Ob Top = *Atoms::Top;
    Ob Bot = *Atoms::Bot;

    //x![=Bot |- Divx=Top
    for (OR::Iterator<RLneg> iter(Bot); iter; iter.next()) {
        Ob x = iter.lhs();
        AssertV(ensure_app(Top, Div,x),
                "repaired x![=Bot |- div x=Top instance: "
                << print(x));
    }

    //xBot=Bot |- x[=Div
    for (ARl_Iterator xB_iter(Bot,Bot); xB_iter; xB_iter.next()) {
        Ob x = xB_iter.lhs();
        AssertV(ensure_less(x, Div),
                "repaired x Bot=Bot |- x[=div instance: "
                << print(x));
    }

    //f=f*div, g=g*div, f _[=g _, f T[=g T |- f[=g
    Ob x = Bot;
    Ob y = Top;
    const Set& fs = get_funs_of_div();

    for (Set::iterator f_iter(fs); f_iter; f_iter.next()) {
        Ob f  = Ob(*f_iter); if (f != find_comp(f,Div)) continue;
        Ob fx = find_app(f,x);
        Ob fy = find_app(f,y);

        for (Set::iterator g_iter(fs); g_iter; g_iter.next()) {
            Ob g = Ob(*g_iter); if (g != find_comp(g,Div)) continue;
            Ob gx = find_app(g,x); if (not is_less(fx,gx)) continue;
            Ob gy = find_app(g,y); if (not is_less(fy,gy)) continue;

            AssertV(ensure_less(f,g),
                    "repaired /\\x:div. f x[=g x instance: "
                    << print(f) << " [= " << print(g));
        }
    }
}
void validate_Unit ()
{
    logger.debug() << "Validating axiom scheme 'unit'" |0;
    Logging::IndentBlock block;

    Ob Unit = *Atoms::Unit;
    Ob Top  = *Atoms::Top;
    Ob I    = *Atoms::I;

    //x![=I |- Unitx=Top
    for (OR::Iterator<RLneg> iter(I); iter; iter.next()) {
        Ob x = iter.lhs();
        AssertV(ensure_app(Top, Unit,x),
                "repaired x![=I |- unit x=Top instance: "
                << print(x));
    }

    //xI[=I |- x[=Unit
    for (Rla_Iterator xI_iter(I); xI_iter; xI_iter.next()) {
        if (not is_less(xI_iter.app(), I)) continue;
        Ob x = xI_iter.lhs();
        AssertV(ensure_less(x, Unit),
                "repaired x I[=I |- x[=unit instance: "
                << print(x));
    }

    //f=f*unit, g=g*unit, f I[=g I, f T[=g T |- f[=g
    Ob x = I;
    Ob y = Top;
    const Set& fs = get_funs_of_unit();

    for (Set::iterator f_iter(fs); f_iter; f_iter.next()) {
        Ob f  = Ob(*f_iter); if (f != find_comp(f,Unit)) continue;
        Ob fx = find_app(f,x);
        Ob fy = find_app(f,y);

        for (Set::iterator g_iter(fs); g_iter; g_iter.next()) {
            Ob g = Ob(*g_iter); if (g != find_comp(g,Unit)) continue;
            Ob gx = find_app(g,x); if (not is_less(fx,gx)) continue;
            Ob gy = find_app(g,y); if (not is_less(fy,gy)) continue;

            AssertV(ensure_less(f,g),
                    "repaired /\\x:unit. f x[=g x instance: "
                    << print(f) << " [= " << print(g));
        }
    }
}
void validate_Semi ()
{
    logger.debug() << "Validating axiom scheme 'semi'" |0;
    Logging::IndentBlock block;

    Ob Semi = *Atoms::Semi;
    Ob Bot  = *Atoms::Bot;
    Ob Top  = *Atoms::Top;
    Ob I    = *Atoms::I;

    //xBot=Bot, xI[=I |- x[=Semi
    for (ARl_Iterator xB_iter(Bot,Bot); xB_iter; xB_iter.next()) {
        Ob x  = xB_iter.lhs();
        Ob xI = find_app(x,I); if (not (xI and is_less(xI,I))) continue;

        AssertV(ensure_less(x,Semi),
                "repaired xBot=Bot, xI[=I |- x[=Semi instance: "
                << print(x));
    }

    for (Lra_Iterator sx_iter(Semi); sx_iter; sx_iter.next()) {
        Ob x  = sx_iter.rhs();
        Ob sx = sx_iter.app();

        //x![=I |- Semix=Top
        if (is_nless(x,I)) {
            AssertV(ensure_equiv(sx,Top),
                    "repaired x![=I |- Semix=Top instance: "
                    << print(x));
        }

        //x![=Bot |- Semix=]I
        else if (converges(x)) {
            AssertV(ensure_less(I,sx),
                    "repaired x![=Bot |- Semix=]I instance: "
                    << print(x));
        }
    }

    //f=f*semi, g=g*semi, f _[=g _, f I[=g I, f T[=g T |- f[=g
    Ob x = Top;
    Ob y = Bot;
    Ob z = I;
    const Set& fs = get_funs_of_semi();

    for (Set::iterator f_iter(fs); f_iter; f_iter.next()) {
        Ob f  = Ob(*f_iter); if (f != find_comp(f,Semi)) continue;
        Ob fx = find_app(f,x);
        Ob fy = find_app(f,y);
        Ob fz = find_app(f,z);

        for (Set::iterator g_iter(fs); g_iter; g_iter.next()) {
            Ob g = Ob(*g_iter); if (g != find_comp(g,Semi)) continue;
            Ob gx = find_app(g,x); if (not is_less(fx,gx)) continue;
            Ob gy = find_app(g,y); if (not is_less(fy,gy)) continue;
            Ob gz = find_app(g,z); if (not is_less(fz,gz)) continue;

            AssertV(ensure_less(f,g),
                    "repaired /\\x:semi. f x[=g x instance: "
                    << print(f) << " [= " << print(g));
        }
    }
}
void validate_Bool ()
{
    logger.debug() << "Validating axiom scheme 'bool'" |0;
    Logging::IndentBlock block;

    Ob Bool = *Atoms::Bool;
    Ob Bot  = *Atoms::Bot;
    Ob Top  = *Atoms::Top;
    Ob K    = *Atoms::K;
    Ob KI   = *Atoms::KI;

    for (Lra_Iterator bx_iter(Bool); bx_iter; bx_iter.next()) {
        Ob x  = bx_iter.rhs();
        Ob bx = bx_iter.app();

        //x![=F |- K[=Bool x
        if (is_nless(x,KI)) {
            AssertV(ensure_less(K,bx),
                    "repaired x![=F |- K[=Bool x instance: "
                    << print(x));
        }
        //x![=K |- F[=Bool x
        if (is_nless(x,K)) {
            AssertV(ensure_less(KI,bx),
                    "repaired x![=K |- F[=Bool x instance: "
                    << print(x));
        }
    }

    Set& xs = OR::temp_set();

    //x![=Bot, x[=K |- Bool x=K
    xs.set_insn(OR::nbelow(Bot), OR::below(K));
    for (Set::iterator x_iter(xs); x_iter; x_iter.next()) { Ob x = Ob(*x_iter);

        AssertV(ensure_app(K, Bool,x),
                "repaired x![=Bot, x[=K |- Bool x=K instance: "
                << print(x));
    }

    //x![=Bot, x[=F |- Bool x=F
    xs.set_insn(OR::nbelow(Bot), OR::below(KI));
    for (Set::iterator x_iter(xs); x_iter; x_iter.next()) { Ob x = Ob(*x_iter);

        AssertV(ensure_app(KI, Bool,x),
                "repaired x![=Bot, x[=F |- Bool x=F instance: "
                << print(x));
    }

    //Note that the axiom
    //  K[=x, F[=x |- Bool x=Top
    //is redundant since
    //  K[=x, F[=x  ==>  J[=x  ==>  T = Bool J [= Bool x [= T

    //f=f*bool, g=g*bool, f _[=g _, f K[=g K, f KI[=g KI, f T[=g T |- f[=g
    Ob w = Top;
    Ob x = Bot;
    Ob y = K;
    Ob z = KI;
    const Set& fs = get_funs_of_bool();

    for (Set::iterator f_iter(fs); f_iter; f_iter.next()) {
        Ob f  = Ob(*f_iter); if (f != find_comp(f,Bool)) continue;
        Ob fw = find_app(f,w);
        Ob fx = find_app(f,x);
        Ob fy = find_app(f,y);
        Ob fz = find_app(f,z);

        for (Set::iterator g_iter(fs); g_iter; g_iter.next()) {
            Ob g = Ob(*g_iter); if (g != find_comp(g,Bool)) continue;
            Ob gw = find_app(g,w); if (not is_less(fw,gw)) continue;
            Ob gx = find_app(g,x); if (not is_less(fx,gx)) continue;
            Ob gy = find_app(g,y); if (not is_less(fy,gy)) continue;
            Ob gz = find_app(g,z); if (not is_less(fz,gz)) continue;

            AssertV(ensure_less(f,g),
                    "repaired /\\x:bool. f x[=g x instance: "
                    << print(f) << " [= " << print(g));
        }
    }
}
void validate_Maybe ()
{
    logger.debug() << "Validating axiom scheme 'maybe'" |0;
    Logging::IndentBlock block;

}
void validate_Sum ()
{
    logger.debug() << "Validating axiom scheme 'sum'" |0;
    Logging::IndentBlock block;

}
void validate_Sset ()
{
    logger.debug() << "Validating axiom scheme 'sset'" |0;
    Logging::IndentBlock block;

}

}

