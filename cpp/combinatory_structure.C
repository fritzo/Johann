
#include "combinatory_structure.h"
#include "obs.h"
#include "apply.h"
#include "compose.h"
#include "join.h"
#include "order.h"
#include "lambda_theories.h"
#include "priority_queue.h"
//#include "fifo_queue.h"
#include "syntax_semantics.h"
#include "files.h"
#include "visual.h"
#include "version.h"
#include <set>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <utility> //for greater, pair

#ifdef __GNUG__
    #include <unordered_map>
    #define MAP_TYPE std::unordered_map

namespace std
{//hash template specialization for heap positions
template<> struct hash<App> {
    size_t operator() (App eqn) const { return hash<size_t>()(Int(eqn)); }
};
template<> struct hash<Ord> {
    size_t operator() (Ord ord) const {
        return hash<size_t>()( (Int(ord.lhs()) << 16)
                             | Int(ord.rhs())         );
    }
};
}
#else
    #include <map>
    #define MAP_TYPE std::map
#endif

//log levels
#define LOG_DEBUG1(mess);
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << mess |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

//thm processing tests
//#define TEST_PROCESSING
#ifdef TEST_PROCESSING
ofstream enforce_log("enforce.log", fstream::out | fstream::trunc);
#define ENFORCE_LOG(mess) enforce_log << mess;
#else
#define ENFORCE_LOG(mess)
#endif

//WARNING: these must match definitions in brain.C
namespace TheBrain
{
//these perform measure operations
extern void extra_create_ob    (Ob ob);
extern void extra_create_app   (App eqn);
extern void extra_create_comp  (Comp eqn);
extern void extra_create_join  (Join eqn);
extern void extra_delete_ob    (Ob ob);
extern void extra_merge_obs    (Ob dep, Ob rep);

//these maintain fuzzy information
extern void extra_resize (Int size, const Int* new2old=NULL);
}

namespace CombinatoryStructure
{

namespace O  = Obs;
namespace AE = Apply;
namespace CE = Compose;
namespace JE = JoinEqn;
namespace OR = Order;
namespace TB = TheBrain;
namespace LT = LambdaTheories;
namespace V  = Visual;

using LambdaTheories::theory;

using namespace Heap;

//internal printing
namespace EX = Expressions;
inline EX::ExprHdl expr (Ob ob) { return EX::parse_ob(ob); }

//properties & flags
bool g_is_initialized(false);
bool isInitialized () { return g_is_initialized; }
bool isCompacted   () { LATER(); return false; }

//stats log
ofstream stats_log;

//atom table for name lookup
//typedef MAP_TYPE<string, const ObHdl> AtomTable;
typedef MAP_TYPE<string, ObHdl> AtomTable; //GCC4 bitches
typedef MAP_TYPE<string, ObHdl> UnknTable;
AtomTable g_atomTable;
UnknTable g_unknTable;
Int num_atoms () { return g_atomTable.size(); }
Int num_unkns () { return g_unknTable.size(); }

//processing queues
//LATER: how should these be ordered? FIFO? priority?
typedef nonstd::priority_queue<Ob> ObQueue;
//typedef nonstd::fifo_queue<App> AppQueue;
typedef nonstd::priority_queue<App>  AppQueue;
typedef nonstd::priority_queue<Comp> CompQueue;
typedef nonstd::priority_queue<Join> JoinQueue;
typedef nonstd::priority_queue<Ord>  OrdQueue;
ObQueue    g_merge_queue;
ObQueue    g_enforce_O_queue;
AppQueue   g_enforce_A_queue;
CompQueue  g_enforce_C_queue;
JoinQueue  g_enforce_J_queue;
OrdQueue   g_enforce_L_queue;
OrdQueue   g_enforce_N_queue;

//======================== structure ========================

//queued merging & enforcement
void saturate ();
inline void enqueue_merge   (Ob   ob);
inline void enqueue_merge   (App  eqn);
inline void enqueue_merge   (Comp eqn);
inline void enqueue_merge   (Join eqn);
inline void enqueue_enforce (Ob   ob);
inline void enqueue_enforce (App  eqn);
inline void enqueue_enforce (Comp eqn);
inline void enqueue_enforce (Join eqn);
inline void enqueue_enforce_L (Ord ord);
inline void enqueue_enforce_N (Ord ord);
inline void merge_enforce (App  dep, App  rep);
inline void merge_enforce (Comp dep, Comp rep);
inline void merge_enforce (Join dep, Join rep);
       void merge_enforce (Ob   dep, Ob   rep);

//coarse-level structural operations
void initialize (Int num_obs, Int num_apps, Int num_comps, Int num_joins, bool is_full)
{
    bool apps_full = is_full and num_apps;
    bool comps_full = is_full and num_comps;
    bool joins_full = is_full and num_joins;
    if (not num_apps) num_apps = num_obs;
    if (not num_comps) num_comps = num_obs;
    if (not num_joins) num_joins = num_obs;
    logger.info() << "Initializing combinatory structure with "
        << num_obs << " obs, " << num_apps << " apps, "
                               << num_comps << " comps, "
                               << num_joins << " joins" |0;
    Logging::IndentBlock block;

    Assert (not isInitialized(), "tried to initialize c.s. twice");
    Assert (num_obs > 0, "tried to initialize CS with zero obs");

    g_is_initialized = true;

    AE::init(num_obs, num_apps, apps_full);
    CE::init(num_obs, num_comps, comps_full);
    JE::init(num_obs, num_joins, joins_full);
    Ob::init(num_obs, is_full);
    Ord::init(num_obs, is_full);

    //start log file
    stats_log.open("stats/cs_stats.log", std::ios_base::app);
    if (stats_log.is_open()) { if (not is_full) log_stats(); }
    else logger.warning() << "could not open cs_stats.log" |0;
}
std::vector<ObHdl> get_basis ()
{
    std::vector<ObHdl> result;
    for (AtomTable::iterator iter = g_atomTable.begin();
            iter!=g_atomTable.end(); ++iter) {
        result.push_back(iter->second);
    }
    return result;
}
void resize (Int num_obs)
{
    logger.info() << "Resizing combinatory structure to "
        << num_obs << " obs" |0;
    Logging::IndentBlock block;

    Ob::resize(num_obs);
    Ord::resize(num_obs);
    AE::resize(num_obs);
    CE::resize(num_obs);
    JE::resize(num_obs);
    TB::extra_resize(num_obs);

    if (DEBUG_LEVEL >= 5) {
        O::validate(3);
        AE::validate(3);
        CE::validate(3);
        JE::validate(3);
        OR::validate(3);
    }
}
void clear ()
{
    logger.info() << "Clearing combinatory structure" |0;
    Logging::IndentBlock block;
    Assert (isInitialized(), "tried to clear uninitialized structure");

    //stop log file
    if (stats_log.is_open()) { log_stats(); stats_log.close(); }

    g_atomTable.clear();
    g_unknTable.clear();

    Ord::clear();
    AE::clear();
    CE::clear();
    JE::clear();
    Ob::clear();

    g_is_initialized = false;
}
void validate (Int level)
{
    logger.debug() << "Validating combinatory structure" |0;
    Logging::IndentBlock block(logger.at_debug());
    Assert (isInitialized(), "tried to validate uninitialized comb. struct.");

    //local validations
    Assert (not g_merge_queue,     "invalid: g_merge_queue is not empty");
    Assert (not g_enforce_O_queue, "invalid: g_enforce_O_queue is not empty");
    Assert (not g_enforce_A_queue, "invalid: g_enforce_A_queue is not empty");
    Assert (not g_enforce_C_queue, "invalid: g_enforce_C_queue is not empty");
    Assert (not g_enforce_J_queue, "invalid: g_enforce_J_queue is not empty");
    Assert (not g_enforce_L_queue, "invalid: g_enforce_L_queue is not empty");
    Assert (not g_enforce_N_queue, "invalid: g_enforce_N_queue is not empty");

    //substructures
    O::validate(level);
    AE::validate(level);
    CE::validate(level);
    JE::validate(level);
    OR::validate(level);
}

//shaping
void compact (const ORing& o_order,
              const ARing& a_order,
              const CRing& c_order,
              const JRing& j_order)
{//policy: reorder & minimize overhead, while preserving structure
    Int num_obs = Ob::numUsed();
    Int num_apps = App::numUsed();
    Int num_comps = Comp::numUsed();
    Int num_joins = Join::numUsed();

    logger.info() << "Compacting structure to " << num_obs << " obs" |0;
    Logging::IndentBlock block;

    //rename node fields
    O::rename_obs(o_order); //names, Rep

    //reorder & reallocate arrays
    Ob::resize(num_obs, o_order.new2old());
    Ord::resize(num_obs, o_order.new2old());

    //reorder eqns
    AE::resize(num_obs, num_apps,  o_order, a_order);
    CE::resize(num_obs, num_comps, o_order, c_order);
    JE::resize(num_obs, num_joins, o_order, j_order);

    //reorder extra stuff
    TB::extra_resize(num_obs, o_order.new2old());
}

//saving/loading atom table
Int data_size () { return num_atoms() * sizeof(Files::ObNamePair); }
void save_to (ostream& os)
{//saves atom names
    logger.debug() << "Saving atoms to stream" |0;

    os << "#\\subsection{Basis}\n\n!using";
    for (AtomTable::iterator iter = g_atomTable.begin();
            iter!=g_atomTable.end(); ++iter) {
        os << ' ' << iter->first;
    }
    os << ".\n\n";
}
void save_to_file (FILE* file)
{//saves atom table
    Int N = num_atoms();
    logger.debug() << "Saving " << N << " atoms to file" |0;
    Logging::IndentBlock block;

    using namespace Files;
    ObNamePair* data = new ObNamePair[N];

    Int n=0;
    for (AtomTable::iterator iter = g_atomTable.begin();
            iter!=g_atomTable.end(); ++iter) {
        const string& name = iter->first;
        LOG_DEBUG1( "saving atom " << name );
        data[n].ob = *(iter->second);
        Assert (name.size() <= ObNamePair::MAX_NAME_SIZE,
                "name is too large to save");
        strcpy(data[n].name, name.c_str());
        ++n;
    }

    safe_fwrite(data, sizeof(ObNamePair), N, file);

    delete[] data;
}
void load_from_file (FILE* file, Int num_atoms)
{//loads atom table
    Int N = num_atoms;
    logger.debug() << "Loading " << N << " atoms from file" |0;
    Logging::IndentBlock block;

    using namespace Files;
    ObNamePair* data = new ObNamePair[N];
    safe_fread(data, sizeof(ObNamePair), N, file);

    Assert (g_atomTable.empty(), "name table not empty before loading");
    for (Int n=0; n<N; ++n) {
        string name = data[n].name;
        LOG_DEBUG1( "loading atom " << name );

        ObHdl ob = ObHdl(Ob(data[n].ob));
        g_atomTable.insert(AtomTable::value_type(name, ob));
    }
    Assert (g_atomTable.size() == N, "duplicated names in loading atoms");

    delete[] data;
}

//diagnostics
void dump (string filename, Int struct_type, bool verbose)
{//writes structure to human-readable files
    logger.info() << "Dumping Comb. Struct. to files "
        << filename << ".* ..."|0;
    Logging::IndentBlock block;
    ofstream file;

    using namespace Symbols;
    if (not struct_type) struct_type = ALL_STRUCT;

    if (struct_type & OBS_STRUCT) {
        //output obs
        logger.info() << "printing obs" |0;
        std::ostringstream o_filename;
        o_filename << filename << ".struct.obs";
        file.open(o_filename.str().c_str());
        file << "# Title: dump of Obs\n";
        file << "# Author: Johann " << VERSION << "\n";
        file << "# Date: " << get_date() << "\n";
        file << "#";
        file << "# num\tname\n";
        file << "#--------------------------------\n";
        for (Ob::sparse_iterator o=Ob::sbegin(); o!=Ob::send(); ++o) {
            Ob ob = *o;
            file << ob << '\t' << expr(ob) << '\n';
        }
        file.close();
    }

    if (struct_type & APP_STRUCT) {
        //output eqns
        logger.info() << "printing app eqns" |0;
        std::ostringstream a_filename;
        a_filename << filename << ".struct.app";
        file.open(a_filename.str().c_str());
        file << "# Title: dump of App Equations\n";
        file << "# Author: Johann " << VERSION << "\n";
        file << "# Date: " << get_date() << "\n";
        file << "#";
        if (verbose) file << "# num\tAPP\tLHS\tRHS\tname\n";
        else         file << "# num\tAPP\tLHS\tRHS\n";
        file << "#----------------------------------------\n";
        for (App::sparse_iterator e=App::sbegin(); e!=App::send(); ++e) {
            App eqn = *e;
            Ob app = AE::get_app(eqn), lhs = get_lhs(eqn), rhs = get_rhs(eqn);
            file << eqn << '\t' << app << '\t' << lhs << '\t' << rhs << '\t';
            if (verbose) {
                file << '\t' << expr(app)
                     << " = " << expr(lhs)
                     << "(" << expr(rhs) << ")\n";
            } else {
                file << '\n';
            }
        }
        file.close();
    }

    if (struct_type & COMP_STRUCT) {
        //output eqns
        logger.info() << "printing comp eqns" |0;
        std::ostringstream c_filename;
        c_filename << filename << ".struct.comp";
        file.open(c_filename.str().c_str());
        file << "# Title: dump of Comp Equations\n";
        file << "# Author: Johann " << VERSION << "\n";
        file << "# Date: " << get_date() << "\n";
        file << "#";
        if (verbose) file << "# num\tCOMP\tLHS\tRHS\tname\n";
        else         file << "# num\tCOMP\tLHS\tRHS\n";
        file << "#----------------------------------------\n";
        for (Comp::sparse_iterator e=Comp::sbegin(); e!=Comp::send(); ++e) {
            Comp eqn = *e;
            Ob cmp = CE::get_comp(eqn), lhs = get_lhs(eqn), rhs = get_rhs(eqn);
            file << eqn << '\t' << cmp << '\t' << lhs << '\t' << rhs << '\t';
            if (verbose) {
                file << '\t' << expr(cmp)
                     << " = (" << expr(lhs)
                     << ")*(" << expr(rhs) << ")\n";
            } else {
                file << '\n';
            }
        }
        file.close();
    }

    if (struct_type & JOIN_STRUCT) {
        //output eqns
        logger.info() << "printing join eqns" |0;
        std::ostringstream j_filename;
        j_filename << filename << ".struct.join";
        file.open(j_filename.str().c_str());
        file << "# Title: dump of Join Equations\n";
        file << "# Author: Johann " << VERSION << "\n";
        file << "# Date: " << get_date() << "\n";
        file << "#";
        if (verbose) file << "# num\tJOIN\tLHS\tRHS\tname\n";
        else         file << "# num\tJOIN\tLHS\tRHS\n";
        file << "#----------------------------------------\n";
        for (Join::sparse_iterator e=Join::sbegin(); e!=Join::send(); ++e) {
            Join eqn = *e;
            Ob join = JE::get_join(eqn), lhs = get_lhs(eqn), rhs = get_rhs(eqn);
            file << eqn << '\t' << join << '\t' << lhs << '\t' << rhs << '\t';
            if (verbose) {
                file << '\t' << expr(join)
                     << " = (" << expr(lhs)
                     << ")|(" << expr(rhs) << ")\n";
            } else {
                file << '\n';
            }
        }
        file.close();
    }

    if (struct_type & ORD_STRUCT) {
        //output order
        logger.info() << "printing order" |0;
        std::ostringstream l_filename;
        l_filename << filename << ".struct.ord";
        file.open(l_filename.str().c_str());
        file << "# Title: dump of Order\n";
        file << "# Author: Johann " << VERSION << "\n";
        file << "# Date: " << get_date() << "\n";
        file << "#";
        file << "# unknown relations are printed as blank\n";
        file << "#";
        file << "# LHS\tRHS\t[?=\t=?]\n";
        file << "#--------------------------------\n";
        for (Ob::sparse_iterator i=Ob::sbegin(); i!=Ob::send(); ++i) {
            Ob lhs = *i;
        for (Ob::sparse_iterator j=Ob::sbegin(); j!=i; ++j) {
            Ob rhs = *j;

            Int state1 = isLessThan(lhs,rhs) | (isNLessThan(lhs,rhs)<<1);
            Int state2 = isLessThan(rhs,lhs) | (isNLessThan(rhs,lhs)<<1);

            if (state1 == 2 and state2 == 2) continue; //distinct

            file << lhs << '\t' << rhs << '\t';
            if (state1 or state2) {
                switch (state1) {
                    case 1: file << "[=";  break;
                    case 2: file << "[!="; break;
                }
                file << '\t';
                switch (state2) {
                    case 1: file << "=]";  break;
                    case 2: file << "=!]"; break;
                }
            }
            file << '\n';
        } }
        file.close();
    }

    if (struct_type & THY_STRUCT) {
        //output theory
        logger.info() << "saving theory" |0;
        std::ostringstream l_filename;
        l_filename << filename << ".theory.jcode";
        file.open(l_filename.str().c_str());
        file << "#\\title{ " << filename << " }\n";
        file << "#\\author{ Johann " << VERSION << " }\n";
        file << "#\\date{ " << get_date() << " }\n";
        file << "#";
        LT::theory()->save_to(file);
        file.close();
    }

    logger.info() << "...done printing." |0;
}
template<class OS>
void _log_stats_to(OS& os)
{
    //obs
    Int o_size = Ob::size();
    Int num_unkns = 0;
    Int core_size = 0;
    Int num_named = 0;
    Int num_marked = 0;
    Int num_used = 0;
    for (Ob::sparse_iterator iter=Ob::sbegin(); iter!=Ob::send(); ++iter) {
        Ob ob = *iter;
        if (O::isUnkn(ob))   ++num_unkns;
        if (O::isInCore(ob)) ++core_size;
        if (O::isNamed(ob))  ++num_named;
        if (O::marked(ob))   ++num_marked;
        if (not O::isPrunable(ob)) ++num_used;
    }
    os  << '\t' << o_size << " obs, " << num_used << " used ("
        << core_size  << " in core, "
        << num_named  << " named, "
        << num_marked << " marked)\n";
    //os  << '\t' << o_size << "\tobs = "
    //    << o_size - num_unkns << " explicit + "
    //    << num_unkns << " implicit \n";

    //app table
    long a_size = App::size();
    long a_supported = o_size * o_size;
    float a_usage = (1.0 * a_size) / (1.0 * a_supported);
    os  << '\t' << 100 * a_usage << "\% apps known = "
        << a_size << " eqns / "
        << a_supported << " supported\n";

    //comp table
    long c_size = Comp::size();
    long c_supported = o_size * o_size;
    float c_usage = (1.0 * c_size) / (1.0 * c_supported);
    os  << '\t' << 100 * c_usage << "\% comps known = "
        << c_size << " eqns / "
        << c_supported << " supported\n";

    //join table
    long j_size = Join::size();
    long j_supported = o_size * o_size;
    float j_usage = (1.0 * j_size) / (1.0 * j_supported);
    os  << '\t' << 100 * j_usage << "\% joins known = "
        << j_size << " eqns / "
        << j_supported << " supported\n";

    //order table
    long l_size = Ord::size_pos();
    long n_size = Ord::size_neg();
    long l_supported = o_size * o_size;
    float l_usage = (1.0 * (l_size + n_size)) / (1.0 * l_supported);
    os  << '\t' << 100 * l_usage << "\% order known = ("
        << l_size << " pos + " << n_size << " neg) / "
        << l_supported << " supported\n";

    //constancy
    if (LT::Atoms::K) {
        long num_const = AE::Lx_support(*LT::Atoms::K).size();
        float o_const = (1.0 * num_const) / o_size;
        float a_const = o_const / a_usage;
        float c_const = o_const / c_usage;
        os  << '\t' << 100*o_const << "\% obs, "
                    << 100*a_const << "\% apps, "
                    << 100*c_const << "\% comps constant";
    }
}
float log_stats (Long time)
{
    const Logging::fake_ostream& log = logger.info() << "Comb. Struct stats:\n";
    _log_stats_to(log);
    log |0;

    Int obs = Ob::size();
    Int ord_pos = Ord::size_pos();
    Int ord_neg = Ord::size_neg();
    float density = static_cast<float>(ord_pos + ord_neg) / sqr(obs);

    //and also to a log file
    if (not stats_log.is_open()) return density;
    stats_log << time << '\t'
              << obs << '\t'
              << App::size() << '\t'
              << Comp::size() << '\t'
              << Join::size() << '\t'
              << ord_pos << '\t'
              << ord_neg
              << std::endl;

    return density;
}
void write_stats_to (ostream& os)
{
    _log_stats_to(os);
    os << std::endl;
}
void write_params_to (ostream& os)
{
    //basis
    AtomTable::iterator iter=g_atomTable.begin();
    os << "\tstructural basis = {" << iter->first;
    for (++iter; iter!=g_atomTable.end(); ++iter) {
        os << "," << iter->first;
    }
    os << '}' << std::endl;
}
bool vis_eqn_table (Int size)
{
    logger.info() << "visualizing eqn table to stats/eqn_table.png" |0;
    Int N = Ob::numUsed();
    V::ColorImage image(N);
    Ob::sparse_iterator end = Ob::send();
    Int i=0,j=0;
    for (Ob::sparse_iterator I=Ob::sbegin(); I!=end; ++I) { j = 0;
    for (Ob::sparse_iterator J=Ob::sbegin(); J!=end; ++J) {
        image.set(i,j,
            AE::find_app(*J,*I) ? 255 : 0,
            CE::find_comp(*J,*I) ? 255 : 0,
            JE::find_join(*J,*I) ? 255 : 0
        );
    ++j; }
    ++i; }

    //save full-size version
    if (not image.save("stats/eqn_table")) return false;

    //save scaled version
    if (size == 0) return image.save("stats/eqn_table_small");
    V::ColorImage scaled = image.shrink((N+size-1)/size);
    scaled.lighten(1);
    return scaled.save("stats/eqn_table_small");
}
bool vis_ord_table (Int size)
{
    logger.info() << "visualizing ord table to stats/ord_table.png" |0;
    Int N = Ob::numUsed();
    V::ColorImage image(N);
    Ob::sparse_iterator end = Ob::send();
    Int i=0,j=0;
    for (Ob::sparse_iterator I=Ob::sbegin(); I!=end; ++I) { j = 0;
    for (Ob::sparse_iterator J=Ob::sbegin(); J!=end; ++J) {
        if      ( isLessThan(*J,*I))    image.set(i,j,255,0,0);
        else if (isNLessThan(*J,*I))    image.set(i,j,0,0,0);
        else                            image.set(i,j,255,255,255);
    ++j; }
    ++i; }

    //save full-size version
    if (not image.save("stats/ord_table")) return false;

    //save scaled version
    if (size == 0) return image.save("stats/ord_table_small");
    V::ColorImage scaled = image.shrink((N+size-1)/size);
    scaled.lighten(1);
    return scaled.save("stats/ord_table_small");
}

//======================== combinator tools ========================

#define CHECK_POS(pos) {Assert5((pos).isValid()and(pos).isUsed(),"bad position: " << pos);}

//unchecked creation
Ob make_atom (string name, Int properties)
{//declares a new atom, unrelated to anything else
    LOG_DEBUG1( "making atom: " << name );
    Assert (g_atomTable.find(name) == g_atomTable.end(),
            "atom already exists before CS::make_atom");
    Ob atom = O::create_atom(properties|O::IN_CORE); //rep tree
    g_atomTable.insert(AtomTable::value_type(name, ObHdl(atom)));
    OR::insert(atom);
    //atom = O::getRep(atom);
    Assert5(atom, "Obs created null atom");
    Assert5(atom == O::getRep(atom), "atom depricated apon creation");
    TB::extra_create_ob(atom);
    enqueue_enforce(atom);
    return atom;
}
App make_app (Ob app, Ob lhs, Ob rhs)
{//creates an equation using app, lhs, rhs
    LOG_DEBUG1( "making app: "
        << "..." << " = (" << expr(lhs) << ")(" << expr(rhs) << ")" );
    CHECK_POS(app);
    CHECK_POS(lhs);
    CHECK_POS(rhs);
    Assert4(AE::find_app(lhs, rhs) != app, "eqn already exist");
    App eqn = AE::create(app, lhs, rhs); //forest
    CHECK_POS(eqn);
    O::insert_app(eqn);
    TB::extra_create_app(eqn);
    enqueue_enforce(eqn);
    return eqn;
}
App make_app (Ob lhs, Ob rhs)
{//creates a new ob and equation using lhs and rhs, returns eqn app=lhs rhs
    LOG_DEBUG1( "making app: (" << expr(lhs) << ")(" << expr(rhs) << ")" );
    CHECK_POS(lhs);
    CHECK_POS(rhs);
    Assert4(!AE::find_app(lhs, rhs), "app already exist");
    Ob app = O::create_app();
    OR::insert(app);
    TB::extra_create_ob(app);
    CHECK_POS(app);
    App eqn = make_app(app, lhs, rhs);
    CHECK_POS(eqn);
    enqueue_enforce(app);
    return eqn;
}
Comp make_comp (Ob cmp, Ob lhs, Ob rhs)
{//creates an equation using cmp, lhs, rhs
    LOG_DEBUG1( "making comp: "
        << "..." << " = (" << expr(lhs) << ")*(" << expr(rhs) << ")" );
    CHECK_POS(cmp);
    CHECK_POS(lhs);
    CHECK_POS(rhs);
    Assert4(CE::find_comp(lhs, rhs) != cmp, "eqn already exist");
    Comp eqn = CE::create(cmp, lhs, rhs); //forest
    CHECK_POS(eqn);
    O::insert_comp(eqn);
    TB::extra_create_comp(eqn);
    enqueue_enforce(eqn);
    return eqn;
}
Comp make_comp (Ob lhs, Ob rhs)
{//creates a new ob and equation using lhs and rhs, returns eqn cmp=lhs rhs
    LOG_DEBUG1( "making comp: (" << expr(lhs) << ")(" << expr(rhs) << ")" );
    CHECK_POS(lhs);
    CHECK_POS(rhs);
    Assert4(!CE::find_comp(lhs, rhs), "cmp already exist");
    Ob cmp = O::create_comp();
    OR::insert(cmp);
    TB::extra_create_ob(cmp);
    CHECK_POS(cmp);
    Comp eqn = make_comp(cmp, lhs, rhs);
    CHECK_POS(eqn);
    enqueue_enforce(cmp);
    return eqn;
}
Join make_join (Ob join, Ob lhs, Ob rhs)
{//creates an equation using join, lhs, rhs
    LOG_DEBUG1( "making join: "
        << "..." << " = (" << expr(lhs) << ")*(" << expr(rhs) << ")" );
    CHECK_POS(join);
    CHECK_POS(lhs);
    CHECK_POS(rhs);
    Assert4(JE::find_join(lhs, rhs) != join, "eqn already exist");
    Join eqn = JE::create(join, lhs, rhs); //forest
    CHECK_POS(eqn);
    O::insert_join(eqn);
    TB::extra_create_join(eqn);
    enqueue_enforce(eqn);
    return eqn;
}
Join make_join (Ob lhs, Ob rhs)
{//creates a new ob and equation using lhs and rhs, returns eqn join=lhs rhs
    LOG_DEBUG1( "making join: (" << expr(lhs) << ")(" << expr(rhs) << ")" );
    CHECK_POS(lhs);
    CHECK_POS(rhs);
    Assert4(!JE::find_join(lhs, rhs), "join already exist");
    Ob join = O::create_join();
    OR::insert(join);
    TB::extra_create_ob(join);
    CHECK_POS(join);
    Join eqn = make_join(join, lhs, rhs);
    CHECK_POS(eqn);
    enqueue_enforce(join);
    return eqn;
}
Ob make_unkn (string& name, Int properties)
{//declares a new unknown, unrelated to anything else
    LOG_DEBUG1( "making unknown: " << name );
    Assert (g_unknTable.find(name) == g_unknTable.end(),
            "unkn already exists before CS::make_unkn");
    Ob unkn = O::create_atom(properties); //rep tree
    g_unknTable.insert(UnknTable::value_type(name, ObHdl(unkn)));
    Assert5(unkn, "Obs created null unkn");
    OR::insert(unkn);
    enqueue_enforce(unkn);
    saturate();
    Assert5(unkn == O::getRep(unkn), "unkn depricated apon creation");
    return unkn;
}
Ob update_unkn (string& name, Ob ob)
{//redefines an unknown's value
    LOG_DEBUG1( "updating unknown: " << name );
    UnknTable::iterator iter = g_unknTable.find(name);
    Assert (iter != g_unknTable.end(),
            "unkn does not exists before CS::update_unkn");
    Ob old_value = *(iter->second);
    iter->second.clear();
    iter->second.set(ob);
    return old_value;
}

//creation/location interface
Ob find_atom (const string& name)
{//finds atom if it exists; returns null ob otherwise
    //search atoms
    AtomTable::iterator pos1 = g_atomTable.find(name);
    if (pos1 != g_atomTable.end()) return *(pos1->second);

    //search unknowns
    UnknTable::iterator pos2 = g_unknTable.find(name);
    if (pos2 != g_unknTable.end()) return *(pos2->second);

    return Ob(0);
}
const string* find_atom_name (Ob ob)
{//returns name of atom if ob is atom, NULL otherwise
    //search atoms
    for (AtomTable::iterator iter=g_atomTable.begin();
            iter!=g_atomTable.end(); ++iter) {
        if (*(iter->second) == ob) return &(iter->first);
    }
    //search unknowns
    for (UnknTable::iterator iter=g_unknTable.begin();
            iter!=g_unknTable.end(); ++iter) {
        if (*(iter->second) == ob) return &(iter->first);
    }
    return NULL;
}
bool name_atom (Ob atom, const string& name)
{//returns true on success
    //check for name conflicts
    Ob old_atom = find_atom(name);
    if (old_atom == atom) {
        logger.debug() << "atom already has given name: " << name |0;
        return true;
    }
    if (old_atom) {
        logger.warning() << "tried name two atoms both: " << name |0;
        return false;
    }
    const string* old_name = find_atom_name(atom);
    if (old_name) {
        logger.warning() << "tried to rename existing atom: "
            << *old_name << " --> " << name |0;
        return false;
    }

    //add name to existing atom
    logger.info() << "defining atom: " << name << " =: " << expr(atom) |0;
    O::addToCore(atom);
    g_atomTable.insert(AtomTable::value_type(name, ObHdl(atom)));
    enqueue_enforce(atom); //for possibly new properties
    return true;
}
bool forget_atom (const string& name)
{//returns true on success
    Ob ob = find_atom (name);
    if (not ob) return false;
    g_atomTable.erase(name);
    return true;
}
std::vector<ObHdl> get_atoms ()
{
    std::vector<ObHdl> result;
    for (AtomTable::iterator i=g_atomTable.begin(); i!=g_atomTable.end(); ++i) {
        result.push_back(i->second);
    }
    return result;
}
std::vector<string> get_atom_names ()
{
    std::vector<string> result;
    for (AtomTable::iterator i=g_atomTable.begin(); i!=g_atomTable.end(); ++i) {
        result.push_back(i->first);
    }
    return result;
}
Ob get_app (Ob lhs, Ob rhs)
{//makes an app only if it cannot be found
    CHECK_POS(lhs);
    CHECK_POS(rhs);
    Ob result = AE::find_app(lhs, rhs);
    if (!result) {
        make_app(lhs, rhs);
        saturate();
        result = AE::find_app(O::getRep(lhs), O::getRep(rhs));
        Assert3(result, "app could not be found after make_app");
    }
    CHECK_POS(result);
    return result;
}
Ob get_comp (Ob lhs, Ob rhs)
{//makes an comp only if it cannot be found
    CHECK_POS(lhs);
    CHECK_POS(rhs);
    Ob result = CE::find_comp(lhs, rhs);
    if (!result) {
        make_comp(lhs, rhs);
        saturate();
        result = CE::find_comp(O::getRep(lhs), O::getRep(rhs));
        Assert3(result, "comp could not be found after make_comp");
    }
    CHECK_POS(result);
    return result;
}
Ob get_join (Ob lhs, Ob rhs)
{//makes an join only if it cannot be found
    CHECK_POS(lhs);
    CHECK_POS(rhs);
    Ob result = JE::find_join(lhs, rhs);
    if (!result) {
        make_join(lhs, rhs);
        saturate();
        result = JE::find_join(O::getRep(lhs), O::getRep(rhs));
        Assert3(result, "join could not be found after make_join");
    }
    CHECK_POS(result);
    return result;
}

//safe creation/pruning
void create_app (Ob lhs, Ob rhs)
{
    Assert2(not AE::find_app(lhs,rhs),
            "tried to create_app an existing app");

    make_app(lhs,rhs);
    saturate();
}
void create_comp (Ob lhs, Ob rhs)
{
    Assert2(not CE::find_comp(lhs,rhs),
            "tried to create_comp an existing comp");

    make_comp(lhs,rhs);
    saturate();
}
void create_join (Ob lhs, Ob rhs)
{
    Assert2(not JE::find_join(lhs,rhs),
            "tried to create_join an existing join");

    make_join(lhs,rhs);
    saturate();
}
void prune_ob (Ob toPrune)
{
    Assert2(O::isPrunable(toPrune), "tried to prune unprunable ob");
    LOG_DEBUG1( "pruning ob " << toPrune );

    AE::remove(toPrune);
    CE::remove(toPrune);
    JE::remove(toPrune);
    OR::remove(toPrune);
    TB::extra_delete_ob(toPrune);
    O::delete_(toPrune);
}

//equational theory tools
Trool query_reln (Ob lhs, Relation reln, Ob rhs)
{
    switch (reln) {
        case EQUAL:
            if (areEquiv(lhs, rhs)) return TRUE;
            if (isNLessThan(lhs, rhs)) return FALSE;
            if (isNLessThan(rhs, lhs)) return FALSE;
            return UNKNOWN;

        case LESS_EQUAL:
            if (isLessThan(lhs, rhs))  return TRUE;
            if (isNLessThan(lhs, rhs)) return FALSE;
            return UNKNOWN;

        case NOT_LEQ:
            if (isNLessThan(lhs, rhs)) return TRUE;
            if (isLessThan(lhs, rhs))  return FALSE;
            return UNKNOWN;

        default:
            Error("queried unknown relation: " << RelationNames[reln]);
            return UNKNOWN;
    }
}
bool assume_equiv (Ob ob1, Ob ob2)
{//assumption followed by queue processing
    CHECK_POS(ob1);
    CHECK_POS(ob2);
    if (ensure_equiv(ob1, ob2)) return true;
    saturate();
    return false;
}
bool assume_app (Ob app, Ob lhs, Ob rhs)
{//assumption followed by queue processing
    CHECK_POS(app);
    CHECK_POS(lhs);
    CHECK_POS(rhs);
    if (ensure_app(app, lhs, rhs)) return true;
    saturate();
    return false;
}
bool assume_comp (Ob cmp, Ob lhs, Ob rhs)
{//assumption followed by queue processing
    CHECK_POS(cmp);
    CHECK_POS(lhs);
    CHECK_POS(rhs);
    if (ensure_comp(cmp, lhs, rhs)) return true;
    saturate();
    return false;
}
bool assume_join (Ob join, Ob lhs, Ob rhs)
{//assumption followed by queue processing
    CHECK_POS(join);
    CHECK_POS(lhs);
    CHECK_POS(rhs);
    if (ensure_join(join, lhs, rhs)) return true;
    saturate();
    return false;
}
bool assume_less (Ob smaller, Ob larger)
{//assumption followed by queue processing
    CHECK_POS(smaller);
    CHECK_POS(larger);
    if (ensure_less(smaller, larger)) return true;
    saturate();
    return false;
}
bool assume_nless (Ob smaller, Ob larger)
{//assumption followed by queue processing
    CHECK_POS(smaller);
    CHECK_POS(larger);
    if (ensure_nless(smaller, larger)) return true;
    saturate();
    return false;
}
bool assume_reln (Ob lhs, Relation reln, Ob rhs)
{
    switch (reln) {
    case EQUAL:      return assume_equiv(lhs, rhs);
    case LESS_EQUAL: return assume_less(lhs, rhs);
    case NOT_LEQ:    return assume_nless(lhs, rhs);
    default:
    logger.error() << "assumed unknown relation: " << RelationNames[reln] |0;
    return false;
    }
}

//enforcement tools for external enforcement

bool g_die_quietly = false;
void die_quietly () { g_die_quietly = true; }
#define AssertC(cond,mess) { if (!(cond)) { \
    if (g_die_quietly) _exit(1); \
    logger.error() << "inconsistent: " << mess << "\nsee data/notcon.*" |0; \
    dump("data/notcon"); \
    abort(); \
    }}

bool ensure_equiv (Ob ob1, Ob ob2)
{//merges two terms and processes queues; returns true if equiv already holds
    LOG_DEBUG1( "ensuring: " << expr(ob1) << " = " << expr(ob2) );
    AssertC(not (areDistinct(ob1, ob2)),
            "ensuring " << expr(ob1) << " = " << expr(ob2));
    CHECK_POS(ob1);  ob1 = O::getRep(ob1);  CHECK_POS(ob1);
    CHECK_POS(ob2);  ob2 = O::getRep(ob2);  CHECK_POS(ob2);
    if (ob1 == ob2) return true;
    Ob dep, rep;
    if (ob1 > ob2) { dep = ob1; rep = ob2; }
    else           { dep = ob2; rep = ob1; }
    merge(dep, rep);
    return false;
}
bool ensure_less (Ob smaller, Ob larger)
{//ensures smaller [= larger; returns true if already true
    LOG_DEBUG1( "ensuring: " << expr(smaller) << " [= " << expr(larger) );
    AssertC(not isNLessThan(smaller, larger),
            "ensuring " << expr(smaller) << " [= " << expr(larger));
    CHECK_POS(smaller);  smaller = O::getRep(smaller);  CHECK_POS(smaller);
    CHECK_POS(larger);   larger  = O::getRep(larger);   CHECK_POS(larger);
    Ord ord(smaller, larger);
    if (OR::ensure_less(ord)) return true;
    enqueue_enforce_L(ord);
    return false;
}
bool ensure_nless (Ob smaller, Ob larger)
{//ensures smaller [!= larger; returns true if already true
    LOG_DEBUG1( "ensuring: " << expr(smaller) << " [!= " << expr(larger) );
    AssertC(not isLessThan(smaller, larger),
            "ensuring " << expr(smaller) << " [!= " << expr(larger));
    CHECK_POS(smaller);  smaller = O::getRep(smaller);  CHECK_POS(smaller);
    CHECK_POS(larger);   larger  = O::getRep(larger);   CHECK_POS(larger);
    Ord ord(smaller, larger);
    if (OR::ensure_nless(ord)) return true;
    enqueue_enforce_N(ord);
    return false;
}
bool ensure_app (Ob app, Ob lhs, Ob rhs)
{//ensures an app exists; returns true if app already exists
    LOG_DEBUG1( "ensuring: " << expr(app)
        << " = " << expr(lhs) << "(" << expr(rhs) << ")" );
    CHECK_POS(app);  app = O::getRep(app);  CHECK_POS(app);
    CHECK_POS(lhs);  lhs = O::getRep(lhs);  CHECK_POS(lhs);
    CHECK_POS(rhs);  rhs = O::getRep(rhs);  CHECK_POS(rhs);
    Ob existing_app = AE::find_app(lhs, rhs);
    if (existing_app) {
        return ensure_equiv (app, existing_app);
    } else {
        make_app (app, lhs, rhs);
        return false;
    }
}
bool ensure_comp (Ob cmp, Ob lhs, Ob rhs)
{//ensures an comp exists; returns true if comp already exists
    LOG_DEBUG1( "ensuring: " << expr(cmp)
        << " = (" << expr(lhs) << ")*(" << expr(rhs) << ")" );
    CHECK_POS(cmp);  cmp = O::getRep(cmp);  CHECK_POS(cmp);
    CHECK_POS(lhs);  lhs = O::getRep(lhs);  CHECK_POS(lhs);
    CHECK_POS(rhs);  rhs = O::getRep(rhs);  CHECK_POS(rhs);
    Ob existing_comp = CE::find_comp(lhs, rhs);
    if (existing_comp) {
        return ensure_equiv (cmp, existing_comp);
    } else {
        make_comp (cmp, lhs, rhs);
        return false;
    }
}
bool ensure_join (Ob join, Ob lhs, Ob rhs)
{//ensures a join exists; returns true if join already exists
    LOG_DEBUG1( "ensuring: " << expr(join)
        << " = (" << expr(lhs) << ")|(" << expr(rhs) << ")" );
    CHECK_POS(join); join = O::getRep(join); CHECK_POS(join);
    CHECK_POS(lhs);  lhs  = O::getRep(lhs);  CHECK_POS(lhs);
    CHECK_POS(rhs);  rhs  = O::getRep(rhs);  CHECK_POS(rhs);
    Ob existing_join = JE::find_join(lhs, rhs);
    if (existing_join) {
        return ensure_equiv (join, existing_join);
    } else {
        make_join (join, lhs, rhs);
        return false;
    }
}

//======================== merging ========================

//merging interface
void merge (Ob dep, Ob rep)
{//merges two obs, a queued operation
    Assert2(dep > rep, "tried to merge obs in incorrect order");
    Assert2(O::rep(dep) == dep, "tried to merge depricated dep ob");
    Assert2(O::rep(rep) == rep, "tried to merge depricated rep ob");
    LOG_DEBUG1( "merging Obs " << expr(dep) << " & " << expr(rep) );
    O::rep(dep) = rep;

    //WARNING: these must be called in this order
    TB::extra_merge_obs(dep, rep);
    O::merge(dep, rep); //merges name, fields, & parse structure

    enqueue_merge(dep);
}
void merge_finally (Ob dep, Ob rep)
{//merge two obs, after dequeuing
    Assert2(dep > rep, "tried to merge_finally obs in incorrect order");
    AE::merge(dep, rep); //merges forests, enqueues other mergers
    CE::merge(dep, rep); //merges forests, enqueues other mergers
    JE::merge(dep, rep); //merges forests, enqueues other mergers
    OR::merge(dep, rep); //merges columns, enqueues enforcements
    O::delete_(dep);

    //move enforcement over
    merge_enforce(dep, rep);
}
void merge (App dep, App rep)
{//merges two app eqns, a queued operation
    LOG_DEBUG1( "merging Apps " << dep << " & " << rep );
    Assert4(O::getRep(get_lhs(dep)) == O::getRep(get_lhs(rep)),
            "tried to merge app eqns with mismatched LHSs");
    Assert4(O::getRep(get_rhs(dep)) == O::getRep(get_rhs(rep)),
            "tried to merge app eqns with mismatched RHSs");
    ensure_equiv(AE::get_app(dep), AE::get_app(rep)); //(possibly) merge eqns
    merge_enforce(dep, rep); //move enforcement over
}
void merge (Comp dep, Comp rep)
{//merges two comp eqns, a queued operation
    LOG_DEBUG1( "merging Comp " << dep << " & " << rep );
    Assert4(O::getRep(get_lhs(dep)) == O::getRep(get_lhs(rep)),
            "tried to merge comp eqns with mismatched LHSs");
    Assert4(O::getRep(get_rhs(dep)) == O::getRep(get_rhs(rep)),
            "tried to merge comp eqns with mismatched RHSs");
    ensure_equiv(CE::get_comp(dep), CE::get_comp(rep)); //(possibly) merge eqns
    merge_enforce(dep, rep); //move enforcement over
}
void merge (Join dep, Join rep)
{//merges two join eqns, a queued operation
    LOG_DEBUG1( "merging join " << dep << " & " << rep );
#if DEBUG_LEVEL >= 4
    Ob lhs_dep = O::getRep(get_lhs(dep));
    Ob rhs_dep = O::getRep(get_rhs(dep));
    Ob lhs_rep = O::getRep(get_lhs(rep));
    Ob rhs_rep = O::getRep(get_rhs(rep));
    JE::sort(lhs_dep,rhs_dep);
    JE::sort(lhs_rep,rhs_rep);
    Assert4(lhs_dep == lhs_rep,
            "tried to merge join eqns with mismatched LHSs");
    Assert4(rhs_dep == rhs_rep,
            "tried to merge join eqns with mismatched RHSs");
#endif
    ensure_equiv(JE::get_join(dep), JE::get_join(rep)); //(possibly) merge eqns
    merge_enforce(dep, rep); //move enforcement over
}
void enforce (App eqn) { enqueue_enforce(eqn); }
void enforce (Comp eqn) { enqueue_enforce(eqn); }
void enforce (Join eqn) { enqueue_enforce(eqn); }
void enforce_less (int lhs, int rhs)
{
    enqueue_enforce_L(Ord(Ob(lhs),Ob(rhs)));
}
void enforce_nless (int lhs, int rhs)
{
    enqueue_enforce_N(Ord(Ob(lhs),Ob(rhs)));
}
ObHdl enforce_atom (const string& name)
{//enqueues neighborhood of ob & processes theorems
    Ob ob = find_atom(name);
    if (!ob) return ObHdl();
    ObHdl atom = ObHdl(ob);

    using namespace Apply;
    for (Lra_Iterator i(ob); not i.done(); i.next()) enqueue_enforce(*i);
    for (Rla_Iterator i(ob); not i.done(); i.next()) enqueue_enforce(*i);
    for (Alr_Iterator i(ob); not i.done(); i.next()) enqueue_enforce(*i);

    using namespace Compose;
    for (Lrc_Iterator i(ob); not i.done(); i.next()) enqueue_enforce(*i);
    for (Rlc_Iterator i(ob); not i.done(); i.next()) enqueue_enforce(*i);
    for (Clr_Iterator i(ob); not i.done(); i.next()) enqueue_enforce(*i);

    using namespace JoinEqn;
    for (Lrj_Iterator i(ob); not i.done(); i.next()) enqueue_enforce(*i);
    for (Jlr_Iterator i(ob); not i.done(); i.next()) enqueue_enforce(*i);

    using namespace Order;
    for (Iterator<LRpos> i(ob); not i.done(); i.next()) enqueue_enforce_L(*i);
    for (Iterator<RLpos> i(ob); not i.done(); i.next()) enqueue_enforce_L(*i);
    for (Iterator<LRneg> i(ob); not i.done(); i.next()) enqueue_enforce_N(*i);
    for (Iterator<RLneg> i(ob); not i.done(); i.next()) enqueue_enforce_N(*i);

    saturate();

    return atom;
}

//enqueuing operations
inline void enqueue_merge (Ob ob)
{
    LOG_DEBUG1( "enqueuing merge of Ob " << ob );
    g_merge_queue.push(ob);
}
inline void enqueue_enforce (Ob ob)
{
    LOG_DEBUG1( "enqueuing enforcement of Ob" );
    g_enforce_O_queue.push(ob);
}
inline void enqueue_enforce (App eqn)
{
    LOG_DEBUG1( "enqueuing enforcement: " << expr(get_app(eqn))
        << " = (" << expr(get_lhs(eqn))
        << ")(" << expr(get_rhs(eqn)) << ")" );
    g_enforce_A_queue.push(eqn);
}
inline void enqueue_enforce (Comp eqn)
{
    LOG_DEBUG1( "enqueuing enforcement: " << expr(get_comp(eqn))
        << " = (" << expr(get_lhs(eqn))
        << ")*(" << expr(get_rhs(eqn)) << ")" );
    g_enforce_C_queue.push(eqn);
}
inline void enqueue_enforce (Join eqn)
{
    LOG_DEBUG1( "enqueuing enforcement: " << expr(get_join(eqn))
        << " = (" << expr(get_lhs(eqn))
        << ")|(" << expr(get_rhs(eqn)) << ")" );
    g_enforce_J_queue.push(eqn);
}
inline void enqueue_enforce_L (Ord ord)
{
    LOG_DEBUG1( "enqueuing enforcement: "
        << expr(ord.lhs()) << " [= " << expr(ord.rhs()) );
    g_enforce_L_queue.push(ord);
}
inline void enqueue_enforce_N (Ord ord)
{
    LOG_DEBUG1( "enqueuing enforcement: "
        << expr(ord.lhs()) << " [= " << expr(ord.rhs()) );
    g_enforce_N_queue.push(ord);
}

//merging enqueued nodes
inline void merge_enforce (App dep, App rep)
{
    LOG_DEBUG1( "merging enforcement of App " << dep );
    g_enforce_A_queue.merge(dep, rep);
}
inline void merge_enforce (Comp dep, Comp rep)
{
    LOG_DEBUG1( "merging enforcement of Comp " << dep );
    g_enforce_C_queue.merge(dep, rep);
}
inline void merge_enforce (Join dep, Join rep)
{
    LOG_DEBUG1( "merging enforcement of Join " << dep );
    g_enforce_J_queue.merge(dep, rep);
}
void merge_L_queue (OrdQueue& queue, Ob dep, Ob rep)
{
    OrdQueue::iterator iter = queue.begin();
    OrdQueue::iterator end  = queue.end();
    while (iter != end) {
        Ob lhs = iter->lhs(), rhs = iter->rhs();
        if (lhs == dep) {
            queue.erase(iter++);
            if (rhs == dep) { queue.push(Ord(rep, rep)); }
            else            { queue.push(Ord(rep, rhs)); }
            continue;
        }
        if (rhs == dep) {
            queue.erase(iter++);
            queue.push(Ord(lhs, rep));
            continue;
        }
        ++iter;
    }
}
inline void merge_N_queue (OrdQueue& queue, Ob dep, Ob rep)
{
    OrdQueue::iterator iter = queue.begin();
    OrdQueue::iterator end  = queue.end();
    while (iter != end) {
        Ob lhs = iter->lhs(), rhs = iter->rhs();
        if (lhs == dep) {
            queue.erase(iter++);
            queue.push(Ord(rep, rhs));
            continue;
        }
        if (rhs == dep) {
            queue.erase(iter++);
            queue.push(Ord(lhs, rep));
            continue;
        }
        ++iter;
    }
}
void merge_enforce (Ob dep, Ob rep)
{
    LOG_DEBUG1( "merging enforcement of Ob " << dep );

    g_enforce_O_queue.merge(dep, rep);
    merge_L_queue(g_enforce_L_queue, dep, rep);
    merge_N_queue(g_enforce_N_queue, dep, rep);
}

//theorem processing
bool saturation_pending ()
{
    return g_merge_queue
        or g_enforce_O_queue or g_enforce_A_queue
        or g_enforce_C_queue or g_enforce_J_queue
        or g_enforce_L_queue or g_enforce_N_queue;
}
void saturate ()
{
    LOG_DEBUG1( "Processing theorem queues" );
    LOG_INDENT_DEBUG1

    Int init_size = Ob::size();

    saturate:

    while (g_merge_queue) {
        LOG_DEBUG1( "processing a merge" );
        LOG_INDENT_DEBUG1
        ENFORCE_LOG("\nm");

        Ob dep = g_merge_queue.pop();
        Ob rep = O::getRep(dep);
        Assert1(dep != rep, "tried to deprecate a rep ob: " << expr(dep));
        merge_finally(dep, rep);

    } if (g_enforce_O_queue) {
        Ob ob = g_enforce_O_queue.pop();
        CHECK_POS(ob);
        LOG_DEBUG1( "processing a positive Ob enforcement: " << expr(ob) );
        LOG_INDENT_DEBUG1
        ENFORCE_LOG("\no");

        theory()->enforce_O(ob);
        goto saturate;

    } if (g_enforce_A_queue) {
        App eqn = g_enforce_A_queue.pop();
        CHECK_POS(AE::get_app(eqn));
        CHECK_POS(get_lhs(eqn));
        CHECK_POS(get_rhs(eqn));
        LOG_DEBUG1( "processing a positive App enforcement: "
                    << expr(AE::get_app(eqn)) << " = ("
                    << expr(get_lhs(eqn)) << ") ("
                    << expr(get_rhs(eqn)) << ")" );
        LOG_INDENT_DEBUG1
        ENFORCE_LOG('a');

        theory()->enforce_A(eqn);
        goto saturate;

    } if (g_enforce_C_queue) {
        Comp eqn = g_enforce_C_queue.pop();
        CHECK_POS(CE::get_comp(eqn));
        CHECK_POS(get_lhs(eqn));
        CHECK_POS(get_rhs(eqn));
        LOG_DEBUG1( "processing a positive Comp enforcement: "
                    << expr(CE::get_comp(eqn)) << " = ("
                    << expr(get_lhs(eqn)) << ")*("
                    << expr(get_rhs(eqn)) << ")" );
        LOG_INDENT_DEBUG1
        ENFORCE_LOG('c');

        theory()->enforce_C(eqn);
        goto saturate;

    } if (g_enforce_J_queue) {
        Join eqn = g_enforce_J_queue.pop();
        CHECK_POS(JE::get_join(eqn));
        CHECK_POS(get_lhs(eqn));
        CHECK_POS(get_rhs(eqn));
        LOG_DEBUG1( "processing a positive Join enforcement: "
                    << expr(JE::get_join(eqn)) << " = ("
                    << expr(get_lhs(eqn)) << ")|("
                    << expr(get_rhs(eqn)) << ")" );
        LOG_INDENT_DEBUG1
        ENFORCE_LOG('j');

        theory()->enforce_J(eqn);
        goto saturate;

    } if (g_enforce_L_queue) {
        Ord ord = g_enforce_L_queue.pop();
        CHECK_POS(ord.lhs());
        CHECK_POS(ord.rhs());
        LOG_DEBUG1( "processing a positive Ord enforcement: "
                    << expr(ord.lhs()) << " [= " << expr(ord.rhs()) );
        LOG_INDENT_DEBUG1
        ENFORCE_LOG('l');

        theory()->enforce_L(ord);
        goto saturate;

    } if (g_enforce_N_queue) {
        Ord ord = g_enforce_N_queue.pop();
        CHECK_POS(ord.lhs());
        CHECK_POS(ord.rhs());
        LOG_DEBUG1( "processing a negative Ord enforcement: "
                    << expr(ord.lhs()) << " [!= " << expr(ord.rhs()) );
        LOG_INDENT_DEBUG1
        ENFORCE_LOG('n');

        theory()->enforce_N(ord);
        goto saturate;
    }

    Int final_size = Ob::size();
    Int difference = init_size - final_size;
    if (difference) {
        LOG_DEBUG1( init_size << " - " << final_size
            << " = " << difference << " equivalence classes merged" );
    }
}

}




