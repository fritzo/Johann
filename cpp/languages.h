#ifndef JOHANN_LANGUAGES_H
#define JOHANN_LANGUAGES_H

#include "definitions.h"
#include "symbols.h"
#include "nodes.h"
#include "linalg.h"
#include <vector>
#include <map>
#include <utility>
#include <cmath>

//WARNING: this must match definition in optimization.h
using LinAlg::Vect;
using LinAlg::Mat;
using LinAlg::Nbhd1;
using LinAlg::VNbhd1;
using LinAlg::VNbhd2;

//WARNING: this must match def in syntax_semantics.h
typedef std::vector<std::pair<ObHdl,Float> > ObPMF;
typedef std::map<Symbols::PropRule,Float> RulePMF;

namespace Languages
{

const Logging::Logger logger("lang", Logging::DEBUG);

/**  Languages.
 * signature: <app:A^2->A, comp:A^2->A, join:A^2->A, c1:A,...,cn:A>
 *
 * TODO: finish updating for composition, join
 *   * entropy derivatives, etc
 *
 */
enum Indices { i_app = 0, i_comp = 1, i_join = 2, i_basis = 3 };
class Langauge
{//includes syntax (terms) and semantics (proofs)
protected:
    //data
    ObPMF m_basis;      //a normalized set of weighted obs
    Float m_P_app;      //a multiplier in (0,1)
    Float m_P_comp;     //a multiplier in (0,1)
    Float m_P_join;     //a multiplier in (0,1)
    Float m_P_basis;    // = 1 - P_app - P_comp - P_join
    RulePMF m_rules;
    Float m_H;     //entropy & perplexity

    Float calc_P_basis () const { return 1.0 - m_P_app - m_P_comp - m_P_join; }
public:
    //raw access
    typedef const ObPMF& PMF;
    Float&   P_app   () { return m_P_app; }
    Float&   P_comp  () { return m_P_comp; }
    Float&   P_join  () { return m_P_join; }
    ObPMF&   basis   () { return m_basis; }
    RulePMF& rules   () { return m_rules; }

    Float get_P_app  () const { return m_P_app; }
    Float get_P_comp () const { return m_P_comp; }
    Float get_P_join () const { return m_P_join; }
    Float P_basis    () const { return m_P_basis; }
    PMF   get_basis  () const { return m_basis; }
    const RulePMF& rules () const { return m_rules; }
    void set_P_app   (Float P_app);
    void set_P_comp  (Float P_comp);
    void set_P_join  (Float P_join);

    //vector-like access
    //WARNING: order changes during update, insertion, & removal
    Int num_words    () const { return m_basis.size(); }
    Int num_rules    () const { return m_rules.size(); }
    Int size         () const { return num_words(); }
    Int vect_size    () const { return i_basis + size(); }
    Float entropy    () const { return m_H; } //entropy of basis
    Float perplexity () const { return expf(m_H); } //perplexity of basis
    Ob     atom       (Int i) const { return *(m_basis[i].first); }
    Float& mass       (Int i)       { return m_basis[i].second; }
    Float  mass       (Int i) const { return m_basis[i].second; }
    Float& operator[] (Int i)       { return m_basis[i].second; }
    Float  operator[] (Int i) const { return m_basis[i].second; }
    Float prob        (Int i) const { return mass(i) * P_basis(); }

    //set-like basis access
protected:
    Int find (ObHdl ob) const;
public:
    bool contains (ObHdl ob) const { return find(ob) != size(); }
    bool insert (ObHdl ob);
    bool insert (ObHdl ob, Float mass);
    void remove (Int n);
    bool remove (ObHdl ob);         //returns false if ob was not found
    Ob prune_smallest ();
    std::vector<Ob> prune_below (Float thresh=1e-6f);

    //rule access
    void default_rules (); //resets
    void normalize_rules ();
    void log_rules () const;

    //parametrization in the simplex (L,P_app) |--> theta : [0,1]^|B|
    Vect to_vect () const;
    void from_vect (const Vect& v);

    //as an algebraic signature
    Float  calc_H   ()               const; // entropy
    Nbhd1  calc_dH  (const Vect& dL) const; // (H,dH)
    VNbhd1 calc_dH  ()               const; // (H,dH/dtheta)
    VNbhd2 calc_ddH ()               const; // (H,dH/dtheta)
    Mat    calc_g   ()               const; // Fisher metric

    //ctors etc
    Langauge (PMF basis, Float P_app=0.4, Float P_comp=0.2, Float P_join=0.1)
        : m_basis(basis), m_P_app(P_app), m_P_comp(P_comp), m_P_join(P_join)
        { default_rules(); }
    Langauge (Float P_app=0.4, Float P_comp=0.2, float P_join=0.1)
        : m_basis(0), m_P_app(P_app), m_P_comp(P_comp), m_P_join(P_join)
        { default_rules(); }
    void clear () { m_basis.clear(); }
    void update ();

    //persistence
    Int data_size ();
    void save_to_file (FILE* file);
    void load_from_file (FILE* file, Int Nwords, Int Nrules);
};

}

typedef Languages::Langauge Lang;

#endif

