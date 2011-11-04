/** Graph format, optimized for i/o and fast matrix-vector multiplication.
 *
 * File Format:
 *
 *   GRAPH
 *
 *   num_verts VERTICES
 *
 *   num_edges EDGES
 *   head_id tail_id weight  // row 0
 *   ...                     // through
 *   head_id tail_id weight  // row num_edges-1
 *
 *   num_eigs EIGENVECTORS  // typically 7 eigenvectors
 *   e0 e1 e2 e3 e4 e5 e6   // row 0, all components of eigenvector
 *   ...                    // through
 *   e0 e1 e2 e3 e4 e5 e6   // row num_verts-1
 *
*/

#ifndef JOHANN_GRAPH_H
#define JOHANN_GRAPH_H

#include <iostream>
#include <string>
#include <utility>

#ifdef __GNUG__
  #define restrict __restrict__
#else // __GNUG__
  #warning keyword 'restrict' ignored
  #define restrict
#endif

class SymmetricLinearForm
{
protected:

  unsigned m_size;

public:

  SymmetricLinearForm () : m_size(0) {}
  SymmetricLinearForm (unsigned size) : m_size(size) {}
  virtual ~SymmetricLinearForm () {}
  
  virtual void apply (const float * restrict x, float * restrict y) const = 0;
};

class Graph : public SymmetricLinearForm
{
public:

  typedef int Vertex;
  struct Edge
  {
    Vertex head, tail;
    float weight;
    Edge () {}
    Edge (Vertex h, Vertex t, float w=1.0f) : head(h), tail(t), weight(w) {}

    friend inline std::ostream& operator<< (std::ostream & os, const Edge & e)
    {
      return os << e.head << ' ' << e.tail << ' ' << e.weight;
    }
    friend inline std::istream& operator>> (std::istream & is, Edge & e)
    {
      return is >> e.head >> e.tail >> e.weight;
    }
  };

protected:

  unsigned m_num_edges;
  float m_max_deg;
  float * m_degs;
  Edge * m_edges;

public:

  Graph () : m_num_edges(0), m_max_deg(0), m_degs(NULL), m_edges(NULL) {}
  Graph (unsigned num_verts, unsigned num_edges);

  void clear ();
  virtual ~Graph () { clear(); }

  // building graphs
  void add_edge (Vertex head, Vertex tail);
  virtual void save (const char * filename) const;

  // loading graphs
  virtual void load (const char * filename);

  const unsigned & num_verts () const { return m_size; }
  const unsigned & num_edges () const { return m_num_edges; }
  const Edge * edges () const { return m_edges; }

  virtual void apply (const float * restrict x, float * restrict y) const;

protected:
  void write_to (std::ostream & os) const;
  void read_from (std::istream & is);
};

class Map : public Graph
{

protected:

  unsigned m_num_eigs;
  float * m_eigs;

public:

  Map () : m_num_eigs(0), m_eigs(NULL) {}
  Map (unsigned num_verts, unsigned num_edges, unsigned num_eigs);

  void clear ();
  virtual ~Map () { clear(); }

  virtual void save (const char * filename) const;
  virtual void load (const char * filename);
  void load_graph (const char * filename, unsigned num_eigs = 7);

  unsigned num_eigs () const { return m_num_eigs; }
  float eig (unsigned v, unsigned n) const
  {
    return m_eigs[n + m_num_eigs * v];
  }
  float & eig (unsigned v, unsigned n)
  {
    return m_eigs[n + m_num_eigs * v];
  }
  float * eigs () { return m_eigs; }
  void convert_eigs ();

protected:
  void write_to (std::ostream & os) const;
  void read_from (std::istream & is);
};

#endif // JOHANN_GRAPH_H

