
#include "syntax_semantics.h"
#include "obs.h"
#include "apply.h"
#include "compose.h"
#include "join.h"
#include "combinatory_structure.h"
#include "brain.h"

//log levels
#define LOG_DEBUG1(mess)
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << mess |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

namespace Expressions
{

namespace M = Measures;
namespace O = Obs;
namespace AE = Apply;
namespace CE = Compose;
namespace JE = JoinEqn;
namespace CS = CombinatoryStructure;
namespace TB = TheBrain;
namespace S = Statements;

//expression lookup
class FindExpr : public StructlFun
{
    Ob& m_ob;
    FindExpr (Ob& ob) : m_ob(ob) {}
    virtual ~FindExpr () {}
public:
    static void call (Ob& ob, Expr& expr) { (FindExpr(ob))(expr); }

    virtual void of_atom  (const string& name)
    {
        m_ob = CS::find_atom(name);
    }
    virtual void of_app (Expr& lhs, Expr& rhs)
    {
        Ob lhs_ob(0);  call(lhs_ob, lhs);  if (!lhs_ob) return;
        Ob rhs_ob(0);  call(rhs_ob, rhs);  if (!rhs_ob) return;
        m_ob = AE::find_app(lhs_ob, rhs_ob);
    }
    virtual void of_comp (Expr& lhs, Expr& rhs)
    {
        Ob lhs_ob(0);  call(lhs_ob, lhs);  if (!lhs_ob) return;
        Ob rhs_ob(0);  call(rhs_ob, rhs);  if (!rhs_ob) return;
        m_ob = CE::find_comp(lhs_ob, rhs_ob);
    }
    virtual void of_join (Expr& lhs, Expr& rhs)
    {
        Ob lhs_ob(0);  call(lhs_ob, lhs);  if (!lhs_ob) return;
        Ob rhs_ob(0);  call(rhs_ob, rhs);  if (!rhs_ob) return;
        m_ob = JE::find_join(lhs_ob, rhs_ob);
    }
};
Ob find_expr (ExprHdl expr)
{//finds ob corresponding to expr if it already exists; fails otherwise
    Ob result(0);
    FindExpr::call(result, *expr);
    return result;
}

//expression creation
class GetExpr : public StructlFun
{
    ObHdl& m_ob;
    bool m_perm;
    GetExpr (ObHdl& ob, bool make_perm) : m_ob(ob), m_perm(make_perm) {}
    virtual ~GetExpr () {}
public:
    static void call (ObHdl& ob, bool make_perm, Expr& expr)
    { GetExpr(ob, make_perm)(expr); }

    virtual void of_atom (const string& name)
    {
        Ob ob = CS::find_atom(name); if (!ob) return;
        m_ob.set(ob);
        if (m_perm) O::addToCore(ob);
    }
    virtual void of_app (Expr& lhs, Expr& rhs)
    {
        ObHdl lhs_ob;  call(lhs_ob, m_perm, lhs);  if (!lhs_ob) return;
        ObHdl rhs_ob;  call(rhs_ob, m_perm, rhs);  if (!rhs_ob) return;
        Ob ob = CS::get_app(*lhs_ob, *rhs_ob);
        m_ob.set(ob);
        if (m_perm) O::addToCore(ob);
    }
    virtual void of_comp (Expr& lhs, Expr& rhs)
    {
        ObHdl lhs_ob;  call(lhs_ob, m_perm, lhs);  if (!lhs_ob) return;
        ObHdl rhs_ob;  call(rhs_ob, m_perm, rhs);  if (!rhs_ob) return;
        Ob ob = CS::get_comp(*lhs_ob, *rhs_ob);
        m_ob.set(ob);
        if (m_perm) O::addToCore(ob);
    }
    virtual void of_join (Expr& lhs, Expr& rhs)
    {
        ObHdl lhs_ob;  call(lhs_ob, m_perm, lhs);  if (!lhs_ob) return;
        ObHdl rhs_ob;  call(rhs_ob, m_perm, rhs);  if (!rhs_ob) return;
        Ob ob = CS::get_join(*lhs_ob, *rhs_ob);
        m_ob.set(ob);
        if (m_perm) O::addToCore(ob);
    }
};
ObHdl get_expr (ExprHdl expr, bool make_perm)
{//builds expr in db and returns correspinding ob; fails if atoms are unknown
    ObHdl result;
    GetExpr::call(result, make_perm, *expr);
    return result;
}

//simplification
class Simplify : public StructlFun
{
    static Float s_create;
    static bool s_large;
    ExprHdl& m_decompiled; //this should be initialized to default result
    ObHdl&   m_compiled;
    Simplify (ExprHdl& decompiled, ObHdl& compiled)
        : m_decompiled(decompiled), m_compiled(compiled) {}
    virtual ~Simplify () {}
public:
    static void call (ExprHdl& decompiled, ObHdl& compiled, Expr& expr)
    { Simplify(decompiled, compiled)(expr); }

    //params
    static void set_create (Float create) { s_create = create; }
    static void set_size (bool large) { s_large = large; }

    virtual void of_atom  (const string& name)
    {
        Ob compiled = CS::find_atom(name); if (!compiled) return;
        m_compiled.set(compiled);
        ExprHdl decompiled = parse_ob(*m_compiled, s_large);
        if (!decompiled) return;
        m_decompiled = decompiled;
    }
    virtual void of_app (Expr& lhs, Expr& rhs)
    {
        ObHdl lhs_ob; ExprHdl lhs_ex(&lhs); call(lhs_ex, lhs_ob, lhs);
        ObHdl rhs_ob; ExprHdl rhs_ex(&rhs); call(rhs_ex, rhs_ob, rhs);
        if (lhs_ob and rhs_ob) {
            Float symbols = TB::brain().get_symbols(*lhs_ob)
                          + TB::brain().get_symbols(*rhs_ob);
            Ob compiled = (symbols > s_create)
                        ? AE::find_app(*lhs_ob, *rhs_ob)
                        : CS::get_app(*lhs_ob, *rhs_ob);
            if (compiled) {
                m_compiled.set(compiled);
                m_decompiled = parse_ob(compiled, s_large);
                if (m_decompiled) return;
            }
        }
        m_decompiled = lhs_ex * rhs_ex;
    }
    virtual void of_comp (Expr& lhs, Expr& rhs)
    {
        ObHdl lhs_ob; ExprHdl lhs_ex(&lhs); call(lhs_ex, lhs_ob, lhs);
        ObHdl rhs_ob; ExprHdl rhs_ex(&rhs); call(rhs_ex, rhs_ob, rhs);
        if (lhs_ob and rhs_ob) {
            Float symbols = TB::brain().get_symbols(*lhs_ob)
                          + TB::brain().get_symbols(*rhs_ob);
            Ob compiled = (symbols > s_create)
                        ? CE::find_comp(*lhs_ob, *rhs_ob)
                        : CS::get_comp(*lhs_ob, *rhs_ob);
            if (compiled) {
                m_compiled.set(compiled);
                m_decompiled = parse_ob(compiled, s_large);
                if (m_decompiled) return;
            }
        }
        m_decompiled = lhs_ex % rhs_ex;
    }
    virtual void of_join (Expr& lhs, Expr& rhs)
    {
        ObHdl lhs_ob; ExprHdl lhs_ex(&lhs); call(lhs_ex, lhs_ob, lhs);
        ObHdl rhs_ob; ExprHdl rhs_ex(&rhs); call(rhs_ex, rhs_ob, rhs);
        if (lhs_ob and rhs_ob) {
            Float symbols = TB::brain().get_symbols(*lhs_ob)
                          + TB::brain().get_symbols(*rhs_ob);
            Ob compiled = (symbols > s_create)
                        ? JE::find_join(*lhs_ob, *rhs_ob)
                        : CS::get_join(*lhs_ob, *rhs_ob);
            if (compiled) {
                m_compiled.set(compiled);
                m_decompiled = parse_ob(compiled, s_large);
                if (m_decompiled) return;
            }
        }
        m_decompiled = lhs_ex | rhs_ex;
    }
};
Float Simplify::s_create = false;
bool Simplify::s_large = false;
ExprHdl simplify (ExprHdl expr, Float create, bool large)
{//alternately linear-reduces and looks up meaning in brain
    //for below
    Simplify::set_create(create);
    Simplify::set_size(large);

    ExprHdl result = expr;
    const Int MAX_SIMPLIFIES = 64;
    for (Int i=0; i<MAX_SIMPLIFIES; ++i) {
        ExprHdl simple = result;
        ObHdl temp;
        Simplify::call(simple, temp, *simple);
        simple = simple->reduce();
        if (simple == result) return result;
        result = simple;
    }
    logger.warning() << "simplification did not terminate" |0;
    return result;
}
ExprHdl simplifier::operator() (const ExprHdl& expr) const
{
    return simplify(expr, m_create);
}

//projection to ob pmfs
inline bool isfinite (Ob ob)
{
    return std::isfinite(TB::brain().get_symbols(ob));
}
class AddExprToPoly : public StructlFun
{
    ObPoly& m_poly;
    Ob& m_ob; //this must be initialized to zero
    Float m_symbols;
    AddExprToPoly (ObPoly& poly, Ob& ob, Float symbols)
        : m_poly(poly), m_ob(ob), m_symbols(symbols) {}
    virtual ~AddExprToPoly () {}

    void add (Ob ob, Expr &expr)
    {
        if (isfinite(ob)) {
            m_poly.add_ob(ob);
        } else {
            logger.warning() << "refusing to add very large expr: " << expr |0;
        }
    }
    void add (Ob ob, string str)
    {
        if (isfinite(ob)) {
            m_poly.add_ob(ob);
        } else {
            logger.warning() << "refusing to add very large atom: " << str |0;
        }
    }
public:
    static void call (ObPoly& poly, Ob& ob, Float symbols, Expr& expr)
    { AddExprToPoly(poly, ob, symbols)(expr); }

    virtual void of_atom (const string& name)
    {
        m_ob = CS::find_atom(name);
        if (m_ob and not isfinite(m_ob)) m_ob = Ob(0);
    }
    virtual void of_app (Expr& lhs, Expr& rhs)
    {
        Ob lhs_ob(0);  call(m_poly, lhs_ob, m_symbols, lhs);
        Ob rhs_ob(0);  call(m_poly, rhs_ob, m_symbols, rhs);
        if (lhs_ob) {
            if (rhs_ob) {
                if ((m_ob = AE::find_app(lhs_ob, rhs_ob))) {
                    //Note: obs with large finite mass
                    //  cause Floating point overflow in basis fitting
                    Float size = TB::brain().get_symbols(m_ob);
                    bool add_ob = std::isfinite(size) and size < m_symbols;
                    if (add_ob) return;       //add app to poly
                    else        m_ob = Ob(0); //only add lhs,rhs
                }
                add(rhs_ob, rhs);
            }
            add(lhs_ob, lhs);
        } else {
            if (rhs_ob) {
                add(rhs_ob, rhs);
            }
        }
        m_poly.add_app();
    }
    virtual void of_comp (Expr& lhs, Expr& rhs)
    {
        Ob lhs_ob(0);  call(m_poly, lhs_ob, m_symbols, lhs);
        Ob rhs_ob(0);  call(m_poly, rhs_ob, m_symbols, rhs);
        if (lhs_ob) {
            if (rhs_ob) {
                if ((m_ob = CE::find_comp(lhs_ob, rhs_ob))) {
                    //Note: obs with large finite mass
                    //  cause Floating point overflow in basis fitting
                    Float size = TB::brain().get_symbols(m_ob);
                    bool add_ob = std::isfinite(size) and size < m_symbols;
                    if (add_ob) return;       //add comp to poly
                    else        m_ob = Ob(0); //only add lhs,rhs
                }
                add(rhs_ob, rhs);
            }
            add(lhs_ob, lhs);
        } else {
            if (rhs_ob) {
                add(rhs_ob, rhs);
            }
        }
        m_poly.add_comp();
    }
    virtual void of_join (Expr& lhs, Expr& rhs)
    {
        Ob lhs_ob(0);  call(m_poly, lhs_ob, m_symbols, lhs);
        Ob rhs_ob(0);  call(m_poly, rhs_ob, m_symbols, rhs);
        if (lhs_ob) {
            if (rhs_ob) {
                if ((m_ob = JE::find_join(lhs_ob, rhs_ob))) {
                    //Note: obs with large finite mass
                    //  cause Floating point overflow in basis fitting
                    Float size = TB::brain().get_symbols(m_ob);
                    bool add_ob = std::isfinite(size) and size < m_symbols;
                    if (add_ob) return;       //add join to poly
                    else        m_ob = Ob(0); //only add lhs,rhs
                }
                add(rhs_ob, rhs);
            }
            add(lhs_ob, lhs);
        } else {
            if (rhs_ob) {
                add(rhs_ob, rhs);
            }
        }
        m_poly.add_join();
    }
};
void add_expr_to_poly (ObPoly& poly, ExprHdl expr, Float symbols)
{//policy: (weak addition) adds each subexpr iff both:
// (1) the ob already exists in the database with finite mass; and
// (2) the ob is smaller than the given max symbol length
    Ob ob(0);
    AddExprToPoly::call(poly, ob, symbols, *expr);
    if (ob and isfinite(ob)) poly.add_ob(ob);
}

//ob parsing
Nodes::App find_parse_app (Ob ob)
{//finds min-complexity parse app
    Nodes::App eqn(0);
    Float min_mass = INFINITY;
    for (AE::Alr_Iterator iter(ob); not iter.done(); iter.next()) {
        Ob lhs = iter.lhs(); if (lhs == ob) continue;
        Ob rhs = iter.rhs(); if (rhs == ob) continue;
        Float mass = M::komp(lhs) + M::komp(rhs);
        if (mass < min_mass) {
            min_mass = mass;
            eqn = *iter;
        }
    }
    return eqn;
}
Nodes::Comp find_parse_comp (Ob ob)
{//finds min-complexity parse comp
    Nodes::Comp eqn(0);
    Float min_mass = INFINITY;
    for (CE::Clr_Iterator iter(ob); not iter.done(); iter.next()) {
        Ob lhs = iter.lhs(); if (lhs == ob) continue;
        Ob rhs = iter.rhs(); if (rhs == ob) continue;
        Float mass = M::komp(lhs) + M::komp(rhs);
        if (mass < min_mass) {
            min_mass = mass;
            eqn = *iter;
        }
    }
    return eqn;
}
Nodes::Join find_parse_join (Ob ob)
{//finds min-complexity parse join
    Nodes::Join eqn(0);
    Float min_mass = INFINITY;
    for (JE::Jlr_Iterator iter(ob); not iter.done(); iter.next()) {
        Ob lhs = iter.lhs(); if (lhs == ob) continue;
        Ob rhs = iter.rhs(); if (rhs == ob) continue;
        Float mass = M::komp(lhs) + M::komp(rhs);
        if (mass < min_mass) {
            min_mass = mass;
            eqn = *iter;
        }
    }
    return eqn;
}

template<class T>
inline Float get_mass (T t) { return TB::brain().get_mass(t); }

#define FAILED {logger.warning()<<"could not parse ob"|0; return ExprHdl();}
ExprHdl parse_small (Ob ob, Int depth=128);
ExprHdl parse_small (Nodes::App eqn, Int depth)
{
    ExprHdl lhs = parse_small(get_lhs(eqn), depth-1); if (!lhs) FAILED;
    ExprHdl rhs = parse_small(get_rhs(eqn), depth-1); if (!rhs) FAILED;
    return lhs * rhs;
}
ExprHdl parse_small (Nodes::Comp eqn, Int depth)
{
    ExprHdl lhs = parse_small(get_lhs(eqn), depth-1); if (!lhs) FAILED;
    ExprHdl rhs = parse_small(get_rhs(eqn), depth-1); if (!rhs) FAILED;
    return lhs % rhs;
}
ExprHdl parse_small (Nodes::Join eqn, Int depth)
{
    ExprHdl lhs = parse_small(get_lhs(eqn), depth-1); if (!lhs) FAILED;
    ExprHdl rhs = parse_small(get_rhs(eqn), depth-1); if (!rhs) FAILED;
    return lhs | rhs;
}
ExprHdl parse_small (Ob ob, Int depth)
{
    if (depth == 0) FAILED;

    //parse as atom
    const string* name = CS::find_atom_name(ob);
    if (name) return build_atom(*name);

    //otherwise find best parse as app, comp, or join
    Nodes::App  a_eqn = find_parse_app(ob);
    Nodes::Comp c_eqn = find_parse_comp(ob);
    Nodes::Join j_eqn = find_parse_join(ob);

    //find complexity of parses
    Float app_mass  = a_eqn ? get_mass(a_eqn) : INFINITY;
    Float comp_mass = c_eqn ? get_mass(c_eqn) : INFINITY;
    Float join_mass = j_eqn ? get_mass(j_eqn) : INFINITY;
    LOG_DEBUG1( ", app mass = " << app_mass
             << ", comp mass = " << comp_mass
             << ", join mass = " << join_mass );

    //compare parses, XXX ignore possible infinities...
    Float min_mass = app_mass;
    min_mass = min(min_mass, comp_mass);
    min_mass = min(min_mass, join_mass);

    //parse subterms
    if (min_mass == app_mass)   return parse_small(a_eqn, depth);
    if (min_mass == comp_mass)  return parse_small(c_eqn, depth);
    if (min_mass == join_mass)  return parse_small(j_eqn, depth);

    Error ("failed to decide on parse type");
}

ExprHdl parse_large (Ob ob, Int depth=128);
ExprHdl parse_large (Nodes::App eqn, Int depth)
{
    ExprHdl lhs = parse_large(get_lhs(eqn), depth-1); if (!lhs) FAILED;
    ExprHdl rhs = parse_large(get_rhs(eqn), depth-1); if (!rhs) FAILED;
    return lhs * rhs;
}
ExprHdl parse_large (Nodes::Comp eqn, Int depth)
{
    ExprHdl lhs = parse_large(get_lhs(eqn), depth-1); if (!lhs) FAILED;
    ExprHdl rhs = parse_large(get_rhs(eqn), depth-1); if (!rhs) FAILED;
    return lhs % rhs;
}
ExprHdl parse_large (Nodes::Join eqn, Int depth)
{
    ExprHdl lhs = parse_large(get_lhs(eqn), depth-1); if (!lhs) FAILED;
    ExprHdl rhs = parse_large(get_rhs(eqn), depth-1); if (!rhs) FAILED;
    return lhs | rhs;
}
ExprHdl parse_large (Ob ob, Int depth)
{
    if (depth == 0) FAILED;

    //find parses as atom, app, comp, join
    const string* name = CS::find_atom_name(ob);
    Nodes::App  a_eqn = find_parse_app(ob);
    Nodes::Comp c_eqn = find_parse_comp(ob);
    Nodes::Join j_eqn = find_parse_join(ob);

    //find complexity of parses
    Float atom_mass = name  ? get_mass(ob)    : INFINITY;
    Float app_mass  = a_eqn ? get_mass(a_eqn) : INFINITY;
    Float comp_mass = c_eqn ? get_mass(c_eqn) : INFINITY;
    Float join_mass = j_eqn ? get_mass(j_eqn) : INFINITY;
    LOG_DEBUG1( "atom mass = " << atom_mass
             << ", app mass = " << app_mass
             << ", comp mass = " << comp_mass
             << ", join mass = " << join_mass );

    //compare parses, XXX ignore possible infinities...
    Float min_mass = atom_mass;
    min_mass = min(min_mass, app_mass);
    min_mass = min(min_mass, comp_mass);
    min_mass = min(min_mass, join_mass);

    //parse subterms
    if (min_mass == atom_mass)  return parse_small(ob, depth);
    if (min_mass == app_mass)   return parse_small(a_eqn, depth);
    if (min_mass == comp_mass)  return parse_small(c_eqn, depth);
    if (min_mass == join_mass)  return parse_small(j_eqn, depth);

    Error ("failed to decide on parse type");
}
#undef FAILED
ExprHdl parse_ob (Ob ob, bool large)
{ return large ? parse_large(ob) : parse_small(ob); }

}


