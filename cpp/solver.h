#ifndef JOHANN_SOLVER_H
#define JOHANN_SOLVER_H

#include "definitions.h"
#include "expressions.h"
#include "simple.h"
#include "search.h"

/** \namespace Solver \brief Constrained hole-filling solver.
 *
 * To Do.
 *   (1) ? How to vectorize, how to search for multiple holes?
 *   (2) ? How to treat hypotheses?
 *   (3) ? Theorems, Proofs, and certified Axioms?
 *
 */

namespace Solver
{

const Logging::Logger logger("solver", Logging::DEBUG);

class Problem : public Search::Vertex
{
    //XXX these two should really be combined;
    //  maybe into a context?
    Simple& simple;
    Nodes grammar;

    VarHdl undef, branch_pt;
public:
    ExprHdl root;   //where problem starts, an piece of code with holes
    VarHdl name;    //what solution is called in constraints
    Nodes children;

    Problem (Simple& simple, ExprHdl root, VarHdl name);
    void add_pattern (ExprHdl patt, float cost)
    { grammar.push_back(Node(patt,cost)); }
    /*TODO
    void add_constraint (StmtHdl statement);
    */

    bool complete (const State& s) const { return not s->contains(undef); }
    void get_children (const Node& s); //encodes consistency
};

}

#endif
