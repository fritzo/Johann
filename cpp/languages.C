
#include "languages.h"
#include "files.h"

namespace Languages
{

//language tools
/** Canonical parametrization in the simplex. TODO update documentation for join
 *
 *   (L,P_app) = (L,a) |--> t = theta : [0,1]^|B|
 *
 * Converting (L,a) to t:
 *   L[i] * (1-a)  =  t
 *
 * Converting t to (L,a):
 *                                         t[i]
 *   a   = 1 - Sum i. t[i]        L[i]  =  -----
 *                                         1 - a
 *  da                           dL[i]     delta(i,j)     t[i] dt[j]
 * ----- = -1                    -----  =  ----------  -  ----------
 * dt[j]                         dt[j]       1 - a        (1 - a)^2
 *
 * Entropy and its derivative:
 *     H   = Sum i. -L[i] log L[i]
 *    dH
 *   ----- = - 1 - log L[i]
 *   dL[i]
 *    dH     H - log L[i]
 *   ----- = ------------
 *   dl[i]      1 - a
 */
void Langauge::set_P_app (Float P_app)
{//safe modifier
    Float UB = 1.0f - m_P_comp - m_P_join;
    if (P_app < 0)  { m_P_app = 0.0f; return; }
    if (P_app > UB) { m_P_app = UB; return; }
    m_P_app = P_app;
}
void Langauge::set_P_comp (Float P_comp)
{//safe modifier
    Float UB = 1.0f - m_P_app - m_P_join;
    if (P_comp < 0)  { m_P_comp = 0.0f; return; }
    if (P_comp > UB) { m_P_comp = UB; return; }
    m_P_comp = P_comp;
}
void Langauge::set_P_join (Float P_join)
{//safe modifier
    Float UB = 1.0f - m_P_app - m_P_comp;
    if (P_join < 0)  { m_P_join = 0.0f; return; }
    if (P_join > UB) { m_P_join = UB; return; }
    m_P_join = P_join;
}
Vect Langauge::to_vect () const
{
    Vect v(vect_size());

    v(i_app)  = m_P_app;
    v(i_comp) = m_P_comp;
    v(i_join) = m_P_join;

    for (Int a=0; a<size(); ++a) {
        v(i_basis+a) = mass(a) * P_basis();
    }

    return v;
}
void Langauge::from_vect (const Vect& v)
{
    Assert(v.size() == vect_size(), "Lang::from_vect got vect of wrong size");

    m_P_app  = v(i_app);
    m_P_comp = v(i_comp);
    m_P_join = v(i_join);
    m_P_basis = calc_P_basis();

    for (Int a=0; a<size(); ++a) {
        mass(a) = v(i_basis+a) / P_basis();
    }
    update();
}
inline Float h_term   (Float p) { return (p>0) ? -p * logf(p) : 0.0; }
inline Float dh_term  (Float p) { return (p>0) ? -1.0 - logf(p) : INFINITY; }
inline Float ddh_term (Float p) { return (p>0) ? -1.0 / p : -INFINITY; }
inline Float g_term   (Float p) { return (p>0) ? -1.0 / p : 0.0; }
inline void iadd_h (Float& h, Float p)
{
    h += h_term(p);
}
inline void iadd_h (Float& h, Float& dh, Float p, Float dp)
{
     h += h_term(p);
    dh += dh_term(p) * dp;
}
inline void iadd_h (Float& h, Vect& dh, Float p, Int i)
{
     h    +=  h_term(p);
    dh(i) += dh_term(p);
}
inline void iadd_h (Float& h, Vect& dh, Mat& ddh, Float p, Int i)
{
      h      +=   h_term(p);
     dh(i)   +=  dh_term(p);
    ddh(i,i) += ddh_term(p);
}
inline void iadd_g (Mat& g, Float p, Int i)
{
    g(i,i) += g_term(p);
}
Float Langauge::calc_H () const
{//entropy of algebraic signature
    //app, comp, and join parts
    Float H = h_term(m_P_app)
            + h_term(m_P_comp)
            + h_term(m_P_join);

    //basis part
    for (Int a=0; a<size(); ++a) {
        H += h_term(prob(a));
    }

    return H;
}
Nbhd1 Langauge::calc_dH (const Vect& dp) const
{//directional derivative WRT simplicial coords
    Assert1(dp.size() == size(), "derivative in improper direction");

    Float  H = 0;
    Float dH = 0;

    //app, comp, and join parts
    iadd_h( H,dH,   m_P_app,dp(i_app) );
    iadd_h( H,dH,  m_P_comp,dp(i_comp) );
    iadd_h( H,dH,  m_P_join,dp(i_join) );

    //basis part
    for (Int a=0; a<size(); ++a) {
        iadd_h( H,dH, prob(a),dp(i_basis+a) );
    }

    return Nbhd1(H,dH);
}
VNbhd1 Langauge::calc_dH () const
{//derivative WRT simplicial coords
    Float H = 0;
    Vect dH(size());

    //app, comp, and join parts
    iadd_h( H,dH,   m_P_app,i_app );
    iadd_h( H,dH,  m_P_comp,i_comp );
    iadd_h( H,dH,  m_P_join,i_join );

    //basis part
    for (Int a=0; a<size(); ++a) {
        iadd_h( H,dH, prob(a),i_basis+a );
    }

    return VNbhd1(H,dH);
}
VNbhd2 Langauge::calc_ddH () const
{//derivative WRT simplicial coords
    Float H = 0;
    Vect dH(size());
    Mat ddH(size(),2);

    //app, comp, and join parts
    iadd_h( H,dH,ddH,  m_P_app,i_app );
    iadd_h( H,dH,ddH, m_P_comp,i_comp );
    iadd_h( H,dH,ddH, m_P_join,i_join );

    //basis part
    for (Int a=0; a<size(); ++a) {
        iadd_h( H,dH,ddH, prob(a),i_basis+a );
    }

    return VNbhd2(H,dH,ddH);
}
Mat Langauge::calc_g () const
{//Fisher metric
    Mat g(size(), 2);

    //app, comp, and join parts
    iadd_g( g,  m_P_app,i_app );
    iadd_g( g, m_P_comp,i_comp );
    iadd_g( g, m_P_join,i_join );

    //basis part
    for (Int a=0; a<size(); ++a) {
        iadd_g( g, prob(a),i_basis+a );
    }

    return g;
}
void Langauge::update ()
{//renormalize, calculate entropy & perplexity
    m_P_basis = calc_P_basis();
    Assert (0.0f < m_P_basis and m_P_basis < 1.0f,
            "bad P_basis: " << m_P_basis << " = 1 - "
            << m_P_app << " - " << m_P_comp << " - " << m_P_join);

    Float total = 0;
    for (Int i=0; i<size(); ++i)   total  += mass(i);
    for (Int i=0; i<size(); ++i)  mass(i) /=  total;
    m_H = calc_H();
}
bool Langauge::insert (ObHdl ob)
{//returns false if ob was already present
    if (find(ob) < size()) {
        logger.warning() << "language already contains atom" |0;
        return false;
    }
    Float mass = expf(-m_H); //geometric mean size
    m_basis.push_back(std::make_pair(ob, mass));
    update();
    return true;
}
bool Langauge::insert (ObHdl ob, Float mass)
{//returns false if ob was already present
    if (find(ob) < size()) {
        logger.warning() << "language already contains atom" |0;
        return false;
    }
    Assert (0 < mass and mass <= 1.0f, "invalid initial mass: " << mass);
    mass = mass / (1.0f+mass); //corrected for normalization
    m_basis.push_back(std::make_pair(ob, mass));
    update();
    return true;
}
void Langauge::remove (Int i)
{//returns false if ob was not present
    Assert (i < size(), "ivalid index");
    logger.debug() << "pruning atom " << i << " with mass " << mass(i) |0;
    for (++i; i<size(); ++i) {
        m_basis[i-1] = m_basis[i];
    }
    m_basis.pop_back();
    update();
}
bool Langauge::remove (ObHdl ob)
{//returns false if ob was not present
    Int i = find(ob);
    if (i == size()) {
        logger.warning() << "language does not contain atom" |0;
        return false;
    }
    remove(i);
    return true;
}
Ob Langauge::prune_smallest ()
{
    Int min_idx = size();
    Float min_mass = INFINITY;
    for (Int i=0; i<size(); ++i) {
        if (mass(i) < min_mass) {
            min_idx = i;
            min_mass = mass(i);
        }
    }
    Ob pruned = atom(min_idx);
    remove(min_idx);
    return pruned;
}
std::vector<Ob> Langauge::prune_below (Float thresh)
{
    ObPMF remaining;
    std::vector<Ob> pruned;
    for (Int i=0; i<size(); ++i) {
        if (mass(i) >= thresh) {
            remaining.push_back(m_basis[i]);
        } else {
            pruned.push_back(atom(i));
            logger.info() << "pruning atom number " << i
                << " with mass " << mass(i) |0;
        }
    }
    m_basis.swap(remaining);
    update();
    return pruned;
}
Int Langauge::find (ObHdl ob) const
{//returns size on failure
    for (Int i=0; i<size(); ++i) {
        if (*ob == atom(i)) return i;
    }
    return size();
}

//rules
void Langauge::default_rules ()
{//default policy: { mu:0.5, nu:0.5, tau:0 }
    using namespace Symbols;
    m_rules.clear();
    m_rules[ RULE_MU  ] = 0.5f;
    m_rules[ RULE_NU  ] = 0.5f;
    m_rules[ RULE_TAU ] = 0.0f;
}
void Langauge::normalize_rules ()
{//renormalizes and prunes nonpositive members
    Float total = 0;
    for (RulePMF::iterator r=m_rules.begin(); r!=m_rules.end();) {
        Float weight = r->second;
        if (weight>0) {
            total += weight;
            ++r;
        } else {
            m_rules.erase(r++);
        }
    }
    if (total >= 0) {
        for (RulePMF::iterator r=m_rules.begin(); r!=m_rules.end(); ++r) {
            r->second /= total;
        }
        return;
    }
    logger.warning() << "failed to normalize rules; reverting to default" |0;
}
void Langauge::log_rules () const
{
    using namespace Symbols;
    const Logging::fake_ostream& log = logger.info() << "rules:";
    for (RulePMF::const_iterator r=m_rules.begin(); r!=m_rules.end(); ++r) {
        log << "  " << PropRuleNames[r->first] << '@' << r->second;
    }
    log |0;
}

//persistence
Int Langauge::data_size ()
{//P_app, P_comp, P_join, basis
    return i_basis * sizeof(Float)
         + (size() + m_rules.size()) * sizeof(Files::IntWMass);
}
void Langauge::save_to_file (FILE* file)
{
    Int W = size(), R = m_rules.size();
    logger.debug() << "Saving " << W << " words and "
                                << R << " rules to file" |0;
    Logging::IndentBlock block;

    //write P_app, P_comp, P_join
    safe_fwrite(&m_P_app,  sizeof(Float), 1, file);
    safe_fwrite(&m_P_comp, sizeof(Float), 1, file);
    safe_fwrite(&m_P_join, sizeof(Float), 1, file);

    //write basis
    using namespace Files;
    IntWMass* data = new IntWMass[W];
    for (Int i=0; i<W; ++i) {
        data[i].first = Int(atom(i));
        data[i].second = mass(i);
    }
    safe_fwrite(data, sizeof(IntWMass), W, file);
    delete[] data;

    //write rules
    data = new IntWMass[R];
    Int i=0;
    for (RulePMF::const_iterator r=m_rules.begin(); r!=m_rules.end(); ++r) {
        data[i].first  = r->first;
        data[i].second = r->second;
        ++i;
    }
    safe_fwrite(data, sizeof(IntWMass), R, file);
    delete[] data;
}
void Langauge::load_from_file (FILE* file, Int Nwords, Int Nrules)
{
    //WARNING: must match load() in meas_lite.C
    using namespace Files;

    Int W = Nwords, R = Nrules;
    logger.debug() << "Loading " << W << " words and "
                                 << R << " rules from file" |0;
    Logging::IndentBlock block;

    //read P_app, P_comp, P_join
    safe_fread(&m_P_app,  sizeof(Float), 1, file);
    safe_fread(&m_P_comp, sizeof(Float), 1, file);
    safe_fread(&m_P_join, sizeof(Float), 1, file);

    //read basis
    IntWMass* data = new IntWMass[W];
    safe_fread(data, sizeof(IntWMass), W, file);
    m_basis.clear();
    for (Int i=0; i<W; ++i) {
        m_basis.push_back(std::make_pair(Ob(data[i].first), data[i].second));
    }
    delete[] data;

    //read rules
    if (R > 0) {
        data = new IntWMass[R];
        safe_fread(data, sizeof(IntWMass), R, file);
        m_rules.clear();
        for (Int i=0; i<R; ++i) {
            Symbols::PropRule rule = Symbols::PropRule(data[i].first);
            if (not Symbols::rule_is_valid(rule)) {
                logger.warning() << "read invalid inference rule: "
                    << Int(rule);
                default_rules();
                break;
            }
            m_rules[rule] = data[i].second;
        }
        delete[] data;
    } else {
        logger.warning() << "rule weights undefined; using defaults" |0;
        default_rules();
    }
    update();
}

}

