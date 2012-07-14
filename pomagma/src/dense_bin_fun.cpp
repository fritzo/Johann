
#include "dense_bin_fun.hpp"
#include "aligned_alloc.hpp"
#include <cstring>

namespace pomagma
{

//ctor & dtor
dense_bin_fun::dense_bin_fun (int num_items)
    : N(num_items),
      M((N+ARG_STRIDE)/ARG_STRIDE),
      m_blocks(pomagma::alloc_blocks<Block4x4W>(M * M)),
      m_set(N,NULL),
      m_Lx_lines(pomagma::alloc_blocks<Line>((N+1) * num_lines())),
      m_Rx_lines(pomagma::alloc_blocks<Line>((N+1) * num_lines())),
      m_temp_line(pomagma::alloc_blocks<Line>(1 * num_lines()))
{
    logger.debug() << "creating dense_bin_fun with "
        << M * M << " blocks" |0;
    POMAGMA_ASSERT (N < (1<<15), "dense_bin_fun is too large"); // FIXME allow larger
    POMAGMA_ASSERTP(m_blocks, sizeof(Line), "blocks");
    POMAGMA_ASSERTP(m_Lx_lines, sizeof(Line), "Lx lines");
    POMAGMA_ASSERTP(m_Rx_lines, sizeof(Line), "Rx lines");
    POMAGMA_ASSERTP(m_temp_line, sizeof(Line), "temp line");

    //initialize to zero
    bzero(m_blocks, M * M * sizeof(Block4x4W));
    bzero(m_Lx_lines, (N+1) * num_lines() * sizeof(Line));
    bzero(m_Rx_lines, (N+1) * num_lines() * sizeof(Line));
}
dense_bin_fun::~dense_bin_fun ()
{
    pomagma::free_blocks(m_blocks);
    pomagma::free_blocks(m_Lx_lines);
    pomagma::free_blocks(m_Rx_lines);
    pomagma::free_blocks(m_temp_line);
}
void dense_bin_fun::move_from (const dense_bin_fun& other)
{//for growing
    logger.debug() << "Copying dense_bin_fun" |0;
    Logging::IndentBlock block;

    //copy data
    unsigned minM = min(M, other.M);
    for (unsigned j_=0; j_<minM; ++j_) {
        int* destin = _block(0,j_);
        const int* source = other._block(0,j_);
        memcpy(destin, source, sizeof(Block4x4W) * minM);
    }

    //copy sets
    unsigned minN = min(N, other.N);
    unsigned minL = min(num_lines(), other.num_lines());
    for (unsigned i=1; i<=minN; ++i) {
        memcpy(get_Lx_line(i), other.get_Lx_line(i), sizeof(Line) * minL);
        memcpy(get_Rx_line(i), other.get_Rx_line(i), sizeof(Line) * minL);
    }
}

//diagnostics
unsigned dense_bin_fun::size () const
{
    unsigned result = 0;
    for (unsigned i=1; i<=N; ++i) {
        result += _get_Lx_set(i).size();
    }
    return result;
}
void dense_bin_fun::validate () const
{
    logger.debug() << "Validating dense_bin_fun" |0;
    Logging::IndentBlock block;

    logger.debug() << "validating line-block consistency" |0;
    for (unsigned i_=0; i_<M; ++i_) {
    for (unsigned j_=0; j_<M; ++j_) {
        const int* block = _block(i_,j_);

        for (unsigned _i=0; _i<ARG_STRIDE; ++_i) {
        for (unsigned _j=0; _j<ARG_STRIDE; ++_j) {
            unsigned i = i_*ARG_STRIDE+_i; if (i==0 or N<i) continue;
            unsigned j = j_*ARG_STRIDE+_j; if (j==0 or N<j) continue;
            int val = _block2value(block,_i,_j);

            if (val) {
                POMAGMA_ASSERT (contains(i,j),
                        "invalid: found unsupported value: "<<i<<','<<j);
            } else {
                POMAGMA_ASSERT (not contains(i,j),
                        "invalid: found supported null value: "<<i<<','<<j);
            }
        }}
    }}

    logger.debug() << "validating left-right line consistency" |0;
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

//dense_bin_fun operations
void dense_bin_fun::remove(const int i,
                           void remove_value(int)) //rem
{
    POMAGMA_ASSERT4(0<i and i<=int(N), "item out of bounds: " << i);

    //(k,i)
    for (Iterator<RHS_FIXED> iter(this,i); not iter.done(); iter.next()) {
        int k = iter.lhs();
        int& dep = value(k,i);
        remove_value(dep);
        _get_Lx_set(k).remove(i);
        dep = 0;
    }
    _get_Rx_set(i).zero();

    //(i,k)
    for (Iterator<LHS_FIXED> iter(this,i); not iter.done(); iter.next()) {
        int k = iter.rhs();
        int& dep = value(i,k);
        remove_value(dep);
        _get_Rx_set(k).remove(i);
        dep = 0;
    }
    _get_Lx_set(i).zero();
}
void dense_bin_fun::merge(const int i, //dep
                          const int j, //rep
                          void merge_values(int,int),   //dep,rep
                          void move_value(int,int,int)) //moved,lhs,rhs
{
    POMAGMA_ASSERT4(j!=i, "in dense_bin_fun::merge, tried to merge with self");
    POMAGMA_ASSERT4(0<i and i<=int(N), "dep out of bounds: " << i);
    POMAGMA_ASSERT4(0<j and j<=int(N), "rep out of bounds: " << j);

    //Note: the spacial case (i,i) --> (i,j) --> (j,j) merges in two steps

    //(k,i) --> (k,j)
    for (Iterator<RHS_FIXED> iter(this,i); iter; iter.next()) {
        int k = iter.lhs();
        int& dep = value(k,i);
        int& rep = value(k,j);
        _get_Lx_set(k).remove(i); //sets m_set
        if (rep) {
            merge_values(dep,rep);
        } else {
            move_value(dep, k, j);
            m_set.insert(j); //ie, _get_Lx_set(k).insert(j), as above
            rep = dep;
        }
        dep = 0;
    }
    dense_set Rx_rep = _get_Rx_set(j);
    dense_set Rx_dep = _get_Rx_set(i);
    Rx_rep.merge(Rx_dep);

    //(i,k) --> (j,k)
    for (Iterator<LHS_FIXED> iter(this,i); iter; iter.next()) {
        int k = iter.rhs();
        int& dep = value(i,k);
        int& rep = value(j,k);
        _get_Rx_set(k).remove(i); //sets m_set
        if (rep) {
            merge_values(dep,rep);
        } else {
            move_value(dep, j, k);
            m_set.insert(j); //ie, _get_Rx_set(k).insert(j), as above
            rep = dep;
        }
        dep = 0;
    }
    dense_set Lx_rep = _get_Lx_set(j);
    dense_set Lx_dep = _get_Lx_set(i);
    Lx_rep.merge(Lx_dep);
}

//intersection iteration
Line* dense_bin_fun::_get_RRx_line (int i, int j) const
{
    Line* i_line = get_Rx_line(i);
    Line* j_line = get_Rx_line(j);
    for (Int k_=0; k_<num_lines(); ++k_) {
        m_temp_line[k_] = i_line[k_] & j_line[k_];
    }
    return m_temp_line;
}
Line* dense_bin_fun::_get_LRx_line (int i, int j) const
{
    Line* i_line = get_Lx_line(i);
    Line* j_line = get_Rx_line(j);
    for (Int k_=0; k_<num_lines(); ++k_) {
        m_temp_line[k_] = i_line[k_] & j_line[k_];
    }
    return m_temp_line;
}
Line* dense_bin_fun::_get_LLx_line (int i, int j) const
{
    Line* i_line = get_Lx_line(i);
    Line* j_line = get_Lx_line(j);
    for (Int k_=0; k_<num_lines(); ++k_) {
        m_temp_line[k_] = i_line[k_] & j_line[k_];
    }
    return m_temp_line;
}

}


