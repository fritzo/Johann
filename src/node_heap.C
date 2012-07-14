
#include "node_heap.h"
#include "nodes.h" //for explicit template instantiation

namespace Heap_
{

//global constants
const size_t MAX_DEPTH = 30;
const size_t MAX_CAPACITY     = (1<<MAX_DEPTH) - 1; //1GB / 64B nodes
const size_t DEFAULT_CAPACITY = (1<<12) - 1; //4k

//================ heap methods ================
template<class Signature> void Heap_<Signature>::init (size_t capacity, bool is_full)
{//allocate heap with IS_USED and NEXT_FREE_NODE set, garbage in other fields
    Assert (capacity < MAX_CAPACITY, "maximum heap capacity exceeded");
    Assert (m_base == NULL, "alloc: heap already allocated");

    if (capacity == 0) capacity = DEFAULT_CAPACITY;

    logger.debug() << "Allocating " << capacity << " nodes" |0;
    Logging::IndentBlock block;

	Assert (capacity < (1 << 30), "capacity exceeds maximum (2^30 - 1)");

    void* allocated = nonstd::alloc_blocks(Signature::size_in_bytes, capacity);
    Assert (allocated != NULL, "allocation failed");
    m_mem = static_cast<Node*>(allocated);
    m_base = m_mem - 1;
    m_capacity = capacity;

    //deal with free node list
    logger.debug() << "initializing free nodes list" |0;
    const size_t last_pos = m_capacity;
    if (is_full) {
        m_nodes_used = m_capacity;
        m_nodes_free = 0;
        m_first_free_node = 0;
        for (size_t pos = 1; pos <= last_pos; ++pos) {
            m_base[pos].isUsed() = true;
        }
    } else {
        m_nodes_used = 0;
        m_nodes_free = m_capacity;
        m_first_free_node = 1;
        for (size_t pos = 1; pos <= last_pos; ++pos) {
            Node& node = m_base[pos];
            node.nextFreeNode() = Pos(1+pos);
            node.isUsed() = false;
        }
        m_base[last_pos].nextFreeNode() = Pos(0); //last node has no more free nodes
    }
}
template<class Signature> void Heap_<Signature>::clear ()
{
    Assert(m_base, "tried to free unallocated nodes");
    logger.debug() << "Deallocating " << m_capacity << " nodes" |0;
    Logging::IndentBlock block;

    nonstd::free_blocks(m_mem);
    m_base = NULL;
    m_capacity = 0;
    m_nodes_used = 0;
    m_nodes_free = 0;
    m_first_free_node = 0;
}
template<class Signature> void Heap_<Signature>::resize (
        size_t new_capacity,
        const oid_t * new2old)
{
    Assert (new_capacity < MAX_CAPACITY, "maximum heap capacity exceeded");
    const size_t old_capacity = m_capacity;
    const size_t last_pos = new_capacity;
    logger.debug() << "Reallocating " << old_capacity << " --> " << new_capacity << " nodes" |0;
    Logging::IndentBlock block;

    //sanity check
    Assert (new_capacity >= m_nodes_used, "tried to resize heap too small");
    Assert (new2old == NULL or new_capacity == m_nodes_used,
            "tried to reorder heap to improper size");

    //XXX: this uses lots of memory
    //allocate new array
    void * const old_allocated(static_cast<void*>(m_mem));
    void * const new_allocated(
            nonstd::alloc_blocks(Signature::size_in_bytes, new_capacity));
    Assert (new_allocated != NULL, "reallocation failed");
    Node * const new_mem = static_cast<Node*>(new_allocated);
    Node * const new_base = new_mem-1;

    //move data over
    if (new2old == NULL) {
        //copy data & fill rest with zeros
        nonstd::clear_block(new_allocated, Signature::size_in_bytes * new_capacity);
        Int min_capacity = min(old_capacity, new_capacity);
        nonstd::copy_blocks (new_allocated, old_allocated, Signature::size_in_bytes, min_capacity);
    } else {
        //copy in specified order
        for (oid_t pos = 1; pos <= last_pos; ++pos) {
            Node & old_node = m_base[new2old[pos]];
            Assert5(old_node.isUsed(), "tried to copy unused node");
            new_base[pos] = old_node;
#if DEBUG_LEVEL >= 3
            old_node.reset();
#endif
        }
    }
    nonstd::free_blocks(old_allocated);

    //update base & size
    m_mem = new_mem;
    m_base = new_base;
    m_capacity = new_capacity;

    //rebuild free node list
    logger.debug() << "updating free nodes list" |0;
    if (new2old != NULL) {
        //there are no free nodes
        m_nodes_free = 0;
        m_first_free_node = 0;
        return;
    }
    if (m_nodes_free == 0) {
        //there were no free nodes
        Assert1(m_first_free_node == 0, "supposedly full heap points to a free node");
        m_first_free_node = m_nodes_used + 1;
        for (oid_t pos = m_first_free_node; pos < last_pos; ++pos) {
            Assert4(not m_base[pos].isUsed(), "supposedly free node appears to be used");
            m_base[pos].nextFreeNode() = Pos(1+pos);
        }
        m_base[last_pos].nextFreeNode() = Pos(0); //last node has no more free nodes
    } else {
        //nothing special, clean free node list up
        Pos * next_free_node = reinterpret_cast<Pos*>(&m_first_free_node);
        for (oid_t pos = 1; pos <= last_pos; ++pos) {
            Node & node = m_base[pos];
            if (not node.isUsed()) {
                *next_free_node = Pos(pos);
                next_free_node = &(node.nextFreeNode());
            }
        }
        *next_free_node = Pos(0);
    }
    m_nodes_free += new_capacity - old_capacity;
}
template<class Signature> typename Signature::Pos Heap_<Signature>::alloc ()
{
    if (not m_nodes_free) {
        logger.debug() << "no more free nodes; resizing heap" |0;
        resize(2 * m_capacity + 1);
    }

    Int pos = m_first_free_node;
    Assert3 (pos, "heap tried to free null node");
    Assert5 (not m_base[pos].isUsed(), "pos not free before allocation");
    Node& node = m_base[pos];
    m_first_free_node = node.nextFreeNode();
    ++m_nodes_used;
    --m_nodes_free;
    node[TypedIndex<Int>(Signature::is_used_field)] = 1;
    Assert5 (Pos(pos).isUsed(), "pos not used after allocation");
    return Pos(pos);
}
template<class Signature> void Heap_<Signature>::free (typename Signature::Pos pos)
{
    Assert3 (m_nodes_used, "no allocated nodes to free");
    Assert5 (Pos(pos).isUsed(), "pos not used before freeing");
    Node& node = *pos;
    node[TypedIndex<Int>(Signature::is_used_field)] = 0;
    node.nextFreeNode() = Pos(m_first_free_node);
    m_first_free_node = pos;
    Assert5 (Pos(pos).isFree(), "pos not free after freeing");
    --m_nodes_used;
    ++m_nodes_free;
}

//================ handle methods ================
template<class Signature>
void Handle_<Signature>::merge (typename Signature::Name* dep, typename Signature::Name* rep)
{
    if (dep == rep) return;
    logger.debug() << "merging names" |0;

    //determine which name/handle pairs are affected
    typename Signature::Handle* null = NULL;
    typename DictType::iterator
        LB = s_dict.lower_bound(DefType(dep,   null)),
        UB = s_dict.lower_bound(DefType(dep+1, null)); // HACK for "upper_bound"
    DictType temp(LB, UB);

    //update names
    dep->merge_with(rep);

    //update s_dict
    s_dict.erase(LB, UB);
    for (typename DictType::iterator iter=temp.begin(); iter!=temp.end(); ++iter) {
        Assert3(iter->first == dep, "handle dict entry had wrong name before merge");
        Handle* hdl = iter->second;
        Assert3(hdl->m_name == dep, "handle had wrong name before merge");
        hdl->m_name = rep;
        s_dict.insert(DefType(rep, hdl));
    }
}

//================ explicit template instantiation ================
//obs
typedef Nodes::ObSignature OSig;
typedef Heap_<OSig> OHeap;
void O_init_heap     (OHeap& heap, size_t capacity) { heap.init(capacity); }
void O_clear_heap    (OHeap& heap) { heap.clear(); }
void O_resize_heap   (OHeap& heap, size_t capacity, const oid_t * new2old) { heap.resize(capacity, new2old); }
Ob   O_alloc         (OHeap& heap) { return heap.alloc(); }
void O_free          (OHeap& heap, Ob pos) { heap.free(pos); }
template void OHeap::init(size_t, bool);

//ob handles
void O_merge_handles (ObName* dep, ObName* rep) { ObHdl::merge(dep, rep); }

//application
typedef Nodes::AppSignature ASig;
typedef Heap_<ASig> AHeap;
void A_init_heap     (AHeap& heap, size_t capacity) { heap.init(capacity); }
void A_clear_heap    (AHeap& heap) { heap.clear(); }
void A_resize_heap   (AHeap& heap, size_t capacity, const oid_t * new2old) { heap.resize(capacity, new2old); }
App  A_alloc         (AHeap& heap) { return heap.alloc(); }
void A_free          (AHeap& heap, App pos) { heap.free(pos); }
template void AHeap::init(size_t, bool);

//composition
typedef Nodes::CompSignature CSig;
typedef Heap_<CSig> CHeap;
void C_init_heap     (CHeap& heap, size_t capacity) { heap.init(capacity); }
void C_clear_heap    (CHeap& heap) { heap.clear(); }
void C_resize_heap   (CHeap& heap, size_t capacity, const oid_t * new2old) { heap.resize(capacity, new2old); }
Comp  C_alloc         (CHeap& heap) { return heap.alloc(); }
void C_free          (CHeap& heap, Comp pos) { heap.free(pos); }
template void CHeap::init(size_t, bool);

//join
typedef Nodes::JoinSignature JSig;
typedef Heap_<JSig> JHeap;
void J_init_heap     (JHeap& heap, size_t capacity) { heap.init(capacity); }
void J_clear_heap    (JHeap& heap) { heap.clear(); }
void J_resize_heap   (JHeap& heap, size_t capacity, const oid_t * new2old) { heap.resize(capacity, new2old); }
Join  J_alloc         (JHeap& heap) { return heap.alloc(); }
void J_free          (JHeap& heap, Join pos) { heap.free(pos); }
template void JHeap::init(size_t, bool);

}

