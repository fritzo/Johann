
#include "statements.h"

namespace Statements
{

//factories
void Conjunction::add (StmtHdl stmt)
{
    Conjunction* conj = stmt->as_conj();
    if (conj) m_terms.insert(m_terms.end(), conj->terms().begin(),
                                            conj->terms().end());
    else m_terms.push_back(stmt);
}
void Disjunction::add (StmtHdl stmt)
{
    Disjunction* disj = stmt->as_disj();
    if (disj) m_terms.insert(m_terms.end(), disj->terms().begin(),
                                            disj->terms().end());
    else m_terms.push_back(stmt);
}

StmtHdl build_equation (ExprHdl pair)
{
    ExprHdl lhs = pair * EX::build_sel(1,2)->as_comb();
    ExprHdl rhs = pair * EX::build_sel(2,2)->as_comb();
    return build_equation(lhs,rhs);
}
StmtHdl build_equal (ExprHdl lhs, ExprHdl rhs)
{
    return build_reln(lhs, EQUAL, rhs);
}
StmtHdl build_less (ExprHdl lhs, ExprHdl rhs)
{
    return build_reln(lhs, LESS_EQUAL, rhs);
}
StmtHdl build_nless (ExprHdl lhs, ExprHdl rhs)
{
    return build_reln(lhs, NOT_LEQ, rhs);
}
StmtHdl build_of_type (ExprHdl term, ExprHdl type)
{
    return build_equal(close(type) * term, term);
}
StmtHdl build_tested (ExprHdl term, ExprHdl test)
{
    return build_equal(semi(test * term), EX::id());
}
StmtHdl build_element (ExprHdl term, ExprHdl pred)
{
    //return build_equal();
    LATER();
    return StmtHdl();
}
StmtHdl build_subtype (ExprHdl smaller, ExprHdl larger)
{
    return build_of_type (close(smaller), power(larger));
}
StmtHdl build_subtest (ExprHdl smaller, ExprHdl larger)
{
    return build_equal (test(smaller), tests(larger,smaller));
}

//buffered binders
StmtHdl Binder::bind (StmtHdl stmt)
{
    switch (m_type) {
    case Symbols::FORALL:  return forall(stmt);
    default:
        logger.error() << "invalid patt: " << Int(m_type) |0;
        return StmtHdl();
    }
}
StmtHdl Binder::forall (StmtHdl stmt)
{
    logger.debug() << "binding forall" |0;
    for (; not m_patts.empty(); m_patts.pop_back()) {
        stmt = new Forall(m_patts.back(), stmt);
    }
    return stmt;
}

//normalization to query normal form
StmtHdl Relationship::query_nf ()
{//query-normal relations are EQUAL, LESS_EQUAL, and NOT_LEQ
    if (m_lhs->isBad() or m_rhs->isBad()) {
        logger.warning() << "ignoring invalid statement: " << *this |0;
        return StmtHdl();
    }
    switch (m_rel) {
        //these are already in query normal form
        case LESS_EQUAL:
        case EQUAL:
        case NOT_LEQ:
            return this;

        //convert derived relations to =,[=,![=
        case OF_TYPE:       return build_of_type    (m_lhs, m_rhs);
        case TESTED:        return build_tested     (m_lhs, m_rhs);
        case ELEMENT:       return build_element    (m_lhs, m_rhs);
        case LESS:          return And(build_less   (m_lhs, m_rhs),
                                       build_nless  (m_rhs, m_lhs));
        case GREATER_EQUAL: return build_less       (m_rhs, m_lhs);
        case NOT_GEQ:       return build_nless      (m_rhs, m_lhs);
        case GREATER:       return And(build_less   (m_rhs, m_lhs),
                                       build_nless  (m_lhs, m_rhs));
        case SUBTYPE:       return build_subtype    (m_lhs, m_rhs);
        case SUPTYPE:       return build_subtype    (m_rhs, m_lhs);
        case SUBTEST:       return build_subtest    (m_lhs, m_rhs);
        case SUPTEST:       return build_subtest    (m_rhs, m_lhs);
        case NEQUAL:        return Not(build_equal  (m_lhs, m_rhs));
        case NOT_OF_TYPE:   return Not(build_of_type(m_lhs, m_rhs));
        case FAILED:        return Not(build_tested (m_lhs, m_rhs));
        case NOT_ELEMENT:   return Not(build_element(m_lhs, m_rhs));
        case NOT_SUBTYPE:   return Not(build_subtype(m_lhs, m_rhs));
        case NOT_SUPTYPE:   return Not(build_subtype(m_rhs, m_lhs));
        case NOT_SUBTEST:   return Not(build_subtest(m_lhs, m_rhs));
        case NOT_SUPTEST:   return Not(build_subtest(m_rhs, m_lhs));
        default:
            logger.warning() << "cannot query_nf unknown relation" |0;
            return StmtHdl();
    }
}
StmtHdl Conjunction::query_nf ()
{
    Conjunction* result = new Conjunction;
    for (unsigned i=0; i<m_terms.size(); ++i) {
        result->add(m_terms[i]->query_nf());
    }
    return result;
}
StmtHdl Disjunction::query_nf ()
{
    Disjunction* result = new Disjunction;
    for (unsigned i=0; i<m_terms.size(); ++i) {
        result->add(m_terms[i]->query_nf());
    }
    return result;
}
StmtHdl Forall::query_nf ()
{
    return m_stmt->query_nf()->_abstract(m_patt);
}
StmtHdl Exists::query_nf ()
{
    logger.error() << "existentials have no query normal form" |0;
    return StmtHdl();
}
StmtHdl Negation::query_nf ()
{
    return m_neg->query_nf()->_neg();
}
StmtHdl Implication::query_nf ()
{
    if (Relationship *reln = m_hyp->as_reln()) {
        if (reln->rel() == TESTED) {
            ExprHdl test = semi(reln->rhs() * reln->lhs());
            StmtHdl result = m_conc->query_nf();
            if (result) result = result->_assuming(test);
            return result;
        }
    }

    logger.error() << "general implications have no query normal form" |0;
    return StmtHdl();
}
StmtHdl Definition::query_nf ()
{
    return m_stmt->_where(m_patt, m_mean);
}

StmtHdl Statement::_neg ()
{
    return query_nf()->_neg();
}
StmtHdl Relationship::_neg ()
{
    switch (m_rel) {
        case LESS_EQUAL:    return      build_nless (m_lhs, m_rhs);
        case NOT_LEQ:       return      build_less  (m_lhs, m_rhs);
        case EQUAL:         return Or(  build_nless (m_lhs, m_rhs),
                                        build_nless (m_rhs, m_lhs));
        default:            return query_nf()->_neg();
    }
}
StmtHdl Conjunction::_neg ()
{
    Disjunction* result = new Disjunction();
    for (unsigned i=0; i<m_terms.size(); ++i) {
        result->add(m_terms[i]->_neg());
    }
    return result;
}
StmtHdl Disjunction::_neg ()
{
    Conjunction* result = new Conjunction();
    for (unsigned i=0; i<m_terms.size(); ++i) {
        result->add(m_terms[i]->_neg());
    }
    return result;
}

StmtHdl Statement::_abstract (PattHdl patt)
{
    return query_nf()->_abstract(patt);
}
StmtHdl Relationship::_abstract (PattHdl patt)
{
    if (m_rel!=EQUAL and m_rel!=LESS_EQUAL) {
        logger.warning() << "cannot quantify relation "
                         << RelationNames[m_rel] |0;
        return StmtHdl();
    }
    if (patt->isBad()) {
        logger.warning() << "bad pattern in hypothesis: " << patt |0;
        return StmtHdl();
    }
    return new Relationship (patt->lambda(m_lhs), m_rel,
                             patt->lambda(m_rhs));
}
StmtHdl Conjunction::_abstract (PattHdl patt)
{
    Conjunction* result = new Conjunction();
    for (unsigned i=0; i<m_terms.size(); ++i) {
        StmtHdl stmt = m_terms[i]->_abstract(patt);
        if (!stmt) { delete result; return StmtHdl(); }
        result->add(stmt);
    }
    return result;
}
StmtHdl Disjunction::_abstract (PattHdl patt)
{
    logger.warning() << "cannot quantify disjunctions" |0;
    return StmtHdl();
}

StmtHdl Statement::_where (PattHdl patt, ExprHdl mean)
{
    return query_nf()->_where(patt, mean);
}
StmtHdl Relationship::_where (PattHdl patt, ExprHdl mean)
{
    if (patt->isBad()) {
        logger.warning() << "bad pattern in hypothesis: " << patt |0;
        return StmtHdl();
    }
    return new Relationship (m_lhs->where(patt,mean), m_rel,
                             m_rhs->where(patt,mean));
}
StmtHdl Conjunction::_where (PattHdl patt, ExprHdl mean)
{
    Conjunction* result = new Conjunction();
    for (unsigned i=0; i<m_terms.size(); ++i) {
        StmtHdl stmt = m_terms[i]->_where(patt,mean);
        if (!stmt) { delete result; return StmtHdl(); }
        result->add(stmt);
    }
    return result;
}
StmtHdl Disjunction::_where (PattHdl patt, ExprHdl mean)
{
    Disjunction* result = new Disjunction();
    for (unsigned i=0; i<m_terms.size(); ++i) {
        StmtHdl stmt = m_terms[i]->_where(patt,mean);
        if (!stmt) { delete result; return StmtHdl(); }
        result->add(stmt);
    }
    return result;
}

StmtHdl Statement::_assuming (ExprHdl test)
{
    return query_nf()->_assuming(test);
}
StmtHdl Relationship::_assuming (ExprHdl test)
{
    if (m_rel!=EQUAL and m_rel!=LESS_EQUAL) {
        logger.warning() << "cannot add hypothesis to relation "
                         << RelationNames[m_rel] |0;
        return StmtHdl();
    }
    return new Relationship (test * m_lhs, m_rel, test * m_rhs);
}
StmtHdl Conjunction::_assuming (ExprHdl test)
{
    Conjunction* result = new Conjunction();
    for (unsigned i=0; i<m_terms.size(); ++i) {
        StmtHdl stmt = m_terms[i]->_assuming(test);
        if (!stmt) { delete result; return StmtHdl(); }
        result->add(stmt);
    }
    return result;
}
StmtHdl Disjunction::_assuming (ExprHdl test)
{
    Disjunction* result = new Disjunction();
    for (unsigned i=0; i<m_terms.size(); ++i) {
        StmtHdl stmt = m_terms[i]->_assuming(test);
        if (!stmt) { delete result; return StmtHdl(); }
        result->add(stmt);
    }
    return result;
}

StmtHdl Relationship::_assume_nf ()
{
    return this;
}
StmtHdl Conjunction::_assume_nf ()
{
    for (unsigned i=0; i<m_terms.size(); ++i) {
        if (!m_terms[i]->_assume_nf()) return StmtHdl();
    }
    return this;
}

//methods of query normal forms
Expr::VarSet Statement::vars ()
{
    return query_nf()->vars();
}
Expr::VarSet Relationship::vars ()
{
    return m_lhs->vars() + m_rhs->vars();
}
Expr::VarSet Conjunction::vars ()
{
    Expr::VarSet result = Expr::VarSet::empty_set();
    for (Int i=0; i<m_terms.size(); ++i) {
        result += m_terms[i]->vars();
    }
    return result;
}
Expr::VarSet Disjunction::vars ()
{
    Expr::VarSet result = Expr::VarSet::empty_set();
    for (Int i=0; i<m_terms.size(); ++i) {
        result += m_terms[i]->vars();
    }
    return result;
}

std::vector<ExprHdl> Statement::relevant_exprs ()
{
    return query_nf()->relevant_exprs();
}
std::vector<ExprHdl> Relationship::relevant_exprs ()
{
    std::vector<ExprHdl> result;
    result.push_back(m_lhs);
    result.push_back(m_rhs);
    return result;
}
std::vector<ExprHdl> Conjunction::relevant_exprs ()
{
    std::vector<ExprHdl> result;
    for (Int i=0; i<m_terms.size(); ++i) {
        std::vector<ExprHdl> part = m_terms[i]->relevant_exprs();
        result.insert(result.end(), part.begin(), part.end());
    }
    return result;
}
std::vector<ExprHdl> Disjunction::relevant_exprs ()
{
    std::vector<ExprHdl> result;
    for (Int i=0; i<m_terms.size(); ++i) {
        std::vector<ExprHdl> part = m_terms[i]->relevant_exprs();
        result.insert(result.end(), part.begin(), part.end());
    }
    return result;
}

StmtHdl Statement::map (const EX::ExprFun& fun)
{
    return query_nf()->map(fun);
}
StmtHdl Relationship::map (const EX::ExprFun& fun)
{
    ExprHdl lhs = fun(m_lhs); if (!lhs) return StmtHdl();
    ExprHdl rhs = fun(m_rhs); if (!rhs) return StmtHdl();
    return new Relationship(lhs, m_rel, rhs);
}
StmtHdl Conjunction::map (const EX::ExprFun& fun)
{
    StmtHdl result = new Conjunction();
    for (Int i=0; i<m_terms.size(); ++i) {
        StmtHdl term = m_terms[i]->map(fun); if (!term) return StmtHdl();
        result->as_conj()->add(term);
    }
    return result;
}
StmtHdl Disjunction::map (const EX::ExprFun& fun)
{
    StmtHdl result = new Disjunction();
    for (Int i=0; i<m_terms.size(); ++i) {
        StmtHdl term = m_terms[i]->map(fun); if (!term) return StmtHdl();
        result->as_disj()->add(term);
    }
    return result;
}

Prob Statement::guess (Guesser& g)
{
    return query_nf()->guess(g);
}
Prob Relationship::guess (Guesser& g)
{
    switch (m_rel) {
        case EQUAL:        return g.equal(m_lhs, m_rhs);
        case LESS_EQUAL:   return g.less(m_lhs, m_rhs);
        default:
            logger.warning() << "unguessable relation type" |0;
            return P_FALSE;
    }
}
Prob Conjunction::guess (Guesser& g)
{
    Prob result = P_TRUE;
    for (unsigned i=0; i<m_terms.size(); ++i) {
        result = Symbols::And(result, m_terms[i]->guess(g));
    }
    return result;
}
Prob Disjunction::guess (Guesser& g)
{
    Prob result = P_FALSE;
    for (unsigned i=0; i<m_terms.size(); ++i) {
        result = Symbols::Or(result, m_terms[i]->guess(g));
    }
    return result;
}

//reflection
StmtHdl Relationship::negation_nf ()
{
    return this;
}
StmtHdl Conjunction::negation_nf ()
{
    Conjunction* result = new Conjunction;
    for (unsigned i=0; i<m_terms.size(); ++i) {
        result->add(m_terms[i]->negation_nf());
    }
    return result;
}
StmtHdl Disjunction::negation_nf ()
{
    Disjunction* result = new Disjunction;
    for (unsigned i=0; i<m_terms.size(); ++i) {
        result->add(m_terms[i]->negation_nf());
    }
    return result;
}
StmtHdl Forall::negation_nf ()
{
    LATER(); //deal with non-variable patterns
    return StmtHdl();
}
StmtHdl Exists::negation_nf ()
{
    LATER(); //deal with non-variable patterns
    return StmtHdl();
}
StmtHdl Negation::negation_nf () { LATER(); return StmtHdl(); }
StmtHdl Implication::negation_nf () { LATER(); return StmtHdl(); }
StmtHdl Definition::negation_nf () { LATER(); return StmtHdl(); }

ExprHdl Statement::to_semi ()
{
    return negation_nf()->to_semi();
}
ExprHdl Relationship::to_semi () { LATER(); return ExprHdl(); }
ExprHdl Conjunction::to_semi () { LATER(); return ExprHdl(); }
ExprHdl Disjunction::to_semi () { LATER(); return ExprHdl(); }
ExprHdl Forall::to_semi () { LATER(); return ExprHdl(); }
ExprHdl Exists::to_semi () { LATER(); return ExprHdl(); }

ExprHdl Statement::to_bool ()
{
    return negation_nf()->to_bool();
}
ExprHdl Relationship::to_bool () { LATER(); return ExprHdl(); }
ExprHdl Conjunction::to_bool () { LATER(); return ExprHdl(); }
ExprHdl Disjunction::to_bool () { LATER(); return ExprHdl(); }
ExprHdl Forall::to_bool () { LATER(); return ExprHdl(); }
ExprHdl Exists::to_bool () { LATER(); return ExprHdl(); }

//pretty printing
StmtHdl Relationship::pretty ()
{
    return new Relationship(m_lhs->pretty(), m_rel, m_rhs->pretty());
}
StmtHdl Forall::pretty ()
{
    return this;
    //this shouldn't be necessary
    //return new Forall(m_patt, m_stmt->pretty());
}

//output LATER: add parens where needed
ostream& Relationship::write_to (ostream& os) const
{
    return os << m_lhs << ' ' << RelationNames[m_rel] << ' ' << m_rhs;
}
ostream& Conjunction::write_to (ostream& os) const
{
    os << m_terms[0];
    for (unsigned i=1; i<m_terms.size(); ++i) {
         os << "  AND  " << m_terms[i];
    }
    return os;
}
ostream& Disjunction::write_to (ostream& os) const
{
    os << m_terms[0];
    for (unsigned i=1; i<m_terms.size(); ++i) {
         os << "  OR  " << m_terms[i];
    }
    return os;
}
ostream& Forall::write_to (ostream& os) const
{
    return os << "(/\\" << m_patt << ". " << m_stmt << ')';
}
ostream& Exists::write_to (ostream& os) const
{
    return os << "(\\/" << m_patt << ". " << m_stmt << ')';
}
ostream& Negation::write_to (ostream& os) const
{
    return os << "NOT " << m_neg;
}
ostream& Implication::write_to (ostream& os) const
{
    return os << m_hyp << "  ==>  " << m_conc;
}
ostream& Definition::write_to (ostream& os) const
{
    return os << '(' << m_patt << ":=" << m_mean << ". " << m_stmt << ')';
}

}


