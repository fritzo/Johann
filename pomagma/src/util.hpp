#ifndef POMAGMA_UTIL_HPP
#define POMAGMA_UTIL_HPP

#include <stdint.h>
#include <cstdlib> // for exit() & abort();
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>

namespace pomagma
{

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
template <class T> inline const char * nameof () { return "???"; }

float get_elapsed_time ();
std::string get_date (bool hour=true);

class noncopyable
{
    noncopyable (const noncopyable &); // intentionally undefined
    void operator= (const noncopyable &); // intentionally undefined
public:
    noncopyable () {}
};

#define POMAGMA_FOR(POMAGMA_type, POMAGMA_var, POMAGMA_init) \
    for (POMAGMA_type POMAGMA_var POMAGMA_init; \
         POMAGMA_var.ok(); \
         POMAGMA_var.next())

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
#define POMAGMA_ASSERT5(cond, mess) POMAGMA_ASSERT_(5, cond, mess)
#define POMAGMA_ASSERT6(cond, mess) POMAGMA_ASSERT_(6, cond, mess)

#define POMAGMA_ASSERT_EQUAL(x, y) \
    POMAGMA_ASSERT((x) == (y), \
            "expected " #x " == " #y "; actual " << (x) << " vs " << (y))

#define POMAGMA_ASSERT_OK \
    POMAGMA_ASSERT5(ok(), "tried to use done iterator")

//----------------------------------------------------------------------------
// Data types

typedef uint32_t oid_t; // object id TODO switch to uint16_t
const size_t MAX_ITEM_DIM = 0xffffUL;

//----------------------------------------------------------------------------
// Words of bits

typedef uint32_t Word; // TODO switch to uint64_t
const size_t BITS_PER_WORD = 8 * sizeof(Word);
const size_t WORD_POS_MASK = BITS_PER_WORD - 1;

class bool_ref
{
    Word & m_word;
    const Word m_mask;

public:

    bool_ref (Word & word, size_t _i)
        : m_word(word),
          m_mask(1 << _i)
    {
        POMAGMA_ASSERT6(_i < BITS_PER_WORD, "out of range: " << _i);
    }
    static bool_ref index (Word * line, size_t i)
    {
        auto I = div(i, BITS_PER_WORD); // either div_t or ldiv_t
        return bool_ref(line[I.quot], I.rem);
    }

    operator bool () const { return m_word & m_mask; } // ATOMIC
    void operator |= (bool b) { m_word |= b * m_mask; } // ATOMIC
    void operator &= (bool b) { m_word &= ~(!b * m_mask); } // ATOMIC
    void zero () { m_word &= ~m_mask; } // ATOMIC
    void one () { m_word |= m_mask; } // ATOMIC
    void invert () { m_word ^= m_mask; } // ATOMIC
};

//----------------------------------------------------------------------------
// Blocks of oid_t

const size_t LOG2_ITEMS_PER_BLOCK = 2;
const size_t ITEMS_PER_BLOCK = 1 << LOG2_ITEMS_PER_BLOCK;
const size_t BLOCK_POS_MASK = ITEMS_PER_BLOCK - 1;
typedef oid_t Block4x4[ITEMS_PER_BLOCK * ITEMS_PER_BLOCK];

inline oid_t & _block2value (oid_t * block, oid_t i, oid_t j)
{
    POMAGMA_ASSERT6(i < ITEMS_PER_BLOCK, "out of range " << i);
    POMAGMA_ASSERT6(j < ITEMS_PER_BLOCK, "out of range " << j);
    return block[(j << LOG2_ITEMS_PER_BLOCK) | i];
}

inline oid_t _block2value (const oid_t * block, oid_t i, oid_t j)
{
    POMAGMA_ASSERT6(i < ITEMS_PER_BLOCK, "out of range " << i);
    POMAGMA_ASSERT6(j < ITEMS_PER_BLOCK, "out of range " << j);
    return block[(j << LOG2_ITEMS_PER_BLOCK) | i];
}

} // namespace pomagma

#endif // POMAGMA_UTIL_HPP
