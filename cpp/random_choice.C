
#include "random_choice.h"
#include "nodes.h"
#include <iomanip>

namespace RandomChoice
{

//ob generator
Generator::Generator
    (FloatMeas& pmf_f, IntMeas& pmf, IntMeas& pmf_l)
    : m_total(0), m_factor(1.0),
      m_pmf_f(pmf_f), m_pmf(pmf), m_pmf_l(pmf_l)
{
    Assert1(sizeof(Long) == sizeof(Float),
            "IntMeas and FloatMeas have different sizes");
    Assert1(pmf_f.offset() != pmf.offset(),
            "invalid Generator: pmf_f and pmf must be distinct");
    Assert1(pmf_f.offset() != pmf_l.offset(),
            "invalid Generator: pmf_f and pmf_l must be distinct");
    Assert1(pmf.offset() != pmf_l.offset(),
            "invalid Generator: pmf and pmf_l must be distinct");
}
void Generator::reset ()
{
    m_total = 0;
    m_factor = 1.0;
    for (Ob::iterator iter=Ob::begin(); iter!=Ob::end(); ++iter) {
        Ob ob = *iter;
        m_pmf  [ob] = 0;
        m_pmf_l[ob] = 0;
    }
}
void Generator::update_all ()
{
    //find Floating-point total
    m_total = 0;
    Double total_f = 0.0;
    for (Ob::iterator iter=Ob::begin(); iter!=Ob::end(); ++iter) {
        Ob ob = *iter;
        m_pmf  [ob] = 0;
        m_pmf_l[ob] = 0;
        if (ob.isUsed()) {
            total_f += m_pmf_f(ob);
        }
    }

    //set scaling factor
    if (total_f > 0) {
        m_factor = static_cast<Double>(AVE_TOTAL) / total_f;
        logger.debug() << "renorm factor = " << m_factor |0;
    } else {
        Assert1(not (total_f<0), "generator had negative total in update_all");
        logger.warning() << "generator had no Float mass in update_all" |0;
        m_factor = 1.0;
        return;
    }

    //calculate masses and left-sums
    //  traverse in reverse order to compute left sums in linear time
    for (Ob::reverse_iterator iter=Ob::rbegin(); iter!=Ob::rend(); ++iter) {
        Ob ob = *iter;
        Long mass = 0;
        if (ob.isUsed()) {
            Assert3(m_pmf_f(ob) >= 0,
                    "negative mass encountered in update_all");
            mass = static_cast<Long>(m_factor * m_pmf_f(ob));
            m_total += mass;
            m_pmf[ob] = mass;
        }

        //calculate left sums
        mass += m_pmf_l[ob];   // may be nonzero even for unused obs
        while (ob and ob.isRightChild()) ob = ob.up();
        if (ob) {
            Assert2(ob.isLeftChild(), "ob is no child (in update_all)");
            m_pmf_l[ob.up()] += mass;
        }
    }
    AssertW(m_total>0, "generator had no integer mass in update_all");
}
Ob Generator::choose ()
{
    Assert (m_total>0, "generator cannot choose; nothing to choose from");
    Long value = 1 + random_int(m_total); //value in [1, m_total]
    Ob ob = Ob::root();
    descending: {
        Long Lsum = m_pmf_l[ob];
        if (value <= Lsum) { //value in [1, Lsum]
            Assert4(ob.hasLeftChild(),
                    "generator can't descend left, value = " << value);
            ob = ob.left();
            goto descending;
        }
        value -= Lsum;

        Long center = m_pmf[ob];
        if (value > center) {
            value -= center;
            //value in [1, ???]
            Assert4(ob.hasRightChild(),
                    "generator can't descend right, value = " << value);
            ob = ob.right();
            goto descending;
        }
        //value in [1, center], ob is chosen
    }
    Assert3(ob.isUsed(), "generator chose unused ob");
    Assert1(m_pmf[ob] > 0, "chose ob with zero mass");
    return ob;
}
void Generator::_update (Ob ob, Signed delta)
{//WARNING: this may use data in "unused" nodes, and thus interfere with node.reset()
    m_total += delta;
    m_pmf(ob) += delta;
    while (ob.hasParent()) {
        bool is_left_child = ob.isLeftChild();
        ob = ob.up();
        if (is_left_child) m_pmf_l[ob] += delta;
    }
}
void Generator::validate (IntMeas pmf_r)
{
    logger.debug() << "Validating Generator" |0;
    Logging::IndentBlock block;

    //validate total
    Long _total = 0;
    for (Ob::iterator iter=Ob::begin(); iter!=Ob::end(); ++iter) {
        Ob ob = *iter;
        _total += m_pmf[ob];
        pmf_r[ob] = 0;
    }
    Assert (_total == m_total, "invalid total");

    //calculate masses and right-sums
    for (Ob::reverse_iterator iter=Ob::rbegin(); iter!=Ob::rend(); ++iter) {
        Ob ob = *iter;
        Long mass = m_pmf[ob] + pmf_r[ob];
        while (ob and ob.isLeftChild()) ob = ob.up();
        if (!ob) continue;
        if (not ob.hasParent()) continue;
        Assert (ob.isRightChild(),
                "invalid: ob is neither left nor right child");
        pmf_r[ob.up()] += mass;
    }

    //validate root sums
    Ob root = Ob::root();
    Assert (m_pmf_l[root] + m_pmf[root] + pmf_r[root] == m_total,
            "invalid: calculated left & right sums do not match total");

    //validate local sums
    for  (Ob::iterator iter=Ob::begin(); iter!=Ob::end(); ++iter) {
        Ob ob = *iter;
        if (ob.hasParent()) {
            if (ob.isLeftChild()) {
                Assert (m_pmf_l[ob.up()] == m_pmf_l[ob]+m_pmf[ob]+pmf_r[ob],
                        "invalid left sum");
            } else {
                Assert (ob.isRightChild(),
                        "invalid: ob is neither left nor right child");
                Assert (pmf_r[ob.up()] == m_pmf_l[ob]+m_pmf[ob]+pmf_r[ob],
                        "invalid right sum");
            }
        }
        if (!ob.hasLeftChild()) {
            Assert (m_pmf_l[ob] == 0,
                    "invalid: ob had nonzero left sum but no left children");
        }
        if (!ob.hasRightChild()) {
            Assert (pmf_r[ob] == 0,
                    "invalid: ob had nonzero right sum but no right children");
        }
    }
}
void Generator::log_histogram (Float thresh)
{
    //calculate cutoff
    Float max_f = 0.0;
    for (Ob::sparse_iterator iter=Ob::sbegin(); iter!=Ob::send(); ++iter) {
        Ob ob = *iter;
        max_f = max(m_pmf_f(ob), max_f);
    }
    const Logging::fake_ostream& log = logger.info()
        << "Gen. histogram (tot = " << m_total
        << ", max = " << max_f
        << ", thresh = " << thresh << ")";
    Float cutoff = thresh * max_f;

    //collect values above cutoff
    typedef std::multiset<std::pair<Float,Long> > Values;
    Values values;
    for (Ob::sparse_iterator iter=Ob::sbegin(); iter!=Ob::send(); ++iter) {
        Ob ob = *iter;
        Float val_f = m_pmf_f(ob);
        if (val_f > cutoff) {
            Long val_i = m_pmf(ob);
            values.insert(std::make_pair(val_f, val_i));
        }
    }
    for (Values::reverse_iterator iter=values.rbegin();
            iter!=values.rend(); ++iter) {
        log << "\n\t" << std::setw(12) << iter->first
                      << std::setw(12) << iter->second;
    }
    log |0;
}

//small generator
SmallGenerator::SmallGenerator (const std::vector<Float>& pmf_f)
    : m_total(0),
      m_pmf(pmf_f.size())
{
    Long N = pmf_f.size();
    Float total = 0;
    for (Long i=0; i<N; ++i) {
        total += pmf_f[i];
    }
    Float factor = MAX_TOTAL / total;
    for (Long i=0; i<N; ++i) {
        Long mass = static_cast<Long>(factor * pmf_f[i]);
        m_pmf[i] = mass;
        m_total += mass;
    }
}
SmallGenerator::SmallGenerator (Float* pmf_f, Long N)
    : m_total(0),
      m_pmf(N)
{
    Float total = 0;
    for (Long i=0; i<N; ++i) {
        total += pmf_f[i];
    }
    Float factor = MAX_TOTAL / total;
    for (Long i=0; i<N; ++i) {
        Long mass = static_cast<Long>(factor * pmf_f[i]);
        m_pmf[i] = mass;
        m_total += mass;
    }
}
Int SmallGenerator::choose () const
{
    Long r = random_int(m_total);
    Int i=0;
    Long sum = m_pmf[0];
    while (sum <= r) {
        ++i;
        sum += m_pmf[i];
    }
    return i;
}

}

