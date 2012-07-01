#ifndef JOHANN_DEFINITIONS_H
#define JOHANN_DEFINITIONS_H

#define LOG_FILE "log/default.log"

#include <stdint.h>
#include <cstdlib> //for exit() & abort();
//#include <cstdio>
#include <iostream>
#include <string>
#include <sstream>
//#include <cassert>
//#include <cmath>
#include <fstream>

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::ostream;
using std::istream;
using std::fstream;
using std::ofstream;
using std::ifstream;

//============================ compiler-specific ============================

#ifndef __STDC_VERSION__
  #define __STDC_VERSION__ 199901L
#endif

#ifdef __GNUG__
  #define restrict __restrict__
#else
  #warning keyword 'restrict' ignored
  #define restrict
#endif

//================================ debugging ================================

#ifndef DEBUG_LEVEL
  #define DEBUG_LEVEL 0
  #define NDEBUG
#endif

#define Error(mess) {logger.error()\
    << mess << "\n\t"\
    << __FILE__ << " : " << __LINE__ << "\n\t"\
    << __PRETTY_FUNCTION__ << "\n" |0;\
    abort();}

#define TODO() {Error("control reached unfinished code:");}
#define DEPRICATED() {Error("control reached depricated code:");}
#define Assert(cond,mess) {if (!(cond)) Error(mess);}

//controlled assertions
#if DEBUG_LEVEL >= 1
  #define Assert1(cond,mess) {Assert(cond,mess)}
#else
  #define Assert1(cond,mess)
#endif
#if DEBUG_LEVEL >= 2
  #define Assert2(cond,mess) {Assert(cond,mess)}
#else
  #define Assert2(cond,mess)
#endif
#if DEBUG_LEVEL >= 3
  #define Assert3(cond,mess) {Assert(cond,mess)}
#else
  #define Assert3(cond,mess)
#endif
#if DEBUG_LEVEL >= 4
  #define Assert4(cond,mess) {Assert(cond,mess)}
#else
  #define Assert4(cond,mess)
#endif
#if DEBUG_LEVEL >= 5
  #define Assert5(cond,mess) {Assert(cond,mess)}
#else
  #define Assert5(cond,mess)
#endif

//special assertions
#define AssertW(cond,mess) {if (!(cond)) {logger.warning() << mess |0;}}

#define AssertV(cond,mess) {if (!(cond)) {logger.invalid() << mess |0;\
    decide_invalid();}}

#define AssertA(cond,mess,action) {if (!(cond)) {logger.error()\
    << mess << "\n\t"\
    << __FILE__ << " : " << __LINE__ << "\n\t"\
    << __PRETTY_FUNCTION__ << "\n" |0;\
    action\
    abort();}}

#define AssertP(ptr,block_size,name) {                      \
    Assert (ptr, "failed to allocate " << name);            \
    AssertW(0 == reinterpret_cast<size_t>(ptr) % block_size,    \
            "bad alignment given to " << name);}

void start_validating ();
void decide_invalid   ();
bool everything_valid ();

//time & resources
float get_elapsed_time ();
void print_resources ();
string get_date (bool hour=true);

//================================ convenience ================================

typedef uint16_t Short;
typedef uint32_t Int;
typedef uint64_t Long;
#ifdef SINGLE_PRECISION_FLOATS
typedef float Float;
typedef double Double;
#else
typedef double Float;
typedef long double Double;
#endif

template<class T> inline void swap (T& x, T& y) { T z=x; x=y; y=z; }

template<class T> inline T min (T x, T y) { return (x < y) ? x : y; }
template<class T> inline T max (T x, T y) { return (x > y) ? x : y; }
template<class T> inline void imax (T& x, T y) { if (y > x) x = y; }
template<class T> inline void imin (T& x, T y) { if (y < x) x = y; }

template <class T> inline int cmp(const T& lhs, const T& rhs)
{ return int(lhs>rhs) - int(lhs<rhs); }

template <class T> inline T safe_div (T num, T denom)
{ return num==0 ? 0.0 : num / denom; }

template <class T> inline T sqr (const T& x) { return x*x; }
//inline Int sqrti (Int x)
//{ return static_cast<Int>(sqrt(static_cast<double>(x))); }
//inline Int powi (Int x, double y)
//{ return static_cast<Int>(pow(static_cast<double>(x),y)); }
Int powi (Int x, Int y);

bool random_bit ();

//this is passed as a template parameter
struct do_nothing { void operator () () {} };

//this is used with template specialization
template <class T> inline const char* nameof () { return "???"; }

template<class T>
inline string _2string (T t) { std::ostringstream s; s << t; return s.str(); }
inline double string2float (string s, int d=0)
{ return s.empty() ? d : atof(s.c_str()); }
inline int    string2int   (string s, int d=0)
{ return s.empty() ? d : atoi(s.c_str()); }
struct compare_nocase {
    public: bool operator() (const string& s, const string& t) const;
};

//================================ i/o ================================

bool is_input_interactive ();
bool is_output_interactive ();

#define safe_fread(PTR, SIZE, COUNT, FILE) \
  Assert(COUNT == fread(PTR, SIZE, COUNT, FILE), "fread failed")
#define safe_fwrite(PTR, SIZE, COUNT, FILE) \
  Assert(COUNT == fwrite(PTR, SIZE, COUNT, FILE), "fwrite failed")

//================================ logging ================================

namespace Logging
{

extern ofstream logFile;
void switch_to_log (string filename);

class fake_ostream
{
    const bool m_live;
public:
    fake_ostream (bool live) : m_live(live) {}
    template <class Message>
    const fake_ostream& operator<< (const Message& message) const
    {
        if (m_live) { logFile << message; }
        return *this;
    }
    const fake_ostream& operator| (int hold) const
    {
        if (m_live) {
            if (hold)   logFile << std::flush; // log << ... |1; flushes
            else        logFile << std::endl;  // log << ... |0; ends line
        }
        return *this;
    }
};

const fake_ostream live_out(true), dead_out(false);

//title/section label
void title (string name);

//indentation stuff
const int length = 64;
const int stride = 2;
const char* const spaces = "                                                                " + length;
extern int indentLevel;
inline void indent ();
inline void outdent ();
inline const char* indentation ()
{ return spaces - min(length, indentLevel * stride); }
class IndentBlock
{
    bool m_active;
public:
    IndentBlock (bool active=true) : m_active(active)
    { if (m_active) indent (); }
    ~IndentBlock () { if (m_active) outdent (); }
};

//log channels
enum LogLevel {ERROR, WARNING, INVALID, INFO, DEBUG};
class Logger
{
private:
    const string m_name;
    const LogLevel m_level;
public:
    Logger (string name, LogLevel level = INFO);
    const fake_ostream& active_log (LogLevel level) const;

    //status
    bool at_ (LogLevel level) const { return level <= m_level; }
    bool at_warning () const { return at_(WARNING); }
    bool at_info    () const { return at_(INFO); }
    bool at_debug   () const { return at_(DEBUG); }

    //heading
    void static heading (char* label);

    //log
    const fake_ostream& log (LogLevel level) const
    { return (level <= m_level) ? active_log(level) : dead_out; }
    const fake_ostream& error   () const { return log(ERROR); }
    const fake_ostream& warning () const { return log(WARNING); }
    const fake_ostream& invalid () const
    { decide_invalid(); return log(INVALID); }
    const fake_ostream& info    () const { return log(INFO); }
    const fake_ostream& debug   () const { return log(DEBUG); }
};
const Logger logger ("general", INFO);

inline void indent ()
{
    ++indentLevel;
}
inline void outdent ()
{
    --indentLevel;
    Assert1(indentLevel >= 0, "indent level underflow");
}

}

#endif

