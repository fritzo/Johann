
#include "meas_lite.h"
#include "files.h"

namespace MeasLite
{

//WARNING: must match defs in .h file
#define for_o for (Int o=1; o<=o_size; ++o)
#define for_a for (const AEqn *e=a_begin, *end=a_end; e!=end; ++e)
#define for_c for (const CEqn *e=c_begin, *end=c_end; e!=end; ++e)
#define for_j for (const JEqn *e=j_begin, *end=j_end; e!=end; ++e)
#define for_i(s) for (S::Iter i=s.begin(), end=s.end(); i!=end; ++i)

//======== structure ========
Int o_size(0), a_size(0), c_size(0), j_size(0);
const AEqn *apps  = NULL, *a_begin = NULL, *a_end = NULL;
const CEqn *comps = NULL, *c_begin = NULL, *c_end = NULL;
const JEqn *joins = NULL, *j_begin = NULL, *j_end = NULL;
Float P_app, P_comp, P_join, P_basis;
Sparse basis("basis");
std::map<string,Ob> atoms;
D* g_prior = NULL;
D* g_parse_size = NULL;

void init_prior (Float tol=1e-6);
void init_parsing ();
void load (string filename)
{
    logger.info() << "Loading structure from " << filename |0;
    Logging::IndentBlock block;

    using namespace Files;

    //open file
    filename = filename + ".jdb";
    FILE* file = fopen(filename.c_str(), "rb");
    Assert (file, "failed to open file " << filename << " for loading");

    //load and check header
    StructFileHeader header;
    header.load_from_file(file);
    start_validating();
    header.validate();
    Assert (everything_valid(), "invalid db file " << filename);

    //set params
    o_size = header.o_size;
    a_size = header.a_size;
    c_size = header.c_size;
    j_size = header.j_size;
    const Logging::fake_ostream& log = logger.info();
    log << "loading "
        << o_size << " obs, "
        << a_size << " apps, "
        << c_size << " comps, "
        << j_size << " joins, "
        << header.w_size << " atoms." |1;

    //load app equations
    Assert ((apps = new(std::nothrow) AEqn[a_size]),
            "failed to allocate " << a_size << " equations");
    fseek_block(file, header.a_data);
    safe_fread(const_cast<AEqn*>(apps), sizeof(AEqn), a_size, file);
    a_begin = apps; a_end = a_begin+a_size;

    log << "." |1; //---------------------------------------------------------

    //load comp equations
    Assert ((comps = new(std::nothrow) CEqn[c_size]),
            "failed to allocate " << c_size << " equations");
    fseek_block(file, header.c_data);
    safe_fread(const_cast<CEqn*>(comps), sizeof(CEqn), c_size, file);
    c_begin = comps; c_end = c_begin+c_size;

    log << "." |1; //---------------------------------------------------------

    //load join equations
    Assert ((joins = new(std::nothrow) JEqn[j_size]),
            "failed to allocate " << j_size << " equations");
    fseek_block(file, header.j_data);
    safe_fread(const_cast<JEqn*>(joins), sizeof(JEqn), j_size, file);
    j_begin = joins; j_end = j_begin+j_size;

    log << "." |1; //---------------------------------------------------------

    //load basis
    //WARNING: must match Language::load_from_file in languages.C
    IntWMass* weights = new IntWMass[header.w_size];
    fseek_block(file, header.L_data);
    safe_fread(&P_app,  sizeof(Float), 1, file);
    safe_fread(&P_comp, sizeof(Float), 1, file);
    safe_fread(&P_join, sizeof(Float), 1, file);
    P_basis = 1.0 - P_app - P_comp - P_join;
    safe_fread(weights, sizeof(IntWMass), header.w_size, file);
    basis.data.insert(weights, weights+header.w_size);
    Assert (basis.data.size() == header.w_size,
            "wrong number of atoms: " << basis.data.size())
    delete[] weights;
    basis.validate();

    log << "." |1; //---------------------------------------------------------

    //load atom names
    //WARNING: must match Brain::load(filename)
    fseek_block(file, header.b_data);
    unsigned N = header.b_size;
    //WARNING: must match CombinatoryStructure::load_from_file(filename,b_size)
    ObNamePair* names = new ObNamePair[N];
    safe_fread(names, sizeof(ObNamePair), N, file);
    for (Int n=0; n<N; ++n) {
        string name = names[n].name;
        //logger.debug() << "loading atom " << name |0;
        atoms[name] = names[n].ob;
    }
    Assert (atoms.size() == N, "duplicated names in loading atoms");
    delete[] names;

    log << "." |1; //---------------------------------------------------------

    fclose(file);
    log << "done" |0;

    //init globals
    g_prior = new D("prior");
    g_parse_size = new D("parse_size");
    init_prior ();
    init_parsing ();
}
void clear ()
{
    logger.info() << "Clearing structure" |0;
    Logging::IndentBlock block;

    basis.clear();
    atoms.clear();
    delete g_prior;
    delete g_parse_size;
    Assert (Dense::num_instances() == 0,
            "Dense instances remain after clearing structure");

    o_size = a_size = c_size = j_size = 0;
    delete[] apps;  apps = NULL;
    delete[] comps; comps = NULL;
    delete[] joins; joins = NULL;
}

//======== parsing + printing ========

Ob parse (std::string expr) { TODO(); }
std::string print (Ob ob) { TODO(); }
void init_parse_size();
void init_parsing ()
{
    logger.info() << "Initializing parsing" |0;
    Logging::IndentBlock block;

    init_parse_size();

    //TODO parse terms as strings
}
//======== measures ========

unsigned Dense::s_num_instances = 0;

//low-level operations
Float D::total ()
{
    double tot = 0;
    for_o tot += m.data[o];
    return tot;
};
Float S::total ()
{
    Float tot = 0;
    for (Sparse::CIter i=m.begin(); i!=m.end(); ++i) tot += i->second;
    return tot;
}
inline Float h_term (Float t) { return t > 0 ? t * logf(t) : 0; }
Float D::entropy ()
{
    double h = 0;
    for_o h += h_term(m.data[o]);
    return - h;
};
Float S::entropy ()
{
    Float h = 0;
    for (Sparse::CIter i=m.begin(); i!=m.end(); ++i) h += h_term(i->second);
    return - h;
}

void Sparse::validate ()
{
    for (CIter i=data.begin(); i!=data.end(); ++i) {
        Ob o = i->first;
        Assert (1 <= o and o <= o_size, "ob out of range: " << o);
    }
}

//======== random generators ========

inline Float random_unif () { return drand48(); }
Float random_normal ()
{//box-muller
    static bool available = true;
    available = not available;

    //x,y are normally distrubuted
    static Float y = 0.0f;
    if (available) return y;

    Float theta = 2.0f * M_PI * random_unif();
    Float r = sqrtf(-2.0f * logf(1.0f - random_unif()));
    Float x = r * cosf(theta);
    y = r * sinf(theta);
    return x;
}

//======== complexity calculations ========

void init_prior (Float tol)
{
    logger.info() << "Initializing complexity prior" |0;
    Logging::IndentBlock block;

    //variables
    D c(prior());
    S c0(basis);

    //calculation
    c = c0;
    c = fix( P_basis * c0
           + P_app * app(c,c)
           + P_comp * comp(c,c)
           + P_join * join(c,c) );
}
void init_parse_size ()
{
    logger.info() << "Initializing parsing" |0;
    Logging::IndentBlock block;

    //variables
    D k(parse_size());
    S k0(basis);

    //set size of atoms
    k = P_basis * k0;

    //iteratively set size of compounds
    for (bool done = true; not done; done = true) {

        //set size of apps
        for (const AEqn *e=a_begin, *end=a_end; e!=end; ++e) {
            Float &k_old = k.data(e->a);
            Float k_new = P_app * k[e->l] * k[e->r];
            if (k_new < k_old) {
                k_new = k_old;
                done = false;
            }
        }

        //set size of comps
        for (const CEqn *e=c_begin, *end=c_end; e!=end; ++e) {
            Float &k_old = k.data(e->c);
            Float k_new = P_comp * k[e->l] * k[e->r];
            if (k_new < k_old) {
                k_new = k_old;
                done = false;
            }
        }

        //set size of joins
        for (const JEqn *e=j_begin, *end=j_end; e!=end; ++e) {
            Float &k_old = k.data(e->j);
            Float k_new = P_join * k[e->l] * k[e->r];
            if (k_new < k_old) {
                k_new = k_old;
                done = false;
            }
        }
    }
}

}


