#ifndef JOHANN_COLLECTIVE_H
#define JOHANN_COLLECTIVE_H

#include "definitions.h"
#include <set>
#include <vector>
#include <utility>

/** The Collective, a massively parallel society of Solomonoff inductors.
 * Each learns to predict/compress its neighbors.
 * Somewhere in the network is an input agent, the connection to the world.
 * Motivation flows through the network to ensure service to the world.
 *
 * XXX very sketchy XXX
 */

namespace Collective
{

const Logging::Logger logger("collect", Logging::DEBUG);

template<class Vertex=unsigned, class Channel=char>
class Network
{
    std::vector<Vertex> m_verts;
    std::map<std::pair<Vertex,Channel>, Channel> m_edges;
public:

    unsigned v_size () { return m_verts.size(); }
    class v_iterator {}; //TODO
    v_iterator v_begin () const;
    v_iterator v_end () const;

    unsigned e_size () { return m_edges.size(); }
    class e_iterator {}; //TODO
    e_iterator e_begin () const;
    e_iterator e_end () const;
};

template<class Vertex=unsigned, class Channel=char>
class RandomNetwork : public Network<Vertex,Channel>
{
public:
    RandomNetwork (unsigned v, unsigned c);
};
template<class Vertex=unsigned, class Channel=char>
RandomNetwork<Vertex,Channel>::RandomNetwork (unsigned V, unsigned C)
{
    //add vertices
    TODO();

    //add edges randomly
    TODO();
}

template<class Message=char, class Channel=char>
class Node
{
public:
    Message& in (Channel c) = 0;
    Message out (Channel c) const = 0;
    float update () = 0; //returns loss
};

template<class Message, class Node>
class Collective
{
    std::vector<Node> m_nodes;
public:
    Collective (Network n) : m_nodes(n);

    void deliver ();
    float update (); //returns loss
};

void Collective::deliver ()
{
    for (Net::e_iterator e=net.e_begin(); e!=net.e_end(); ++e) {
        Node& lhs = m_nodes[e.lhs()];
        Node& rhs = m_nodes[e.rhs()];
        Channel c = e.channel();
        lhs.in(c) = rhs.out(c);
        rhs.in(c) = lhs.out(c);
    }
}

float Collective::update ()
{
    float loss = 0.0f;
    for (Net::v_iterator v=net.v_begin(); v!=net.v_end(); ++v) {
        loss += m_nodes[v]->update();
    }
}

}

#endif
