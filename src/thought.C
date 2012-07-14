
#include "thought.h"
#include "obs.h"
#include "order.h"
#include "apply.h"
#include "compose.h"
#include "join.h"
#include "combinatory_structure.h"
#include "lambda_theories.h"
#include "symbols.h"
#include "syntax_semantics.h"
#include "brain.h"
#include "complexity.h"
#include "random_choice.h"
#include <set>
#include <cstdlib> //for drand48
#include <cmath>

//log levels
#define LOG_DEBUG1(mess);
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << mess |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

namespace Thought
{

namespace O  = Obs;
namespace AE = Apply;
namespace CE = Compose;
namespace JE = JoinEqn;
namespace OR = Order;
namespace M  = Measures;
namespace C  = Complexity;
namespace CS = CombinatoryStructure;
namespace EX = Expressions;
namespace LT = LambdaTheories;
namespace TB = TheBrain;
namespace RC = RandomChoice;

using TB::brain;
using LT::theory;
using M::comp; using M::rel;
typedef Ob:: sparse_iterator OIter;

//random generator tools
Ob get_random_ob (Meas mu, Float mu_total)
{//linear-time version of updated generator
    Double total = mu_total * drand48();
    while (true) {
        for (OIter iter=Ob::sbegin(), end=Ob::send(); iter!=end; ++iter) {
            Ob ob = *iter;
            total -= mu(ob);
            if (total <= 0) return ob;
        }
    }
}
void get_random_parse (Ob app, Ob& lhs, Ob& rhs,
                       Float mu_app, Meas mu_lhs, Meas mu_rhs)
{//sets references to lhs,rhs
    Double total = mu_app * drand48();
    while (true) {
        for (AE::Alr_Iterator iter(app); iter.ok(); iter.next()) {
            total -= mu_lhs(iter.lhs()) * mu_rhs(iter.rhs());
            if (total <= 0) {
                lhs = iter.lhs();
                rhs = iter.rhs();
                return;
            }
        }
    }
}
void get_random_parse (Ob app, Ob lhs, Ob& rhs,
                       Float mu_app, Float mu_lhs, Meas mu_rhs)
{//sets reference to rhs
    Double total = mu_app * drand48();
    while (true) {
        for (AE::ALr_Iterator iter(app,lhs); iter.ok(); iter.next()) {
            total -= mu_lhs * mu_rhs(iter.rhs());
            if (total <= 0) {
                rhs = iter.rhs();
                return;
            }
        }
    }
}


//================ types of interests ================

//default (abstract)
void Interest::save_params_to (ostream& os)
{
    os << "!think about everything\n";
}
void Interest::calibrate_lang () { C::calc_Z(brain().lang()); }
Float Interest::get_cost ()
{//relentropy of cool wrt comp
    Meas cool = Meas::alloc();
    init_R_term(cool);
    Double D = C::calc_relentropy(comp, cool);
    cool.free();
    return D;
}

//measure
void Interest_in_meas::write_params_to (ostream& os)
{
    os << "\tinterested in cool stuff" << std::endl;
}
void Interest_in_meas::init_R_term (Meas term, Float total)
{
    _recompile();
    OIter beg = Ob::sbegin();
    OIter end = Ob::send();

    //calculate scaling factor
    Double my_total = 0.0;
    for (OIter i=beg; i!=end; ++i) { Ob ob = *i;
        my_total += m_cool(ob);
    }
    if (not (my_total > 0)) {
        logger.warning() << "measure had no mass" |0;
        return;
    }

    //normalize
    Float factor = total / my_total;
    for (OIter i=beg; i!=end; ++i) { Ob ob = *i;
        term(ob) += factor * m_cool(ob);
    }
}

//obs (abstract)
void Interest_in_obs::write_params_to (ostream& os)
{
    os << "\tob interest max symbols = " << m_symbols_factor << std::endl;
}
void Interest_in_obs::write_stats_to (ostream& os)
{
    os << "\tcontext cost = " << get_cost();
    os << "\n\tob polynomial with " << m_poly.size()  << " obs + "
                                    << m_poly.apps()  << " apps + "
                                    << m_poly.comps() << " comps + "
                                    << m_poly.joins() << " joins" << std::endl;
}
void Interest_in_obs::calibrate_lang ()
{
    C::fit_lang_to_obs(brain().lang(), m_poly, brain().elegance());
}
void Interest_in_obs::init_R_term (Meas term, Float total)
{
    if (m_poly.total() == 0) {
        logger.warning() << "zero interest in obs" |0;
        return;
    }

    Float factor = total / m_poly.total();
    for (Int i=0; i<m_poly.size(); ++i) {
        term(m_poly.ob(i)) += factor * m_poly.num(i);
    }
}
Float Interest_in_obs::get_cost ()
{
    return C::get_cost_of_poly(brain().lang(), m_poly, brain().elegance());
}
void Interest_in_obs::update (bool tryhard)
{
    _recompile(tryhard ? INFINITY : -1);
}

//exprs : obs
void Interest_in_exprs::save_params_to (ostream& os)
{
    if (m_exprs.empty()) return;
    os << "!think about {\n\t" << m_exprs[0];
    for (unsigned i=1; i<m_exprs.size(); ++i) {
        os << ",\n\t" << m_exprs[i];
    }
    os << "\n}\n";
}
void Interest_in_exprs::write_params_to (ostream& os)
{
    os << "\tinterested in exprs\n";
    Interest_in_obs::write_params_to(os);
}
void Interest_in_exprs::_recompile (Float factor)
{//weakly adds subexprs from list
    if (factor < 0) factor = m_symbols_factor;
    Float symbols = factor * brain().mean_symbols();
    logger.info() << "compiling exprs up to " << symbols << " symbols" |0;

    m_poly.clear();
    for (unsigned i=0; i<m_exprs.size(); ++i) {
        EX::add_expr_to_poly(m_poly, m_exprs[i], symbols);
    }
    logger.info() << "compiled " << m_exprs.size() << " exprs into "
                                 << m_poly.size()  << " obs + "
                                 << m_poly.apps()  << " apps + "
                                 << m_poly.comps() << " comps + "
                                 << m_poly.joins() << " joins" |0;
    logger.debug() << "poly: " << m_poly |0;
}

//context : exprs
void Interest_in_context::_recompile (Float factor)
{//weakly adds subexprs from context.defs and context.problems
    logger.info() << "Collecting exprs from context." |0;
    Logging::IndentBlock block;

    //add definitions
    _clear();
    for (Context::def_iter i=m_context.simp_begin();
            i!=m_context.simp_end(); ++i) {
        _add(i->second);
    }

    Interest_in_exprs::_recompile(factor);
}
void Interest_in_context::save_params_to (ostream& os)
{
    os << "!think about context\n";
}
void Interest_in_context::write_params_to (ostream& os)
{
    os << "\tinterested in the context\n";
    Interest_in_exprs::write_params_to(os);
}

//theory : exprs
void Interest_in_theory::_add (std::vector<StmtHdl> ss)
{
    for (unsigned i=0; i<ss.size(); ++i) _add(ss[i]);
}
void Interest_in_theory::_recompile (Float factor)
{//weakly adds subexprs from context.defs and context.problems
    logger.info() << "Collecting exprs from theory." |0;
    Logging::IndentBlock block;

    _clear();
    _add(theory()->get_axioms());
    _add(theory()->get_problems(Symbols::TRUE));
    _add(theory()->get_problems(Symbols::UNKNOWN));

    Interest_in_exprs::_recompile(factor);
}
void Interest_in_theory::save_params_to (ostream& os)
{
    os << "!think about theory\n";
}
void Interest_in_theory::write_params_to (ostream& os)
{
    os << "\tinterested in theory\n";
    Interest_in_exprs::write_params_to(os);
}

//mixed
void MixedInterests::save_params_to (ostream& os)
{
    m_int1->save_params_to(os);
}
void MixedInterests::init_R_term (Meas term, Float total)
{//a weighted sum
    Float factor = total / (m_c1 + m_c2);
    m_int1->init_R_term(term, factor * m_c1);
    m_int2->init_R_term(term, factor * m_c2);
}
void MixedInterests::write_params_to (ostream& os)
{
    os << "\tmixed interests with " << m_c1 << " : " << m_c2 << " ratio:\n";
    m_int1->write_params_to(os);
    m_int2->write_params_to(os);
}
void MixedInterests::write_stats_to (ostream& os)
{
    m_int1->write_stats_to(os);
    m_int2->write_stats_to(os);
}
void MixedInterests::update (bool tryhard)
{
    m_int1->update(tryhard);
    m_int2->update(tryhard);
}

//================ types of motives ================

const unsigned long MAX_SAMPLES = 1000;

//general motive
bool Motive::expand ()
{
    //XXX TODO make this do something realistic
    return expand_to (Ob::size() + 1);
}
void Motive::cleanup ()
{
    //XXX TODO make this do something realistic
    return;
}
bool Motive::expand_to (Int target)
{
    logger.info() << "expanding to " << target
                  << " at beta = " << brain().beta() |0;
    Logging::IndentBlock block;

    bool success = true;
    Int orig_size = Ob::size();
    m_obs_tried = 0;
    m_obs_created = 0;
    m_obs_tried = 0;

    setup: _setup();

    while (success and Ob::size() < target) {
        switch (_create_random_term()) {
            case FAILURE: success = false; break;
            case SUCCESS: break;
            case HOTTER:  goto setup;
        }
    }
    log_stats(Ob::size() - orig_size);

    cleanup();
    return success;
}
void Motive::log_stats (Int diff)
{
    Float efficiency = (100.0f * m_obs_created) / m_obs_tried;
    if (efficiency < 2.0f) {
        logger.warning() << "generator efficiency = " << efficiency << '%' |0;
    }

    logger.info() << "created " << diff
                  << " new obs from " << m_obs_created << " new eqns" |0;
}

//complexity-based expansion
Motive_to_ponder::~Motive_to_ponder ()
{
    if (m_cool_beta) {
        brain().clear_generator(m_client_id);
        m_cool_beta->free();
        delete m_cool_beta;
    }
}
void Motive_to_ponder::write_params_to (ostream& os)
{
    os << "\tmotivated to ponder" << std::endl;
}
void Motive_to_ponder::_setup ()
{//sets cool^beta & generator
    if (m_cool_beta == NULL) {
        m_cool_beta = new Meas(Meas::alloc());
        C::calc_power(m_cool, brain().beta(), *m_cool_beta);
    }
    brain().ensure_generator(m_client_id, *m_cool_beta);
}
void Motive_to_ponder::cleanup ()
{//clears generator & cool^beta
    if (m_cool_beta == NULL) return; //nothing to clean up
    brain().clear_generator(m_client_id);
    m_cool_beta->free();
    delete m_cool_beta;
    m_cool_beta = NULL;
}
bool Motive_to_ponder::expand ()
{
    _setup ();
    m_obs_tried = 0;
    m_obs_created = 0;
    switch (_create_random_term()) {
        case SUCCESS: return true;
        case HOTTER: cleanup(); return true;
        default: return false;
    }
}
Motive::Created Motive_to_ponder::_create_random_term ()
{
    if (brain().generator_empty()) {
        logger.warning() << "no lhs,rhs found; something's wrong" |0;
        return FAILURE;
    }
    Float a = brain().get_P_app();
    Float c = brain().get_P_comp();
    Float j = brain().get_P_join();
    if (RC::random_bernoulli( a/(a+c+j) ))  return _create_random_app();
    if (RC::random_bernoulli( c/(c+j) ))    return _create_random_comp();
    else                                    return _create_random_join();
}
Motive::Created Motive_to_ponder::_create_random_app ()
{
    Ob lhs,rhs,app;
    do {
        if (++m_obs_tried > MAX_SAMPLES) {
            logger.warning() << "no new terms found; raising temperature" |0;
            if (brain().raise_temp()) return HOTTER;
            else                      return FAILURE;
        }

        lhs = brain().get_random_ob();
        rhs = brain().get_random_ob();
        LOG_DEBUG1( "trying (" << lhs << ", " << rhs << ")" );
        app = AE::find_app(lhs, rhs);
    } while (app);
    ++m_obs_created;
    CS::create_app(lhs, rhs);
    return SUCCESS;
}
Motive::Created Motive_to_ponder::_create_random_comp ()
{
    Ob lhs,rhs,cmp;
    do {
        if (++m_obs_tried > MAX_SAMPLES) {
            logger.warning() << "no new comp found; raising temperature" |0;
            if (brain().raise_temp()) return HOTTER;
            else                      return FAILURE;
        }

        lhs = brain().get_random_ob();
        rhs = brain().get_random_ob();
        LOG_DEBUG1( "trying (" << lhs << ", " << rhs << ")" );
        cmp = CE::find_comp(lhs, rhs);
    } while (cmp);
    ++m_obs_created;
    CS::create_comp(lhs, rhs);
    return SUCCESS;
}
Motive::Created Motive_to_ponder::_create_random_join ()
{
    Ob lhs,rhs,join;
    do {
        if (++m_obs_tried > MAX_SAMPLES) {
            logger.warning() << "no new join found; raising temperature" |0;
            if (brain().raise_temp()) return HOTTER;
            else                      return FAILURE;
        }

        lhs = brain().get_random_ob();
        rhs = brain().get_random_ob();
        LOG_DEBUG1( "trying (" << lhs << ", " << rhs << ")" );
        join = JE::find_join(lhs, rhs);
    } while (join);
    ++m_obs_created;
    CS::create_join(lhs, rhs);
    return SUCCESS;
}

//================ memory models ================

//basic memory
Memory::Memory () : m_important(rel) {}
void Memory::write_params_to (ostream& os)
{
    os << "\tbasic memory" << std::endl;
}
void Memory::write_stats_to (ostream& os)
{
    os << "\ttotal memory weight = " << m_important.total() << std::endl;
}
bool Memory::contract_to (Int target)
{
    logger.info() << "Forgetting: contracting to " << target |0;
    Logging::IndentBlock block;

    bool progress = true;
    Meas unimportant = Meas::alloc();

    while (Ob::size() > target and progress) {
        Int start_size = Ob::size();

        //define unimportance
        unimportant.set(0);
        std::vector<Ob> worthless;
        Float beta = brain().beta();
        Float total = 0.0f;
        for (OIter iter=Ob::sbegin(), end=Ob::send(); iter!=end; ++iter) {
            Ob ob = *iter;
            if (not O::isPrunable(ob)) continue;
            Float imp = powf(m_important(ob), beta);
            if (imp > 0) { total += unimportant(ob) = 1.0f / imp; }
            else         { worthless.push_back(ob); }
        }

        //pick off worst value if total is infinite
        if (worthless.empty() and std::isinf(total)) {
            Ob worst_ob(0);
            Float worst_val = 0.0f;
            for (OIter iter=Ob::sbegin(), end=Ob::send(); iter!=end; ++iter) {
                Ob ob = *iter;
                Float val = unimportant(ob);
                if (val > worst_val) {
                    worst_ob = ob;
                    worst_val = val;
                }
            }
            logger.info() << "picking off ob with least importance "
                          << m_important(worst_ob) |0;
            unimportant(worst_ob) = 0;
            worthless.push_back(worst_ob);
        }

        //prune all worthess obs first, even if beyond target
        for (int i=worthless.size()-1; i>=0; --i) {
            logger.debug() << "pruning worthless ob" |0;
            CS::prune_ob(worthless[i]);
        }

        //prune unimportant nodes
        brain().set_generator(unimportant);
        while (Ob::size() > target) {
            if (brain().generator_empty()) {
                logger.warning() << "empty generator; raising temperature" |0;
                if (not brain().raise_temp()) brain().log_generator();
                break;
            }

            Ob ob = brain().get_random_ob();
            CS::prune_ob(ob);
        }
        brain().clear_generator();

        progress = Ob::size() < start_size;
    }

    unimportant.free();
    return progress;
}

//two-level memory
TwoLevelMemory::TwoLevelMemory (Int& scale, Float how_hip)
    : Memory(Meas::alloc()),
      m_was_imp(Meas::alloc()),
      m_now_imp(rel),
      m_time_scale(scale),
      m_how_hip(how_hip)
{
    m_was_imp.set(0);
    m_important.set(0);
}
TwoLevelMemory::~TwoLevelMemory ()
{
    m_important.free();
    m_was_imp.free();
}
void TwoLevelMemory::write_params_to (ostream& os)
{
    os << "\ttwo-level memory: hipness = " << m_how_hip << std::endl;
}
void TwoLevelMemory::remember_this (Float weight)
{
    C::calc_weighted_sum(1-weight, m_was_imp,
                         weight,   m_now_imp,
                                   m_was_imp);
}
bool TwoLevelMemory::contract_to (Int target)
{
    Float diff = Ob::size() - target;
    Float old_part = exp(-diff / m_time_scale);
    C::calc_weighted_sum(  old_part, m_was_imp,
                         1-old_part, m_now_imp,
                                     m_was_imp);
    C::calc_weighted_sum(  m_how_hip, m_now_imp,
                         1-m_how_hip, m_was_imp,
                                      m_important);
    return Memory::contract_to(target);
}

}


