#ifndef JOHANN_SYNTAX_SEMANTICS_H
#define JOHANN_SYNTAX_SEMANTICS_H

#include "definitions.h"
#include "nodes.h" //for ObHdl
#include "order.h" //for Rel
#include "measures.h" //for ObPoly
#include "expressions.h"
#include "statements.h"


namespace Expressions
{

//expr --> ob
Ob find_expr (ExprHdl expr);
ObHdl get_expr (ExprHdl expr, bool make_perm=false);

//ob --> expr
Nodes::App find_parse_app (Ob ob);
Nodes::Comp find_parse_comp (Ob ob);
Nodes::Join find_parse_join (Ob ob);
ExprHdl parse_ob (Ob ob, bool large=false);

//simplification
ExprHdl simplify (ExprHdl expr, Float create=0.0, bool large=false);
class simplifier : public ExprFun
{
    const Float m_create;
public:
    simplifier (Float create=0.0) : m_create(create) {}
    virtual ~simplifier () {}
    virtual ExprHdl operator() (const ExprHdl& expr) const;
};

//projection
void add_expr_to_poly (ObPoly& poly, ExprHdl expr, Float symbols=INFINITY);

}

#endif
