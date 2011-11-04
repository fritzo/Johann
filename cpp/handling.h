#ifndef JOHANN_HANDLING_H
#define JOHANN_HANDLING_H

#include "definitions.h"

//log levels
#define LOG_DEBUG1(mess)
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << mess |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

//TODO: switch from reference counting to garbage collecting

//DEBUG, to avoid name mangling
extern int g_numHandledObjects;
extern int g_numHandles;

namespace Handling
{

const Logging::Logger logger("handle", Logging::DEBUG);

//extern int g_numHandledObjects;
class HandledObject
{
    //reference counting
    mutable int m_ref_count;
public:
    int ref_count () const { return m_ref_count; }
    void inc_ref_count () const { ++m_ref_count; }
    void dec_ref_count () const
    {
        --m_ref_count;
        if (!m_ref_count) delete this;
    }

    //construction
public:
    HandledObject () : m_ref_count (0)
    {
        LOG_DEBUG1( "creating HandledObject" );
        ++g_numHandledObjects;
    }
protected:
    virtual ~HandledObject ()
    {
        Assert4 (m_ref_count == 0,
                "Handles still reference HandledObject upon destruction");
        LOG_DEBUG1( "deleting HandledObject" );
        --g_numHandledObjects;
    }
};

//extern int g_numHandles;
template<class Object> //object must inherit from HandledObject
class Handle
{
    protected:
    typedef Handle<Object> MyType;
    Object *m_object;
public:
    Handle () : m_object(NULL) { ++g_numHandles; }
    Handle (Object* object) : m_object(object)
    {
        if (m_object != NULL) {
            LOG_DEBUG1( "creating Handle to " << *m_object );
            m_object->inc_ref_count();
        } else {
            LOG_DEBUG1( "creating Null Handle" );
        }
        ++g_numHandles;
    }
    virtual ~Handle ()
    {
        if (m_object != NULL) {
            LOG_DEBUG1( "deleting Handle to " << *m_object );
            m_object->dec_ref_count();
        } else {
            LOG_DEBUG1( "deleting Null Handle" );
        }
        --g_numHandles;

    }
    void clear ()
    {
        if (m_object != NULL) {
            LOG_DEBUG1( "clearing Handle to " << *m_object );
            m_object->dec_ref_count();
            m_object = NULL;
        }
    }
    void set (Object& object)
    {
        LOG_DEBUG1( "setting Handle to " << *m_object );
        clear();
        object.inc_ref_count();
        m_object = &object;
    }

    //copying
    Handle (const MyType& other) : m_object(other.m_object)
    {
        if (m_object != NULL) {
            LOG_DEBUG1( "copy-constructing Handle to " << *m_object );
            m_object->inc_ref_count();
        } else {
            LOG_DEBUG1( "copy-constructing Null Handle" );
        }
        ++g_numHandles;
    }
    Handle& operator= (const MyType& other)
    {
        if (m_object != NULL) {
            m_object->dec_ref_count();
        }
        m_object = other.m_object;
        if (m_object != NULL) {
            LOG_DEBUG1( "copying Handle to " << *m_object );
            m_object->inc_ref_count();
        } else {
            LOG_DEBUG1( "copying Null Handle" );
        }
        return *this;
    }

    //dereferncing
    operator bool () const { return m_object != NULL; }
    //bool null () const { return m_object == NULL; }
    Object& operator*  () const { return *m_object; }
    Object* operator-> () const { return m_object; }

    //comparison, WARNING: null handles are not equal to each other
    bool operator== (const MyType& other) const
    { return m_object and other.m_object and *m_object == *other; }
    bool operator!= (const MyType& other) const
    { return not (m_object and other.m_object and  *m_object == *other); }
    size_t hash () const { return reinterpret_cast<size_t>(m_object); }
};

inline int numHandledObjects () { return g_numHandledObjects; }
inline int numHandles () { return g_numHandles; }

}

#undef LOG_DEBUG1
#undef LOG_INDENT_DEBUG1

#endif
