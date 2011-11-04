
#include "brain.h"
#include "version.h"
#include "random_choice.h"
#include "linalg.h"
#include "complexity.h"
#include "obs.h"
#include "order.h"
#include "apply.h"
#include "compose.h"
#include "join.h"
#include "expressions.h"
#include "statements.h"
#include "combinatory_structure.h"
#include "syntax_semantics.h"
#include "thought.h"
#include "reorder.h"
#include "sorting.h"
#include "files.h"
#include "visual.h"
#include "socket_tools.h"
#include <sstream>
#include <algorithm>

//log levels
#define LOG_DEBUG1(mess);
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << mess |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

namespace TheBrain
{

namespace RC = RandomChoice;
namespace LA = LinAlg;
namespace C  = Complexity;
namespace M  = Measures;
namespace O  = Obs;
namespace OR = Order;
namespace AE = Apply;
namespace CE = Compose;
namespace JE = JoinEqn;
namespace CS = CombinatoryStructure;
namespace LT = LambdaTheories;
namespace EX = Expressions;
namespace ST = Statements;
namespace T  = Thought;
namespace V  = Visual;
using namespace Symbols;

typedef Reorder::Reordering<Ob>  ORing;
typedef Reorder::Reordering<App> ARing;
typedef Reorder::Reordering<Comp> CRing;
typedef Reorder::Reordering<Join> JRing;

using M::comp; using M::komp; using M::rel;
typedef M::VectMeas VMeas;

//================ random ob generators ================
//choice measures
typedef const M::Measure<Long> IntMeas;
IntMeas o_pmf(0x3), o_pmf_l(0x4);
enum { STATIC_MEASURES = 5 * sizeof(Float) }; //comp, komp, rel, o_pmf, o_pmf_l

//generators
RC::Generator *g_generator = NULL;
Brain::Client Brain::set_generator (Meas mu)
{
    if (g_generator != NULL) delete g_generator;
    g_generator = new RC::Generator (mu, o_pmf, o_pmf_l);
    g_generator->update_all();
    if (++m_client == 0) ++m_client; //never let client be zero
    return m_client;
}
void Brain::ensure_generator (Client &c, Meas mu)
{
    if (c != m_client) c = set_generator(mu);
    else Assert1(g_generator != NULL, "client owns NULL generator");
};
void Brain::clear_generator (Client c)
{
    if (c != m_client++) return;
    Assert1(g_generator != NULL, "tried to clear generator twice");
    delete g_generator;
    g_generator = NULL;
}
Ob Brain::get_random_ob ()
{
    Assert1(g_generator != NULL, "tried to get_random_ob without generator");
    Assert1(not g_generator->empty(), "tried to get ob from empty generator");
    return g_generator->choose();
}
bool Brain::generator_empty ()
{
    Assert1(g_generator != NULL, "tried to test emptiness without generator");
    return g_generator->empty();
}
void Brain::log_generator ()
{
    Assert1(g_generator, "tried to log non-existant generator");
    g_generator->log_histogram();
}

//================ complexity operations ================

//measure access
Float Brain::get_total   () const { return C::get_ZA(); }
Float Brain::get_entropy () const { return C::get_HA(); }
Float Brain::get_mass (Ob ob)
{
    for (ObPMF::const_iterator i=basis().begin(); i!=basis().end(); ++i) {
        if (ob == *(i->first)) {
            return -log(P_basis() * i->second);
        }
    }
    return INFINITY; //ob is not an atom
}
Float Brain::get_mass (App eqn)
{
    return -log(brain().P_app()) + komp(get_lhs(eqn)) + komp(get_rhs(eqn));
}
Float Brain::get_mass (Comp eqn)
{
    return -log(brain().P_comp()) + komp(get_lhs(eqn)) + komp(get_rhs(eqn));
}
Float Brain::get_mass (Join eqn)
{
    return -log(brain().P_join()) + komp(get_lhs(eqn)) + komp(get_rhs(eqn));
}
Float Brain::get_complexity (Ob ob) { return comp(ob); }
Float Brain::get_complexity (App eqn)
{
    return comp(get_lhs(eqn)) * brain().P_app() * comp(get_rhs(eqn));
}
Float Brain::get_complexity (Comp eqn)
{
    return comp(get_lhs(eqn)) * brain().P_comp() * comp(get_rhs(eqn));
}
Float Brain::get_complexity (Join eqn)
{
    return comp(get_lhs(eqn)) * brain().P_join() * comp(get_rhs(eqn));
}
Float Brain::get_relevance (Ob ob) { return rel(ob) / comp(ob); }
Float Brain::get_symbols (Ob ob)
{
    Float info = -log(get_complexity(ob));
    return info / get_lang_entropy();
}
Float Brain::get_symbols (App eqn)
{
    Float info = -log(get_complexity(eqn));
    return info / get_lang_entropy();
}
Float Brain::get_symbols (Comp eqn)
{
    Float info = -log(get_complexity(eqn));
    return info / get_lang_entropy();
}
Float Brain::mean_symbols ()
{//mean number of symbols per ob
    Float info = C::get_HA();
    return info / get_lang_entropy();
}
void Brain::plot_funs_of_eps (Int num_points)
{
    //plots inverse perplexity
    if (num_points < 2) {
        logger.error() << "too few points to plot" |0;
        return;
    }

    ofstream plot_file("plots/funs_of_eps.text");
    if (not plot_file.is_open()) {
        logger.error() << "could not open file for plotting" |0;
        return;
    }

    plot_file << "# P_app, Z(A), P=exp(-H), N=exp(H), H, 1/H, |Bot| \n";

    Float old_P_app = P_app(); //save existing value
    Float stepsize = 1.0 / num_points;
    for (Int i=0; i<num_points; ++i) {
        P_app() = (i + 0.5) * stepsize;
        calc_measures(false);

        plot_file   << get_P_app()
            << '\t' << get_total()   //Z(A), enumerated mass
            << '\t' << get_mean_prob()    //P, mean probability?
            << '\t' << get_perplexity()   //N, perplexity
            << '\t' << get_entropy()      //H, entropy
            << '\t' << 1.0/get_entropy()  //iH, inverse entropy
            << '\t' << get_complexity(*LT::Atoms::Bot) //unsolvable mass
            << '\n';
    }
    plot_file.close();

    //clean up
    P_app() = old_P_app;
    calc_measures(false);
}
void Brain::calc_perturb ()
{
    //LATER: revise fuzzy order here

    //calculate relevant measures
    if (not Ob::full()) compact();
    calc_measures(false);

    //calculate very high-precision complexity
    Float old_tol = C::get_tolerance();
    Float new_tol = 1e-12;
    logger.info() << "increasing tolerance: "
        << old_tol << " --> " << new_tol |0;
    C::set_tolerance(new_tol);
    C::calc_Z(m_lang);
    C::set_tolerance(old_tol);
}
bool Brain::save_map (Int dim)
{//returns true on error
    logger.info() << "Saving map" |0;
    Logging::IndentBlock block;

    logger.info() << "finding coordinates" |0;
    calc_perturb();                             //get ready, compacts
    VMeas coords = VMeas::alloc(dim+1);         //first coord is not a dim
    Vect eigs = C::calc_coords(m_lang, coords); //calculate coords
    Vect units(dim+1);
    for (Int i=0; i<dim+1; ++i) {
        units(i) = expf(eigs(i));               //converte eigs to units
    }
    coords *= units;                            //scale by units

    ofstream file("stats/default.map");
    if (!file) { logger.info() << "could not open map file" |0; return true; }

    logger.info() << "saving params" |0;
    file << "PARAMS\n"
         << Ob::size() << '\n'
         << App::size() << '\n'
         << Comp::size() << '\n'
         << P_app() << '\n'
         << P_comp() << '\n'
         << P_join() << '\n'
         << std::endl;

    //XXX needs to be updated for compositions
    logger.info() << "saving complexity and coords" |0;
    file << "COORDS\n";
    for (Ob::sparse_iterator I=Ob::sbegin(); I!=Ob::send(); ++I) { Ob ob = *I;
        file << comp(ob) << '\t';
        Int eqn = Int(EX::find_parse_app(ob));
        if (!eqn) eqn = Int(AE::find_app_eqn(*(LT::theory()->I), ob));
        file << eqn-1; //Careful: 1-based indexing
        for (Int i=0; i<1+dim; ++i) {
            file << '\t' << coords(ob)[i];
        }
        file << '\n';
    }
    file << std::endl;
    coords.free();

    logger.info() << "saving app equations" |0;
    file << "APPS\n";
    for (App::sparse_iterator iter=App::sbegin(), end=App::send();
            iter!=end; ++iter) { App eqn = *iter;
        //Careful: 1-based indexing
        file << get_app(eqn) - 1 << '\t'
             << get_lhs(eqn) - 1 << '\t'
             << get_rhs(eqn) - 1 << '\n';
    }
    file << std::endl;

    logger.info() << "saving comp equations" |0;
    file << "COMPS\n";
    for (Comp::sparse_iterator iter=Comp::sbegin(), end=Comp::send();
            iter!=end; ++iter) { Comp eqn = *iter;
        //Careful: 1-based indexing
        file << get_comp(eqn) - 1 << '\t'
             << get_lhs(eqn) - 1 << '\t'
             << get_rhs(eqn) - 1 << '\n';
    }
    file << std::endl;

    logger.info() << "saving join equations" |0;
    file << "JOINS\n";
    for (Join::sparse_iterator iter=Join::sbegin(), end=Join::send();
            iter!=end; ++iter) { Join eqn = *iter;
        //Careful: 1-based indexing
        file << get_join(eqn) - 1 << '\t'
             << get_lhs(eqn) - 1 << '\t'
             << get_rhs(eqn) - 1 << '\n';
    }
    file << std::endl;

    logger.info() << "saving labels" |0;
    file << "LABELS\n";
    for (Ob::sparse_iterator I=Ob::sbegin(); I!=Ob::send(); ++I) { Ob ob = *I;
        file << EX::parse_ob(ob) << '\n';
    }
    file << std::endl;

    logger.info() << "saving pretty labels" |0;
    file << "PRETTY\n";
    for (Ob::sparse_iterator I=Ob::sbegin(); I!=Ob::send(); ++I) { Ob ob = *I;
        file << EX::parse_ob(ob, true)->pretty() << '\n';
    }
    file << std::endl;

    file.close();
    logger.info() << "done." |0;
    return false;
}
bool Brain::send_lang_to (string address)
{
    logger.info() << "Sending language to server" |0;
    Logging::IndentBlock block;

    SocketTools::Client client(address);
    if (not client) {
        logger.error() << "could not connect to server" |0;;
        return false;
    }

    client.write("set app");
    client.write(_2string(-log(get_P_app())));

    for (Int i=0; i<lang().size(); ++i) {
        client.write("set mass");
        ExprHdl expr = EX::parse_ob(lang().atom(i));
        client.write(expr->str());
        client.write(_2string(-log(lang().mass(i))));
        client.read(); //ignore messages
    }

    bool result = client;
    client.write("exit");
    return result;
}
class ParseCache
{
    MAP_TYPE<Int, ExprHdl> m_cache;
public:
    ExprHdl operator() (Ob ob) const { return m_cache.find(Int(ob))->second; }
    ParseCache ()
    {
        for (Ob::sparse_iterator i=Ob::sbegin(); i!=Ob::send(); ++i) {
            Ob ob = *i;
            ExprHdl e = EX::parse_ob(ob);
            m_cache[Int(ob)] = e ? e->reduce() : e;
        }
    }
};
int Brain::send_eqns_to (string address)
{
    logger.info() << "Sending equations to server" |0;
    Logging::IndentBlock block;

    logger.info() << "caching ob parses" |0;
    ParseCache parse;

    SocketTools::Client client(address);
    if (not client) {
        logger.error() << "could not connect to server" |0;;
        return 0;
    }

    logger.info() << "sending app equations" |0;
    unsigned a_sent=0;
    for (App::iterator iter=App::begin(); iter!=App::end(); ++iter) {
        App eqn = *iter;
        Ob a = get_app(eqn);
        Ob l = get_lhs(eqn);
        Ob r = get_rhs(eqn);

        ExprHdl app = parse(a); if (!app) continue;
        ExprHdl lhs = parse(l); if (!lhs) continue;
        ExprHdl rhs = parse(r); if (!rhs) continue;
        ExprHdl l_r = (lhs * rhs)->reduce(); if (l_r == app) continue;

        if (!client) {
            logger.error() << "server disconnected" |0;
            break;
        }
        client.write("assume");
        client.write(l_r->str());
        client.write(app->str());
        client.read(); //ignore messages
        ++a_sent;
    }

    logger.info() << "sending comp equations" |0;
    unsigned c_sent=0;
    for (Comp::iterator iter=Comp::begin(); iter!=Comp::end(); ++iter) {
        Comp eqn = *iter;
        Ob c = get_comp(eqn);
        Ob l = get_lhs(eqn);
        Ob r = get_rhs(eqn);

        ExprHdl cmp = parse(c); if (!cmp) continue;
        ExprHdl lhs = parse(l); if (!lhs) continue;
        ExprHdl rhs = parse(r); if (!rhs) continue;
        ExprHdl l_r = (lhs % rhs)->reduce(); if (l_r == cmp) continue;

        if (!client) {
            logger.error() << "server disconnected" |0;
            break;
        }
        client.write("assume");
        client.write(l_r->str());
        client.write(cmp->str());
        client.read(); //ignore messages
        ++c_sent;
    }

    logger.info() << "sending join equations" |0;
    unsigned j_sent=0;
    for (Join::iterator iter=Join::begin(); iter!=Join::end(); ++iter) {
        Join eqn = *iter;
        Ob j = get_join(eqn);
        Ob l = get_lhs(eqn);
        Ob r = get_rhs(eqn);

        ExprHdl join = parse(j); if (!join) continue;
        ExprHdl lhs  = parse(l); if (!lhs) continue;
        ExprHdl rhs  = parse(r); if (!rhs) continue;
        ExprHdl l_r = (lhs | rhs)->reduce(); if (l_r == join) continue;

        if (!client) {
            logger.error() << "server disconnected" |0;
            break;
        }
        client.write("assume");
        client.write(l_r->str());
        client.write(join->str());
        client.read(); //ignore messages
        ++j_sent;
    }

    client.write("exit");
    logger.info() << "sent " << a_sent << " apps + "
                             << c_sent << " comps + "
                             << j_sent << " comps" |0;
    return a_sent + c_sent + j_sent;
}

//======================== brain class ========================

Brain* Brain::s_unique_instance = NULL; //unique instance
Brain::Brain (LT::MagmaTheory* theory)
    : m_theory(theory),
      m_am_alive(false),
      m_lang(),
      m_guessing(false),
      m_guess(NULL),
      m_client(1),
      m_granularity(32),
      m_time_scale(1000),
      m_clock(0),
      m_age(0),
      m_density(1.0),
      m_beta(1.0),
      m_elegance(-2.0),
      m_interest(NULL),
      m_motive(NULL),
      m_memory(NULL)
{
    logger.debug() << "starting brain" |0;
    Assert (s_unique_instance == NULL,
            "tried to start a second brain");
    s_unique_instance = this;
}
Brain::~Brain ()
{
    if (am_alive()) clear();
    delete m_theory;
    s_unique_instance = NULL;
}
void Brain::initialize (Int num_obs, const BasisType& basis)
{
    logger.info() << "Initializing brain" |0;
    Logging::IndentBlock block;

    //init comb. struct.
    CS::initialize(num_obs);
    m_measures.initialize(STATIC_MEASURES);
    for (BasisType::const_iterator i=basis.begin(); i!=basis.end(); ++i) {
        CS::make_atom(*i);
    }

    _init_misc();
}
void Brain::_init_misc (bool init_lang)
{
    m_theory->init_basis();
    EX::initialize(CS::get_atom_names());
    CS::saturate();

    //set thought, memory type
    set_size();
    think_about_everything(false);
    m_memory = new T::TwoLevelMemory(m_time_scale, 0.3);
    init_basis_measure(init_lang);

    log_params();

    start_living();
}
void Brain::clear ()
{
    logger.info() << "Clearing brain" |0;
    Logging::IndentBlock block;
    Assert (am_alive(), "tried to clear dead brain");
    stop_living();

    cleanup();
    if (g_generator) clear_generator();
    if (m_guessing)  stop_guessing();
    if (m_interest)  { delete m_interest;  m_interest  = NULL; }
    if (m_motive)    { delete m_motive;    m_motive    = NULL; }
    if (m_memory)    { delete m_memory;    m_memory    = NULL; }
    m_measures.clear();

    m_theory->clear();
    EX::clear();
    CS::clear();
    m_lang.clear();
}
void Brain::resize (Int size, const Int* new2old)
{
    logger.debug() << "resizing brain" |0;

    m_measures.resize(size, new2old);
    if (g_generator) g_generator->update_all();

    if (m_guessing) {
        Fuzzy::Order *_new = m_guess->resized(size, new2old);
        delete m_guess;
        m_guess = _new;
    }
}

//saving & loading
bool Brain::save (string filename)
{
    logger.info() << "Saving db to " << filename << " ..." |0;
    Logging::IndentBlock block;

    using namespace Files;

    //compact structure before anything else
    compact();

    //prepare header
    //    calculate data sizes
    Int b_size = CS::num_atoms();
    Int o_size = Ob::size();
    Int a_size = App::size();
    Int c_size = Comp::size();
    Int j_size = Join::size();
    Int w_size = m_lang.num_words();
    Int r_size = m_lang.num_rules();

    const Int START_OFFSET = bytes2blocks(sizeof(StructFileHeader));
    Int offset = START_OFFSET;

    Int o_data = offset; offset += bytes2blocks( O::data_size());
    Int b_data = offset; offset += bytes2blocks(CS::data_size());
    Int a_data = offset; offset += bytes2blocks( AE::data_size());
    Int c_data = offset; offset += bytes2blocks( CE::data_size());
    Int j_data = offset; offset += bytes2blocks( JE::data_size());
    Int l_data = offset; offset += bytes2blocks(OR::data_size());
    Int L_data = offset; offset += bytes2blocks(m_lang.data_size());

    //    construct header & validate
    StructFileHeader header(
            VERSION,
            m_age,
            b_size, o_size, a_size, c_size, j_size, w_size, r_size,
            b_data, o_data, a_data, c_data, j_data, l_data, L_data);
    header.validate();

    //open file
    FILE* file = fopen(filename.c_str(), "wb");
    if (!file) {
        logger.warning() << "failed to open file " << filename
            << " for saving" |0;
        return false;
    }

    fseek_block(file, 0);       header.save_to_file(file);  fill_block(file);
    fseek_block(file, o_data);  O::save_to_file(file);      fill_block(file);
    fseek_block(file, b_data);  CS::save_to_file(file);     fill_block(file);
    fseek_block(file, a_data);  AE::save_to_file(file);     fill_block(file);
    fseek_block(file, c_data);  CE::save_to_file(file);     fill_block(file);
    fseek_block(file, j_data);  JE::save_to_file(file);     fill_block(file);
    fseek_block(file, l_data);  OR::save_to_file(file);     fill_block(file);
    fseek_block(file, L_data);  m_lang.save_to_file(file);  fill_block(file);

    fclose(file);

    logger.info() << "...done saving." |0;
    return true;
}
bool Brain::load (string filename)
{
    logger.info() << "Loading db from " << filename << ": ..." |0;
    Logging::IndentBlock block;

    using namespace Files;

    //open file
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file) {
        logger.warning() << "failed to open file " << filename
            << " for loading" |0;
        return false;
    }

    //read header
    StructFileHeader header;
    //fseek_block(file, 0);
    header.load_from_file(file);
    start_validating();
    header.validate();
    if (not everything_valid()) {
        logger.warning() << "db file " << filename << " had invalid header" |0;
        fclose(file);
        return false;
    }

    if (am_alive()) clear();

    //set clock
    m_age = header.age;

    //set up combinatory structure
    CS::initialize(header.o_size, header.a_size, header.c_size, header.j_size,
            true);
    m_measures.initialize(STATIC_MEASURES);

    //load data
    fseek_block(file, header.o_data);   O::load_from_file(file);
    fseek_block(file, header.b_data);  CS::load_from_file(file, header.b_size);
    fseek_block(file, header.a_data);   AE::load_from_file(file);
    fseek_block(file, header.c_data);   CE::load_from_file(file);
    fseek_block(file, header.j_data);   JE::load_from_file(file);
    fseek_block(file, header.l_data);  OR::load_from_file(file);
    fseek_block(file, header.L_data);
        m_lang.load_from_file(file, header.w_size, header.r_size);

    fclose(file);

    _init_misc(false);

    logger.info() << "...done loading." |0;
    return true;
}

//shaping
bool Brain::expand_to (Int target)
{//incrementally expands by a constant ratio until full
    if (target <= Ob::size()) return true; //nothing to do
    logger.info() << "Incrementally expanding" |0;
    Logging::IndentBlock block;
    bool success = true;

    calc_measures(false);
    while (Ob::size() < target) {
        Int start_size = Ob::size();
        Int sub_target = min(target, start_size + m_granularity);
        sub_target = min(sub_target, 2 * Ob::size()); //when very small
        if (not m_motive->expand_to(sub_target)) {
        //if (Ob::size() == start_size) {
            logger.warning() << "expansion failed :(" |0;
            success = false;
            break;
        }
        CS::log_stats(m_age);
        calc_measures(false);
    }

#if DEBUG_LEVEL >= 4
    validate();
#endif

    return success;
}
bool Brain::contract_to (Int target)
{
    while (Ob::size() > target) {
        calc_measures(false);
        Int sub_target = max(target, Ob::size() - m_granularity);
        if (m_memory->contract_to(sub_target)) CS::log_stats(m_age);
        else return false;
#if DEBUG_LEVEL >= 4
        validate();
#endif
    }
    return true;
}
void Brain::compact (bool sort_eqns)
{
    logger.info() << ( sort_eqns ? "Compacting (sorting eqns)"
                                 : "Compacting" ) |0;
    Logging::IndentBlock block;

    //compress & sort obs
    calc_measures(false);
    Ob::array<Float> o_rank;
    o_rank = comp;
    const ORing o_order(o_rank);

    //compress & sort eqns
    if (sort_eqns) {
        //app equations
        App::array<Int> *a_rank = new(std::nothrow) App::array<Int>;
        Assert (a_rank, "no memory for app sorting");
        for (App::sparse_iterator iter=App::sbegin(), end=App::send();
                iter!=end; ++iter) { App eqn = *iter;
            Int lhs = o_order.old2new(get_lhs(eqn));
            Int rhs = o_order.old2new(get_rhs(eqn));
            (*a_rank)(eqn) = lhs | (rhs << 16);
        }
        const ARing a_order(*a_rank);
        delete a_rank;

        //comp equations
        Comp::array<Int> *c_rank = new(std::nothrow) Comp::array<Int>;
        Assert (c_rank, "no memory for comp sorting");
        for (Comp::sparse_iterator iter=Comp::sbegin(), end=Comp::send();
                iter!=end; ++iter) { Comp eqn = *iter;
            Int lhs = o_order.old2new(get_lhs(eqn));
            Int rhs = o_order.old2new(get_rhs(eqn));
            (*c_rank)(eqn) = lhs | (rhs << 16);
        }
        const CRing c_order(*c_rank);
        delete c_rank;

        //join equations
        Join::array<Int> *j_rank = new(std::nothrow) Join::array<Int>;
        Assert (c_rank, "no memory for join sorting");
        for (Join::sparse_iterator iter=Join::sbegin(), end=Join::send();
                iter!=end; ++iter) { Join eqn = *iter;
            Int lhs = o_order.old2new(get_lhs(eqn));
            Int rhs = o_order.old2new(get_rhs(eqn));
            (*j_rank)(eqn) = lhs | (rhs << 16);
        }
        const JRing j_order(*j_rank);
        delete j_rank;

        CS::compact(o_order, a_order, c_order, j_order);
    }

    //just compress eqns
    else {
        const ARing a_order;
        const CRing c_order;
        const JRing j_order;
        CS::compact(o_order, a_order, c_order, j_order);
    }

    //shrink measure memory usage
    m_measures.shrink();

#if DEBUG_LEVEL >= 4
    validate();
#endif
}

//thought control
void Brain::set_size (Int s)
{
    if (s == 0) s = Ob::size();
    logger.info() << "resizing brain to " << s << " concepts" |0;
    m_min_size = s;
    m_max_size = s + m_granularity;
}
void Brain::set_granularity (Int g)
{
    if (g < 1) {
        logger.warning() << "granularity too small; setting to 1" |0;
        g = 1;
    }
    if (g > 256) {
        logger.warning() << "granularity too large; setting to 256" |0;
        g = 256;
    }
    m_granularity = g;
    m_max_size = m_min_size + m_granularity;
}
const Float MIN_TEMP = 1e-2f, MAX_TEMP = 1e2f;//, TEMP_STEP = expf(0.1f);
const Float MIN_BETA = 1e-2f, MAX_BETA = 1e2f, BETA_STEP = expf(-0.1f);
void Brain::set_temperature (Float t)
{
    imax(t, MIN_TEMP);
    imin(t, MAX_TEMP);
    m_beta = 1.0 / t;
}
bool Brain::raise_temp ()
{
    if (m_beta <= MIN_BETA) return false;
    m_beta *= BETA_STEP;
    imax(m_beta, MIN_BETA);
    return true;
}
bool Brain::lower_temp ()
{
    if (m_beta >= MAX_BETA) return false;
    m_beta /= BETA_STEP;
    imin(m_beta, MAX_BETA);
    return true;
}
void Brain::set_P_ (Float P_app, Float P_comp, Float P_join)
{
    m_lang.set_P_app(P_app);
    m_lang.set_P_comp(P_comp);
    m_lang.set_P_join(P_join);
    m_lang.update();
    calc_measures(false);
}
void Brain::set_P_app (Float P_app)
{
    m_lang.set_P_app(P_app);
    m_lang.update();
    calc_measures(false);
}
void Brain::set_P_comp (Float P_comp)
{
    m_lang.set_P_comp(P_comp);
    m_lang.update();
    calc_measures(false);
}
void Brain::set_P_join (Float P_join)
{
    m_lang.set_P_join(P_join);
    m_lang.update();
    calc_measures(false);
}
void Brain::think_in (Float P_app, Float P_comp, Float P_join)
{
    const std::vector<ObHdl>& all = CS::get_basis();
    std::vector<ObHdl> selected;
    for (Int i=0; i<all.size(); ++i) {
        const ObHdl& ob = all[i];
        if (*ob == *LT::Atoms::Bot) continue;
        if (*ob == *LT::Atoms::Top) continue;
        selected.push_back(ob);
    }

    think_in(selected, P_app, P_comp, P_join);
}
void Brain::think_in (const std::vector<ObHdl>& obs,
        Float P_app, Float P_comp, Float P_join)
{//thinks about given lexicon, or CS::basis if lexicon is empty
    if (obs.empty()) { think_in(); return; }

    //set basis
    basis().clear();
    Float atom_size = 1.0; //all the same
    for (Int i=0; i<obs.size(); ++i) {
        basis().push_back(std::make_pair(obs[i], atom_size));
    }

    set_P_(P_app, P_comp, P_join);
}
void Brain::think_in (ObPMF& obs, Float P_app, Float P_comp, Float P_join)
{//thinks about given lexicon, or CS::basis if lexicon is empty
    if (obs.empty()) { think_in(); return; }

    //set basis
    m_lang.clear();
    for (Int i=0; i<obs.size(); ++i) {
        ObHdl ob = obs[i].first;
        Float mass = expf(-obs[i].second);  //mass is exp(-size)
        basis().push_back(std::make_pair(ob,mass));
    }

    set_P_(P_app, P_comp, P_join);    //updates language
}
void Brain::think_about (Thought::Interest *interest,
                         Thought::Motive *motive,
                         bool resting)
{
    if (m_interest != NULL) delete m_interest; m_interest = interest;
    if (m_motive   != NULL) delete m_motive;   m_motive   = motive;
    if (resting) rest();
}
void Brain::think_about_exprs (std::vector<ExprHdl> exprs)
{
    think_about(new T::Interest_in_exprs(exprs),
                new T::Motive_to_ponder(comp));
}
void Brain::think_about_context (Context& context)
{
    //this keeps a little objectivity
    //WARNING: make sure subjective is first, as it alone calibrates language
    T::Interest* subjective = new T::Interest_in_context(context);
    T::Interest* objective  = new T::Interest_in_meas(comp);
    think_about(new T::MixedInterests(0.5f, subjective, 0.5f, objective),
                new T::Motive_to_ponder(comp));
}
void Brain::think_about_theory ()
{
    //this keeps a little objectivity
    //WARNING: make sure subjective is first, as it alone calibrates language
    T::Interest* subjective = new T::Interest_in_theory();
    T::Interest* objective  = new T::Interest_in_meas(comp);
    think_about(new T::MixedInterests(0.5f, subjective, 0.5f, objective),
                new T::Motive_to_ponder(comp));
}
void Brain::think_about_everything (bool resting)
{//thinks WRT complexity measure
    think_about(new T::Interest_in_meas(comp),
                new T::Motive_to_ponder(comp),
                resting);
}
bool Brain::think (Int num_cycles)
{//incrementally contracts & expands (by some granularity)
    logger.debug() << "Thinking for " << num_cycles |0;
    Logging::IndentBlock block;

    if (not contract_to(m_min_size)) return false;
    for (Int i=0; i<num_cycles; ++i) {
        if (not expand_to(m_max_size)) return false;
        if (not contract_to(m_min_size)) return false;
    }
    return true;
}
bool Brain::think ()
{//minimal thought
    bool result;

    //add an ob if below max_size
    if (Ob::size() < m_max_size) {
        if ((m_max_size - Ob::size()) % (1+m_granularity) == 0) {
            calc_measures(false);
            CS::log_stats(m_age);
        }
        result = m_motive->expand();
    }

    //otherwise contract to min_size
    else {
        logger.info() << "contracting from " << m_max_size
            << " to " << m_min_size
            << " at beta = " << m_beta |0;
        m_motive->cleanup();
        calc_measures(false);
        Int target = max(m_min_size, Ob::size() - m_granularity);
        result = m_memory->contract_to(target);

        //if density is sufficiently high, increase size
        Float density = CS::log_stats(m_age);
        if (density > m_density) {
            logger.info() << "desired density met ("
                << density << " > " << m_density
                << "); increasing size" |0;
            set_size(m_min_size + m_granularity);
        }
    }

    return result;
}
void Brain::update (bool tryhard)
{
    m_interest->update(tryhard);
    calc_measures(false);
}

//resting, every ~1000 ob deletions
void Brain::_tick_tock ()
{//advance clock
    ++m_age;
    if (m_clock < m_time_scale) ++m_clock;
}
void Brain::_rest () { m_clock = m_time_scale; }
bool Brain::time_to_rest ()
{//whether to take a nap
    if (m_clock < m_time_scale) return false;
    m_clock = m_time_scale - m_granularity;
    return true;
}
void Brain::rest ()
{//performs slow maintenance operations
    //beforehand kernel calls update(true);
    calc_measures(true);
    m_clock = 0;
    //afterwards kernel calls update();
}

//measure operations
void Brain::init_basis_measure (bool init_lang)
{
    logger.info() << "Initializing basis measure" |0;
    Logging::IndentBlock block;

    //define language
    std::vector<ObHdl> atoms = CS::get_basis();
    Assert1 (!atoms.empty(), "tried to initialize measures with empty basis");

    //set basis probabilities for atoms
    Float mass = 1.0 / atoms.size();
    for (Int i=0; i<atoms.size(); ++i) {
        Ob ob = *(atoms[i]);
        comp(ob) = mass;
        komp(ob) = -log(mass);
        rel (ob) = mass;
    }

    if (init_lang) think_in();
    calc_measures(true);
}
void Brain::calc_measures (bool optimize)
{
    if (optimize) {
        logger.info() << "Optimizing measures" |0;
        Logging::IndentBlock block;

        m_interest->calibrate_lang();

        if (guessing()) calc_P();
        else            C::calc_Z(m_lang);
        C::calc_R(m_lang, *m_interest);

        m_memory->remember_this();

        logger.info()
            << "P_app = " << P_app()
            << ", P_comp = " << P_comp()
            << ", P_join = " << P_join()
            << ", Z(A) = 1 - " << 1.0 - get_total()
            << ", N = " << get_perplexity() |0;
    } else {
        C::calc_Z(m_lang);
        C::calc_R(m_lang, *m_interest);
    }
    C::calc_ZHA(m_lang);
}
void Brain::calc_P ()
{//creates/updates fuzzy order relation
    Assert (m_guessing, "tried to guess while not guessing");
    logger.info() << "Calculating [= complexity" |0;
    Logging::IndentBlock block;

    //calculate relevant measures
    if (not Ob::full()) compact();

    //propagate evidence
    m_lang.normalize_rules(); //to clean up
    C::calc_P(m_lang, *m_guess);
}
Float Brain::calc_cost ()
{
    calc_measures(true); //this should be as accurate as possible
    Float cost = m_interest->get_cost();
    if (std::isfinite(cost)) return cost;
    else                     return INFINITY;
}

//guessing information
void Brain::start_guessing ()
{
    if (m_guessing) return;
    m_guessing = true;
    m_guess = new Fuzzy::Order(Ob::numUsed());
    calc_P(); //update
}
void Brain::stop_guessing ()
{
    if (not m_guessing) return;
    m_guessing = false;
    delete m_guess;
    m_guess = NULL;
}
void Brain::set_guessing (bool whether)
{
    logger.info() << (whether ? "starting guessing" : "stopping guessing") |0;
    Logging::IndentBlock block;

    if (whether and not m_guessing) start_guessing();
    if (m_guessing and not whether) stop_guessing();
}
Prob Brain::guess_less (Ob lhs, Ob rhs)
{//fuzzy estimation of P(lhs [= rhs)
    return m_guessing ? 1.0 - m_guess->data(lhs,rhs) : Symbols::P_FALSE;
}
class Guesser : public ST::Guesser
{
public:
    virtual ~Guesser () {}
    virtual Prob less (ExprHdl lhs, ExprHdl rhs)
    {
        ObHdl lhs_ob = EX::get_expr(lhs->reduce());
        ObHdl rhs_ob = EX::get_expr(rhs->reduce());
        if (not (lhs_ob and rhs_ob)) return P_FALSE;
        return brain().guess_less (*lhs_ob, *rhs_ob);
    }
    virtual Prob equal (ExprHdl lhs, ExprHdl rhs)
    {
        ObHdl lhs_ob = EX::get_expr(lhs->reduce());
        ObHdl rhs_ob = EX::get_expr(rhs->reduce());
        if (not (lhs_ob and rhs_ob)) return P_FALSE;
        return And(brain().guess_less (*lhs_ob, *rhs_ob),
                   brain().guess_less (*rhs_ob, *lhs_ob));
    }
};
Guesser guesser;
Prob Brain::guess (StmtHdl s)
{
    return s->guess(guesser);
}

//measure diagnostics
class LessMass_
{//ordered by a measure
    const Meas m_mass;
public:
    LessMass_ () : m_mass(0) { Error("default-constructed a LessMass_"); }
    LessMass_ (Meas mass) : m_mass(mass) {}
    bool operator() (Ob lhs, Ob rhs) { return m_mass(lhs) < m_mass(rhs); }
};
typedef nonstd::N_Best<Ob,LessMass_> LeastMass;
std::vector<std::pair<Ob,Float> > Brain::get_simplest (Int N)
{
    //find best
    LessMass_ less(comp);
    LeastMass best(N, less, Ob::sbegin(), Ob::send());
    std::vector<Ob> obs(best.begin(), best.end());

    //collect masses
    N = obs.size(); //fewer may have been found
    std::vector<std::pair<Ob,Float> > result(N,std::make_pair(Ob(0),0.0));
    for (Int i=0; i<obs.size(); ++i) {
        Ob ob = obs[i];
        result[i].first = ob;
        result[i].second = -log(comp(ob));
    }
    return result;
}
std::vector<std::pair<Ob,Float> > Brain::get_most_relevant (Int N)
{
    logger.debug() << "finding " << N << " most relevant obs" |0;
    //define relevance(x) = rel(x) / comp(x)
    Meas relevance = Meas::alloc();
    M::calc_ratio (rel, comp, relevance);
    LessMass_ less(relevance);

    //the best object should only exist while relevence does
    LeastMass* best = new LeastMass(N, less, Ob::sbegin(), Ob::send());
    std::vector<Ob> obs(best->begin(), best->end());
    delete best;

    //collect masses while they're around
    N = obs.size(); //fewer may have been found
    std::vector<std::pair<Ob,Float> > result(N,std::make_pair(Ob(0),0.0));
    for (Int i=0; i<N; ++i) {
        Ob ob = obs[i];
        result[i].first = ob;
        result[i].second = relevance(ob);
    }

    relevance.free();
    return result;
}
std::vector<std::pair<Ob,Float> > Brain::get_sketchiest (Int N)
{
    //find sketchiest obs
    Meas sketch = Meas::alloc();
    C::calc_Q(sketch);
    LessMass_ less(sketch);

    //the best object should only exist while relevence does
    LeastMass* best = new LeastMass(N, less, Ob::sbegin(), Ob::send());
    std::vector<Ob> obs(best->begin(), best->end());

    //collect masses while they're around
    N = obs.size(); //fewer may have been found
    std::vector<std::pair<Ob,Float> > result(N,std::make_pair(Ob(0),0.0));
    for (Int i=0; i<N; ++i) {
        Ob ob = obs[i];
        result[i].first = ob;
        result[i].second = sketch(ob);
    }
    delete best;

    sketch.free();
    return result;
}
struct LessEvidence_
{
    typedef Conjecture Item;
    bool operator() (Item x, Item y)
    {
        return (x.evidence < y.evidence)
            or (x.evidence == y.evidence
                    and (x.lhs < y.lhs
                     or (x.lhs == y.lhs and x.rhs < y.rhs)));
    }
};
std::vector<Conjecture> Brain::get_conjectures (Int N)
{//generates conjectures from available data
    logger.info() << "Generating conjectures" |0;
    Logging::IndentBlock block;

    //propagate information
    bool was_guessing = m_guessing; //otherwise, delete m_guess when done
    if (was_guessing) calc_P();
    else              start_guessing();

    //find the N (x[=y)-pairs with the most supporting evidence
    logger.info() << "finding " << N
        << " [=-relations with most supporting evidence" |0;
    nonstd::N_Best<Conjecture, LessEvidence_> best(N);
    Int num_obs = Ob::numUsed();
    for (Int _x=1; _x<=num_obs; ++_x) { Ob x(_x);
    for (Int _y=1; _y<=num_obs; ++_y) { Ob y(_y);
    if (not OR::contains(x,y)) {
        best.insert(Conjecture(x, y, m_guess->data(x,y)));
    }}}

    //clean up
    if (not was_guessing) stop_guessing();
    return std::vector<Conjecture>(best.begin(), best.end());
}
inline Float safe_log (Float t) { return std::isfinite(t) ? log(t) : 0.0; }
bool Brain::vis_ob_mass (Int size, Int ratio)
{//both on a log scale
    logger.info() << "visualizing ob masses to stats/ob_mass.png" |0;

    Int N = Ob::numUsed();
    Int M = (N + ratio - 1) / ratio;

    //find scales
    calc_measures(false);
    Float max_comp = -INFINITY;
    Float max_rel  = -INFINITY;
    Ob::sparse_iterator end = Ob::send();
    for (Ob::sparse_iterator I=Ob::sbegin(); I!=end; ++I) { Ob ob = *I;
        if (not (comp(ob) > 0)) continue;
        Float c = -safe_log(comp(ob));  imax(max_comp, c);
        Float r = -safe_log(rel(ob));   imax(max_rel, r);
    }

    //draw image
    V::ColorImage image(N,M);
    image.set(0);
    Int i=0;
    for (Ob::sparse_iterator I=Ob::sbegin(); I!=end; ++I) { Ob ob = *I;

        //check for bad mass
        if (not (comp(ob) > 0)) {
            Int inCore = O::isInCore(ob) ? 0 : 255;
            for (Int j=0; j<M; ++j) {
                image(i,j,V::RED)   = 255;
                image(i,j,V::GREEN) = inCore;
                image(i,j,V::BLUE)  = 0;
            }
        }

        //draw mass
        Float c = -safe_log(comp(ob)) / max_comp;
        Int height = V::round((M-1) * c);
        Int inCore = O::isInCore(ob) ? 0 : 255;
        for (Int j=0; j<height; ++j) { Int k = M-j-1;
            image(i,k,V::RED)   = 255;
            image(i,k,V::GREEN) = inCore;
        }

        //draw relevance
        Float r = -safe_log(rel(ob)) / max_rel;
        height = V::round((M-1) * r);
        for (Int j=0; j<height; ++j) { Int k = M-j-1;
            image(i,k,V::BLUE) = 255;
        }

        ++i;
    }

    //save full-size version
    if (not image.save("stats/ob_mass")) return false;

    //save scaled version
    if (size == 0) return image.save("stats/ob_mass_small");
    V::ColorImage scaled = image.shrink((N+size-1)/size);
    return scaled.save("stats/ob_mass_small");
}

//language development
bool Brain::retract (ObHdl ob)
{
    if (not m_lang.remove(ob)) return false;
    logger.info() << "removing concept from language: " << EX::parse_ob(*ob) |0;
    calc_measures(true);
    return true;
}
bool Brain::extend (ObHdl ob)
{
    if (not m_lang.insert(ob)) return false;
    logger.info() << "extending language with: " << EX::parse_ob(*ob) |0;
    calc_measures(true);
    return true;
}
bool Brain::extend (std::vector<ObHdl> obs)
{
    bool extended = false;
    for (Int i=0; i<obs.size(); ++i) {
        ObHdl ob = obs[i];
        if (not m_lang.insert(ob)) continue;
        logger.info() << "extending language with: " << EX::parse_ob(*ob) |0;
    }
    if (not extended) return false;

    calc_measures(true);
    return true;
}

//ob primitives
void Brain::create_ob (Ob ob)
{
    comp(ob) = 0.0;
    komp(ob) = INFINITY;
    rel(ob) = 0.0;
    m_measures.create_ob(ob);
    if (g_generator) g_generator->insert(ob);
    if (m_guessing) m_guess->insert(ob);
}
void Brain::create_app (App eqn)
{
    Ob lhs = get_lhs(eqn);
    Ob rhs = get_rhs(eqn);
    Ob app = get_app(eqn);
         comp(app) +=     P_app()  * comp(lhs) * comp(rhs);
    imin(komp(app) , -log(P_app()) + komp(lhs) + komp(rhs));
    if (g_generator) g_generator->update(app); //sometimes unnecessary
}
void Brain::create_comp (Comp eqn)
{
    Ob lhs = get_lhs(eqn);
    Ob rhs = get_rhs(eqn);
    Ob cmp = get_comp(eqn);
         comp(cmp) +=     P_comp()  * comp(lhs) * comp(rhs);
    imin(komp(cmp) , -log(P_comp()) + komp(lhs) + komp(rhs));
    if (g_generator) g_generator->update(cmp); //sometimes unnecessary
}
void Brain::create_join (Join eqn)
{
    Ob lhs = get_lhs(eqn);
    Ob rhs = get_rhs(eqn);
    Ob join = get_join(eqn);
         comp(join) +=     P_join()  * comp(lhs) * comp(rhs);
    imin(komp(join) , -log(P_join()) + komp(lhs) + komp(rhs));
    if (g_generator) g_generator->update(join); //sometimes unnecessary
}
void Brain::merge_obs (Ob dep, Ob rep)
{
         comp(rep) += comp(dep);   comp(dep) = 0.0;
    imin(komp(rep) ,  komp(dep));  komp(dep) = INFINITY;
    m_measures.merge_obs(dep,rep);
    if (g_generator) g_generator->merge(dep, rep);
    if (m_guessing) m_guess->merge(dep,rep);
}
void Brain::delete_ob (Ob ob)
{
    _tick_tock();
    comp(ob) = 0.0;
    komp(ob) = INFINITY;
    rel(ob) = 0.0;
    if (g_generator) g_generator->remove(ob);
    if (m_guessing) m_guess->remove(ob);
}

//diagnostics
typedef std::pair<ObHdl, Float> ObWMass;
struct MoreMass_
{
    bool operator() (const ObWMass& lhs, const ObWMass& rhs)
    { return lhs.second > rhs.second; }
};
ostream& operator<< (ostream& os, ObPMF pmf)
{
    //sort first
    std::sort(pmf.begin(), pmf.end(), MoreMass_());

    os << "\n\t\t";
    Float offset = 1.0 + log(pmf[0].second);
    os << EX::parse_ob(*(pmf[0].first)) << "\t@ " << 1.0;
    for (Int i=1; i<pmf.size(); ++i) {
        os << ",\n\t\t" << EX::parse_ob(*(pmf[i].first))
           << "\t@ "    << offset - log(pmf[i].second);
    }
    os << ".";
    return os;
}
void Brain::write_lang_to (ostream& os)
{
    os << "\t!set P_app = " << P_app();
    os << "\n\t!set P_comp = " << P_comp();
    os << "\n\t!set P_join = " << P_join();
    os << "\n\t!think in " << m_lang.basis()
       << " # perplexity " << m_lang.perplexity();
    //os << "\n\tinference rules = {";
    //for (RulePMF::const_iterator r=rules().begin(); r!=rules().end(); ++r) {
    //    os << "\n\t\t" << Symbols::PropRuleNames[r->first]
    //       << "\t@ " << -log(r->second) << ',';
    //}
    //os << "\n\t}";
    os << '\n';

    m_interest->write_params_to(os);
    m_motive->write_params_to(os);
    m_memory->write_params_to(os);
}
void Brain::write_stats_to (ostream& os)
{
    C::write_stats_to(os);
    m_interest->write_stats_to(os);
    m_memory->write_stats_to(os);
    os << "\t" << (100.0 - (100.0 *m_clock) / m_time_scale) << "\% awake, ";
    os << m_age << " obs old";
    os << std::endl;
}
void Brain::save_params_to (ostream& os)
{
    os << "#\\subsection{Brain Parameters}\n\n";
    os << "!set size = "        << Ob::size()       << "\n"; //fit to size
    os << "!set granularity = " << m_granularity    << "\n";
    os << "!set time-scale = "  << m_time_scale     << "\n";
    os << "!set density = "     << m_density        << "\n";
    os << "!set temperature = " << (1.0 / m_beta)  << "\n";
    os << "!set elegance = "    << m_elegance       << "\n";
    m_interest -> save_params_to(os);
    m_memory   -> save_params_to(os);
    os << "\n";
}
void Brain::write_params_to (ostream& os)
{
    os << "\tbrain size = [" << m_min_size << " < " << m_max_size << "]";
    os << "\n\ttime-scale = " << m_time_scale;
    os << "\n\tdensity = " << m_density;
    os << "\n\ttemperature = " << (1.0 / m_beta);
    os << "\n\telegance = " << m_elegance;
    os << "\n\t" << (m_guessing ? "guessing" : "not guessing");
    os << "\n";

    m_interest->write_params_to(os);
    m_motive->write_params_to(os);
    m_memory->write_params_to(os);
}
void Brain::log_params ()
{
    const Logging::fake_ostream& log = logger.info() << "Brain parameters:";
    log << "\n\tP_app = "  << P_app();
    log << ", P_comp = " << P_comp();
    log << ", P_join = " << P_join();
    log << ", Z(A) = " << C::get_ZA();
    log |0;
}

//debugging
void Brain::validate (Int level)
{
    logger.info() << "Validating brain (level " << level << ")" |0;
    Logging::IndentBlock block;

    CS::validate(level);
    LT::theory()->validate(level);

    if ((level >= 2) and g_generator) {
        //a bit abusive since o_pmf_r is an IntMeas...
        IntMeas o_pmf_r = IntMeas::alloc();
        g_generator->validate(o_pmf_r);
        o_pmf_r.free();
    }
}

//================ global functions ================

//globals linked to combinatory_structure
void extra_create_ob    (Ob ob)          { brain().create_ob(ob); }
void extra_create_app   (App eqn)        { brain().create_app(eqn); }
void extra_create_comp  (Comp eqn)       { brain().create_comp(eqn); }
void extra_create_join  (Join eqn)       { brain().create_join(eqn); }
void extra_delete_ob    (Ob ob)          { brain().delete_ob(ob); }
void extra_merge_obs    (Ob dep, Ob rep) { brain().merge_obs(dep, rep); }
void extra_resize (Int size, const Int* new2old)
{ brain().resize(size, new2old); }

}



