#ifndef JOHANN_SYMBOLS_H
#define JOHANN_SYMBOLS_H

namespace Symbols
{

//relations
enum Relation
{
    NO_RELN,
    EQUAL, NEQUAL,
    OF_TYPE, NOT_OF_TYPE,
    TESTED, FAILED,
    ELEMENT, NOT_ELEMENT,
    LESS, GREATER,
    LESS_EQUAL, NOT_LEQ, GREATER_EQUAL, NOT_GEQ,
    SUBTYPE, NOT_SUBTYPE, SUPTYPE, NOT_SUPTYPE,
    SUBTEST, NOT_SUBTEST, SUPTEST, NOT_SUPTEST
};
const char*const RelationNames[] =
{
    "",
    "=", "!=",
    ":", "!:",
    "::", "!::",
    " in ", " not in ",
    "[[", "]]",
    "[=", "![=", "=]", "!=]",
    "<:", "!<:", ":>", "!:>"
    "<::", "!<::", "::>", "!::>"
};
const bool IsPositive[] =
{
    false,
    true, false,
    true, false,
    true, false,
    true, false,
    false, false,
    true, false, true, false,
    true, false, true, false,
    true, false, true, false
};

//variable binders
enum BinderType { NO_BINDER, LAMBDA, FORALL, EXISTS };
const char*const BinderNames[] = { "", "\\", "/\\", "\\/" };

//modal operators
enum ModalOp { CIRCLE, BOX, DIAMOND };

//three-valued logic
enum Trool { FALSE, UNKNOWN, TRUE };
inline Trool Or  (Trool lhs, Trool rhs) { return lhs>rhs ? lhs : rhs; }
inline Trool And (Trool lhs, Trool rhs) { return lhs<rhs ? lhs : rhs; }
inline Trool Not (Trool arg)            { return Trool(int(TRUE) - int(arg)); }
inline Trool YesOrMaybe (bool b) { return b ? TRUE : UNKNOWN; }
inline Trool YesOrNo    (bool b) { return b ? TRUE : FALSE; }
const char*const TroolNames[] = { "false", "unknown", "true" };

//continuous-valued-logic
typedef Float Prob;
const Prob P_TRUE = 1.0;
const Prob P_FALSE = 0.0;
inline Prob Not (Prob p) { return 1.0 - p; }
inline Prob And (Prob p, Prob q) { return p * q; }
inline Prob Or (Prob p, Prob q) { return Not(And(Not(p), Not(q))); }
inline void bound_prob (Prob& p)
{
    if (p < 0.0) p = 0.0;
    else
    if (p > 1.0) p = 1.0;
}

//intuitionistic probability
struct IProb
{
    Prob pos, neg;
    IProb () {}
    IProb (Float p, Float n) : pos(p), neg(n) {}

    //operations
    void bound () { bound_prob(pos); bound_prob(neg); }
    IProb& operator+= (const IProb& other)
    { pos += other.pos; neg += other.neg; return *this; }
    IProb operator+ (const IProb& other) const
    { return IProb(pos+other.pos, neg+other.neg); }
};
const IProb IP_TRUE (1.0,0.0);
const IProb IP_UNKN (0.0,0.0);
const IProb IP_FALSE(0.0,1.0);

//boolean properties
enum Property
{
    P_IS_USED,
    P_IN_CORE,
    P_INJECTIVE,
    P_LINEAR,
    P_NONCONST,
    P_DETERMIN,
    P_SEQUENTIAL,
    P_MEANINGFUL,
    P_CONSISTENT,
    P_VALID, //i.e., meaningful and consistent
    P_MARKED = 16
};
enum BoolPropertyMask
{
    IS_USED    = 1<<P_IS_USED,      //always true for allocated nodes
    IN_CORE    = 1<<P_IN_CORE,
    INJECTIVE  = 1<<P_INJECTIVE,
    LINEAR     = 1<<P_LINEAR,       //i.e., contains no S
    NONCONST   = 1<<P_NONCONST,     //i.e., contains no K; a lambda-I term
    DETERMIN   = 1<<P_DETERMIN,     //i.e., contains no R
    SEQUENTIAL = 1<<P_SEQUENTIAL,   //i.e., contains no J
    STRUCTURAL = LINEAR | NONCONST | DETERMIN | SEQUENTIAL,
    //a property P is structural iff
    //  P(a)  <==>  \/l,r. P(l) and P(r) and  a = l r
    MARKED     = 1<<P_MARKED
};
inline BoolPropertyMask get_mask (Property prop)
{
    return BoolPropertyMask(1 << prop);
}
const char*const PropertyNames[] =
{
    "used",
    "in core",
    "injective",
    "linear",
    "nonconstant",
    "deterministic",
    "sequential",
    "meaningful",
    "consistent",
    "valid"
};

//structure types
enum StructType
{
    OBS_STRUCT  = 1,
    APP_STRUCT  = 2,
    COMP_STRUCT = 4,
    JOIN_STRUCT = 8,
    ORD_STRUCT  = 16,
    THY_STRUCT  = 32,
    ALL_STRUCT  = OBS_STRUCT | APP_STRUCT | COMP_STRUCT | JOIN_STRUCT
                             | ORD_STRUCT | THY_STRUCT
};

//propagation rules
enum PropRule
{
    RULE_MU,
    RULE_NU,
    RULE_TAU
};
const char*const PropRuleNames[] = {"mu","nu","tau"};
inline bool rule_is_valid (PropRule rule)
{
    switch (rule) {
        case RULE_MU:
        case RULE_NU:
        case RULE_TAU:
            return true;
        default:
            return false;
    }
}

}

using Symbols::Trool;
using Symbols::Prob;
/*
using Symbols::TRUE;
using Symbols::UNKNOWN;
using Symbols::FALSE;
*/

#endif
