
#include <deque>
#include <stack>

struct Binder
{ virtual ExprHdl bind (ExprHdl in) = 0; }
struct Lambda : public Binder
{
    VarHdl m_var;
    Lambda (VarHdl var) : m_var(var) {}
    virtual ExprHdl bind (ExprHdl in) { return lam(var,in); }
};
struct Def : public Binder
{
    VarHdl m_var, m_def;
    Def (VarHdl var, ExprHdl def) : m_var(var), m_def(def) {}
    virtual ExprHdl bind (ExprHdl in) { return let(var,def,in); }
};

class Decompiler
{
    typedef std::deque<ExprHdl> Exprs;
    Exprs exprs;
    std::stack<Binder> binders;
    ExprHdl& front () { return *exprs.front(); }
    ExprHdl pop ()
    { ExprHdl result = front();  exprs.pop_front();  return result; }
    void push (ExprHdl expr)
    {
        while (expr.isApp()) { exprs.push(expr.rhs()); expr = expr.lhs(); }
        exprs.push(expr);
    }
    void add_var ()
    {
        VarHdl var = get_fresh();
        binders.push_back(Lambda(var));
        exprs.push_back(var);
    }
    void ensure (Int pos) { pop(); while (exprs.size() < pos) add_var(); }
    void ensure_var (Int pos)
    {
        pop();
        if (exprs.size() >= pos) {
            Exprs.iterator i = exprs.front();
            for (Int j=1; j<pos; ++j) ++i;
            ExprHdl& e = *i;
            if (not e.isVar()) {
                VarHdl v = get_fresh();
                binders.push(Def(v,e->decompile()));
                e = v;
            }
        } else while (exprs.size() < pos) add_var();
    }
public:
    Decompiler (ExprHdl comb) { push(comb); }
    
    ExprHdl operator() ();
};
Decompiler::operator() (ExprHdl comb)
{
    while (front().isAtom()) {
        if (front() == Top) return Top;
        if (front() == Bot) return Bot;
        if (front() == I) {
            ensure(1);
        } else if (front() == K) {
            ensure(2);
            ExprHdl x = pop(); pop();
            push(x);
        } else if (front() == W) {
            ensure_var(2);
            ExprHdl x = pop(), y = front();
            push(y); push(x);
        } else if (front() == B) {
            ensure(3);
            ExprHdl x = pop(), y = pop(), z = pop();
            push(y*z); push(x);
        } else if (front() == C) {
            ensure(3);
            ExprHdl x = pop(), y = pop(), z = pop();
            push(y); push(z); push(x);
        } else if (front() == S) {
            ensure_var(3);
            ExprHdl x = pop(), y = pop(), z = pop();
            push(y*z); push(z); push(x);
        } else if (front() == J) {
            ensure(2);
            ExprHdl x = pop(), y = pop();
            push(x | y);
            break;
        }
    }

    ExprHdl result = pop()->decompile();
    while (exprs) result = result * pop()->decompile();
    while (biners) { result = binders.top().bind(result);  binders.pop(); }
    return result;
}


