
typedef Expr* Name;

template<class T> inline void swap (T& x, T& y) { T z=x; x=y; y=z; }

enum ExprType { VAR, CST, APP };
class UnifyExpr
{
    typedef UnifyExpr UE;

    //rep trees
    UE *m_rep;
    unsigned m_count;

    //data
    void* m_data[2];
public:

    //union-find
    inline void UE::set_rep (UE* rep);      //takes care of ref counts
    UE* get_rep ();                         //collapses rep path
    void merge (UE* rep);                   //unweighted, will not swap
    static void merge (UE* dep, UE* rep);   //weighted, may swap dep/rep

    //type dispatch
    ExprType type () const { return !m_data[1] ? VAR : !m_data[0] ? CST : APP; }
    bool isVar () const { return !m_data[1]; }
    bool isCst () const { return !m_data[0]; }
    bool isApp () const { return m_data[0] and m_data[1]; }

    //data access
    Name var () const { return reinterpret_cast<Name*>(m_data)[0]; }
    Name cst () const { return reinterpret_cast<Name*>(m_data)[1]; }
    UE*  lhs () const { return reinterpret_cast<UnionExpr*>(m_data)[0]; }
    UE*  rhs () const { return reinterpret_cast<UnionExpr*>(m_data)[1]; }
    void var (Name n) { reinterpret_cast<Name*>(m_data)[0] = n; }
    void cst (Name n) { reinterpret_cast<Name*>(m_data)[1] = n; }
    void lhs (UE* l)  { reinterpret_cast<UnionExpr*>(m_data)[0] = l; }
    void rhs (UE* r)  { reinterpret_cast<UnionExpr*>(m_data)[1] = r; }

    //initializers for pool management
    typedef MAP_TYPE<Expr*,UE*> Table;
private:
    static Table s_table;
    static UE *s_pool, *s_top;
    static unsigned s_size;
    static new_ () { ++s_top; Assert2(s_top < s_pool+s_size, "overflow"); }
public:
    static void resize (unsigned size);
    static void reset ();

    //in-place constructors
    static UE* new_var (Name n) { new_(); s_top->var(n); return s_top; }
    static UE* new_cst (Name n) { new_(); s_top->cst(n); return s_top; }
    static UE* new_app (UE* l, UE* r)
    { new_(); s_top->lhs(l); s_top->rhs(r); return s_top; }

    //Expr <--> UnifyExpr translations
    UE* new_expr (ExprHdl e);
    ExprHdl expr () const;
};
typedef UnifyExpr UE;

//pool management
UE* UE::s_pool = NULL;
UE* UE::s_top = NULL;
unsigned UE::s_size = 0;
UE::Table UE::s_table;
void UE::resize (unsigned size)
{
    if (s_size) { delete s_pool; s_pool = NULL; }
    if (size) { s_pool = new UE[size]; }
    s_size = size;
}
void UE::reset (unsigned size)
{
    if (s_size < size) resize(max(size,s_size*2));
    if (!s_size) return;
    bzero(s_pool, s_size * sizeof(UE));
    s_top = s_pool;
    s_table.clear();
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
    return !m_rep ? this : set_rep(m_rep->get_rep());
}
void UE::merge (UE* dep)
{//unweighted union
    dep->get_rep()->set_rep(get_rep());
}
void UE::merge (UE* dep, UE* rep)
{//weighted union
    dep = dep->get_rep();
    rep = rep->get_rep();
    if (dep->m_count > rep->m_count) swap(rep,dep);
    dep->set_rep(rep);
}

bool cyclic_unify (UE* lhs, UE* rhs)
{//unifies two dags into a directed possibly-cyclic graph, or fails
    if (lhs == rhs) return true;
    if (rhs->isVar()) swap(lhs,rhs);
    ExprType lhs_type = lhs->type();
    ExprType rhs_type = rhs->type();

    switch (lhs_type) {
        case VAR: {
            if (rhs_type == VAR) UE::merge(lhs,rhs); //depricate the less-ref'd
            else                 lhs->merge(rhs);    //define lhs := rhs
            return true;
        }
        case CST: {
            Assert3 (rhs_type != VAR, "invalid rhs type");
            return false;
        }
        case APP: {
            if (rhs_type == CST) return false;
            Assert3 (rhs_type == APP, "invalid rhs type");
            return cyclic_unify(rhs->lhs()->get_rep(), lhs->lhs()->get_rep())
               and cyclic_unify(rhs->rhs()->get_rep(), lhs->rhs()->get_rep());
        }
    }
}

UE* acyclic_unify (UE* lhs, UE* rhs)
{//check for cycles
    if (not cyclic_unify(lhs,rhs)) return NULL;
    UE* result = lhs->get_rep();
    LATER();
}

//Expr <--> UnifyExpr translations
UE* UE::new_expr (ExprHdl e)
{//creates a UE dag corresponding to an Expr dag
    Table::iterator pos = s_table->find(&*e);
    if (pos != s_table.end()) return *pos;

    UE* result;
    if (e->isApp()) {
        result = new_app(new_expr(e->lhs()), new_expr(e->rhs()));
    } else { if (e->isVar()) {
        result = new_var(&*e);
    } else { Assert3(e->isConst(), "bad expr type: " << e);
        result = new_cst(&*e);
    } }

    s_table[&*e] = result;
    return result;
}
ExprHdl UE::expr () const
{//WARNING: only works on acyclic, rep-reduced graphs
    Assert3(!m_rep, "tried to convert non-rep");
    switch (type()) {
        case APP: return lhs()->expr() * rhs->expr();
        case VAR: return var();
        case CST: return cst();
    }
}

//XXX: how to deal with cycles?

