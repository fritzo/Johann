#ifndef POMAGMA_DEFINITIONS_H
#define POMAGMA_DEFINITIONS_H

#include <stdint.h>
#include <cstdlib> // for exit() & abort();
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>

//----------------------------------------------------------------------------
// Compiler-specific

#ifndef __STDC_VERSION__
    #define __STDC_VERSION__ 199901L
#endif // __STDC_VERSION__

#ifndef restrict
    #ifdef __GNUG__
        #define restrict __restrict__
    #else // __GNUG__
        #warning keyword 'restrict' ignored
        #define restrict
    #endif // __GNUG__
#endif // restrict

//----------------------------------------------------------------------------
// Debugging

#ifndef POMAGMA_DEBUG_LEVEL
#define POMAGMA_DEBUG_LEVEL 0
#endif // POMAGMA_DEBUG_LEVEL

//----------------------------------------------------------------------------
// Convenience

namespace pomagma
{

typedef uint32_t oid_t;

template<class T> inline T min (T x, T y) { return (x < y) ? x : y; }
template<class T> inline T max (T x, T y) { return (x > y) ? x : y; }

inline size_t random_int (size_t LB, size_t UB)
{
    return LB + lrand48() % (UB - LB);
}

inline bool random_bool (double prob)
{
    return drand48() < prob;
}

// this is used with template specialization
template <class T> inline const char* nameof () { return "???"; }

float get_elapsed_time ();
std::string get_date (bool hour=true);

//----------------------------------------------------------------------------
// Logging

const std::string g_log_level_name[4] =
{
    "\e[7;31merror   \e[0;39m",  // error   - reverse red
    "\e[31mwarning \e[0;39m",    // warning - red
    "\e[32minfo    \e[0;39m",    // info    - green
    "\e[33mdebug   \e[0;39m"     // debug   - yellow
};

class Log
{
    static std::ofstream s_log_file;
    static const unsigned s_log_level;

    std::ostringstream m_message;

public:

    static unsigned level () { return s_log_level; }

    Log (unsigned level)
    {
        m_message << std::left << std::setw(12) << get_elapsed_time();
        m_message << g_log_level_name[min(4u, level)];
    }

    ~Log ()
    {
       m_message << std::endl;
       s_log_file << m_message.str() << std::flush;
    }

    template<class T> Log & operator<< (const T & t)
    {
        m_message << t;
        return * this;
    }

    static void title (std::string name)
    {
        s_log_file
            << "\e[32m" // green
            << name << " " << get_date()
            << "\e[0;39m"
            << std::endl;
    }
};

#define POMAGMA_WARN(message) { if (Log::level() >= 1) { Log(1) << message; } }
#define POMAGMA_INFO(message) { if (Log::level() >= 2) { Log(2) << message; } }
#define POMAGMA_DEBUG(message) { if (Log::level() >= 3) { Log(3) << message; } }

#define POMAGMA_ERROR(message) { Log(0) \
    << message << "\n\t" \
    << __FILE__ << " : " << __LINE__ << "\n\t" \
    << __PRETTY_FUNCTION__ << "\n"; \
    abort(); }

#define POMAGMA_ASSERT(cond, mess) { if (not (cond)) POMAGMA_ERROR(mess) }

#define POMAGMA_ASSERT_(level, cond, mess) \
    { if (POMAGMA_DEBUG_LEVEL >= (level)) POMAGMA_ASSERT(cond, mess) }

#define POMAGMA_ASSERT1(cond, mess) POMAGMA_ASSERT_(1, cond, mess)
#define POMAGMA_ASSERT2(cond, mess) POMAGMA_ASSERT_(2, cond, mess)
#define POMAGMA_ASSERT3(cond, mess) POMAGMA_ASSERT_(3, cond, mess)
#define POMAGMA_ASSERT4(cond, mess) POMAGMA_ASSERT_(4, cond, mess)
#define POMAGMA_ASSERT5(cond, mess) POMAGMA_ASSERT_(5, cond, mess)

#define POMAGMA_ASSERT_EQUAL(x, y) \
    POMAGMA_ASSERT((x) == (y), \
            "expected " #x " == " #y "; actual " << (x) << " vs " << (y))

} // namespace pomagma

#endif
