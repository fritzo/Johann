
#include "search.h"

namespace Search
{

/// Search node as point in path. \brief see Vertex for typical template param.
class Node;
typedef std::vector<Node*> Nodes;
class Node
{
    Vertex* m_vertex;
    Node* m_parent;
public:
    Node (Vertex* v, Node* p=NULL) : m_vertex(v), m_parent(p) {}
    ~Node () {}

    Nodes branch ();
};
Nodes Node::branch ()
{
    Verts verts = m_vertex.neighbors();
    TODO();
}

/// A* Searcher.
class Astar : public Searcher
{
    nonstd::priority_queue<Node> m_queue;
public:
    virtual bool searching ();
    virtual void step ();
    virtual std::vector<Vertex*> path ();
    virtual Vertex* soln ();
};
bool Astar::searching ()
{//whether to keep searching
    return m_solns.size() < m_num_solns and not m_nonterminal.empty();
}
bool Astar::step (unsigned num_solns)
{
    logger.info() << "A* stepping" |0;
    Logging::IndentBlock block;

    Node* best = m_queue.pop();
    Nodes branches = best.branch();
    for (unsigned i=0; i<nodes.size; ++i) {
        Node* n = branches[i];
        if (n.is_terminal()) m_solns.insert(n);
        else                 m_queue.push(n);
    }
    return searching();
}
Searcher* Astar_search (Vertex* start, unsigned num_solns)
{
    return new Astar (start, num_solns);
}

/* OLD
unsigned Searcher::search (unsigned num_solns)
{
    logger.info() << "Searching for " << num_solutions << " solutions"
                  << ", limit = " << m_limit |0;
    Logging::IndentBlock block;

    Nodes& c = problem.children;
    while (not empty() and m_solns.size() < num_solns) {
        problem.get_children(m_queue.pop()); //sets problem.children
        for (unsigned i=0; i<c.size(); ++i) {
            const Node& n = c[i];
            if (problem.complete(s)) m_solns.insert(n);
            else                     m_queue.push(n);
        }

        while (m_queue.size() > m_limit) m_queue.prune();
    }
    return m_solns.size();
}
*/

}

