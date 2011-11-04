
#include "graph.h"
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <cmath>
#include <string>

#define LOG(mess) { std::cout << mess << std::endl; }
#define LOG1(mess)
#define DEBUG(mess) LOG("DEBUG " << mess)
#define DEBUG1(mess)
#define ERROR(mess) { std::cerr << mess << std::endl; exit(1); }
#define WARN(mess) { std::cerr << "WARNING " << mess << std::endl; }
#define ASSERT(cond,mess) { if (!(cond)) { ERROR(mess); } }
#define EXPECT(cond,mess) { if (!(cond)) { WARN(mess); } }

#define ASSERT_CLOSE(x,y) { \
  ASSERT(fabs(x-y) < 1e-4, \
         "expected " #x " = " #y ", actual " << x << " vs " << y); }
#define EXPECT_CLOSE(x,y) { \
  EXPECT(fabs(x-y) < 1e-4, \
         "expected " #x " = " #y ", actual " << x << " vs " << y); }

Graph::Graph (unsigned num_verts, unsigned num_edges)
  : SymmetricLinearForm(num_verts),
    m_num_edges(num_edges),
    m_degs(new float[num_verts]),
    m_edges(new Edge[num_edges])
{}

Map::Map (unsigned num_verts, unsigned num_edges, unsigned num_eigs)
  : Graph(num_verts, num_edges),
    m_num_eigs(num_eigs),
    m_eigs(new float[m_size * m_num_eigs])
{}

/** Matrix multiplication for eigenvalue computations.
 *
 * Let A be this graph's adjacency matrix,
 * and D be the diagonal matrix of degrees, i.e.,
 *   D(i,i) = sum_j A(i,j) = sum_j A(j,i)
 * Then to find the largest few eigenvectors of the laplacian A - D,
 * we solve the eigenvalue problem
 *   (A - D D) x = t x
 * by shifting by +2 max_deg, to guarantee all eigenvalues are positive.
 *
 * Note that eigenvalues are currently discarded.
 */
void Graph::apply (const float * restrict x, float * restrict y) const
{
  for (unsigned v = 0; v < m_size; ++v) {
    y[v] = (2.0f * m_max_deg - m_degs[v]) * x[v];
  }

  for (Edge *e = m_edges, *end = m_edges + m_num_edges; e != end; ++e) {
    unsigned head = e->head;
    unsigned tail = e->tail;
    float weight = e->weight;

    ASSERT(head < m_size, "head out of range: " << head);
    ASSERT(tail < m_size, "tail out of range: " << tail);

    y[head] += weight * x[tail];
    y[tail] += weight * x[head];
  }
}

void Map::convert_eigs ()
{
  LOG("rescale eigenvectors to have unit covariance");

  for (unsigned e = 0; e < m_num_eigs; ++e) {
    float norm2 = 0;
    for (unsigned v = 0; v < m_size; ++v) {
      norm2 += pow(eig(v,e), 2);
    }
    float scale = pow(norm2, -0.5);
    for (unsigned v = 0; v < m_size; ++v) {
      eig(v,e) *= scale;
    }
  }
}

void Graph::clear()
{
  DEBUG1("clearing graph");
  m_size = 0;
  m_num_edges = 0;
  if (m_degs) { delete[] m_degs; m_degs = NULL; }
  if (m_edges) { delete[] m_edges; m_edges = NULL; }
}

void Map::clear ()
{
  DEBUG1("clearing map");
  m_num_eigs = 0;
  if (m_eigs) { delete[] m_eigs; m_eigs = NULL; }
}

//----( high-level file i/o )-------------------------------------------------

void Graph::save (const char * filename) const
{
  LOG("saving graph to " << filename);
  std::ofstream file(filename);
  write_to(file);
}

void Graph::load (const char * filename)
{
  LOG("loading graph from " << filename);
  clear();
  std::ifstream file(filename);
  read_from(file);
}

void Map::save (const char * filename) const
{
  LOG("saving map to " << filename);
  std::ofstream file(filename);
  write_to(file);
}

void Map::load (const char * filename)
{
  LOG("loading map from " << filename);
  clear();
  std::ifstream file(filename);
  read_from(file);
}

void Map::load_graph (const char * filename, unsigned num_eigs)
{
  clear();
  Graph::load(filename);

  LOG("allocating space for " << num_eigs << " eigenvectors");
  m_num_eigs = num_eigs;
  ASSERT(m_eigs == NULL, "eigs not cleared before allocating");
  m_eigs = new float[m_size * num_eigs];
}

//----( low level output )----------------------------------------------------

void Graph::write_to (std::ostream & os) const
{
  os << "GRAPH" << "\n\n";

  os << m_size << " VERTICES\n\n";

  os << m_num_edges << " EDGES\n";
  if (m_num_edges) {
    ASSERT(m_edges, "edges null when writing");
    for (unsigned i = 0; i < m_num_edges; ++i) {
      os << m_edges[i] << '\n';
    }
  }
}

void Map::write_to (std::ostream & os) const
{
  Graph::write_to(os);
  os << '\n';

  os << m_num_eigs << " EIGENVECTORS\n";
  if (m_num_eigs) {
    ASSERT(m_eigs, "eigs null when writing");
    for (unsigned v = 0; v < m_size; ++v) {
      for (unsigned n = 0; n < m_num_eigs; ++n) {
        os << ' ' << eig(v,n);
      }
      os << '\n';
    }
  }
}

//----( low level output )----------------------------------------------------

#define ASSERT_TAG(label) {\
  std::string tag; is >> tag; \
  ASSERT(tag == label, "expected " << label << " but read " << tag); }

void Graph::read_from (std::istream & is)
{
  ASSERT_TAG("GRAPH");
  LOG("loading graph");

  is >> m_size; ASSERT_TAG("VERTICES");
  LOG(" with " << m_size << " vertices");
  m_degs = new float[m_size];

  is >> m_num_edges; ASSERT_TAG("EDGES");
  LOG(" and " << m_num_edges << " edges");
  ASSERT(m_edges == NULL, "edges not cleared before reading");
  m_edges = new Edge[m_num_edges];
 
  for (unsigned v = 0; v < m_size; ++v) {
    m_degs[v] = 0;
  }
  for (unsigned i = 0; i < m_num_edges; ++i) {
    Edge & e = m_edges[i];
    is >> e;

    ASSERT(e.head < m_size, "head out of range: " << e.head);
    ASSERT(e.tail < m_size, "tail out of range: " << e.tail);

    m_degs[e.head] += e.weight;
    m_degs[e.tail] += e.weight;
  }
  m_max_deg = 0;
  for (unsigned v = 0; v < m_size; ++v) {
    m_max_deg = std::max(m_max_deg, m_degs[v]);
  }
  LOG("max degree = " << m_max_deg);
}

void Map::read_from (std::istream & is)
{
  Graph::read_from(is);

  is >> m_num_eigs; ASSERT_TAG("EIGENVECTORS");
  LOG(" and " << m_num_eigs << " eigenvectors");
  ASSERT(m_eigs == NULL, "eigs not cleared before reading");
  m_eigs = new float[m_size * m_num_eigs];

  for (unsigned v = 0; v < m_size; ++v) {
    for (unsigned n = 0; n < m_num_eigs; ++n) {
      is >> eig(v,n);
    }
  }
}


