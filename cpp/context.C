
#include "context.h"
#include "syntax_semantics.h"
#include <algorithm>

//params
#define BLOCK_SIZE 5

namespace Contexts
{

namespace EX = Expressions;

//simplification
void Context::simplify (bool create)
{
    logger.debug() << "simplifying definitions" |0;
    Logging::IndentBlock block;

    m_simple.clear();
    m_names.clear();
    for (def_iter i=def_begin(); i!=def_end(); ++i) {
        _name(i->first, i->second);
    }
}

//definition
ExprHdl Context::meaning_of (VarHdl name) const
{//returns defn or the variable
    logger.debug() << "finding meaning of " << name |0;
    def_citer i = m_defs.find(&*name);
    return (i != m_defs.end()) ? i->second : ExprHdl(&*name);
}
ExprHdl Context::simple_meaning (VarHdl name) const
{//returns defn or the variable
    logger.debug() << "finding simle meaning of " << name |0;
    def_citer i = m_simple.find(&*name);
    return (i != m_simple.end()) ? i->second : ExprHdl(&*name);
}
VarHdl Context::name_of (ExprHdl meaning) const
{//returns name or Null handle
    logger.debug() << "finding name of " << meaning |0;
    Var * null = NULL;
    std::pair<Expr*,Var*> key(&*meaning, null);
    Names::const_iterator i = m_names.find(&*meaning);
    return (i != m_names.end()) ? i->second : VarHdl();
}
bool Context::operator() (VarHdl name, ExprHdl meaning)
{
    //prevent multiple definition
    if (m_defs.find(&*name) != m_defs.end()) {
        logger.error() << name << " is already defined" |0;
        return false;
    }

    _define(name,meaning);
    _name(name,meaning);
    return true;
}
void Context::_define (VarHdl name, ExprHdl meaning)
{
    logger.debug() << "adding def: " << name << " := " << meaning |0;
    def_iter i = m_defs.find(&*name);
    if (i == m_defs.end()) {
        logger.debug() << "  (making new definition)" |0;
        m_defs[&*name] = meaning;
    } else {
        logger.error() << "redefinition is incorrectly implemented" |0;

        logger.debug() << "  (replacing existing definition)" |0;
        def_iter j = m_simple.find(&*name);
        if (j != m_simple.end()) {
            m_names.erase(&*(j->second));
            m_simple.erase(j);
        }
        //FIXME XXX replace occurrences in other definitions by previous def.
        m_defs[&*name] = meaning;
    }
}
void Context::_name (VarHdl name, ExprHdl meaning)
{
    meaning = expand(meaning)->as_pure();
    if (not meaning->isBad()) {
        logger.debug() << "adding name: " << name << " = " << meaning |0;
        meaning = EX::simplify(meaning);
        if (m_names.find(&*meaning) == m_names.end()) { //don't replace
            m_simple[&*name] = meaning;
            m_names[&*meaning] = name;
        }
    }
}
bool Context::define (PattHdl patt, ExprHdl meaning)
{
    if (patt->isBad()) return false;
    m_raw.push_back(RawDef(patt, meaning));
    return patt->define(*this, meaning);
}

//expansion & compression
class Expand : public EX::StructlFun
{
    static const Context* s_context;
    ExprHdl& m_expr;
    Int m_depth;
    Expand (ExprHdl& expr, Int depth) : m_expr(expr), m_depth(depth) {}
    virtual ~Expand () {}
public:
    static void set_context (const Context* context) { s_context = context; }
    static void call (ExprHdl& expr, Int depth) { Expand(expr, depth)(*expr); }

    virtual void of_var (VarHdl var)
    {
        Assert2(m_depth, "Expand called at depth 0");
        ExprHdl expanded = s_context->meaning_of(var);
        if (expanded != m_expr) {
            m_expr = expanded;
            Int depth = m_depth - 1; //one expansion has already taken place
            if (depth) call(m_expr, depth);
        }
    }
    virtual void of_app (Expr& lhs, Expr& rhs)
    {
        Assert2(m_depth, "Expand called at depth 0");
        ExprHdl e_lhs = &lhs;  call(e_lhs, m_depth);
        ExprHdl e_rhs = &rhs;  call(e_rhs, m_depth);
        m_expr = EX::build_app(e_lhs, e_rhs);
    }
    virtual void of_comp (Expr& lhs, Expr& rhs)
    {
        Assert2(m_depth, "Expand called at depth 0");
        ExprHdl e_lhs = &lhs;  call(e_lhs, m_depth);
        ExprHdl e_rhs = &rhs;  call(e_rhs, m_depth);
        m_expr = e_lhs % e_rhs;
    }
    virtual void of_join (Expr& lhs, Expr& rhs)
    {
        Assert2(m_depth, "Expand called at depth 0");
        ExprHdl e_lhs = &lhs;  call(e_lhs, m_depth);
        ExprHdl e_rhs = &rhs;  call(e_rhs, m_depth);
        m_expr = e_lhs | e_rhs;
    }
};
const Context* Expand::s_context;
ExprHdl Context::expand (ExprHdl small, Int depth) const
{//expands by given depth
    if (depth) {
        Expand::set_context(this);
        Expand::call(small, depth);
    }
    return small;
}
class Compress : public EX::StructlFun
{
    static const Context* s_context;
    ExprHdl& m_expr;
    Int m_depth;
    Compress (ExprHdl& expr, Int depth) : m_expr(expr), m_depth(depth) {}
    virtual ~Compress () {}
public:
    static void set_context (const Context* context) { s_context = context; }
    static void call (ExprHdl& expr, Int depth) { Compress(expr,depth)(*expr); }

    virtual void of_var (VarHdl var)
    {
        //look for meaning of whole
        VarHdl name = s_context->name_of(m_expr);
        if (name) m_expr = name;
    }
    virtual void of_app (Expr& lhs, Expr& rhs)
    {
        if (m_depth) {
            //look for meaning of parts
            ExprHdl c_lhs = &lhs;  call(c_lhs, m_depth-1);
            ExprHdl c_rhs = &rhs;  call(c_rhs, m_depth-1);
            m_expr = EX::build_app(c_lhs, c_rhs);
        } else {
            //look for meaning of whole
            VarHdl name = s_context->name_of(m_expr);
            if (name) m_expr = name;
        }
    }
    virtual void of_comp (Expr& lhs, Expr& rhs)
    {
        if (m_depth) {
            //look for meaning of parts
            ExprHdl c_lhs = &lhs;  call(c_lhs, m_depth-1);
            ExprHdl c_rhs = &rhs;  call(c_rhs, m_depth-1);
            m_expr = c_lhs % c_rhs;
        } else {
            //look for meaning of whole
            VarHdl name = s_context->name_of(m_expr);
            if (name) m_expr = name;
        }
    }
    virtual void of_join (Expr& lhs, Expr& rhs)
    {
        if (m_depth) {
            //look for meaning of parts
            ExprHdl c_lhs = &lhs;  call(c_lhs, m_depth-1);
            ExprHdl c_rhs = &rhs;  call(c_rhs, m_depth-1);
            m_expr = c_lhs | c_rhs;
        } else {
            //look for meaning of whole
            VarHdl name = s_context->name_of(m_expr);
            if (name) m_expr = name;
        }
    }
};
const Context* Compress::s_context;
ExprHdl Context::compress (ExprHdl large, Int depth) const
{//compresses to given depth
    Compress::set_context(this);
    Compress::call(large, depth);
    return large;
}

//output
ostream& operator<< (ostream& os, Context& c)
{
    std::vector<string> consts;
    for (Context::def_iter i=c.def_begin(); i!=c.def_end(); ++i) {
        consts.push_back(*(i->first->name()));
    }

    std::sort(consts.begin(), consts.end(), compare_nocase());

    for (unsigned i=0; i<consts.size(); ++i) {
        os << consts[i] << ' ';
    }

    return os;
}
void Context::write_to (ostream& os)
{
    for (Int i=0; i<m_raw.size(); ++i) {
        os << m_raw[i].patt << " := " << m_raw[i].meaning << ".\n\n";
    }
}
void Context::save_to (ostream& os)
{
    os << "#\\subsection{Context}\n\n";
    for (RawDefs::iterator i=m_raw.begin(); i!=m_raw.end(); ++i) {
        os << i->patt << " := " << i->meaning << ".\n\n";
    }
}
void Context::write_stats_to (ostream& os)
{
    os << '\t' << m_raw.size() << " patterned defs = "
        << m_defs.size() << " atomic defs" << std::endl;
}

}


