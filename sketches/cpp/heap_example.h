
/* Note: 
The trick will be to define everything in the correct order:
  * each node has fields of multiple pos types,
  * each pos references its heap in dereferencing, and
  * each heap references Node and Pos methods.
*/

#include <set> //for choose (n)

template<int _U, int _UL, int _ULR>
class HeapPDF
{
public:
    enum { U(_U), UL(_UL), ULR(_ULR) };
};

//assumes Node & Pos have methods
//  Pos Node::nextFreeNode ();
//  void Node::reset ();
//  Pos::Pos (int);
template<class Node, class Pos>
class Heap
{
    Node* m_nodes;
    int m_nodes_free;
    int m_nodes_used;
    Pos m_first_free_node; //can this be a Pos?
public:
    Heap ()
        : m_nodes(NULL),
          m_nodes_free(0),
          m_nodes_used(0),
          m_first_free_node(0)
    {}
    ~Heap () { if (m_nodes) delete[] (1+m_nodes); }
    bool init (Int size);
    bool resize (); //dynamic or static?

    //node allocation
    Pos alloc ();
    void free (Pos pos);

    //position operations: can these be made private?
    ...

    //iteration
    ...

    //reordering
    template<int key> Pos* reorder ();

    //random selection
    template<class heapPDF> void buildSums ();
    template<class heapPDF> Pos choose ();
    template<class heapPDF> Pos choose (int n);
};

class MyNode;
class MyPos;
typedef Heap<MyNode, MyPos> MyHeap;
extern MyHeap myNodes;

