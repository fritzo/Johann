
#include "unification.h"
#include <cstdlib>
#include <cstring>

//log levels
//#define LOG_DEBUG1(mess)
//#define LOG_INDENT_DEBUG1
#define LOG_DEBUG1(mess) {logger.debug() << mess |0;}
#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

namespace Unification
{

/** To Do:
 * * implement free lists and resetting only of variable nodes;
 *   this way ground terms can be saved/cached across calls.
 */

typedef std::string Name;

//multisets for size comparison
template<class T> class multiset
{
    typedef std::map<T,unsigned> Map;
    Map m_map;
public:

    //modification
    void insert (T t) {
        typename Map::iterator i = m_map.find(t);
        if (i == m_map.end()) m_map[t] = 1;
        else ++(i->second);
    }
    unsigned operator[] (T t) const {
        typename Map::const_iterator i = m_map.find(t);
        if (i == m_map.end()) return 0;
        else return *i;
    }

    //properties
    bool empty () const { return m_map.empty(); }
    bool operator <= (const multiset<T>& s) const { TODO(); }
    bool operator <  (const multiset<T>& s) const { TODO(); }
};

//lengths of terms
class Length
{//a linear multinomial in arbitrarily many variables
    float m_atoms;
    multiset<Name> m_vars;
public:

    //construction
    Length (float a) : m_atoms(a) {}    //for atoms
    Length (Name n) : m_atoms(0) { m_vars.insert(n); }
    Length operator + (const Length& l) const { TODO(); }

    //properties
    bool operator <= (const Length& l) const {
        return m_atoms <= l.m_atoms and m_vars <= l.m_vars;
    }
    bool operator <  (const Length& l) const { TODO(); }
    bool operator >= (const Length& l) const { TODO(); }
    bool operator >  (const Length& l) const { TODO(); }
    bool ground () const { return m_vars.empty(); }
};

enum ExprType { VAR, CST, APP };
const char* ExprTypeName[] = { "VAR","CST","APP" };
class UnifyExpr
{
    typedef UnifyExpr UE;

    //data
    //  structure
    typedef Expr* Name;
    union Misc { Int uo; Name name; };
    Misc m_data[2];
    //  rep trees
    UE* m_rep;
    unsigned m_count;
    //  cycle checking
    bool m_seen;    //whether the depth-first stack intersects this node
    bool m_acyclic; //whether this node has been proven acyclic
    //  parse,size caching
    Expr* m_expr;
    unsigned m_size; //an index into s_lengths

    //initializers for pool management
    static UE* s_pool;
public:
    //WARNING: null pointers are not allowed
    class UHdl //or UnifyHandle
    {
        Int m_pos;
    public:
        UHdl () {}
        explicit UHdl (Int pos) : m_pos(pos) {}
        operator Int () const { return m_pos; }

        UHdl& operator++ () { ++m_pos; return *this; }
        UnifyExpr& operator*  () const { return s_pool[m_pos]; }
        UnifyExpr* operator-> () const { return s_pool + m_pos; }
    };
private:
    static unsigned s_size;
    static UHdl s_top;
    typedef MAP_TYPE<Expr*,UHdl> Table;
    static Table s_expr2u;
    static std::vector<Length> s_lengths;
public:
    static void resize (unsigned size=1024);
    static void grow (); //grows, copies, and fixes UE pointers
    static void reset ();
    static void clear_cache ();

    //union-find
    inline void set_rep (UE* rep);          //takes care of ref counts
    UE* get_rep ();                         //collapses rep path
    void merge (UE* rep);                   //unweighted, will not swap
    static void merge (UE* dep, UE* rep);   //weighted, may swap dep/rep
    bool is_rep () { return !m_rep; }

    //data access
    Name var () const { return m_data[0].name; }
    Name cst () const { return m_data[1].name; }
    UHdl lhs () const { return UHdl(m_data[0].uo); }
    UHdl rhs () const { return UHdl(m_data[1].uo); }
private:
    void var (Name n) { m_data[0].name = n; }
    void cst (Name n) { m_data[1].name = n; }
    void app (UHdl l, UHdl r) { m_data[0].uo = l; m_data[1].uo = r; }

    //type dispatch
public:
    bool isVar () const { return !m_data[1].uo; }
    bool isCst () const { return !m_data[0].uo; }
    bool isApp () const { return not (isVar() or isCst()); }
    ExprType type () const { return isVar() ? VAR : isCst() ? CST : APP; }

    //in-place constructors
private:
    static void new_ () {
        LOG_DEBUG1( "allocating new UnifyExpr" )
        if (++s_top >= s_size) grow();
    }
public:
    static UHdl new_var (Name n) { new_(); s_top->var(n); return s_top; }
    static UHdl new_cst (Name n) { new_(); s_top->cst(n); return s_top; }
    static UHdl new_app (UHdl l,UHdl r)
                                 { new_(); s_top->app(l,r); return s_top; }
    //cycle-checking
    bool is_acyclic ();

    //Expr <--> UnifyExpr translations
    static UHdl new_expr (ExprHdl e);
    ExprHdl expr ();
    const Length& size ();
};
typedef UnifyExpr UE;
typedef UE::UHdl UHdl;

//pool management
UE* UE::s_pool = NULL;
UHdl UE::s_top = UHdl(0);
unsigned UE::s_size = 0;
UE::Table UE::s_expr2u;
std::vector<Length> UE::s_lengths;
void UE::resize (unsigned size)
{
    logger.debug() << "resizing UnifyExpr pool to " << size |0;

    if (s_size) { delete[] s_pool; s_pool = NULL; }
    if (size) { s_pool = new UE[size]; }
    s_top = UHdl(0);
    s_size = size;
}
void UE::grow ()
{
    if (!s_size) { reset(); return; }

    //allocate new memory
    unsigned new_size = 2 * s_size;
    logger.debug() << "growing UnifyExpr pool to " << new_size |0;
    UE* new_pool = new UE[new_size];

    //copy data
    memcpy(new_pool, s_pool, s_size * sizeof(UE));
    bzero(new_pool + s_size, s_size * sizeof(UE));

    //finish up
    delete[] s_pool;
    s_pool = new_pool;
    s_size = new_size;
}
void UE::reset ()
{
    if (!s_size) resize();
    bzero(s_pool, s_size * sizeof(UE));
    s_top = UHdl(0);
    s_expr2u.clear();
    s_lengths.clear();
    s_lengths.push_back(0);
}
void UE::clear_cache ()
{
    for (unsigned i=1; i<s_size; ++i) { UE& u = s_pool[i];
        u.m_rep = NULL;
        u.m_count = 0;
        u.m_seen = false;
        u.m_acyclic = false;
        u.m_expr = NULL;
        u.m_size = 0;
    }
    s_lengths.clear();
    s_lengths.push_back(0);
}

//union-find
inline void UE::set_rep (UE* rep)
{
    if (m_rep) --m_rep->m_count;
    m_rep = rep;
    ++m_rep->m_count;
}
UE* UE::get_rep ()
{//collapsing find
    if (not m_rep) return this;
    set_rep(m_rep->get_rep());
    return m_rep;
}
void UE::merge (UE* rep)
{//unweighted union
    get_rep()->set_rep(rep->get_rep());
}
void UE::merge (UE* dep, UE* rep)
{//weighted union
    dep = dep->get_rep();
    rep = rep->get_rep();
    if (dep->m_count > rep->m_count) swap(rep,dep);
    dep->set_rep(rep);
}

//basic unification
bool cyclic_unify (UE* lhs, UE* rhs)
{//unifies two dags into a directed possibly-cyclic graph, or fails
    if (lhs == rhs) return true;
    if (rhs->isVar()) swap(lhs,rhs);
    ExprType lhs_type = lhs->type();
    ExprType rhs_type = rhs->type();
    LOG_DEBUG1( "unifying " << ExprTypeName[lhs_type]   //<< " " << lhs->expr()
                << " with " << ExprTypeName[rhs_type] ) //<< " " << rhs->expr()
    LOG_INDENT_DEBUG1

    switch (lhs_type) {
        case VAR: {
            LOG_DEBUG1( "merging VAR" );
            if (rhs_type == VAR) UE::merge(lhs,rhs); //depricate the less-ref'd
            else                 lhs->merge(rhs);    //define lhs := rhs
            return true;
        }
        case CST: {
                LOG_DEBUG1( "can't unify distinct CSTs" )
            Assert3 (rhs_type != VAR, "invalid rhs type");
            return false;
        }
        case APP: {
            if (rhs_type == CST) {
                LOG_DEBUG1( "can't unify APP with CST" )
                return false;
            }
            Assert3 (rhs_type == APP, "invalid rhs type");
            return cyclic_unify(rhs->lhs()->get_rep(), lhs->lhs()->get_rep())
               and cyclic_unify(rhs->rhs()->get_rep(), lhs->rhs()->get_rep());
        }
        default: Error("unknown ExprType");
    }
}

//cycle-detection
//TODO how to allow cycles?
bool UE::is_acyclic ()
{//tests for well-foundedness
    LOG_DEBUG1( "checking well-foundedness" )
    LOG_INDENT_DEBUG1

    Assert2(is_rep(), "tried to test well-foundedness of dep node");
    if (m_acyclic) return true;                 //already checked
    if (not isApp()) return m_acyclic = true;   //leaf node
    if (m_seen) {
        LOG_DEBUG1( "cycle found!" )
        return false;                          //stack self-intersects
    }
    m_seen = true;                              //insert in stack
    m_acyclic = lhs()->get_rep()->is_acyclic()  //  descend left
            and rhs()->get_rep()->is_acyclic(); //  descend right
    m_seen = false;                             //remove from stack
    return m_acyclic;                           //return cached result
}
UE* acyclic_unify (UE* lhs, UE* rhs)
{//check for cycles, flatten rep trees
    if (not cyclic_unify(lhs,rhs)) return NULL;
    UE* result = lhs->get_rep();
    return result->is_acyclic() ? result : NULL;
}

//Expr <--> UnifyExpr translations
UHdl UE::new_expr (ExprHdl e)
{//creates a UE dag corresponding to an Expr dag
    Table::iterator pos = s_expr2u.find(&*e);
    if (pos != s_expr2u.end()) return pos->second;

    UHdl result;
    if (e->isApp()) {
        LOG_DEBUG1( "creating APP " << e )
        LOG_INDENT_DEBUG1
        result = new_app(new_expr(e->lhs()), new_expr(e->rhs()));
        Assert3(result->isApp(), "app is not an APP");
    } else { if (e->isVar()) {
        LOG_DEBUG1( "creating VAR " << e )
        LOG_INDENT_DEBUG1
        result = new_var(&*e);
        Assert3(result->isVar(), "var is not an VAR");
    } else { Assert3(e->isConst(), "bad expr type: " << e);
        LOG_DEBUG1( "creating CST " << e )
        LOG_INDENT_DEBUG1
        result = new_cst(&*e);
        Assert3(result->isCst(), "const is not a CST");
    } }

    s_expr2u[&*e] = result;
    return result;
}
ExprHdl UE::expr ()
{//WARNING: only works on acyclic graphs
    Assert3(is_rep(), "tried to convert non-rep to expr");
    LOG_DEBUG1( "parsing " << ExprTypeName[type()] )
    LOG_INDENT_DEBUG1

    if (m_expr) return m_expr;
    switch (type()) {
        case VAR: return m_expr = var();
        case CST: return m_expr = cst();
        case APP: {
            ExprHdl result = lhs()->get_rep()->expr()
                           * rhs()->get_rep()->expr();
            m_expr = &*result;
            return result;
        }
        default: Error("unknown ExprType");
    }
}
const Length& UE::size ()
{
    Assert3(is_rep(), "tried to measure non-rep");
    LOG_DEBUG1( "finding size of " << ExprTypeName[type()] )
    LOG_INDENT_DEBUG1

    if (m_size) return s_lengths[m_size];
    m_size = s_lengths.size();
    switch (type()) {
        case VAR: s_lengths.push_back(*(var()->name()));            break;
        case CST: s_lengths.push_back(1.0f); /* XXX weight? */      break;
        case APP: s_lengths.push_back( lhs()->get_rep()->size()
                                     + rhs()->get_rep()->size() );  break;
        default: Error("unknown ExprType");
    }
    return s_lengths.back();
}

//top-level interface
ExprHdl unify (ExprHdl e1, ExprHdl e2)
{
    UE::reset();
    UHdl o1 = UE::new_expr(e1);
    UHdl o2 = UE::new_expr(e2);

    UE* u1 = &*o1;
    UE* u2 = &*o2;
    if (UE* result = acyclic_unify(u1,u2)) {
        Assert3(result->is_rep(), "acyclic_unify returned non-rep");
        //return result->expr()->pretty();
        return result->expr();
    } else {
        return ExprHdl();
    }
}

class SubAppIterator //iterates over strict sub-apps of a term
{
    enum Dir { LEFT, RIGHT };
    Dir m_dir;
    std::vector<std::pair<Dir,UE*> > m_stack;
    void leftStar () { TODO(); }
public:
    SubAppIterator (UE& root) { TODO(); }

    operator bool () { return not m_stack.empty(); }
    void next () { TODO(); }

    UE& operator *  () { return *(m_stack.back().second); }
    UE* operator -> () { return m_stack.back().second; }

    ExprHdl subs (ExprHdl here) { TODO(); }
};
Apps critical_pairs (App s_t, App u_v)
{//finds critical pairs of s<=t~u=>v
    //alpha-vary the terms to prevent collision
    TODO();

    //first pre-load all terms
    UE::reset();
    UHdl s = UE::new_expr(s_t.lhs);
    UHdl t = UE::new_expr(s_t.rhs);
    UHdl u = UE::new_expr(u_v.lhs);
    UHdl v = UE::new_expr(u_v.rhs);

    //XXX this should really be length-then-lex, not just length
#define if_critical if ((not (s->get_rep()->size() > t->get_rep()->size())) \
                    and (not (u->get_rep()->size() < v->get_rep()->size())))
    Apps CP;
    //there are three types of overlaps WRT the sub-term ordering: =, <|, |>
    //case t = u
    if (acyclic_unify(&*t,&*u)) {
        if_critical CP.push_back( App(s->get_rep()->expr(),
                                      v->get_rep()->expr()) );
    }
    //case t <| u
    for (SubAppIterator i(*t); i; i.next()) {
        UE::clear_cache();
        if (acyclic_unify(&*i, &*u)) {
            if_critical CP.push_back( App(i.subs(s->get_rep()->expr()),
                                                 v->get_rep()->expr()  ) );
        }
    }
    //case t |> u
    for (SubAppIterator i(*u); i; i.next()) {
        UE::clear_cache();
        if (acyclic_unify(&*t, &*i)) {
            if_critical CP.push_back( App(       s->get_rep()->expr(),
                                          i.subs(v->get_rep()->expr()) ) );
        }
    }
#undef if_critical

    return CP;
}

}

