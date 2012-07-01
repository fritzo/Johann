#ifndef JOHANN_SEARCH_H
#define JOHANN_SEARCH_H

#include "definitions.h"
#include "priority_queue.h"
#include <vector>
#include <set>

/** \namespace Search \brief Graph search algorithms.
 *
 * Note: We're searching for states, not paths;
 *   since cost is path-independent, nodes can ignore their parents,
 *   except in SMA* retraction.
 *
 * To Do:
 *   (0) work out a fucking design before writing C++ code idiot!
 *   (1) finish abstract A* search
 *   (2) implement abstract SMA* search
 */

namespace Search
{

const Logging::Logger logger("search", Logging::DEBUG);

typedef float Cost;

/** Abstract vertex class for search states.
 * \note These should be flyweight classes.
 */
class Vertex
{
public:
    virtual ~Vertex () {}

    typedef std::pair<Vertex,Cost> Edge;    ///< edge = (neighbor,costs)
    virtual std::vector<Edge> nbhd () = 0;  ///< generates neighborhood
    virtual Cost future_cost () = 0;        ///< heuristic for future cost
    virtual bool is_terminal () = 0;        ///< whether vertex is a solution

    virtual bool operator== (const Vertex&) = 0;  ///< equality testing
    virtual bool operator!= (const Vertex&) = 0;  ///< apartness testing
};
/// Generic searcher. \brief searches for the k best solutions
class Searcher
{
protected:
    const unsigned m_num_solns;
    std::set<Vertex> m_solns;
public:
    Searcher (unsigned num_solns) : m_num_solns (num_solns) {}
    virtual ~Searcher ();

    virtual bool searching () = 0;              ///< returns true until done
    virtual void step () = 0;                   ///< performs a branch step
    virtual std::vector<Vertex*> path () = 0;   ///< returns current best path
    virtual Vertex* soln () = 0;                ///< returns best vertex so far
};

/// A* Searcher factory
Searcher* Astar_search   (Vertex* start, unsigned num_solns);

/// Simplified Memory-bounded A* Searcher factory
Searcher* SMAstar_search (Vertex* start, unsigned num_solns);

/* OLD

//search state
typedef ExprHdl State;

//search node
class Node
{
    float   m_cost;
    State   m_state;
public:
    Node (const State& state, float cost) : m_state(state), m_cost(cost) {}

    //access
    float cost  () const { return m_cost; }
    float state () const { return m_state; }

    //priority-then-arbitrary ordering for queuing
    bool operator<  (const Node& n) const
    {
        if (m_cost < n.m_cost) return true;
        if (m_cost > n.m_cost) return false;
        return m_state.hash() < n.m_state.hash();
    }
    bool operator<= (const Node& n) const
    {
        if (m_cost < n.m_cost) return true;
        if (m_cost > n.m_cost) return false;
        return m_state.hash() <= n.m_state.hash();
    }
};
typedef std::vector<Node> Nodes;

class Problem
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

//A* search algorithm
class Searcher
{
    Problem& m_problem;
    nonstd::priority_queue<Node> m_queue;
    std::set<Node> m_solns;
    unsigned m_limit;
public:

    Searcher (Problem& problem, unsigned limit=1<<20)
        : m_problem(problem), m_limit(limit)
    {
        m_queue.push(Node(problem.root));
    }

    bool empty () const { return m_queue.empty(); }
    const std::set<Node>& nodes
    unsigned search (unsigned num_solns=1); //returns number of solutions found
};

//SMA* = simplified memory-limited A*
//TODO
*/

}

#endif
