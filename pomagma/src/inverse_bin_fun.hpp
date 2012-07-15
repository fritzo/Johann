#ifndef POMAGMA_INVERSE_BIN_FUN_HPP
#define POMAGMA_INVERSE_BIN_FUN_HPP

#include "util.hpp"
#include "dense_bin_fun.hpp"
#include <vector>
#include <utility>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_unordered_set.h>

namespace pomagma
{

typedef std::vector<
    tbb::concurrent_unordered_set<std::pair<oid_t, oid_t>>> Vxx_Data;

typedef tbb::concurrent_unordered_map<
    std::pair<oid_t, oid_t>,
    tbb::concurrent_unordered_set<oid_t>> VXx_Data;

inline void unsafe_erase (VXx_Data & VXx_data, oid_t V, oid_t X, oid_t x)
{
    auto i = VXx_data.find(std::make_pair(V, X));
    POMAGMA_ASSERT1(i != VXx_data.end(),
            "double erase: " << V << "," << X << "," << x);
    i->second.unsafe_erase(x);
    if (i->second.empty()) {
        VXx_data.unsafe_erase(i);
    }
}

class inverse_bin_fun : noncopyable
{
    const size_t m_item_dim;

    Vxx_Data m_Vlr_data;
    VXx_Data m_VLr_data;
    VXx_Data m_VRl_data;

public:

    inverse_bin_fun (size_t item_dim)
        : m_item_dim(item_dim),
          m_Vlr_data(m_item_dim),
          m_VLr_data(),
          m_VRl_data()
    {
    }

    ~inverse_bin_fun ()
    {
    }

    void insert (oid_t lhs, oid_t rhs, oid_t val);
    void unsafe_remove (oid_t lhs, oid_t rhs, oid_t val);
    void validate (dense_bin_fun & fun);

    class Vlr_Iterator;

    enum { LHS_FIXED = false, RHS_FIXED = true };
    template<bool idx> class VXx_Iterator;
};

inline void inverse_bin_fun::insert (oid_t lhs, oid_t rhs, oid_t val)
{
    m_Vlr_data[val].insert(std::make_pair(lhs, rhs));
    m_VLr_data[std::make_pair(val, lhs)].insert(rhs);
    m_VRl_data[std::make_pair(val, rhs)].insert(lhs);
}

inline void inverse_bin_fun::unsafe_remove (oid_t lhs, oid_t rhs, oid_t val)
{
    m_Vlr_data[val].unsafe_erase(std::make_pair(lhs, rhs));
    unsafe_erase(m_VLr_data, val, lhs, rhs);
    unsafe_erase(m_VRl_data, val, rhs, lhs);
}

//----------------------------------------------------------------------------
// Iteration

class inverse_bin_fun::Vlr_Iterator : noncopyable
{
    inverse_bin_fun & m_fun;
    tbb::concurrent_unordered_set<std::pair<oid_t, oid_t>>::iterator m_iter;
    tbb::concurrent_unordered_set<std::pair<oid_t, oid_t>>::iterator m_end;
    oid_t m_val;

public:

    // construction
    Vlr_Iterator (inverse_bin_fun & fun)
        : m_fun(fun),
          m_val(0)
          // XXX FIXME is it ok to default-construct m_iter, m_end?
    {
    }
    Vlr_Iterator (inverse_bin_fun & fun, oid_t val)
        : m_fun(fun),
          m_val(val)
    {
        begin();
    }

    // traversal
    void begin ()
    {
        auto i = m_fun.m_Vlr_data[m_val];
        m_iter = i.begin();
        m_end = i.end();
    }
    void begin (oid_t val) { m_val = val; begin(); }
    bool ok () const { return m_iter != m_end; }
    void next () { ++m_iter; }

    // dereferencing
private:
    void _deref_assert () const
    {
        POMAGMA_ASSERT5(m_val and ok(),
                "dereferenced done inverse_bin_fun::Iterator");
    }
public:
    oid_t value () const { _deref_assert(); return m_val; }
    oid_t lhs () const { _deref_assert(); return m_iter->first; }
    oid_t rhs () const { _deref_assert(); return m_iter->second; }
};

//----------------------------------------------------------------------------
// Range iteration

template<bool idx>
class inverse_bin_fun::VXx_Iterator : noncopyable
{
    inverse_bin_fun & m_fun;
    tbb::concurrent_unordered_set<oid_t>::iterator m_iter;
    tbb::concurrent_unordered_set<oid_t>::iterator m_end;
    std::pair<oid_t, oid_t> m_pair;

public:

    // construction
    VXx_Iterator (inverse_bin_fun & fun)
        : m_fun(fun),
          m_pair(0,0)
    {
    }
    VXx_Iterator (inverse_bin_fun & fun, oid_t val, oid_t fixed)
        : m_fun(fun),
          m_pair(val, fixed)
    {
        begin();
    }

    // traversal
    void begin ()
    {
        auto & map = idx ? m_fun.m_VRl_data : m_fun.m_VLr_data;
        auto i = map.find(m_pair);
        if (i != map.end()) {
            m_iter = i->second.begin();
            m_end = i->second.end();
        } else {
            m_iter = m_end;
        }
    }
    void begin (oid_t val, oid_t fixed)
    {
        m_pair = std::make_pair(val, fixed);
        begin();
    }
    bool ok () const { return m_iter != m_end; }
    void next  () { ++m_iter; }

    // dereferencing
private:
    void _deref_assert () const
    {
        POMAGMA_ASSERT5(m_pair.first and  m_pair.second and ok(),
                "dereferenced done inverse_bin_fun::RangeIterator");
    }
public:
    oid_t value () const { _deref_assert(); return m_pair.first; }
    oid_t fixed () const { _deref_assert(); return m_pair.second; }
    oid_t moving () const { _deref_assert(); return *m_iter; }
    oid_t lhs () const { return idx ? moving() : fixed(); }
    oid_t rhs () const { return idx ? fixed() : moving(); }
};

} // namespace pomagma

#endif // POMAGMA_INVERSE_BIN_FUN_HPP
