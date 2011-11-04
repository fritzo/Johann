#ifndef JOHANN_MEASURES_H
#define JOHANN_MEASURES_H

#include "definitions.h"
#include "nodes.h"
#include "linalg.h"
#include <vector>
#include <utility>
#include <cmath>

//WARNING: this must match definition in optimization.h
using LinAlg::Vect;
using LinAlg::Mat;
using LinAlg::Nbhd1;
using LinAlg::VNbhd1;
using LinAlg::VNbhd2;

namespace Measures
{

const Logging::Logger logger("meas", Logging::DEBUG);

//================ approximating polynomial ================
class ObPolynomial
{
    typedef std::pair<ObHdl, Int> Pair;
    std::vector<Pair> m_obs;
    Int m_apps, m_comps, m_joins;
public:

    ObPolynomial () : m_obs(0), m_apps(0), m_comps(0) {}

    //construction
    void add_ob   (Ob ob); //increments norm
    void add_app  () { ++m_apps; }
    void add_comp () { ++m_comps; }
    void add_join () { ++m_joins; }
    void clear () { m_obs.clear(); m_apps = 0; m_comps = 0; m_joins = 0; }

    //component access
    Ob  ob    (Int i) const { return *(m_obs[i].first); }
    Int num   (Int i) const { return m_obs[i].second; }
    Int total () const;                     //number of obs
    Int apps  () const { return m_apps; }   //number of apps
    Int comps () const { return m_comps; }  //number of comps
    Int joins () const { return m_joins; }  //number of joins

    //list access
    Int size () const { return m_obs.size(); }
    const Pair& operator[] (Int i) const { return m_obs[i]; }

    void write_to (ostream& os) const;
};
inline ostream& operator<< (ostream& os, ObPolynomial poly)
{ poly.write_to(os); return os; }

//================ ob measure management ================

//a memory manager for measures
class Manager
{
    static Int s_static, s_size, s_obs; //invariant: size = 64<<n for some n
    static char* s_raw;                 //zero-based
    static void* s_data;                //one-based
    static std::vector<bool> s_used;
public:
    Manager () {} //instances only provide notation

    //sizing
    void initialize (Int static_size);
    void clear ();
    void resize (Int new_obs, const Int* new2old = NULL);
    void shrink ();
private:
    static void grow ();
    static void _resize_bytes (Int new_size);

    //usage
    static bool get_used (Int beg, Int end);
    static void set_used (Int beg, Int end, bool value);
    static Int num_used ();

    //allocation, only to be used by measure classes
    static Int _alloc (Int block, Int number);
    static void _free (Int block, Int offset, Int number);
public:
    template<class Mass> static Int _alloc (Int size=1)
    { return _alloc(sizeof(Mass), size); }
    template<class Mass> static void _free (Int offset, Int size=1)
    { _free(sizeof(Mass), offset, size); }

    //indexing
    static void* data (Ob ob)
    { return static_cast<char*>(s_data) + s_size * Int(ob); }
    template <class Mass>
    static Mass* data (Ob ob)
    { Assert5(ob.isUsed(), "accessed unused ob's measure");
        return static_cast<Mass*>(data(ob)); }
    template <class Mass>
    static Mass* DATA (Ob ob)
    { Assert5(ob.isValid(), "accessed invalid ob's measure");
        return static_cast<Mass*>(data(ob)); }

    //support operations
    // WARNING: these only work correctly for floats
    void create_ob (Ob ob) const;
    void merge_obs (Ob dep, Ob rep) const;
};

//================ dense measure classes ================

//measure class defined on positions
//note: measures are pointer-like, so const measures can still change value
template<class Mass>
class Measure
{
    typedef Measure<Mass> MyType;
    const Int m_offset;
public:
    Int offset () const { return m_offset; }

    //allocation
    Measure (Int offset) : m_offset(offset) {}
    static MyType alloc () { return MyType(Manager::_alloc<Mass>()); }
    void free () const { Manager::_free<Mass>(m_offset); }

    //equality testing, of depencence, not value; thus not publically available
    bool operator== (const Measure<Mass>& other) const
    { return m_offset == other.m_offset; }
    bool operator!= (const Measure<Mass>& other) const
    { return m_offset != other.m_offset; }

    //indexing: () for used, [] for unused
    Mass& data (Ob ob) const { return Manager::data<Mass>(ob)[m_offset]; }
    Mass& DATA (Ob ob) const { return Manager::DATA<Mass>(ob)[m_offset]; }
    Mass& operator() (Ob ob) const { return data(ob); }
    Mass& operator[] (Ob ob) const { return DATA(ob); }

    //operations
    void set (Mass m) const;
    Mass total () const; //total should start at 0
    const MyType& operator*= (Mass m) const;
    void ensure_normal () const;
    void ensure_nonneg () const;
};

//specialized at Float
//compare with LinAlg::Tensor class
class VectMeas
{
    typedef VectMeas MyType;
    const Int m_offset;
    const Int m_dim;  //e.g. dcomp is a |lang|-dimensional vector
    const Int m_rank; //e.g. the metric g is a rank-2 tensor
    const Int m_size; // = dim * rank
public:
    Int offset () const { return m_offset; }
    Int size   () const { return m_size; }
    Int dim    () const { return m_dim; }
    Int rank   () const { return m_rank; }

    //allocation
    VectMeas (Int offset, Int dim, Int rank=1)
        : m_offset(offset), m_dim(dim), m_rank(dim), m_size(powi(dim,rank)) {}
    static MyType alloc (Int dim, Int rank=1)
    { return MyType(Manager::_alloc<Float>(powi(dim,rank)), dim, rank); }
    void free () const { Manager::_free<Float>(m_offset, m_size); }

    //indexing: () for used, [] for unused
    Float* data (Ob ob) const { return Manager::data<Float>(ob) + m_offset; }
    Float* DATA (Ob ob) const { return Manager::DATA<Float>(ob) + m_offset; }
    Float* operator() (Ob ob) const { return data(ob); }
    Float* operator[] (Ob ob) const { return DATA(ob); }
    Measure<Float> operator[] (Int offset) const
    {//slicing
        Assert1(offset < m_size, "offset out of range");
        return Measure<Float>(m_offset + offset);
    }

    //operations
    void set (Float m) const;
    const MyType& operator*= (Float m) const;
    const MyType& operator*= (const Vect& m) const;
};

#define iter_over_Obs (Ob::iterator iter=Ob::begin(); iter!=Ob::end(); ++iter)
#define sparse_iter_over_Obs (Ob::sparse_iterator iter=Ob::sbegin(); iter!=Ob::send(); ++iter)
#define iter_over_Set (typename Set::iterator iter=set.begin(), end=set.end(); iter!=end; ++iter)

template<class Mass>
void Measure<Mass>::set (Mass m) const
{
    for sparse_iter_over_Obs { Ob ob = *iter;
        data(ob) = m;
    }
}
template<class Mass>
Mass Measure<Mass>::total () const
{
    Mass result(0);
    for sparse_iter_over_Obs { Ob ob = *iter;
        result += data(ob);
    }
    return result;
}
template<class Mass>
const Measure<Mass>& Measure<Mass>::operator*= (Mass m) const
{
    for sparse_iter_over_Obs { Ob ob = *iter;
        data(ob) *= m;
    }
    return *this;
}
template<class Mass>
void Measure<Mass>::ensure_normal () const
{
    for sparse_iter_over_Obs { Ob ob = *iter;
        Mass& m = data(ob);
        if (not std::isfinite(m)) m = 0;
    }
}
template<class Mass>
void Measure<Mass>::ensure_nonneg () const
{
    for sparse_iter_over_Obs { Ob ob = *iter;
        Mass& m = data(ob);
        if (m < 0) m = 0;
    }
}

//copying, sparse by default
template<class Mass>
void copy (const Measure<Mass>& src, Heap::TypedIndex<Mass> dest)
{
    for sparse_iter_over_Obs { Ob ob = *iter;
        ob(dest) = src(ob);
    }
}
template<class Mass>
void copy (Heap::TypedIndex<Mass> src, const Measure<Mass>& dest)
{
    for sparse_iter_over_Obs { Ob ob = *iter;
        dest(ob) = ob(src);
    }
}
template<class Mass>
void copy (const Measure<Mass>& src, const Measure<Mass>& dest)
{
    for sparse_iter_over_Obs { Ob ob = *iter;
        dest(ob) = src(ob);
    }
}
template<class Mass>
void dense_copy (const Measure<Mass>& src, const Measure<Mass>& dest)
{
    for iter_over_Obs { Ob ob = *iter;
        dest[ob] = src[ob];
    }
}

//interaction with sets
template<class Mass, class Set>
void unif_over_set (
        Measure<Mass> result,
        const Set set)
{
    //reset existing mass
    for sparse_iter_over_Obs { Ob ob = *iter;
        result(ob) = 0.0;
    }

    //add new mass
    const Mass element_mass = 1.0 / set.size();
    for iter_over_Set { Ob ob = *iter;
        result(ob) = element_mass;
    }
}
template<class Mass, class Set>
void shift_towards_set (
        Measure<Mass> initial,
        Measure<Mass> result,
        const Set& set, //a set of ObHandles
        const Mass t)
{
    //reduce existing mass
    const Mass one_minus_t = 1.0 - t;
    for sparse_iter_over_Obs { Ob ob = *iter;
        result(ob) = one_minus_t * initial(ob);
    }

    //add new mass
    const Mass element_mass = t / set.size();
    for iter_over_Set { Ob ob = **iter;
        result(ob) += element_mass;
    }
}

//misc operations
template<class Mass>
void calc_and (Measure<Mass> a, Measure<Mass> b, Measure<Mass> ab)
{
    for sparse_iter_over_Obs { Ob ob = *iter;
        ab(ob) = a(ob) * b(ob);
    }
}
template<class Mass>
void calc_ratio (Measure<Mass> a, Measure<Mass> b, Measure<Mass> ab)
{
    for sparse_iter_over_Obs { Ob ob = *iter;
        ab(ob) = a(ob) / b(ob);
    }
}

#undef iter_over_Obs
#undef sparse_iter_over_Obs
#undef iter_over_Set

}

typedef Measures::ObPolynomial ObPoly;
typedef Measures::Measure<Float> Meas;
using Measures::VectMeas;
typedef App::array<Float> AMeas;
typedef Comp::array<Float> CMeas;
typedef Join::array<Float> JMeas;

//static measures, see brain.C
namespace Measures { const Meas comp(0), komp(1), rel(2); }

#endif
