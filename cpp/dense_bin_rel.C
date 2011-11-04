
#include "dense_bin_rel.h"
#include <cstring>

namespace nonstd
{

//ctor & dtor
dense_bin_rel::dense_bin_rel (int num_items, bool is_full)
    : N(num_items),
      M( (N+LINE_STRIDE) / LINE_STRIDE ),
      N_up(M * LINE_STRIDE),
      NUM_LINES(M * N_up),
      m_support(N),
      m_Lx_lines(new(std::nothrow) Line[NUM_LINES]),
      m_Rx_lines(new(std::nothrow) Line[NUM_LINES]),
      m_set(N,NULL),
      m_temp_line(new(std::nothrow) Line[M])
{
    logger.debug() << "creating dense_bin_rel with " << M << " lines" |0;
    Assert (N_up <= (1<<16), "dense_bin_rel is too large");
    AssertP(m_Lx_lines, sizeof(Line), "Lx lines");
    AssertP(m_Rx_lines, sizeof(Line), "Rx lines");
    AssertP(m_temp_line, sizeof(Line), "temp line");

    //initialize to zeros
    bzero(m_Lx_lines, sizeof(Line) * NUM_LINES);
    bzero(m_Rx_lines, sizeof(Line) * NUM_LINES);

    //fill if necessary
    if (is_full) m_support.insert_all();
}
dense_bin_rel::~dense_bin_rel ()
{
    delete[] m_Lx_lines;
    delete[] m_Rx_lines;
    delete[] m_temp_line;
}
void dense_bin_rel::move_from (const dense_bin_rel& other, const Int* new2old)
{
    logger.debug() << "Copying dense_bin_rel" |0;
    Logging::IndentBlock block;

    if (DEBUG_LEVEL >= 4) other.validate();

    //copy support
    m_support.move_from(other.m_support, new2old);

    //WARNING: assumes this has been done
    //bzero(m_Lx_lines, sizeof(Line) * NUM_LINES);
    //bzero(m_Rx_lines, sizeof(Line) * NUM_LINES);

    if (new2old == NULL) {
        logger.debug() << "copying by column and by row" |0;
        //copy rows and columns
        int minN = min(N, other.N);
        int minM = min(M, other.M);
        for (int i=1; i<=minN; ++i) {
            memcpy(get_Lx_line(i), other.get_Lx_line(i), sizeof(Line) * minM);
            memcpy(get_Rx_line(i), other.get_Rx_line(i), sizeof(Line) * minM);
        }
    } else {
        logger.debug() << "copying and reordering" |0;
        //copy & reorder WIKKIT SLOW
        for (unsigned i_new=1; i_new<=N; ++i_new) {
            if (not supports(i_new)) continue;
            unsigned i_old = new2old[i_new];

            for (unsigned j_new=1; j_new<=N; ++j_new) {
                if (not supports(j_new)) continue;
                unsigned j_old = new2old[j_new];

                if (other.contains(i_old,j_old)) insert(i_new,j_new);
            }
        }
    }
    if (DEBUG_LEVEL >= 4) validate();
}

//diagnostics
unsigned dense_bin_rel::size () const
{//supa-slow, try not to use
    unsigned result = 0;
    for (dense_set::iterator i = m_support.begin(); i; i.next()) {
        result += _get_Lx_set(*i).size();
    }
    return result;
}
void dense_bin_rel::validate () const
{
    logger.debug() << "Validating dense_bin_rel" |0;
    Logging::IndentBlock block;

    m_support.validate();

    unsigned num_pairs = 0;

    //validate sets
    for (unsigned i=0; i<N_up; ++i) {
        _get_Lx_set(i).validate();
        _get_Rx_set(i).validate();
    }

    //check emptiness of null lines
    Assert (_get_Lx_set(0).empty(), "Lx(0) line not empty");
    Assert (_get_Rx_set(0).empty(), "Rx(0) line not empty");

    dense_set Lx(N_up, NULL), Rx(N_up, NULL);
    for (unsigned i=1; i<=N; ++i) {
        bool sup_i = supports(i);
        Lx.init(get_Lx_line(i));

        Assert(i or not sup_i, "br supports null element");

        for (unsigned j=1; j<=N; ++j) {
            bool sup_ij = sup_i and supports(j);
            Rx.init(get_Rx_line(j));

            bool Lx_ij = Lx.contains(j);
            bool Rx_ij = Rx.contains(i);
            num_pairs += Rx_ij;

            Assert (Lx_ij == Rx_ij,
                    "invalid: Lx,Rx disagree at " << i << "," << j
                    << ", Lx is " << Lx_ij << ", Rx is " << Rx_ij  );

            Assert (sup_ij or not Lx_ij,
                    "invalid: Lx unsupported at " << i << "," << j );

            Assert (sup_ij or not Rx_ij,
                    "invalid: Rx unsupported at " << i << "," << j );
        }
    }

    unsigned true_size = size();
    Assert (num_pairs == true_size,
            "invalid: incorrect number of pairs: "
            << num_pairs << " should be " << true_size);
}
void dense_bin_rel::validate_disjoint (const dense_bin_rel& other) const
{
    logger.debug() << "Validating disjoint pair of dense_bin_rels" |0;
    Logging::IndentBlock block;

    //validate supports agree
    Assert (m_support.capacity() == other.m_support.capacity(),
            "invalid: disjoint dense_bin_rel support capacities disagree");
    Assert (m_support.size() == other.m_support.size(),
            "invalid: disjoint dense_bin_rel support sizes disagree");
    Assert (m_support == other.m_support,
            "invalid: disjoint dense_bin_rel supports disagree");

    //validate disjointness
    for (dense_set::iterator i = m_support.begin(); i; i.next()) {
        Assert (_get_Lx_set(*i).disjoint(other._get_Lx_set(*i)),
                "invalid: dense_bin_rels intersect at row " << i);
    }
}
void dense_bin_rel::print_table (unsigned n) const
{
    if (n == 0) n = N;
    for (unsigned i=1; i<=n; ++i) {
        cout << '\n';
        for (unsigned j=1; j<=n; ++j) {
            cout << (contains(i,j) ? 'O' : '.');
        }
    }
    cout << std::endl;
}

//dense_bin_rel operations
void dense_bin_rel::remove_Lx (const dense_set& is, int j)
{
    //slower version
    //for (dense_set::iterator i = is.begin(); i; i.next()) {
    //    remove_Lx(*i,j);
    //}

    //faster version
    unsigned mask = ~(1 << (j % LINE_STRIDE));
    int offset = j / LINE_STRIDE;
    Line* lines = m_Lx_lines + offset;
    for (dense_set::iterator i = is.begin(); i; i.next()) {
         lines[*i * M] &= mask;
    }
}
void dense_bin_rel::remove_Rx (int i, const dense_set& js)
{
    //slower version
    //for (dense_set::iterator j = js.begin(); j; j.next()) {
    //    remove_Rx(i,*j);
    //}

    //faster version
    unsigned mask = ~(1 << (i % LINE_STRIDE));
    int offset = i / LINE_STRIDE;
    Line* lines = m_Rx_lines + offset;
    for (dense_set::iterator j = js.begin(); j; j.next()) {
         lines[*j * M] &= mask;
    }
}
void dense_bin_rel::remove (int i)
{
    Assert4(supports(i), "tried to remove unsupported element " << i);

    _get_Lx_set(i);  remove_Rx(i,m_set);  m_set.zero();     //remove column
    _get_Rx_set(i);  remove_Lx(m_set,i);  m_set.zero();     //remove row

    m_support.remove(i);
}
void dense_bin_rel::ensure_inserted (int i, const dense_set& js,
                                     void (*change)(int,int))
{
    dense_set diff(N,m_temp_line), dest(N,get_Lx_line(i));
    if (dest.ensure(js, diff)) {
        for (dense_set::iterator k = diff.begin(); k; k.next()) {
            insert_Rx(i,*k);
            change   (i,*k);
        }
    }
}
void dense_bin_rel::ensure_inserted (const dense_set& is, int j,
                                     void (*change)(int,int))
{
    dense_set diff(N,m_temp_line), dest(N,get_Rx_line(j));
    if (dest.ensure(is, diff)) {
        for (dense_set::iterator k = diff.begin(); k; k.next()) {
            insert_Lx(*k,j);
            change   (*k,j);
        }
    }
}
void dense_bin_rel::merge (int i, int j,             //dep,rep
                           void (*move_to)(int,int)) //typically enforce_
{//policy: call move_to if i~k but not j~k
    Assert4(j!=i, "dense_bin_rel tried to merge item with self");
    Assert4(supports(i) and supports(j),
            "dense_bin_rel tried to merge unsupported items");

    dense_set diff(N,m_temp_line), rep(N,NULL), dep(N,NULL);

    //merge rows (i,_) into (j,_)
    dep.init(get_Lx_line(i));
    remove_Rx(i,dep);
    rep.init(get_Lx_line(j));
    if (rep.merge(dep, diff)) {
        for (dense_set::iterator k = diff.begin(); k; k.next()) {
            insert_Rx(j,*k);
            move_to  (j,*k);
        }
    }

    //merge cols (_,i) into (_,j)
    dep.init(get_Rx_line(i));
    remove_Lx(dep,i);
    rep.init(get_Rx_line(j));
    if (rep.merge(dep, diff)) {
        for (dense_set::iterator k = diff.begin(); k; k.next()) {
            insert_Lx(*k,j);
            move_to  (*k,j);
        }
    }

    m_support.merge(i,j);
}

//saving/loading, quicker rather than smaller
Int dense_bin_rel::data_size () const
{
    return 2 * sizeof(Line) * NUM_LINES;
}
void dense_bin_rel::write_to_file (FILE* file)
{
    safe_fwrite(m_Lx_lines, sizeof(Line), NUM_LINES, file);
    safe_fwrite(m_Rx_lines, sizeof(Line), NUM_LINES, file);
}
void dense_bin_rel::read_from_file (FILE* file)
{
    //WARNING: assumes support is full
    safe_fread(m_Lx_lines, sizeof(Line), NUM_LINES, file);
    safe_fread(m_Rx_lines, sizeof(Line), NUM_LINES, file);
}

//iteration
void dense_bin_rel::iterator::_find_rhs ()
{
    while (m_lhs) {
        m_rhs_set.init(m_rel.get_Lx_line(*m_lhs));
        m_rhs.begin();
        if (m_rhs) {
            _rhs(); _lhs();
            Assert5(m_rel.contains(m_pos),
                    "br::iterator landed outside of relation: "
                    << m_pos.lhs << "," << m_pos.rhs);
            return;
        }
        m_lhs.next();
    }
    _finish();
}

}

