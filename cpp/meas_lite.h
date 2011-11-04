#ifndef JOHANN_MEASURES_H
#define JOHANN_MEASURES_H

#include "definitions.h"
#include <cstring>
#include <cmath>
#include <map>
#include <vector>
#include <utility>

#define EXPR_DEBUG(mess) {logger.debug() << mess |0;}
//#define EXPR_DEBUG(mess)

/** Lite Measure Tools.
 *
 * Language.
 *
 * <pre>
 *   S x,y,z;       atomic sparse measures
 *   D x,y,z;       atomic dense measures
 *   dense(-)       convert to a dense measure
 *
 *   x.clear();     reset to zero
 *   x + y          add measures pointwise
 *   x * y          multiply measures pointwise
 *   t * x          scale by a number
 *
 *   prior()        the syntactic probability measure
 *   normal(-)      normalize a measure
 *   normal(-,t)    normalize a measure to total t
 *
 *   x = fix(-);    compute a fixedpoint of an expression
 *   x = fix(-,t);  compute a fixedpoint to precision t
 *
 *   app(-,-)       function application
 *   comp(-,-)      function composition
 *   join(-,-)      function join
 *   ldiv(-,-,-)    left-inverse application
 *   rdiv(-,-,-)    right-inverse application
 * </pre>
 *
 * TODO: add inverse composition
 * TODO: add information ordering
 *
 * TODO: add parsing, in and out
 * TODO: add dense and sparse methods .save(string filename)
 *   !think about
 *       S @ 0.3,
 *       K @ 0.2,
 *       ...
 *       S S(S S S) @ 0.00001 .
 * TODO: add method threshhold : Dense x Float --> Sparse
 */
namespace MeasLite
{

namespace { const Logging::Logger logger("meas-lt", Logging::DEBUG); }

typedef Short Ob;
struct AEqn { Ob a,l,r; };
struct CEqn { Ob c,l,r; };
struct JEqn { Ob j,l,r; };

//structure
extern Int o_size, a_size, c_size, j_size;
extern const AEqn *apps,  *a_begin, *a_end;
extern const CEqn *comps, *c_begin, *c_end;
extern const JEqn *joins, *j_begin, *j_end;
void load (string filename);
void clear ();

//parsing + printing
Ob parse (std::string expr);
std::string print (Ob ob);

//======== sparse measures ========

struct Sparse
{
    typedef std::map<Int,Float> Map;
    typedef Map::const_iterator CIter;

    Map data;
    string name;
private:
    //reference counting
    friend class S;
    unsigned m_ref_count;
    void operator++ () { ++m_ref_count; }
    void operator-- () { if (not --m_ref_count) delete this; }

    Sparse (const Sparse& other) { Error("copy-constructed a Sparse"); }
public:
    Sparse (string n) : name(n) {}
    void clear () { data.clear(); }

    typedef Map::iterator Iter;
    Iter begin () { return data.begin(); }
    Iter end () { return data.end(); }

    //element access, for templates
    Float operator[] (Int o)
    {
        CIter i=data.find(o);
        return i==data.end() ? 0 : i->second;
    }

    void validate ();
};

//======== dense measures ========

class D;
struct Dense
{
    Float*const data;
    std::string name;
private:
    //reference counting
    friend class D;
    unsigned m_ref_count;
    void operator++ () { ++m_ref_count; }
    void operator-- () { if (not --m_ref_count) delete this; }

    static unsigned s_num_instances;
    Dense (const Dense& other):data(NULL) { Error("copy-constructed a Dense"); }
public:
    static unsigned num_instances () { return s_num_instances; }
    Dense (string n)
        : data(new(std::nothrow) Float[1+o_size]), name(n), m_ref_count(0)
    {
        Assert (data, "failed to allocate vector of size " << o_size);
        logger.debug() << "creating Dense" |0;
        ++s_num_instances;
    }
private:
    ~Dense ()
    {
        logger.debug() << "deleting Dense" |0;
        delete[] data;
        --s_num_instances;
    }
public:
    void clear ()
    {
        EXPR_DEBUG( name << " = 0" )
        bzero(data, sizeof(Float) * (1+o_size));
    }
};

//======== expression templates ========

//atomic measures
class S
{
    Sparse& m;
public:
    S (const S& s)  : m(s.m)                 { ++m; }
    S (Sparse&_m) : m(_m)                    { ++m; }
    S (string name) : m(*(new Sparse(name))) { ++m; }
    ~S ()                                    { --m; }
    void clear () { m.clear(); }
    const std::string& name () const { return m.name; }

    //element access
    Sparse::Map& data () { return m.data; }
    Float& data (Int o) { return m.data[o]; }
    Float operator[] (Int o) { return m[o]; }
    void eval () {}

    //iteration
    typedef Sparse::Iter Iter;
    Iter begin () { return m.begin(); }
    Iter end () { return m.end(); }

    //assignment
    inline void operator= (const S& s);
    template<class E> inline void operator=  (E e);
    template<class E> inline void operator+= (E e);
    template<class E> inline void operator*= (E e);

    //properties
    Float total ();
    Float entropy ();
};
class D
{
    Dense& m;
public:
    D (const D& d)  : m(d.m)                { ++m; }
    D (Dense& _m)   : m(_m)                 { ++m; }
    D (string name) : m(*(new Dense(name))) { ++m; }
    ~D ()                                   { --m; }
    void clear () { m.clear(); }
    const std::string& name () const { return m.name; }

    //element access
    Float* data () { return m.data; }
    Float& data (Int o)
    {
        Assert3(1 <= o and o <= o_size, "index out of range: " << o);
        return m.data[o];
    }
    Float operator[] (Int o)
    {
        Assert3(1 <= o and o <= o_size, "index out of range: " << o);
        return m.data[o];
    }
    void eval () {}

    //assignment
    inline void operator= (const D& d);
    template<class E> inline void operator=  (E e);
    template<class E> inline void operator+= (E e);
    template<class E> inline void operator*= (E e);

    //properties
    Float total ();
    Float entropy ();
};

//convert between sparse <--> dense measure
template<class E> inline D dense (E e) { D d("dense"); set(d,e); return d; }
template<> inline D dense (D d) { return d; }
template<class E> inline S sparse (E e, Float thresh);
template<> inline S sparse (S s, Float thresh);

//special measures
extern D* g_prior;
inline D& prior () { return *g_prior; }
extern D* g_parse_size;
inline D& parse_size () { return *g_parse_size; }

//scalar multiplication
template<class E> struct scale
{
    Float t; E e;
    scale (Float _t, E _e) : t(_t), e(_e) {}
    void eval () { e.eval(); }
    Float operator[] (Int o) { return t * e[o]; }
};

//pointwise addition
template<class E1, class E2> struct sum
{
    E1 e1; E2 e2;
    sum (E1 _e1, E2 _e2) : e1(_e1), e2(_e2) {}
    void eval () { e1.eval(); e2.eval(); }
    Float operator[] (Int o) { return e1[o] + e2[o]; }
};

//pointwise multiplication
template<class E1, class E2> struct prod
{
    E1 e1; E2 e2;
    prod (E1 _e1, E2 _e2) : e1(_e1), e2(_e2) {}
    void eval () { e1.eval(); e2.eval(); }
    Float operator[] (Int o) { return e1[o] * e2[o]; }
};

//function application
template<class E1,class E2> struct App
{
    E1 l; E2 r; D* a;
    App (E1 _l, E2 _r) : l(_l), r(_r), a(NULL) {}
    ~App () { if (a) delete a; }
    void eval ();
    Float operator[] (Int o) { return a->operator[](o); }
};

//function composition
template<class E1,class E2> struct Comp
{
    E1 l; E2 r; D* c;
    Comp (E1 _l, E2 _r) : l(_l), r(_r), c(NULL) {}
    ~Comp () { if (c) delete c; }
    void eval ();
    Float operator[] (Int o) { return c->operator[](o); }
};

//function join
template<class E1,class E2> struct Join
{
    E1 l; E2 r; D* j;
    Join (E1 _l, E2 _r) : l(_l), r(_r), j(NULL) {}
    ~Join () { if (j) delete j; }
    void eval ();
    Float operator[] (Int o) { return j->operator[](o); }
};

//Floating point ops
template<class E> struct pow
{
    E e; Float t;
    pow (E _e, Float _t) : e(_e), t(_t) {}
    void eval () { e.eval(); }
    Float operator[] (Int o) { return powf(e[o],t); }
};
template<class E> struct call
{
    Float (*f)(Float); E e;
    call (Float (*_f)(Float), E _e) : f(_f), e(_e) {}
    void eval () { e.eval(); }
    Float operator[] (Int o) { return f(e[o]); }
};
template<class E> inline call<E> sqrt (E e) { return call<E>(::sqrt,e); }
template<class E> inline call<E> log  (E e) { return call<E>(::log, e); }
template<class E> inline call<E> exp  (E e) { return call<E>(::exp, e); }

//random normal likelihood
Float random_normal ();
struct random
{
    Float sigma;
    random (Float s=1) : sigma(s) {}
    void eval () {}
    Float operator[] (Int) const { return sigma * random_normal(); }
};

//======== evaluated expressions ========

//WARNING: must match defs in .C file
#define for_o for (Int o=1; o<=o_size; ++o)
#define for_a for (const AEqn *e=a_begin, *end=a_end; e!=end; ++e)
#define for_c for (const CEqn *e=c_begin, *end=c_end; e!=end; ++e)
#define for_j for (const JEqn *e=j_begin, *end=j_end; e!=end; ++e)
#define for_i(s) for (S::Iter i=s.begin(), end=s.end(); i!=end; ++i)

//normalization
template<class E> struct Normal
{
    E e;
    Float scale,tot;
    Normal(E _e, Float _t=1.0f) : e(_e), scale(0), tot(_t) {}
    void eval () { e.eval(); scale=0; for_o scale+=e[o]; scale = tot/scale; }
    Float operator[] (Int o) { return scale * e[o]; }
};
template<class E> inline Normal<E> normal (E e, Float t=1.0f) { return Normal<E>(e,t); }


//fixedpoint
template<class E> struct Fix
{
    E e; Float tol;
    Fix (E _e, Float t) : e(_e), tol(t) {}
};
template<class E> inline Fix<E> fix(E e, Float t=1e-6) { return Fix<E>(e,t); }

/* LATER
template<class Ea,class El,class Er> struct Ldiv
{
    Ea a; El l; Er r;
    Ldiv (Ea _a, El _l, Er _r) : a(_a), l(_l), r(_r) {}
    void eval () { a.eval(); l.eval(); r.eval(); }
};
template<class Ea,class El> inline Ldiv<Ea,El,D>
    ldiv (El l, Ea a) { return Ldiv<Ea,El,D>(a,l,prior()); }
template<class Ea,class El,class Er> inline Ldiv<Ea,El,Er>
    ldiv (El l, Ea a, Er r) { return Ldiv<Ea,El,Er>(a,l,r); }

template<class Ea,class El,class Er> struct Rdiv
{
    Ea a; El l; Er r;
    Rdiv (Ea _a, El _l, Er _r) : a(_a), l(_l), r(_r) {}
    void eval () { a.eval(); l.eval(); r.eval(); }
};
template<class Ea,class Er> inline Rdiv<Ea,D,Er>
    rdiv (Ea a, Er r) { return Rdiv<Ea,D,Er>(a,prior(),r); }
template<class Ea,class El,class Er> inline Rdiv<Ea,El,Er>
    rdiv (Ea a, Er r, El l) { return Rdiv<Ea,El,Er>(a,l,r); }
*/

//======== implementations ========

//diagnostics
template<class O>                   inline O& operator<< (O&o, Sparse& s)       { return o << s.name;}
template<class O>                   inline O& operator<< (O&o, Dense& d)        { return o << d.name;}
template<class O>                   inline O& operator<< (O&o, S& s)            { return o << s.name();}
template<class O>                   inline O& operator<< (O&o, D& d)            { return o << d.name();}
template<class O,class E>           inline O& operator<< (O&o, Normal<E>& n)    { return o << "normal(" << n.e << ", " << n.tot << ')'; }
template<class O,class E>           inline O& operator<< (O&o, scale<E>& s)     { return o << "scale(" << s.t << ", " << s.e << ')'; }
template<class O,class E1,class E2> inline O& operator<< (O&o, prod<E1,E2>& p)  { return o << "prod(" << p.e1 << ", " << p.e2 << ')'; }
template<class O,class E1,class E2> inline O& operator<< (O&o, sum<E1,E2>& s)   { return o << "sum(" << s.e1 << ", " << s.e2 << ')'; }
template<class O,class E1,class E2> inline O& operator<< (O&o, App<E1,E2>& lr)  { return o << "app(" << lr.l << ", " << lr.r << ')'; }
template<class O,class E1,class E2> inline O& operator<< (O&o, Comp<E1,E2>& lr) { return o << "comp(" << lr.l << ", " << lr.r << ')'; }
template<class O,class E1,class E2> inline O& operator<< (O&o, Join<E1,E2>& lr) { return o << "join(" << lr.l << ", " << lr.r << ')'; }
template<class O,class E>           inline O& operator<< (O&o, pow<E>& p)       { return o << "pow(" << p.e << ", " << p.t << ')'; }
template<class O,class E>           inline O& operator<< (O&o, call<E>& c)      { return o << "call(..., " << c.e << ')'; }
template<class O,class E>           inline O& operator<< (O&o, Fix<E>& f)       { return o << "fix(" << f.e << ')'; }
template<class O>                   inline O& operator<< (O&o, random& r)       { return o << "random(" << r.sigma << ')'; }
/* LATER
template<class O,class Ea,class El,class Er> inline O& operator<< (O&o, Ldiv<Ea,El,Er>& la) { return o << "ldiv(" << la.l << ", " << la.a << ')'; }
template<class O,class Ea,class El,class Er> inline O& operator<< (O&o, Rdiv<Ea,El,Er>& ar) { return o << "rdiv(" << ar.a << ", " << ar.r << ')'; }
*/

//threshholding
template<class E> inline S sparse (E e, Float thresh)
{
    D d = dense(e);
    S result("thresh'd");
    for_o {
        Float d_o = d[o];
        if (d_o > thresh) result.data(o) = d_o;
    }
    return result;
}
template<> inline S sparse (S s, Float thresh)
{
    S result("thresh'd");
    for_i(s) {
        Int o = i->first;
        Float s_o = i->second;
        if (s_o > thresh) result.data(o) = s_o;
    }
    return result;
}

//Float tools
inline Float safe_inv (Float q) { return q > 0 ? 1/q : 0; }
inline Float safe_div (Float p, Float q) { return q > 0 ? p/q : 0; }

//assignment -- where all the computation happens
inline void set (S dest, S src)
{
    EXPR_DEBUG( "S " << dest << " = S " << src )
    dest.data() = src.data();
}
template<class E> inline void set (D dest, E src)
{
    EXPR_DEBUG( "D " << dest << " = " << src )
    src.eval();
    for_o dest.data(o) = src[o];
}
template<> void inline set (D dest, S src)
{
    EXPR_DEBUG( "D " << dest << " = S " << src )
    dest.clear();
    for_i(src) dest.data(i->first) = i->second;
}
template<> inline void set (D dest, D src)
{
    EXPR_DEBUG( "D " << dest << " = D " << src )
    memcpy(dest.data(), src.data(), sizeof(Float) * (1+o_size));
}

//logical operations
template<class E> inline void
    iadd (D d, E e) { e.eval(); for_o d.data(o) += e[o]; }
template<class E> inline void
    imul (D d, E e) { e.eval(); for_o d.data(o) *= e[o]; }
inline void imul (D d, Float t) { for_o d.data(o) *= t; }
inline void imul (S s, Float t) { for_i(s) i->second *= t; }

//functional operations
template<class E1,class E2> void App<E1,E2>::eval ()
{
    if (!a) a = new D("a-temp");
    a->clear();
    l.eval(); r.eval();
    for_a a->data(e->a) += l[e->l] * r[e->r];
}
template<class E1,class E2> void Comp<E1,E2>::eval ()
{
    if (!c) c = new D("c-temp");
    c->clear();
    l.eval(); r.eval();
    for_c c->data(e->c) += l[e->l] * r[e->r];
}
template<class E1,class E2> void Join<E1,E2>::eval ()
{
    if (!j) j = new D("j-temp");
    j->clear();
    l.eval(); r.eval();
    for_j j->data(e->j) += l[e->l] * r[e->r];
}
/* LATER
template<class E1,class E2> inline void iadd (D _r, Ldiv<E1,E2> la)
{
    EXPR_DEBUG( _r << " += " << la.l << " \\ " << la.a )
    la.eval();
    D _a = dense(la.a);     Float *restrict a = _a.data();
    D _l = dense(la.l);     Float *restrict l = _l.data();
                            Float *restrict r = _r.data();
    D _p = prior();         Float *restrict p = _p.data();
    D _part("part");        Float *restrict part = _part.data();

    //find comp part
    part.clear();
    for_a part[e->a] += p[e->r] * l[e->l];
    for_o part[o] = safe_div(a[o], part[o]);

    //propagate
    for_a r[e->r] += p[e->r] * l[e->l] * part[e->a];
}
template<class E1,class E2> inline void iadd (D _l, Rdiv<E1,E2> ar)
{
    EXPR_DEBUG( _l << " += " << ar.a << " / " << ar.r )
    ar.eval();
    D _a = dense(ar.a);     Float *restrict a = _a.data();
                            Float *restrict l = _l.data();
    D _r = dense(ar.r);     Float *restrict r = _r.data();
    D _p = prior();         Float *restrict p = _p.data();
    D _part("part");        Float *restrict part = _part.data();

    //find comp part
    part.clear();
    for_a part[e->a] += r[e->r] * p[e->l];
    for_o part[o] = safe_div(a[o], part[o]);

    //propagate
    for_a l[e->l] += r[e->r] * p[e->l] * part[e->a];
}
template<class E1,class E2> inline void
    set (D _r, Ldiv<E1,E2> la) { _r.clear(); iadd(_r,la); }
template<class E1,class E2> inline void
    set (D _l, Rdiv<E1,E2> ar) { _l.clear(); iadd(_l,ar); }
*/

//fixedpoints
template<class E> void set (D d, Fix<E> f)
{
    logger.debug() << "Computing fixed point." |0;
    Logging::IndentBlock block;

    EXPR_DEBUG( "D " << d << " = fix(" << f.e << ')' )
    for (double stepsize=INFINITY; not (stepsize < f.tol);) {
        stepsize = 0;
        for_o {
            f.e.eval();
            Float& old_o = d.data(o);
            Float new_o = f.e[o];
            stepsize += fabs(new_o - old_o);
            //make sure to converge on whether e[o] is nonzero
            if (new_o * old_o == 0 and new_o + old_o > 0) stepsize = INFINITY;
            old_o = new_o;
        }
        logger.debug() << "stepsize = " << stepsize |0;
    }
}

#undef for_o
#undef for_a
#undef for_c
#undef for_j
#undef for_i

//operators
template<class E>                       inline scale<E>             operator* (Float t, E e)            { return scale<E>(t,e); }

template<class E>                       inline sum<D,E>             operator+ (D d, E e)                { return sum<D,E>(d,e); }
template<class E1,class E2>             inline sum<scale<E1>,E2>    operator+ (scale<E1> e1, E2 e2)     { return sum<scale<E1>,E2>(e1,e2); }
template<class E1,class E2,class E3>    inline sum<prod<E1,E2>,E3>  operator+ (prod<E1,E2> s, E3 e)     { return sum<prod<E1,E2>,E3>(s,e); }
template<class E1,class E2,class E3>    inline sum<App<E1,E2>,E3>   operator+ (App<E1,E2> s, E3 e)      { return sum<App<E1,E2>,E3>(s,e); }
template<class E1,class E2,class E3>    inline sum<Comp<E1,E2>,E3>  operator+ (Comp<E1,E2> s, E3 e)     { return sum<Comp<E1,E2>,E3>(s,e); }
template<class E1,class E2,class E3>    inline sum<Join<E1,E2>,E3>  operator+ (Join<E1,E2> s, E3 e)     { return sum<Join<E1,E2>,E3>(s,e); }
template<class E1,class E2,class E3>    inline sum<sum<E1,E2>,E3>   operator+ (sum<E1,E2> s, E3 e)      { return sum<sum<E1,E2>,E3>(s,e); }
template<class E1,class E2>             inline sum<pow<E1>,E2>      operator+ (pow<E1> e1, E2 e2)       { return sum<pow<E1>,E2>(e1,e2); }
template<class E1,class E2>             inline sum<call<E1>,E2>     operator+ (call<E1> e1, E2 e2)      { return sum<call<E1>,E2>(e1,e2); }
template<class E1,class E2>             inline sum<Normal<E1>,E2>   operator+ (Normal<E1> n, E2 e)      { return sum<Normal<E1>,E2>(n,e); }
template<class E>                       inline sum<random,E>        operator+ (random r, E e)           { return sum<random,E>(r,e); }

template<class E>                       inline prod<D,E>            operator* (D d, E e)                { return prod<D,E>(d,e); }
template<class E1,class E2>             inline prod<scale<E1>,E2>   operator* (scale<E1> e1, E2 e2)     { return prod<scale<E1>,E2>(e1,e2); }
template<class E1,class E2,class E3>    inline prod<prod<E1,E2>,E3> operator* (prod<E1,E2> s, E3 e)     { return prod<prod<E1,E2>,E3>(s,e); }
template<class E1,class E2,class E3>    inline prod<sum<E1,E2>,E3>  operator* (sum<E1,E2> s, E3 e)      { return prod<sum<E1,E2>,E3>(s,e); }
template<class E1,class E2,class E3>    inline prod<App<E1,E2>,E3>  operator* (App<E1,E2> s, E3 e)      { return prod<App<E1,E2>,E3>(s,e); }
template<class E1,class E2,class E3>    inline prod<Comp<E1,E2>,E3> operator* (Comp<E1,E2> s, E3 e)     { return prod<Comp<E1,E2>,E3>(s,e); }
template<class E1,class E2,class E3>    inline prod<Join<E1,E2>,E3> operator* (Join<E1,E2> s, E3 e)     { return prod<Join<E1,E2>,E3>(s,e); }
template<class E1,class E2>             inline prod<pow<E1>,E2>     operator* (pow<E1> e1, E2 e2)       { return prod<pow<E1>,E2>(e1,e2); }
template<class E1,class E2>             inline prod<call<E1>,E2>    operator* (call<E1> e1, E2 e2)      { return prod<call<E1>,E2>(e1,e2); }
template<class E1,class E2>             inline prod<Normal<E1>,E2>  operator* (Normal<E1> n, E2 e)      { return prod<Normal<E1>,E2>(n,e); }
template<class E>                       inline prod<random,E>       operator* (random r, E e)           { return prod<random,E>(r,e); }

template<class E1, class E2>            inline App<E1,E2>           app       (E1 l, E2 r)              { return App<E1,E2>(l,r); }
template<class E1, class E2>            inline Comp<E1,E2>          comp      (E1 l, E2 r)              { return Comp<E1,E2>(l,r); }
template<class E1, class E2>            inline Join<E1,E2>          join      (E1 l, E2 r)              { return Join<E1,E2>(l,r); }

//delayed inlines
inline void S::operator= (const S& s) { set(*this,s); }
template<class E> inline void S::operator=  (E e) { set(*this,e); }
template<class E> inline void S::operator+= (E e) { iadd(*this,e); }
template<class E> inline void S::operator*= (E e) { imul(*this,e); }

inline void D::operator= (const D& d) { set(*this,d); }
template<class E> inline void D::operator=  (E e) { set(*this,e); }
template<class E> inline void D::operator+= (E e) { iadd(*this,e); }
template<class E> inline void D::operator*= (E e) { imul(*this,e); }

}

#endif
