#ifndef JOHANN_MARKET_H
#define JOHANN_MARKET_H

#include "definitions.h"
#include "expressions.h"
#include "thread_tools.h"
#include <set>
//#include "statements.h"

namespace Market
{

const Logging::Logger logger("market", Logging::DEBUG);

namespace TT = ThreadTools;
using TT::Lock;

class Problem
{
public:
    virtual ~Problem () {}

protected:
    virtual void _simplify ();
public:
    void simplify () { LOCK_SYNTAX  _simplify();  UNLOCK_SYNTAX }
};

class StmtProb : public Problem
{
public:
    virtual ~StmtProb () {}
};

class Market
{
    typedef std::set<Problem*> Probs;
    TT::ThreadSafe<Probs> m_problems;
    typedef Probs::iterator prob_iter;
public:
    unsigned size () const { return m_problems->size(); }
    void clear ();
    void update ();

    //stats
    void write_stats (ostream& out) const;

    Market () {}
    ~Market () { clear(); }
};

}

#endif
