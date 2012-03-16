#ifndef JOHANN_NODE_HEAP_H
#define JOHANN_NODE_HEAP_H

#include "definitions.h"
#include <set>     //for set
#include <utility> //for pair
#include <cstring> //for bzero
#include "aligned_alloc.h" //for clear_block, alloc_blocks, free_blocks
#include "small_ptr.h"

//log levels
#define LOG_DEBUG1(mess)
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << mess |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

namespace Heap
{

//type-conscious array index
template<class ValueType>
class TypedIndex
{
    const int m_value;
public:
    //acts like an integer
    TypedIndex (int value) : m_value(value) {}
    //TypedIndex& operator = (int value) { m_value = value; return *this; }
    operator int () const { return m_value; }
    //operator int& () { return m_value; }

    //but knows typing information
    typedef ValueType valueType;
};

}

namespace Heap_ //only node definitions should use this
{

const Logging::Logger logger("heap",Logging::INFO);

using namespace Heap;

//================================ nodes ================================

//a multi-purpose memory block using reinterpret_cast instead of union
template<class Signature>
class Node_
{
    typedef typename Signature::Node Node;
    typedef typename Signature::Heap Heap;
    typedef typename Signature::Pos  Pos;
    typedef typename Signature::Name Name;

    enum { size_in_bytes = Signature::size_in_bytes };
    char m_data[size_in_bytes];
public:
    //custom memory management
    //should these be private and Heap<MyType,Pos>::free and ::alloc be friends?
    void reset      () { nonstd::clear_block(this, size_in_bytes); }
    void min_reset  () { m_data[TypedIndex<Int>(Signature::is_used_field)] = 0; }

    Pos&  nextFreeNode ()      { return reinterpret_cast<Pos*>(m_data)[0]; }
    Int&  isUsed  ()           { return reinterpret_cast<Int*>(m_data)[Signature::is_used_field]; }
    Name* name    ()           { return reinterpret_cast<SmallPtr<Name>*>(m_data)[0]; }
    void  setName (Name* name) { reinterpret_cast<SmallPtr<Name>*>(m_data)[0] = SmallPtr<Name>(name); }
    bool  isNamed ()           { return reinterpret_cast<SmallPtr<Name>*>(m_data)[0]; }

    //unsafe indexing
    template <class typedIndex>
    typename typedIndex::valueType& operator [] (typedIndex index)
    { return reinterpret_cast<typename typedIndex::valueType*>(m_data)[index]; }
    Pos& operator [] (int index)
    { return reinterpret_cast<Pos*>(m_data)[index]; }

    //safe indexing
    template <class typedIndex>
    typename typedIndex::valueType& operator () (typedIndex index)
    {
#if DEBUG_LEVEL >= 5
        static const int min_index = 0;
        static const int max_index = size_in_bytes / sizeof(typename typedIndex::valueType);
        Assert5(index >= min_index, "index out of bounds, too low");
        Assert5(index <  max_index, "index out of bounds, too high");
#endif
        return reinterpret_cast<typename typedIndex::valueType*>(m_data)[index];
    }
    Pos& operator () (int index)
    {
#if DEBUG_LEVEL >= 5
        static const int min_index = 0;
        static const int max_index = size_in_bytes / sizeof(Pos);
        Assert5(index >= min_index, "index out of bounds, too low");
        Assert5(index <  max_index, "index out of bounds, too high");
#endif
        return reinterpret_cast<Pos*>(m_data)[index];
    }
};

//================================ node heap ================================

template<class Signature>
class Heap_
{
    typedef typename Signature::Node Node;
    typedef typename Signature::Heap Heap;
    typedef typename Signature::Pos  Pos;
    typedef typename Signature::Name Name;

    Node *m_base, *m_mem; //keep real memory pointer to appease valgrind
    Int   m_capacity;
    Int   m_nodes_free;
    Int   m_nodes_used;
    Int   m_first_free_node; //this is really a Pos

public:
    Heap_ ()
        : m_base(NULL), m_mem(NULL),
          m_capacity(0),
          m_nodes_free(0),
          m_nodes_used(0),
          m_first_free_node(0)
    { Assert (Signature::is_used_field, "Signature::is_used_field must be nonzero"); }

    //diagnostics
    bool cleared () const { return m_base==NULL; }
    Int capacity     () const { return m_capacity; }
    Int size         () const { return m_nodes_used; }
    Int nodesUsed    () const { return m_nodes_used; }
    Int nodesFree    () const { return m_nodes_free; }

    //memory allocation
    void init   (Int size=0, bool is_full=false);
    void resize (Int size, const Int* new2old=NULL);
    void clear ();
    ~Heap_ () { if (m_base) clear(); }

    //node allocation
    Pos alloc ();
    void free (Pos pos);

    //basic Pos interface
    Node& operator [] (int offset) { return m_base[offset]; }
    Node* operator + (int offset) { return m_base + offset; }
};

//================================ positions ================================

#define IF_VALID {Assert5(isValid(), "accessed invalid pos: " << m_offset);}
#define IF_USED {Assert5(isUsed(), "accessed free pos: " << m_offset);}
template<class Signature>
class Pos_
{
    typedef typename Signature::Node Node;
    typedef typename Signature::Heap Heap;
    typedef typename Signature::Pos  Pos;
    typedef typename Signature::Name Name;

    static Heap s_heap;

    Int m_offset;
public:
    //no constructors or destructors: this is used with reinterpret_cast
    Pos_ () {}
    explicit Pos_ (Int offset) : m_offset(offset) {} //for conversions
    //inline Pos& operator = (Int offset) { m_offset = offset; return *this; }
    operator Int& () { return m_offset; }
    operator const Int () const { return m_offset; }
    void      reset ()              { IF_VALID s_heap[m_offset].reset(); }

    //simple math
    bool operator <  (const Pos& other) const { return m_offset <  other.m_offset; }
    bool operator <= (const Pos& other) const { return m_offset <= other.m_offset; }
    bool operator >  (const Pos& other) const { return m_offset >  other.m_offset; }
    bool operator >= (const Pos& other) const { return m_offset >= other.m_offset; }
    Pos& operator ++ () { ++m_offset; return *this; }
    Pos& operator -- () { --m_offset; return *this; }

    //dereferencing
    bool  isValid     () const { return (1<=m_offset) and (m_offset<=capacity()); }
    bool  isUsed      () const { IF_VALID return s_heap[m_offset](TypedIndex<Int>(Signature::is_used_field)); }
    bool  isFree      () const { return !isUsed(); }
    bool  isNamed     () const { IF_VALID return s_heap[m_offset].isNamed(); }
    Node& operator *  () const { IF_VALID IF_USED return s_heap[m_offset]; }
    Node* operator -> () const { IF_VALID IF_USED return s_heap + m_offset; }

    //indexing
    template <class typedIndex>
    typename typedIndex::valueType& operator [] (typedIndex index) const
    { IF_VALID return s_heap[m_offset][index]; }
    Pos&                            operator [] (Int index)        const
    { IF_VALID return s_heap[m_offset][index]; }

    template <class typedIndex>
    typename typedIndex::valueType& operator () (typedIndex index) const
    { IF_VALID IF_USED  return s_heap[m_offset](index); }
    Pos&                            operator () (Int index)        const
    { IF_VALID IF_USED  return s_heap[m_offset](index); }

    //heap diagnostics
    static bool cleared  () { return s_heap.cleared(); }
    static bool empty    () { return !s_heap.nodesUsed(); }
    static bool full     () { return !s_heap.nodesFree(); }
    static Int  size     () { return s_heap.size(); }
    static Int  capacity () { return s_heap.capacity(); }
    static Int  numUsed  () { return s_heap.nodesUsed();}
    static Int  numFree  () { return s_heap.nodesFree();}

    //allocation
    static void init    (Int capacity=0, bool is_full=false) { s_heap.init(capacity, is_full); }
    static void resize  (Int capacity, const Int* new2old=NULL) { s_heap.resize(capacity, new2old); }
    static void clear   ()        { s_heap.clear(); }
    static Pos  alloc   ()        { return s_heap.alloc(); }
    static void free    (Pos pos) { s_heap.free(pos); }
    //XXX this seems to have problems with compiler overoptimization or sth ???
    //static void free    (Pos pos) { Assert3(!pos.isNamed(), "freeing named pos"); s_heap.free(pos); }

    //heap position
    static Pos root () { return Pos(1); }
    Pos up        () const { return Pos(m_offset >> 1); }
    Pos left      () const { return Pos(m_offset << 1); }
    Pos right     () const { return Pos((m_offset << 1) | 1); }
    Pos leftStar  () const;
    Pos rightStar () const;
    bool isRightChild  () const { return m_offset % 2; }
    bool isLeftChild   () const { return !(m_offset % 2); }
	bool hasParent     () const { return m_offset > 1; }
    bool hasLeftChild  () const { return left() <= capacity(); }
    bool hasRightChild () const { return right() <= capacity(); }

    //stl-style interators
    //yeah, these are iterators dereferencing to positions dereferencing to nodes.
    class iterator
    {
        typedef iterator MyType;
        Pos m_pos;
    public:
        iterator (Pos pos) : m_pos(pos) {}
        bool operator == (const MyType& other) { return m_pos == other.m_pos; }
        bool operator != (const MyType& other) { return m_pos != other.m_pos; }
        MyType& operator ++ () { ++m_pos; return *this; }
        Pos& operator *  () { return m_pos; }
        Pos* operator -> () { return &m_pos; }
    };
    class reverse_iterator
    {
        typedef reverse_iterator MyType;
        Pos m_pos;
    public:
        reverse_iterator (Pos pos) : m_pos(pos) {}
        bool operator == (const MyType& other) { return m_pos == other.m_pos; }
        bool operator != (const MyType& other) { return m_pos != other.m_pos; }
        MyType& operator ++ () { --m_pos; return *this; }
        Pos& operator *  () { return m_pos; }
        Pos* operator -> () { return &m_pos; }
    };
    class sparse_iterator
    {
        typedef sparse_iterator MyType;
        Pos m_pos;
        Int m_nodes_left;
    public:
        sparse_iterator (Pos pos, Int nodes_left) : m_pos(pos), m_nodes_left(nodes_left) {}
        bool operator == (const MyType& other) const { return m_nodes_left == other.m_nodes_left; }
        bool operator != (const MyType& other) const { return m_nodes_left != other.m_nodes_left; }
        MyType& operator ++ () {
            --m_nodes_left;
            if (!m_nodes_left) return *this;
            do { ++m_pos; } while (!m_pos.isUsed());
            return *this;
        }
        Pos& operator *  () const { return const_cast<Pos&>(m_pos); }
        Pos* operator -> () const { return const_cast<Pos*>(m_pos); }
    };
    class rev_sps_iterator
    {
        typedef rev_sps_iterator MyType;
        Pos m_pos;
        Int m_nodes_left;
    public:
        rev_sps_iterator (Pos pos, Int nodes_left) : m_pos(pos), m_nodes_left(nodes_left) {}
        bool operator == (const MyType& other) { return m_nodes_left == other.m_nodes_left; }
        bool operator != (const MyType& other) { return m_nodes_left != other.m_nodes_left; }
        MyType& operator ++ () {
            --m_nodes_left;
            if (!m_nodes_left) return *this;
            do { --m_pos; } while (!m_pos.isUsed());
            return *this;
        }
        Pos& operator *  () { return m_pos; }
        Pos* operator -> () { return &m_pos; }
    };
    static iterator         begin   () { return Pos(1); }
    static iterator         end     () { return Pos(1+capacity()); }
    static reverse_iterator rbegin  () { return Pos(capacity()); }
    static reverse_iterator rend    () { return Pos(0); }
    static sparse_iterator  sbegin  () { return ++sparse_iterator(Pos(0),size()+1); }
    static sparse_iterator  send    () { return sparse_iterator(Pos(0),0); }
    static rev_sps_iterator rsbegin () { return ++rev_sps_iterator(Pos(capacity()+1),size()+1); }
    static rev_sps_iterator rsend   () { return rev_sps_iterator(Pos(0),0); }

    //arrays
    template<class T> class array
    {
        T*const m_data;
    public:
        array () : m_data(new(std::nothrow) T[1+Pos::capacity()])
        {
            Assert (m_data, "failed to allocate Pos-array");
            bzero(m_data, sizeof(T) * (1+Pos::capacity()));
        }
        ~array () { delete[] m_data; }

        T  operator() (Pos pos) const { return m_data[Int(pos)]; }
        T& operator() (Pos pos)       { return m_data[Int(pos)]; }

        template<class vect>
        array<T>& operator= (const vect& v)
        {
            for (typename Pos::sparse_iterator i=Pos::sbegin();
                    i!=Pos::send(); ++i) { Pos pos = *i;
                m_data[Int(pos)] = v(pos);
            }
            return *this;
        }
    };
};
#undef IF_VALID
#undef IF_USED

//================================ names ================================

template<class Signature>
class Name_
{
    typedef typename Signature::Node Node;
    typedef typename Signature::Heap Heap;
    typedef typename Signature::Pos  Pos;
    typedef typename Signature::Name Name;

    static Int s_numNames;

    Int m_int; //integer version of position
    Int m_numRefs;
    inline       Pos& m_pos ()       { return *reinterpret_cast<Pos*>(&m_int); }
    inline const Pos& m_pos () const { return *reinterpret_cast<const Pos*>(&m_int); }

    Name_& operator= (const Name_&) { Assert(0, "tried to copy a Name"); }
    Name_ (const Name_&) { Assert(0, "tried to copy-construct a Name"); }
    Name_ (Pos pos) : m_int(pos), m_numRefs (1) //Names always start with a reference
    {
        Assert1(m_pos(), "null node named");
        Assert1(m_pos()->name() == NULL, "tried to name a node twice");
        m_pos()->setName(this);
        ++s_numNames;
    }
    //the private destructor forces Names to be on the heap (a.o.t. the stack)
    //  so dec_ref_count can delete this,
    //  (as in More Effective C++, item 27)
    ~Name_ () {
        Assert2(!m_numRefs, "Name deleted while still referenced");
        if (m_pos().cleared()) return; //heap has been cleared, just exit
        if (m_pos()) m_pos()->setName(NULL);
        --s_numNames;
    }
public:
    static Name_* build (Pos pos) { return new Name_(pos); }

    //reference counting
    void inc_ref_count () { ++m_numRefs; }
    void dec_ref_count () { --m_numRefs; if (!m_numRefs) delete this; }

    //dereferencing
    const Pos& operator *  () const { return m_pos(); }
    const Pos* operator -> () const { return &(m_pos()); }

    //diagnostics
    static Int number () { return s_numNames; }

    //merging
    void merge_with (Name* rep) {
        Assert3(rep != this, "tried to merge name with itself");
        rep->m_numRefs += m_numRefs;
        if (DEBUG_LEVEL >= 2) m_numRefs = 0;
        delete this;
    }

    //repositioning
    inline void move_to (Pos new_pos) {
        //move name from one ob to another
        Assert3(m_pos()->name() == this, "in move_to, invalid name at old position");
        m_pos()->setName(NULL);
        m_pos() = new_pos;
        Assert3(m_pos()->name() == NULL, "in move_to, invalid name at new position");
        m_pos()->setName(this);
    }
    inline void move_to_tandem (Pos new_pos) {
        //move name & wait for ob to move with name
        Assert3(m_pos()->name() == this, "in move_to, invalid name at old position");
        m_pos() = new_pos;
    }
};

//================================ handles ================================

//Note: Null handles (handles to Ob(0)) are dangerous.
//  They should not be copied or constructed except explicitly.
template<class Signature>
class Handle_
{
    typedef typename Signature::Node   Node;
    typedef typename Signature::Heap   Heap;
    typedef typename Signature::Pos    Pos;
    typedef typename Signature::Name   Name;
    typedef typename Signature::Handle Handle;
    typedef std::pair<const Name*, Handle*> DefType;
public:
    typedef std::set<DefType> DictType;
private:
    static DictType s_dict;

    Name* m_name;
public:
    void set (Pos pos)
    {
        Assert1 (m_name == NULL, "tried to set a Handle twice");
        Assert1 (pos, "tried to set Handle to null pos");
        m_name = pos->name();
        if (m_name == NULL)  m_name = Name::build(pos);
        else m_name->inc_ref_count();
        s_dict.insert(DefType(m_name, this));
    }
    void clear ()
    {
        if (m_name == NULL) return;
        m_name->dec_ref_count();
        s_dict.erase(DefType(m_name, this));
        m_name = NULL;
    }

    Handle_ () : m_name(NULL) {} //necc for default allocator
    Handle_ (Name* name) : m_name(name)
    {
        Assert1(m_name != NULL, "handle created from null name");
        m_name->inc_ref_count();
        s_dict.insert(DefType(m_name, this));
    }
    Handle_ (const Handle& handle) : m_name(handle.m_name)
    {
        Assert1(m_name != NULL, "null handle copied in constructor");
        m_name->inc_ref_count();
        s_dict.insert(DefType(m_name, this));
    }
    explicit Handle_ (Pos pos) : m_name(pos->name())
    {
        if (m_name == NULL)  m_name = Name::build(pos);
        else m_name->inc_ref_count();
        s_dict.insert(DefType(m_name, this));
    }
    ~Handle_ () { clear(); }

    //assignment
    Handle& operator = (const Handle& other)
    {
        Assert1(other.m_name != NULL, "tried to copy null handle");
        if ((this == &other) or (m_name == other.m_name)) return *this;
        clear();
        m_name = other.m_name;
        m_name->inc_ref_count();
        s_dict.insert(DefType(m_name, this));
        return *this;
    }

    //dereferencing
    operator bool () const { return m_name != NULL; }
    const Pos& operator *  () const { return m_name->operator*(); }
    const Pos* operator -> () const { return m_name->operator->(); }

    //name merging
    static void merge (Name* dep, Name* rep);

    //diagnostics
    //static Int number () { return s_dict.size(); }
};

}

#undef LOG_DEBUG1
#undef LOG_INDENT_DEBUG1

#endif

