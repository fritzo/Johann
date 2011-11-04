
#include "fuzzy.h"
#include "aligned_alloc.h"
#include "version.h"
#include <fstream>
#include <iomanip>

namespace Fuzzy
{

using namespace Symbols;

//================ fuzzy order relation ================

Order::Order (Int size)
    : m_size(size),
      m_mem(static_cast<Prob*>(
                  nonstd::alloc_blocks(sizeof(Prob), sqr(size)) )),
      m_data(m_mem - 1 - size)
{
    Assert (m_mem!=NULL, "failed to allocate Order");
    clear();
}
Order::~Order () { nonstd::free_blocks(m_mem); }

//sizing
Order* Order::resized (Int size, const Int* new2old)
{
    Order *result = new Order(size);

    if (new2old == NULL) { //just expand
        for (Int i=0; i<m_size; ++i) {
            nonstd::copy_blocks(result->mem(i), mem(i), sizeof(Prob), m_size);
        }
    }

    else { //reorder, typically contracting
        for (Int i_new=1; i_new<=size; ++i_new) { Int i_old = new2old[i_new];
        for (Int j_new=1; j_new<=size; ++j_new) { Int j_old = new2old[j_new];
            result->data(i_new,j_new) = data(i_old,j_old);
        }}
    }

    return result;
}

//batch operations
void Order::clear ()
{
    nonstd::clear_block(m_mem, sizeof(Prob) * sqr(m_size));
}
void Order::move_from (const Order& other)
{
    Assert (other.m_size == m_size,
            "tried to move_from Order of wrong size");
    nonstd::copy_blocks(m_mem, other.m_mem, sizeof(Prob), sqr(m_size));
}

//support operations
void Order::insert (Ob ob)
{//clear new row & column
    Int i = Int(ob);
    for (Ob::sparse_iterator iter=Ob::sbegin(), end=Ob::send();
            iter!=end; ++iter) {
        Int j = Int(*iter);
        data(i,j) = P_FALSE;
        data(j,i) = P_FALSE;
    }
}
void Order::merge (Ob dep, Ob rep)
{//take maximum of (possibly correlated) information
    Int i = Int(dep);
    Int j = Int(rep);
    for (Ob::sparse_iterator iter=Ob::sbegin(), end=Ob::send();
            iter!=end; ++iter) {
        Int k = Int(*iter);
        data(j,k) += data(i,k);
        data(k,j) += data(k,i);
    }
}

//output
bool Order::save_to_file (string filename) const
{//returns true on error
    if (m_size==0) return false;
    logger.info() << "Writing Order to file " << filename << " ..."|0;
    Logging::IndentBlock block;
    FILE* file = fopen(filename.c_str(), "wb");
    if (!file) {
        logger.error() << "failed to open file " << filename |0;
        return true;
    }
    safe_fwrite(m_mem, sizeof(Prob), sqr(m_size), file);
    fclose(file);
    logger.info() << "...done" |0;
    return false;
}
bool Order::write_to_file (string filename) const
{//returns true on error
    if (m_size==0) return false;
    logger.info() << "Writing Order to file " << filename << " ..."|0;
    Logging::IndentBlock block;
    ofstream file(filename.c_str());
    if (!file) {
        logger.error() << "failed to open file " << filename |0;
        return true;
    }

    //header
    //file << "#title: perturbation matrix\n";
    //file << "#author: Johann " << VERSION << "\n";
    //file << m_size << "\t" << m_size << "\n";

    //data
    for (Int i=0; i<m_size; ++i) {
        for (Int j=0; j<m_size; ++j) {
            file << mem(i,j) << '\t';
        }
        file << '\n';
    }

    logger.info() << "...done" |0;
    return false;
}
bool Order::write_to_python (string filename) const
{//returns true on error
    if (m_size==0) return false;
    logger.info() << "Writing Order to file " << filename << " ..."|0;
    Logging::IndentBlock block;
    ofstream file(filename.c_str());
    if (!file) {
        logger.error() << "failed to open python file " << filename |0;
        return true;
    }

    //header
    file << "#title: perturbation matrix\n";
    file << "#author: Johann " << VERSION << "\n";
    file << "from numpy import array\n\n";

    //data
    const Int ENTRIES_PER_LINE = 8;
    file << "P = array([\n";
    for (Int i=0; i<m_size; ++i) {
        file << "    [";
        for (Int j=0; j<m_size; ++j) {
            file << mem(i,j) << ", ";
            if ((1+j)%ENTRIES_PER_LINE == 0) {
                file << "\n        ";
            }
        }
        file << "],\n";
    }
    file << "])\n";

    logger.info() << "...done" |0;
    return false;
}

}

