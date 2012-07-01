#ifndef NONSTD_PARAM_TOOLS_H
#define NONSTD_PARAM_TOOLS_H

#include "definitions.h"
#include <vector>

namespace nonstd
{

template<class Value>
class param_stack
{
    Value m_value;
    std::vector<Value> m_history;
public:
    //default value is required
    param_stack (Value value) : m_value(value), m_history(1, value) {}

    Value operator() () const { return m_value; }
    void push (Value value)
    {
        m_value = value;
        m_history.push_back(value);
    }
    void pop ()
    {
        if (m_history.empty()) return;
        m_history.pop_back();
        m_value = m_history.back();
    }
    void set (Value value)
    {
        m_value = value;
        m_history.clear();
        m_history.push_back(m_value);
    }
    void reset () { set(m_history[0]); }
};

}

#endif

