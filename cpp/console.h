#ifndef JOHANN_CONSOLE_H
#define JOHANN_CONSOLE_H

#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <stack> //for ContextStack
#include <cmath>
#include "definitions.h"
#include "version.h"
#include "symbols.h"
#include "expressions.h"
#include "statements.h"
#include "substitute.h"
#include "kernel.h" //must come after expressions.h for LOCK_SYNTAX_DEBUG

//log levels
#define LOG_DEBUG1(mess)
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << mess |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

const Logging::Logger logger("console", Logging::INFO);
const Logging::Logger user_logger("user", Logging::INFO); //don't change this

namespace K =  Kernel;
namespace EX = Expressions;
namespace ST = Statements;
namespace S = Substitute;
using K::context;

//output tools
//TODO this should be moved to set_wrap.h
template <class T> ostream& operator<< (ostream& os, const std::set<T>& s)
{
    os << '{';
    typename std::set<T>::iterator pos = s.begin();
    if (pos != s.end()) {
        os << *pos;
        for(++pos; pos != s.end(); ++pos) {
            os << ", " << *pos;
        }
    }
    return os << '}';
}
//TODO this should be moved to vector_wrap.h
template <class T> ostream& operator<< (ostream& os, const std::vector<T>& v)
{
    os << '[';
    int N = v.size();
    if (N) {
        os << v[0];
        for(int i=1; i<N; ++i) {
            os << ", " << v[i];
        }
    }
    return os << ']';
}

//various text
const char* const in_prompt  = "\e[33m(U)\e[0m ";
const char* const deb_prompt = "\e[33m(d)\e[0m ";
const char* const out_prompt = "\e[32m(J)\e[0m ";
const char* const warn_prompt= "\e[31m(w)\e[0m ";
const char* const err_prompt = "\e[7;31m(!)\e[0m ";

//i/o
ostream& j_output ();
ostream& j_debug ();
ostream& j_info ();
ostream& j_warning (int lines_delayed=0);
ostream& j_error (int lines_delayed=0);
extern bool g_interactive_input;
extern bool g_interactive_output;
extern bool g_quiet;
extern bool g_quietly;
extern bool g_skimming;
void set_quietly (bool quietly);
int get_line_number ();
string get_location ();
extern string default_name; //for save,load

//file reading stack, implemented in console.y
bool listening ();
bool interactive ();
void push_input ();
bool push_server ();
void clear_files ();
bool push_file (string filename, bool skimming=false, bool whine=true);
bool pop_file ();
void restart (bool notation=true);

//various typedefs for bison/lex
typedef std::vector<std::pair<ExprHdl, Float> > ExprPMF;
using ST::Binder;

//buffers for unhandled objects
template <class T>
class Buffer
{
private:
    std::set<T*> buffer;
    typedef typename std::set<T*>::iterator Iter;
public:
    void insert (T* t) { buffer.insert( t); }
    //void insert (T  t) { buffer.insert(&t); }
    void clear ();
};
template <class T>
void Buffer<T>::clear ()
{
    LOG_DEBUG1( "clearing buffer" )
    for (Iter i=buffer.begin(); i!=buffer.end(); ++i) delete *i;
    buffer.clear();
}
extern Buffer<string>                tempStrings;
extern Buffer<std::vector<string> >  tempStringLists;
extern Buffer<ExprHdl>               tempExprs;
extern Buffer<VarHdl>                tempVars;
extern Buffer<std::vector<ExprHdl> > tempExprLists;
extern Buffer<ExprPMF>               tempExprPMFs;
extern Buffer<std::vector<Int> >     tempIntLists;
extern Buffer<PattHdl>               tempPatts;
extern Buffer<std::vector<PattHdl> > tempPattLists;
extern Buffer<Binder>                tempBinders;
extern Buffer<StmtHdl>               tempStmts;
inline void clear_buffers ()
{
    tempStrings.clear();
    tempStringLists.clear();
    tempExprs.clear();
    tempVars.clear();
    tempExprLists.clear();
    tempExprPMFs.clear();
    tempIntLists.clear();
    tempPatts.clear();
    tempPattLists.clear();
    tempBinders.clear();
    tempStmts.clear();
}

//context stack
enum ContextType { Vacuum, Paren, Square, Angle, Squiggle, Quote, NoEOL };
class ContextStack : public std::stack<ContextType>
{
    typedef std::stack<ContextType> Base;
public:
    void clear ();
    void pop (ContextType t);
    ContextType top () { return empty() ? Vacuum : Base::top(); }
};
extern ContextStack contextStack;
void pause_EOL ();
void resume_EOL ();

//start-state stack
typedef std::vector<int> StartStack;
extern StartStack* startStack;

#endif

