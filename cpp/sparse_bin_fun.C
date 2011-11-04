
#include "sparse_bin_fun.h"
#include "aligned_alloc.h"
#include <cstring>

namespace nonstd
{

//ctor & dtor
sparse_bin_fun::sparse_bin_fun (int num_items)
    : N(num_items),
      m_map(),
      m_set(N,NULL),
      m_Lx_lines(new(std::nothrow) Line[(N+1) * num_lines()]),
      m_Rx_lines(new(std::nothrow) Line[(N+1) * num_lines()]),
      m_temp_line(new(std::nothrow) Line[1 * num_lines()])
{
    logger.debug() << "creating sparse_bin_fun for " << N * N << " values" |0;
    Assert (N < (1<<15), "sparse_bin_fun is too large");
    Assert (m_Lx_lines != NULL, "Lx line allocation failed");
    Assert (m_Rx_lines != NULL, "Rx line allocation failed");
    Assert (m_temp_line != NULL, "int line allocation failed");

    //initialize to zero
    bzero(m_Lx_lines, (N+1) * num_lines() * sizeof(Line));
    bzero(m_Rx_lines, (N+1) * num_lines() * sizeof(Line));
}
sparse_bin_fun::~sparse_bin_fun ()
{
    delete[] m_Lx_lines;
    delete[] m_Rx_lines;
    delete[] m_temp_line;
}
void sparse_bin_fun::move_from (sparse_bin_fun& other)
{//for growing
    logger.debug() << "Copying sparse_bin_fun" |0;
    Logging::IndentBlock block;

    //copy data
    m_map.swap(other.m_map);

    //copy sets
    unsigned minN = min(N, other.N);
    unsigned minL = min(num_lines(), other.num_lines());
    for (unsigned i=1; i<=minN; ++i) {
        memcpy(get_Lx_line(i), other.get_Lx_line(i), sizeof(Line) * minL);
        memcpy(get_Rx_line(i), other.get_Rx_line(i), sizeof(Line) * minL);
    }
}

//diagnostics
void sparse_bin_fun::validate () const
{
    logger.debug() << "Validating sparse_bin_fun" |0;
    Logging::IndentBlock block;

    logger.debug() << "validating data" |0;
    for (unsigned i=0; i<N; ++i) {
    for (unsigned j=0; j<N; ++j) {

        if (m_map.find(Key(i,j)) != m_map.end()) {
            Assert (i and j and contains(i,j),
                    "invalid: found unsupported value: "<<i<<','<<j);
        } else {
            Assert (not (i and j and contains(i,j)),
                    "invalid: found supported null value: "<<i<<','<<j);
        }
    }}

    logger.debug() << "validating lines" |0;
    for (unsigned i=1; i<=N; ++i) {
        dense_set L_set(_get_Lx_set(i));

        for (unsigned j=1; j<=N; ++j) {
            dense_set R_set(_get_Rx_set(j));

            if (L_set.contains(j) and not R_set.contains(i)) {
                logger.error() << "L-set exceeds R-set: " << i << "," << j |0;
            }
            if (R_set.contains(i) and not L_set.contains(j)) {
                logger.error() << "R-set exceeds L-set: " << i << "," << j |0;
            }
        }
    }
}

//sparse_bin_fun operations
void sparse_bin_fun::remove(const int i,
                           void remove_value(int)) //rem
{
    Assert4(0<i and i<=int(N), "item out of bounds: " << i);

    //(k,i)
    for (Iterator<RHS_FIXED> iter(this,i); not iter.done(); iter.next()) {
        int k = iter.lhs();
        Map::iterator dep = m_map.find(Key(k,i));
        Assert4(dep != m_map.end(), "tried to remove absent element");
        remove_value(dep->second);
        _get_Lx_set(k).remove(i);
        m_map.erase(dep);
    }
    _get_Rx_set(i).zero();

    //(i,k)
    for (Iterator<LHS_FIXED> iter(this,i); not iter.done(); iter.next()) {
        int k = iter.rhs();
        Map::iterator dep = m_map.find(Key(i,k));
        Assert4(dep != m_map.end(), "tried to remove absent element");
        remove_value(dep->second);
        _get_Rx_set(k).remove(i);
        m_map.erase(dep);
    }
    _get_Lx_set(i).zero();
}
void sparse_bin_fun::merge(const int i, //dep
                          const int j, //rep
                          void merge_values(int,int),   //dep,rep
                          void move_value(int,int,int)) //moved,lhs,rhs
{
    Assert4(j!=i, "in sparse_bin_fun::merge, tried to merge with self");
    Assert4(0<i and i<=int(N), "dep out of bounds: " << i);
    Assert4(0<j and j<=int(N), "rep out of bounds: " << j);

    //(k,i) --> (k,i)
    for (Iterator<RHS_FIXED> iter(this,i); iter; iter.next()) {
        int k = iter.lhs();
        Map::iterator dep = m_map.find(Key(k,i));
        Map::iterator rep = m_map.find(Key(k,j));
        Assert4(dep != m_map.end(), "tried to merge absent element");
        _get_Lx_set(k).remove(i);
        if (rep != m_map.end()) {
            merge_values(dep->second,rep->second);
        } else {
            move_value(dep->second, k, j);
            m_set.insert(j);
            m_map[Key(k,j)] = dep->second;
        }
        m_map.erase(dep);
    }
    dense_set Rx_rep = _get_Rx_set(j);
    dense_set Rx_dep = _get_Rx_set(i);
    Rx_rep.merge(Rx_dep);

    //(i,k) --> (j,k)
    for (Iterator<LHS_FIXED> iter(this,i); iter; iter.next()) {
        int k = iter.rhs();
        Map::iterator dep = m_map.find(Key(i,k));
        Map::iterator rep = m_map.find(Key(j,k));
        _get_Rx_set(k).remove(i);
        if (rep != m_map.end()) {
            merge_values(dep->second,rep->second);
        } else {
            move_value(dep->second, j, k);
            m_set.insert(j);
            m_map[Key(j,k)] = dep->second;
        }
        m_map.erase(dep);
    }
    dense_set Lx_rep = _get_Lx_set(j);
    dense_set Lx_dep = _get_Lx_set(i);
    Lx_rep.merge(Lx_dep);
}

//intersection iteration
Line* sparse_bin_fun::_get_RRx_line (int i, int j) const
{
    Line* i_line = get_Rx_line(i);
    Line* j_line = get_Rx_line(j);
    for (Int k_=0; k_<num_lines(); ++k_) {
        m_temp_line[k_] = i_line[k_] & j_line[k_];
    }
    return m_temp_line;
}
Line* sparse_bin_fun::_get_LRx_line (int i, int j) const
{
    Line* i_line = get_Lx_line(i);
    Line* j_line = get_Rx_line(j);
    for (Int k_=0; k_<num_lines(); ++k_) {
        m_temp_line[k_] = i_line[k_] & j_line[k_];
    }
    return m_temp_line;
}
Line* sparse_bin_fun::_get_LLx_line (int i, int j) const
{
    Line* i_line = get_Lx_line(i);
    Line* j_line = get_Lx_line(j);
    for (Int k_=0; k_<num_lines(); ++k_) {
        m_temp_line[k_] = i_line[k_] & j_line[k_];
    }
    return m_temp_line;
}

}


