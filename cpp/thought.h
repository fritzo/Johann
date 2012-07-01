#ifndef JOHANN_THOUGHT_H
#define JOHANN_THOUGHT_H

#include "definitions.h"
#include "nodes.h"
#include "order.h"
#include "expressions.h"
#include "context.h"
#include "measures.h"
#include "fuzzy.h"
#include <vector>
#include <utility>


/** \namespace Thought.
 * Thinking has three aspects:
 *  * Interests -  likelihood mass functions on terms (or proofs)
 *  * Motives -    criteria to expand the database
 *  * Memories -   criteria to contract the database,
 *                 and adjustments to relevance for multiple time scales
 */
namespace Thought
{

const Logging::Logger logger("thought", Logging::DEBUG);

//=========================================================
/** Types of interests, likelihood mass functions on terms.
 */
class Interest
{//abstract interest
public:

    //interface for brain
    virtual void calibrate_lang ();
    virtual Float get_cost (); //relentropy of cool wrt comp

    //interface for complexity
    /// initializes relevance computation with usage
    virtual void init_R_term (Meas term, Float total=1.0) = 0;

    //ctors & dtors
    Interest () {}
    virtual ~Interest () {}
    virtual void save_params_to  (ostream& os);
    virtual void write_params_to (ostream& os) = 0;
    virtual void write_stats_to  (ostream& os) {}
    virtual void update (bool tryhard=false) {}
};

class Interest_in_meas : public Interest
{//interest in some Meas-specified notion of cool
protected:
    Meas m_cool;
    virtual void _recompile () {}; //do nothing by default
public:

    //interface for complexity
    virtual void init_R_term (Meas term, Float total=1.0);

    //ctors & dtors
    Interest_in_meas (Meas cool) : m_cool(cool) {}
    virtual ~Interest_in_meas () {}
    virtual void write_params_to (ostream& os);
};

class Interest_in_obs : public Interest
{//interest in understanding obs
protected:
    ObPoly  m_poly;
    Float m_symbols_factor;
    virtual void _recompile (Float factor=-1.0) = 0;
public:

    //measure interface
    virtual void init_R_term (Meas term, Float total=1.0);
    virtual void calibrate_lang ();
    virtual Float get_cost ();

    //ctors & dtors
    Interest_in_obs () : m_symbols_factor(4.0) {}
    virtual ~Interest_in_obs () {}
    virtual void write_params_to (ostream& os);
    virtual void write_stats_to  (ostream& os);
    virtual void update (bool tryhard=false);

};

class Interest_in_exprs : public Interest_in_obs
{//interest in understanding an expression
protected:
    std::vector<ExprHdl> m_exprs;
    virtual void _recompile (Float factor=-1.0);

    //internal constructors
    Interest_in_exprs () {}
    void _clear () { m_exprs.clear(); }
    void _add (ExprHdl e) { m_exprs.push_back(e); }
    void _add (std::vector<ExprHdl> es) {
        m_exprs.insert(m_exprs.end(),es.begin(),es.end());
    }
public:

    //ctors & dtors
    Interest_in_exprs (std::vector<ExprHdl> es) : m_exprs(es) { _recompile(); }
    virtual ~Interest_in_exprs () {}
    virtual void save_params_to  (ostream& os);
    virtual void write_params_to (ostream& os);

};

class Interest_in_context : public Interest_in_exprs
{//interest in understanding the context
protected:
    Context& m_context;
    virtual void _recompile(Float factor=-1.0);
public:
    Interest_in_context (Context& c) : m_context(c) { _recompile(); }
    virtual ~Interest_in_context () {}
    virtual void save_params_to  (ostream& os);
    virtual void write_params_to (ostream& os);
};

class Interest_in_theory : public Interest_in_exprs
{//interest in understanding the theory
protected:
    virtual void _recompile(Float factor=-1.0);

    void _add (StmtHdl s) { Interest_in_exprs::_add(s->relevant_exprs()); }
    void _add (std::vector<StmtHdl> ss);
public:
    Interest_in_theory () { _recompile(); }
    virtual ~Interest_in_theory () {}
    virtual void save_params_to  (ostream& os);
    virtual void write_params_to (ostream& os);
};

class MixedInterests : public Interest
{//weighted mixture of two interests
protected:
    Float m_c1, m_c2;
    Interest *m_int1, *m_int2;
public:

    //measure interface
    virtual void init_R_term (Meas term, Float total=1.0);
    virtual void calibrate_lang () { m_int1->calibrate_lang(); }
    virtual void update (bool tryhard=false);

    //ctors & dtors
    MixedInterests (Float c1, Interest* int1,
                    Float c2, Interest* int2)
        : m_c1(c1), m_c2(c2), m_int1(int1), m_int2(int2) {}
    virtual ~MixedInterests () { delete m_int1; delete m_int2; }
    virtual void save_params_to  (ostream& os);
    virtual void write_params_to (ostream& os);
    virtual void write_stats_to  (ostream& os);

};

//====================================================
/** Types of motives, criteria to expand the database.
 */
class Motive
{
protected:
    unsigned long m_obs_tried, m_obs_created;
    Int m_client_id; //for generator

    //expansion & contraction
    virtual void _setup () {}; //before expanding
    enum Created { FAILURE, SUCCESS, HOTTER };
    virtual Created _create_random_term () { return FAILURE; } //undefined
    void log_stats (Int diff);
public:
    virtual bool expand (); //setup or expand by one ob
    virtual void cleanup (); //after expanding
    virtual bool expand_to (Int target);

    //ctors & dtors
    Motive () : m_client_id(0) {}
    virtual ~Motive () { cleanup(); }
    virtual void write_params_to (ostream& os) = 0;
};

class Motive_to_ponder : public Motive
{
protected:
    Meas m_cool;
    Meas *m_cool_beta;

    //expansion & contraction
    virtual void _setup ();
    Created _create_random_app ();
    Created _create_random_comp ();
    Created _create_random_join ();
    virtual Created _create_random_term ();
public:
    virtual void cleanup ();
    virtual bool expand ();

    //ctors & dtors
    Motive_to_ponder (Meas cool) : m_cool(cool), m_cool_beta(NULL) {}
    virtual ~Motive_to_ponder ();
    virtual void write_params_to (ostream& os);
};

//===================================================
/** Memory models, criteria to contract the database.
 * Also responsible for short- and long-term scales.
 */
class Memory
{
protected:
    Meas m_important;
    Memory (Meas imp) : m_important(imp) {}
public:

    //ctors & dtors
    Memory ();
    virtual ~Memory () {}
    virtual void save_params_to (ostream& os) {}
    virtual void write_params_to (ostream& os);
    virtual void write_stats_to (ostream& os);

    //meory interface
    virtual void remember_this (Float weight = 0.5f) {} //do nothing
    virtual bool contract_to (Int target);
};

class TwoLevelMemory : public Memory
{
protected:
    Meas m_was_imp, m_now_imp;
    Int &m_time_scale; //in units of how much has been forgotten
    Float m_how_hip;
public:

    //ctors & dtors
    TwoLevelMemory (Int& scale, Float how_hip=0.5);
    virtual ~TwoLevelMemory ();
    virtual void save_params_to (ostream& os) {} //TODO set hipness
    virtual void write_params_to (ostream& os);

    //meory interface
    virtual void remember_this (Float weight = 0.5f);
    virtual bool contract_to (Int target);
};

}

#endif
