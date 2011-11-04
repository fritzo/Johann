
#include "parser.h"
#include <fstream>

namespace PP
{

//parsing interface
ExprHdl Driver::parse_expr (istream& in, ostream& err, FileType ftype)
{
    ResultType type = parse(in,err,ftype);
    ExprHdl result = result_expr ? result_expr : EX::bad();
    result_expr.clear();
    result_subs.clear();
    if (type == SUBS) { err << "parsed subs when expecting expr\n"; }
    return result;
}
SubsHdl Driver::parse_subs (istream& in, ostream& err, FileType ftype)
{
    ResultType type = parse(in,err,ftype);
    SubsHdl result = result_subs ? result_subs : Subs::id();
    result_expr.clear();
    result_subs.clear();
    if (type == EXPR) { err << "parsed expr when expecting subs\n"; }
    return result;
}


//context stack
const char* contextSymb[] = {"  ","()","<>","[]"};
bool Driver::pop_context (ContextType t)
{
    if (m_contextStack.empty()) {
        error() << "extra closing " << contextSymb[t]
                << " at EOL" << std::endl;
        return false;
    }
    if (top_context() != t) {
        error() << "mismatched " << contextSymb[t]
                << " inside " << contextSymb[top_context()] << std::endl;
        m_contextStack.pop();
        return false;
    }
    m_contextStack.pop();
    return true;
}
bool Driver::end_context ()
{
    bool result = true;
    while (not m_contextStack.empty()) {
        result = pop_context() and result;
    }
    return result;
}

}

