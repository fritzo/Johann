#ifndef NONSTD_SPARSE_BIN_FUN_H
#define NONSTD_SPARSE_BIN_FUN_H

#include "definitions.h"
#include "dense_set.h"

//hash functions
#ifdef __GNUG__
    #include "hash_map.h"
    #define MAP_TYPE std::unordered_map
#else
    #include <map>
    #define MAP_TYPE std::map
#endif

namespace nonstd
{

using Logging::logger;

//WARNING: zero/null items are not allowed

class sparse_bin_fun
{
    typedef sparse_bin_fun MyType;

    //data, as hash table
    const unsigned N;     //item dimension
    typedef std::pair<size_t,size_t> Key;
    typedef MAP_TYPE<Key,int> Map;
    Map m_map;

    //sparse sets for iteration
    mutable dense_set m_set; //this is a temporary
    Line* m_Lx_lines;
    Line* m_Rx_lines;
    mutable Line* m_temp_line; //this is a temporary

    //set wrappers
    unsigned num_lines () const { return m_set.num_lines(); }
public:
    Line* get_Lx_line (int i) const { return m_Lx_lines + (i*num_lines()); }
    Line* get_Rx_line (int i) const { return m_Rx_lines + (i*num_lines()); }
private:
    dense_set& _get_Lx_set (int i) { return m_set.init(get_Lx_line(i)); }
    dense_set& _get_Rx_set (int i) { return m_set.init(get_Rx_line(i)); }
    const dense_set& _get_Lx_set (int i) const
    { return m_set.init(get_Lx_line(i)); }
    const dense_set& _get_Rx_set (int i) const
    { return m_set.init(get_Rx_line(i)); }

    //intersection wrappers
    Line* _get_RRx_line (int i, int j) const;
    Line* _get_LRx_line (int i, int j) const;
    Line* _get_LLx_line (int i, int j) const;

    //ctors & dtors
public:
    sparse_bin_fun (int num_items);
    ~sparse_bin_fun ();
    void move_from (sparse_bin_fun& other); //for growing

    //function calling
    inline int  get_value    (int lhs, int rhs) const;

    //attributes
    unsigned size     () const { return m_map.size(); }
    unsigned capacity () const { return N*N; }
    unsigned sup_capacity () const { return N; }
    void validate () const;

    //element operations
    void insert (int lhs, int rhs, int val)
    {
        m_map[Key(lhs,rhs)] = val;
        _get_Lx_set(lhs).insert(rhs);
        _get_Rx_set(rhs).insert(lhs);
    }
    void remove (int lhs, int rhs)
    {
        Map::iterator val = m_map.find(Key(lhs,rhs));
        Assert4(val != m_map.end(), "tried to remove absent item");
        m_map.erase(val);
        _get_Lx_set(lhs).remove(rhs);
        _get_Rx_set(rhs).remove(lhs);
    }
    bool contains (int lhs, int rhs) const
    { return _get_Lx_set(lhs).contains(rhs); }

    //support operations
    void remove   (const int i,
                   void remove_value(int)); //rem
    void merge    (const int i, const int j,
                   void merge_values(int,int),    //dep,rep
                   void move_value(int,int,int)); //moved,lhs,rhs

    //================ iteration over a line ================
    enum { LHS_FIXED = false, RHS_FIXED = true };
    template<int idx> class Iterator
    {
        dense_set           m_set;
        dense_set::iterator m_iter;
        const sparse_bin_fun *m_fun;
        int m_lhs, m_rhs;

        void _set_pos () { if (idx) m_lhs = *m_iter; else m_rhs = *m_iter; }
    public:
        //traversal
        operator bool () const { return m_iter; }
        bool done () const { return m_iter.done(); }
        void begin () { m_iter.begin(); if (not done()) _set_pos(); }
        void begin (int fixed)
        {
            if (idx) { m_rhs=fixed; m_set.init(m_fun->get_Rx_line(fixed)); }
            else     { m_lhs=fixed; m_set.init(m_fun->get_Lx_line(fixed)); }
            begin();
        }
        void next () { m_iter.next(); if (not done()) _set_pos(); }

        //construction
        Iterator (const sparse_bin_fun* fun)
            : m_set(fun->N, NULL), m_iter(&m_set), m_fun(fun),
              m_lhs(0), m_rhs(0) {}

        Iterator (const sparse_bin_fun* fun, int fixed)
            : m_set(fun->N, idx ? fun->get_Rx_line(fixed)
                                : fun->get_Lx_line(fixed)),
              m_iter(&m_set), m_fun(fun), m_lhs(fixed), m_rhs(fixed)
        { begin(); }

        Iterator (const sparse_bin_fun* fun, int fixed, dense_set& subset)
            : m_set(fun->N, subset),
              m_iter(&m_set), m_fun(fun), m_lhs(fixed), m_rhs(fixed)
        {
            m_set *= idx ? fun->get_Rx_line(fixed)
                         : fun->get_Lx_line(fixed);
            begin();
        }

        //dereferencing
    private:
        void _deref_assert () const
        { Assert5(not done(), "dereferenced done dense_set::iter"); }
    public:
        int lhs () const { _deref_assert(); return m_lhs; }
        int rhs () const { _deref_assert(); return m_rhs; }
        int value () const
        { _deref_assert(); return m_fun->get_value(m_lhs,m_rhs); }
    };

    //================ intersection iteration over 2 lines ================
    class RRxx_Iter
    {
        dense_set           m_set;
        dense_set::iterator m_iter;
        const sparse_bin_fun *m_fun;
        int m_lhs, m_rhs1, m_rhs2;
    public:
        //traversal
        void begin () { m_iter.begin(); if (not done()) m_lhs = *m_iter; }
        void begin (int fixed1, int fixed2)
        {
            m_set.init(m_fun->_get_RRx_line(fixed1, fixed2));
            m_iter.begin();
            if (not done()) {
                m_rhs1 = fixed1;
                m_rhs2 = fixed2;
                m_lhs = *m_iter;
            }
        }
        operator bool () const { return m_iter; }
        bool done () const { return m_iter.done(); }
        void next () { m_iter.next(); if (not done()) m_lhs = *m_iter; }

        //construction
        RRxx_Iter (const sparse_bin_fun* fun)
            : m_set(fun->N, NULL), m_iter(&m_set), m_fun(fun) {}

        //dereferencing
        int lhs    () const { return m_lhs; }
        int value1 () const { return m_fun->get_value(m_lhs,m_rhs1); }
        int value2 () const { return m_fun->get_value(m_lhs,m_rhs2); }
    };
    class LRxx_Iter
    {
        dense_set           m_set;
        dense_set::iterator m_iter;
        const sparse_bin_fun *m_fun;
        int m_lhs1, m_rhs2, m_rhs1;
    public:
        //traversal
        void begin () { m_iter.begin(); if (not done()) m_rhs1 = *m_iter; }
        void begin (int fixed1, int fixed2)
        {
            m_set.init(m_fun->_get_LRx_line(fixed1, fixed2));
            m_iter.begin();
            if (not done()) {
                m_lhs1 = fixed1;
                m_rhs2 = fixed2;
                m_rhs1 = *m_iter;
            }
        }
        operator bool () const { return m_iter; }
        bool done () const { return m_iter.done(); }
        void next () { m_iter.next(); if (not done()) m_rhs1 = *m_iter; }

        //construction
        LRxx_Iter (const sparse_bin_fun* fun)
            : m_set(fun->N, NULL), m_iter(&m_set), m_fun(fun) {}

        //dereferencing
        int rhs1   () const { return m_rhs1; }
        int lhs2   () const { return m_rhs1; }
        int value1 () const { return m_fun->get_value(m_lhs1,m_rhs1); }
        int value2 () const { return m_fun->get_value(m_rhs1,m_rhs2); }
    };
    class LLxx_Iter
    {
        dense_set           m_set;
        dense_set::iterator m_iter;
        const sparse_bin_fun *m_fun;
        int m_lhs1, m_lhs2, m_rhs;
    public:
        //traversal
        void begin () { m_iter.begin(); if (not done()) m_rhs = *m_iter; }
        void begin (int fixed1, int fixed2)
        {
            m_set.init(m_fun->_get_LLx_line(fixed1, fixed2));
            m_iter.begin();
            if (not done()) {
                m_lhs1 = fixed1;
                m_lhs2 = fixed2;
                m_rhs = *m_iter;
            }
        }
        operator bool () const { return m_iter; }
        bool done () const { return m_iter.done(); }
        void next () { m_iter.next(); if (not done()) m_rhs = *m_iter; }

        //construction
        LLxx_Iter (const sparse_bin_fun* fun)
            : m_set(fun->N, NULL), m_iter(&m_set), m_fun(fun) {}

        //dereferencing
        int rhs    () const { return m_rhs; }
        int value1 () const { return m_fun->get_value(m_lhs1,m_rhs); }
        int value2 () const { return m_fun->get_value(m_lhs2,m_rhs); }
    };
};
//function calling
inline int sparse_bin_fun::get_value (int lhs, int rhs) const
{
    Map::const_iterator val = m_map.find(Key(lhs,rhs));
    return val == m_map.end() ? 0 : val->second;
}

}

#endif

