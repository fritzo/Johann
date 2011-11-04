
#include "axiom_tools.h"

//log levels
#define LOG_DEBUG1(mess);
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << mess |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

namespace LambdaTheories
{

//vector order
void ensure_less (const Set& xs, Ob y)
{
    y = O::getRep(y);
    return OR::ensure_less(xs, y);
}
void ensure_less (Ob x, const Set& ys)
{
    x = O::getRep(x);
    return OR::ensure_less(x, ys);
}
void ensure_nless (const Set& xs, Ob y)
{
    y = O::getRep(y);
    return OR::ensure_nless(xs, y);
}
void ensure_nless (Ob x, const Set& ys)
{
    x = O::getRep(x);
    return OR::ensure_nless(x, ys);
}

//multiple compound structure
bool ensure_apps (Ob L1, Ob R1, Ob L2, Ob R2)
{//ensures apps either agree or don't exist; returns true if so
    LOG_DEBUG1( "ensuring: "
        << expr(L1) << "(" << expr(R1) << ")" << " = "
        << expr(L2) << "(" << expr(R2) << ")" )
    L1 = O::getRep(L1);
    R1 = O::getRep(R1);
    L2 = O::getRep(L2);
    R2 = O::getRep(R2);
    Ob A1 = AE::find_app(L1, R1);
    Ob A2 = AE::find_app(L2, R2);
    if (A1) {
        if (A2) return ensure_equiv(A1, A2);
        CS::make_app(O::getRep(A1), L2, R2);
        return false;
    }
    if (A2) {
        CS::make_app(O::getRep(A2), L1, R1);
        return false;
    }
    return true;
}
bool ensure_comps (Ob L1, Ob R1, Ob L2, Ob R2)
{//ensures comps either agree or don't exist; returns true if so
    LOG_DEBUG1( "ensuring: ("
        << expr(L1) << ")*(" << expr(R1) << ")" << " = ("
        << expr(L2) << ")*(" << expr(R2) << ")" )
    L1 = O::getRep(L1);
    R1 = O::getRep(R1);
    L2 = O::getRep(L2);
    R2 = O::getRep(R2);
    Ob C1 = CE::find_comp(L1, R1);
    Ob C2 = CE::find_comp(L2, R2);
    if (C1) {
        if (C2) return ensure_equiv(C1, C2);
        CS::make_comp(O::getRep(C1), L2, R2);
        return false;
    }
    if (C2) {
        CS::make_comp(O::getRep(C2), L1, R1);
        return false;
    }
    return true;
}
bool ensure_joins (Ob L1, Ob R1, Ob L2, Ob R2)
{//ensures joins either agree or don't exist; returns true if so
    LOG_DEBUG1( "ensuring: ("
        << expr(L1) << ")|(" << expr(R1) << ")" << " = ("
        << expr(L2) << ")|(" << expr(R2) << ")" )
    L1 = O::getRep(L1);
    R1 = O::getRep(R1);
    L2 = O::getRep(L2);
    R2 = O::getRep(R2);
    Ob J1 = JE::find_join(L1, R1);
    Ob J2 = JE::find_join(L2, R2);
    if (J1) {
        if (J2) return ensure_equiv(J1, J2);
        CS::make_join(O::getRep(J1), L2, R2);
        return false;
    }
    if (J2) {
        CS::make_join(O::getRep(J2), L1, R1);
        return false;
    }
    return true;
}
bool ensure_app_comp (Ob L1, Ob R1, Ob L2, Ob R2)
{//ensures app and comp either agree or don't exist; returns true if so
    LOG_DEBUG1( "ensuring: "
        << expr(L1) << "(" << expr(R1) << ")" << " = ("
        << expr(L2) << ")*(" << expr(R2) << ")" )
    L1 = O::getRep(L1);
    R1 = O::getRep(R1);
    L2 = O::getRep(L2);
    R2 = O::getRep(R2);
    Ob A = AE::find_app(L1, R1);
    Ob C = CE::find_comp(L2, R2);
    if (A) {
        if (C) return ensure_equiv(A, C);
        CS::make_comp(O::getRep(A), L2, R2);
        return false;
    }
    if (C) {
        CS::make_app(O::getRep(C), L1, R1);
        return false;
    }
    return true;
}
bool ensure_app_join (Ob L1, Ob R1, Ob L2, Ob R2)
{//ensures app and join either agree or don't exist; returns true if so
    LOG_DEBUG1( "ensuring: "
        << expr(L1) << "(" << expr(R1) << ")" << " = ("
        << expr(L2) << ")|(" << expr(R2) << ")" )
    L1 = O::getRep(L1);
    R1 = O::getRep(R1);
    L2 = O::getRep(L2);
    R2 = O::getRep(R2);
    Ob A = AE::find_app(L1, R1);
    Ob J = JE::find_join(L2, R2);
    if (A) {
        if (J) return ensure_equiv(A, J);
        CS::make_join(O::getRep(A), L2, R2);
        return false;
    }
    if (J) {
        CS::make_app(O::getRep(J), L1, R1);
        return false;
    }
    return true;
}
bool ensure_join_comp (Ob L1, Ob R1, Ob L2, Ob R2)
{//ensures join and comp either agree or don't exist; returns true if so
    LOG_DEBUG1( "ensuring: ("
        << expr(L1) << ")|(" << expr(R1) << ")" << " = ("
        << expr(L2) << ")*(" << expr(R2) << ")" )
    L1 = O::getRep(L1);
    R1 = O::getRep(R1);
    L2 = O::getRep(L2);
    R2 = O::getRep(R2);
    Ob J = JE::find_join(L1, R1);
    Ob C = CE::find_comp(L2, R2);
    if (J) {
        if (C) return ensure_equiv(J, C);
        CS::make_comp(O::getRep(J), L2, R2);
        return false;
    }
    if (C) {
        CS::make_join(O::getRep(C), L1, R1);
        return false;
    }
    return true;
}

}

