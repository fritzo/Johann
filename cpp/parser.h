#ifndef JOHANN_CONSOLE_H
#define JOHANN_CONSOLE_H

#include <iostream>
#include <vector>
#include <stack> //for ContextStack
#include "definitions.h"
#include "expressions.h"
#include "substitute.h"
//#include "statements.h" //LATER
#include "symbols.h"
#include "buffer.h"
#include "output.h"

//log levels
#define LOG_DEBUG1(mess)
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << mess |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

// Announce to Flex the prototype we want for lexing function, ...
#define YY_DECL                                     \
PP::Parser::token_type                              \
PPlex (PP::Parser::semantic_type* yylval,           \
       PP::Parser::location_type* yylloc,    \
       void* yyscanner)
// ... and declare it for the parser's sake.
//YY_DECL;

namespace PP
{

const Logging::Logger logger("parser", Logging::DEBUG);

namespace EX = Expressions;
using EX::Binder;

//context stack
enum ContextType { Vacuum, Paren, Angle, Brace };
class ContextStack : public std::stack<ContextType>
{
    typedef std::stack<ContextType> Base;
public:
    void clear ();
    ContextType pop (ContextType t);
    ContextType top () { return empty() ? Vacuum : Base::top(); }
};

//parser driver
enum FileType { JCODE, JTEXT };
class Driver
{
    istream* m_input;
    ostream* m_error;
public:
    istream& input () { Assert(m_input, "no input"); return (*m_input); }
    ostream& error () { Assert(m_error, "no input"); return (*m_error); }

    //implemented in parser.l
    void* scanner;
    string file;
    Driver (string fname="");
    virtual ~Driver ();

    //context stack
private:
    std::stack<ContextType> m_contextStack;
public:
    ContextType top_context () const
        { return m_contextStack.empty() ? Vacuum : m_contextStack.top(); }
    void push_context (ContextType c) { m_contextStack.push(c); }
    bool pop_context  (ContextType c=Vacuum);   //true on success
    bool end_context ();                        //true on success

    //buffers for temporary variables
private:
    nonstd::buffer<string>                t_strings;
    nonstd::buffer<ExprHdl>               t_exprs;
    nonstd::buffer<VarHdl>                t_vars;
    nonstd::buffer<PattHdl>               t_patts;
    nonstd::buffer<SubsHdl>               t_subs;
    nonstd::buffer<std::vector<ExprHdl> > t_exprLists;
    nonstd::buffer<std::vector<PattHdl> > t_pattLists;
    nonstd::buffer<Binder>                t_binders;
    void clear_buffers ()
    {
        t_strings.clear();
        t_exprs.clear();
        t_vars.clear();
        t_subs.clear();
        t_exprLists.clear();
        t_binders.clear();
    }
public:
    void buffer (string               *p) { t_strings   .insert(p); }
    void buffer (ExprHdl              *p) { t_exprs     .insert(p); }
    void buffer (VarHdl               *p) { t_vars      .insert(p); }
    void buffer (PattHdl              *p) { t_patts     .insert(p); }
    void buffer (SubsHdl              *p) { t_subs      .insert(p); }
    void buffer (std::vector<ExprHdl> *p) { t_exprLists .insert(p); }
    void buffer (std::vector<PattHdl> *p) { t_pattLists .insert(p); }
    void buffer (Binder               *p) { t_binders   .insert(p); }

    //parsing interface
private:
    //PP::Parser sets these, parse() clears them
    ExprHdl result_expr;
    SubsHdl result_subs;
public:
    void set_expr (ExprHdl e) { result_expr = e; }
    void set_subs (SubsHdl s) { result_subs = s; }

    enum ResultType { ERROR, EXPR, SUBS };
    ExprHdl parse_expr (istream& in, ostream& err, FileType ftype=JCODE);
    SubsHdl parse_subs (istream& in, ostream& err, FileType ftype=JCODE);
    ResultType parse   (istream& in, ostream& err, FileType ftype=JCODE);
};

//start-state stack
typedef std::vector<int> StartStack;
extern StartStack* startStack;

}

#endif

