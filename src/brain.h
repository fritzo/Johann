#ifndef JOHANN_BRAIN_H
#define JOHANN_BRAIN_H

#include "definitions.h"
#include "nodes.h"
#include "measures.h"
#include "languages.h"
#include "fuzzy.h"
#include "context.h"
#include "lambda_theories.h"
#include "thought.h"
#include <set>
#include <vector>
#include <utility>

namespace TheBrain
{

const Logging::Logger logger("brain", Logging::DEBUG);

//for ob measure output
ostream& operator<< (ostream& os, ObPMF pmf); //copies & sorts first

struct Conjecture
{
    Ob lhs, rhs;
    Float evidence;
    Conjecture (Ob _lhs, Ob _rhs, Float _evidence)
        : lhs(_lhs), rhs(_rhs), evidence(_evidence) {}
    Conjecture () : lhs(0), rhs(0), evidence(1.0) {}
};

//======================== brain class ========================

class Brain
{//basic brain using measure calculations
    static Brain* s_unique_instance;
    friend inline Brain& brain ();
    LambdaTheories::MagmaTheory *m_theory;
protected:

    //state flags
    bool m_am_alive;
    void start_living () { m_am_alive = true; }
    void stop_living  () { m_am_alive = false; }
    bool am_alive () { return m_am_alive; }
public:

    //language of concepts
protected:
    Lang m_lang;
public:
    Lang&    lang    () { return m_lang; }
    Float&   P_app   () { return m_lang.P_app(); }
    Float&   P_comp  () { return m_lang.P_comp(); }
    Float&   P_join  () { return m_lang.P_join(); }
    ObPMF&   basis   () { return m_lang.basis(); }
    RulePMF& rules   () { return m_lang.rules(); }

    const Lang&    get_lang    () const { return m_lang; }
    Float          get_P_app   () const { return m_lang.get_P_app(); }
    Float          get_P_comp  () const { return m_lang.get_P_comp(); }
    Float          get_P_join  () const { return m_lang.get_P_join(); }
    const ObPMF&   get_basis   () const { return m_lang.get_basis(); }
    const RulePMF& get_rules   () const { return m_lang.rules(); }
    Float          P_basis     () const { return m_lang.P_basis(); }

    bool retract (ObHdl ob);
    bool extend (ObHdl ob);
    bool extend (std::vector<ObHdl> obs);

    //proof information
protected:
    bool m_guessing;
    Fuzzy::Order *m_guess;
public:
    bool guessing () const { return m_guessing; }
    //Fuzzy::Order& guess () { Assert(m_guessing, "not guessing"); return *m_guess; }
    void set_guessing (bool whether); //whether to maintain fuzzy info
    void start_guessing ();
    void stop_guessing ();
    Prob guess_less (Ob lhs, Ob rhs);
    Prob guess (StmtHdl stmt);

    //measure access
    Float get_total      () const;
    Float get_entropy    () const;
    Float get_mean_prob  () const { return expf(-get_entropy()); }
    Float get_perplexity () const { return expf(get_entropy()); }
    Float get_lang_entropy    () const { return m_lang.entropy(); }
    Float get_lang_perplexity () const { return m_lang.perplexity(); }
    Float get_mass (Ob ob);
    Float get_mass (App eqn);
    Float get_mass (Comp eqn);
    Float get_mass (Join eqn);
    static Float get_complexity (Ob ob);
    static Float get_complexity (App eqn);
    static Float get_complexity (Comp eqn);
    static Float get_complexity (Join eqn);
    static Float get_relevance (Ob ob);
    //static Float get_relevance (App eqn);
    //static Float get_relevance (Comp eqn);
    //static Float get_relevance (Join eqn);
    Float get_symbols (Ob ob);
    Float get_symbols (App eqn);
    Float get_symbols (Comp eqn);
    Float get_symbols (Join eqn);
    Float mean_symbols ();
    void plot_funs_of_eps (size_t num_steps=32);
protected:
    void calc_perturb ();
public:
    bool save_map (size_t dim=6);
    bool send_lang_to (string address);
    static int send_eqns_to (string address);

    //measure diagnostics
    std::vector<std::pair<Ob,Float> > get_simplest (size_t N=32);
    std::vector<std::pair<Ob,Float> > get_most_relevant (size_t N=32);
    std::vector<std::pair<Ob,Float> > get_sketchiest (size_t N=32);
    std::vector<Conjecture> get_conjectures (size_t N=64);
    bool vis_ob_mass (size_t size=0, size_t ratio=4);

    //measure calculation interface
    Measures::Manager m_measures;
protected:
    void init_basis_measure (bool init_lang=true);
    void calc_measures (bool optimize);
    void calc_P ();
    Float calc_cost ();

public:
    //special node actions, dealing with measures
    void create_ob    (Ob  ob);
    void create_app   (App eqn);
    void create_comp  (Comp eqn);
    void create_join  (Join eqn);
    void merge_obs    (Ob  dep, Ob  rep);
    void delete_ob    (Ob  ob);

    // random node sampling
    typedef size_t Client;
private:
    Client m_client;
public:
    Client set_generator (Meas mu);
    void ensure_generator (Client & c, Meas mu);
    void clear_generator (Client c=0);
    bool generator_empty();
    Ob get_random_ob (); //assumes generator is nonempty
    void log_generator ();

    //structure-measure control
    bool expand_to (size_t target);
    bool contract_to (size_t target);
    void compact (bool sort_eqns = true);
    void write_lang_to (ostream& os);
    void write_stats_to (ostream& os);
    void save_params_to (ostream& os);
    void write_params_to (ostream& os);
    void log_params ();

    //thought & memory control
private:
    size_t m_min_size;     // lower bound on number of concepts
    size_t m_max_size;     // upper bound on number of concepts
    size_t m_granularity;  // concepts per think cycle, = max-min
    size_t m_time_scale;   // time between scheduled rests
    size_t m_clock;        // elapsed time since rest, in concepts deleted
    Long m_age;         // total elapsed time,      in concepts deleted
    Float m_density;    // desired density of ord table
    Float m_beta;       // inverse temperature
    Float m_elegance;   // attention to aesthetics
    Thought::Interest *m_interest;
    Thought::Motive   *m_motive;
    Thought::Memory   *m_memory;
public:
    size_t granularity () const { return m_granularity; }
    size_t time_scale  () const { return m_time_scale; }
    Long age        () const { return m_age; }
    Float density   () const { return m_density; }
    Float beta      () const { return m_beta; }
    Float elegance  () const { return m_elegance; }
    void set_size (size_t s = 0);
    void set_granularity (size_t g);
    void set_time_scale (size_t t) { m_time_scale = t; }
    void set_density (Float d) { m_density = max(0.0,min(1.0, d)); }
    void set_temperature (Float t);
    void set_elegance    (Float e) { m_elegance = e; }
    bool raise_temp (); //true on success
    bool lower_temp (); //true on success
    void think_in (Float P_app=0.4, Float P_comp=0.2, Float P_join=0.1);
    void think_in (const std::vector<ObHdl>& obs,
                   Float P_app=0.4, Float P_comp=0.2, Float P_join=0.1);
    void think_in (ObPMF& obs,
                   Float P_app=0.4, Float P_comp=0.2, Float P_join=0.1);
    void set_P_     (Float P_app=0.4, Float P_comp=0.2, Float P_join=0.1);
    void set_P_app  (Float P_app=0.4);
    void set_P_comp (Float P_comp=0.2);
    void set_P_join (Float P_join=0.1);
protected:
    void think_about (Thought::Interest *interest,
                      Thought::Motive   *motive,
                      bool resting=true);
public:
    void think_about_exprs (std::vector<ExprHdl> exprs);
    void think_about_context (Context& context);
    void think_about_theory ();
    void think_about_everything (bool resting=true);
    bool think (size_t num_cycles);
    bool think (); //minimal thought
    void update (bool tryhard=false);
    void cleanup () { m_motive->cleanup(); }

    //resting
private:
    void _tick_tock (); // advances clock by one tick
    void _rest ();      // advances clock to end
public:
    bool time_to_rest ();
    void rest ();

    //main control
    Brain (LambdaTheories::MagmaTheory* theory);
    virtual ~Brain ();
    typedef std::set<string> BasisType;
    virtual void initialize (size_t num_obs, const BasisType& basis);
protected:
    virtual void _init_misc (bool init_lang=true);
public:
    virtual void clear ();
    virtual void resize (size_t size, const oid_t * new2old = NULL);
    bool save (string filename);
    bool load (string filename);
    virtual void validate (size_t level = 3); //ranges in 0,1,2,3,4,5
};

inline Brain& brain ()
{
    Assert5(Brain::s_unique_instance != NULL,
            "tried to access non-existing brain");
    return *Brain::s_unique_instance;
}

}

#endif
