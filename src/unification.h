#ifndef JOHANN_UNIFICATION_H
#define JOHANN_UNIFICATION_H

#include "definitions.h"
#include "expressions.h"
#include <map>

namespace Unification
{

const Logging::Logger logger("unify", Logging::DEBUG);

ExprHdl unify (ExprHdl lhs, ExprHdl rhs);

struct App
{
    ExprHdl lhs,rhs;
    App () {}
    App(ExprHdl l, ExprHdl r) : lhs(l), rhs(r) {}
};
typedef std::vector<App> Apps;
Apps critical_pairs (App& s_t, App& u_v);

}

#endif
