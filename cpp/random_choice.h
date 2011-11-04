#ifndef JOHANN_RANDOM_CHOICE_H
#define JOHANN_RANDOM_CHOICE_H

#include "definitions.h"
#include "measures.h"
#include <cmath> //for rand
#include <cstdlib> //for abs
#include <set>

namespace RandomChoice
{//integer measure classes for random choice WRT a pmf

namespace M = Measures;

const Logging::Logger logger("random", Logging::INFO);

//constants for choice measures, using 64-bit integers
const Long MAX_TOTAL = Long(1)<<62; //safety factor of 4
const Long AVE_TOTAL = Long(1)<<61; //safety factor of 8
const Long MIN_TOTAL = Long(1)<<60; //safety factor of 16
typedef int64_t Signed;

//random numbers
inline bool random_bernoulli (Double p) { return drand48() < p; }
inline Long random_int (Long UB)
{//returns a (pseudo-)random integer in [ 0 , UB )
    Long result = lrand48();
    result <<= 32;  //anything between 24,...,48 works
    result ^= lrand48();
    result %= UB;
    Assert5(result < UB, "random_int out of bounds");
    return result;
}

//measure, for creation
class Generator
{
    Long m_total;
    Float m_factor; //retained for on-the-fly insertion calculation
    typedef const M::Measure<Float> FloatMeas;
    typedef const M::Measure<Long>  IntMeas;
    FloatMeas m_pmf_f; //Floating-point version
    IntMeas   m_pmf;   //integer measure
    IntMeas   m_pmf_l; //integer left-sum

    inline void _remove (Ob ob);
    inline void _insert (Ob ob);
    inline bool _check_total ();
private:
    void _update (Ob ob, Signed delta);
public:
    Generator (FloatMeas& pmf_f, IntMeas& pmf, IntMeas& pmf_l);

    //random choice
    bool empty () const { return m_total == 0; }
    void reset ();
    void update_all ();
    Ob choose ();

    //support control
    void remove (Ob ob)
    {
        _remove(ob); _check_total();
        Assert3(m_pmf(ob) == 0, "Gen: nonzero node mass after removal");
    }
    void insert (Ob ob)
    {
        Assert3(m_pmf(ob) == 0, "Gen: nonzero node mass before insertion");
        _insert(ob); _check_total();
    }
    void merge (Ob dep, Ob rep)
    {
        _remove(dep); _remove(rep); _insert(rep); _check_total();
        Assert3(m_pmf(dep) == 0, "Gen: nonzero node mass after merger");
    }
    void update (Ob ob)
    { _remove(ob); _insert(ob); _check_total(); }

    //misc
    void validate (IntMeas pmf_r);
    void log_histogram (Float thresh = 1e-3f);
};
inline void Generator::_remove (Ob ob)
{
    m_pmf_f(ob) = 0; //needed for update_all
    Long delta = -m_pmf(ob);
    _update(ob, delta);
}
inline void Generator::_insert (Ob ob)
{
    Float delta_f = m_factor * m_pmf_f(ob);
    if (delta_f > AVE_TOTAL) {
        logger.debug() << "very large value inserted; truncating" |0;
        delta_f = AVE_TOTAL;
    }
    Signed delta = static_cast<Signed>(delta_f);
    Assert3(delta >= 0,
            "Gen: item with negative mass inserted: delta = " << delta);
    Assert3(m_pmf(ob) == 0, "item had nonzero mass prior to insertion");
    if (delta) _update(ob, delta);
}
inline bool Generator::_check_total ()
{//renormalizes if necessary, returns whether total was renormalized
    if (m_total and ((m_total<MIN_TOTAL) or (MAX_TOTAL<m_total))) {
        //logger.debug() << "total is "
        //    << (100.0f * abs(m_total - AVE_TOTAL)) / AVE_TOTAL
        //    << "\% away from optimal; renormalizing" |0;
        update_all ();
        return true;
    }
    return false;
}

//small generator for little pmfs
class SmallGenerator
{
    Long m_total;
    std::vector<Long> m_pmf;
public:
    SmallGenerator (const std::vector<Float>& pmf_f);
    SmallGenerator (Float* pmf_f, Long N);
    Int choose () const;
};

}

#endif
