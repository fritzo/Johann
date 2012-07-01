
#include "solver.h"

namespace Solver
{

namespace EX = Expressions;

Problem::Problem (Simple& s, ExprHdl r, VarHdl n);
    : simple(s),
      root(r),
      name(n),
      undef(EX::build_var("undefined")), //so user can enter patterns
      branch_pt(Var::get_fresh())
{
    //build grammar
    add_pattern(undef * undef, app_cost);
    TODO(); //add variables from simple
    //Note: the min cost of an atom in the grammar should be zero;
    //  this way a partial term's current cost is
}

void Problem::get_children const (const Node& node)
{
    children.clear(); //result

    //decide which hole to branch on
    TODO();
    ExprHdl ready = TODO(); //with branch_pt replacing undef

    //try all possible replacements
    for (unsigned i=0; i<grammar.size(); ++i) {
        ExprHdl candidate = ready->substitute(branch_pt, grammar[i]);

        //req simple normal form
        if (simple(candidate) != candidiate) continue;

        //check consistency
        TODO();
        /*TODO
        for lhs,rhs in constraints {
            lhs = lhs->subs(name, candidate);
            rhs = rhs->subs(name, candidate);
            if (invalid(lhs,rhs)) {
                candidate.clear();
                break;
            }
        }
        */
        if (candidate) children.push_back(candidate);
    }
}

}

