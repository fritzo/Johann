#ifndef JOHANN_FUZZY_H
#define JOHANN_FUZZY_H

#include "definitions.h"
#include "symbols.h"
#include "nodes.h"

namespace Fuzzy
{//data structures for for fuzzy/probabilistic logic

const Logging::Logger logger("fuzzy", Logging::DEBUG);

using Symbols::Prob;

//fuzzy order relation
class Order
{
    const Int m_size;

    //zero-based indexing
    Prob*const m_mem; //keep real memory pointer to appease valgrind
          Prob* mem ()       { return m_mem; }
    const Prob* mem () const { return m_mem; }
          Prob* mem (Int row)       { return m_mem + m_size * row; }
    const Prob* mem (Int row) const { return m_mem + m_size * row; }
    Prob& mem (Int lhs, Int rhs)       { return m_mem[m_size * lhs + rhs]; }
    Prob  mem (Int lhs, Int rhs) const { return m_mem[m_size * lhs + rhs]; }

    //one-based indexing
    Prob*const m_data;

    //prohibit implicit copying
    Order (const Order&) : m_size(0), m_mem(NULL), m_data(NULL)
    { Assert(0,"copy-constructing an Order"); }
    void operator= (const Order&) { Assert(0,"copying an Order"); }
public:
    Order (Int size);
    ~Order ();

    //sizing
    Int size () const { return m_size; }
    Order* resized (Int size, const Int* new2old=NULL);

    //batch operations
    void clear ();
    void move_from (const Order& other);

    //element operations
    Prob& data (Int lhs, Int rhs)
    {
        Assert5(0 < lhs and lhs <= m_size, "lhs out-of-bounds");
        Assert5(0 < rhs and rhs <= m_size, "rhs out-of-bounds");
        return m_data[m_size * lhs + rhs];
    }
    Prob data (Int lhs, Int rhs) const
    {
        Assert5(0 < lhs and lhs <= m_size, "lhs out-of-bounds");
        Assert5(0 < rhs and rhs <= m_size, "rhs out-of-bounds");
        return m_data[m_size * lhs + rhs];
    }
    Prob& operator() (Int i, Int j)       { return data(i,j); }
    Prob  operator() (Int i, Int j) const { return data(i,j); }
    Prob& operator() (Ob i, Ob j)       { return data(Int(i),Int(j)); }
    Prob  operator() (Ob i, Ob j) const { return data(Int(i),Int(j)); }

    //support operations
    void insert (Ob ob);
    void remove (Ob ob) {} //do nothing
    void merge (Ob dep, Ob rep);

    //output, returning true on error
    bool save_to_file    (string filename) const;
    bool write_to_file   (string filename) const;
    bool write_to_python (string filename) const;
};

}

#endif
