
#include "definitions.h"
#include "node_heap.h"
#include <iostream>
#include <vector>

using namespace std;
using namespace Heap;

Logging::Logger logger("test");

namespace Nodes
{

class FooSignature_
{
public:
    typedef FooSignature_ MySig;
    enum { size_in_bytes = 64, is_used_field = 1 };
    typedef Heap_::Node_   <MySig> Node;
    typedef Heap_::Heap_   <MySig> Heap;
    typedef Heap_::Pos_    <MySig> Pos;
    typedef Heap_::Name_   <MySig> Name;
    typedef Heap_::Handle_ <MySig> Handle;
};

}

typedef Nodes::FooSignature_::Pos    Foo;
typedef Nodes::FooSignature_::Name   FooName;
typedef Nodes::FooSignature_::Handle FooHdl;

Foo::Heap Foo::s_heap;


//================================ tests ================================

inline Foo create ()
{
    Foo foo = Foo::alloc();
    foo->setName(NULL);
    return foo;
}
void init_test(int depth)
{
    logger.info() << "Testing heap initialization" |0;
    Logging::IndentBlock block;

    Foo::init(1<<depth);
    Assert(Foo::empty(), "heap nonempty after creation");
}
void alloc_test()
{
    logger.info() << "Testing node allocation" |0;
    Logging::IndentBlock block;

    Int capacity = Foo::capacity();

    Int counter = 0;
    logger.info() << "allocating nodes" |0;
    while (!Foo::full()) {
        create();
        ++ counter;
    }
    Assert (counter == capacity, "wrong number of nodes allocated");

    logger.info() << "freeing nodes" |0;
    for (Int i=0; i < capacity; ++i) {
        Foo::free(Foo(1+(5*i)%capacity));
    }
    Assert(Foo::empty(), "heap not empty after all nodes freed");
}
void iter_test()
{
    logger.info() << "Testing node iterators" |0;
    Logging::IndentBlock block;

    logger.debug() << "allocating all nodes" |0;
    while (!Foo::full()) create();

    logger.info() << "trying (forward) iterator" |0;
    Int count = 0;
    for (Foo::iterator i=Foo::begin(); i!=Foo::end(); ++i) ++count;
    Assert (count == Foo::capacity(),
            "iterator traversed incorect number of nodes");

    logger.info() << "trying reverse_iterator" |0;
    count = 0;
    for (Foo::iterator i=Foo::begin(); i!=Foo::end(); ++i) ++count;
    Assert (count == Foo::capacity(),
            "reverse_iterator traversed incorect number of nodes");

    logger.debug() << "freeing all nodes" |0;
    for (Foo::reverse_iterator i=Foo::rbegin(); i!=Foo::rend(); ++i) Foo::free(*i);
    logger.debug() << "allocating nodes to half capacity" |0;
    for (Int i=0; i< Foo::capacity()/2; ++i) create();

    logger.info() << "trying sparse_iterator" |0;
    count = 0;
    for (Foo::sparse_iterator i=Foo::sbegin(); i!=Foo::send(); ++i) ++count;
    Assert (count == Foo::size(),
            "sparse_iterator traversed incorect number of nodes");

    logger.debug() << "freeing all nodes" |0;
    for (Foo::sparse_iterator i=Foo::sbegin(); i!=Foo::send(); ++i) Foo::free(*i);
    Assert (Foo::size() == 0, "nodes remain after pruning");
}


int main ()
{
    Logging::switch_to_log("test.log");
    Logging::title("Running Heap Test");

    int depth = 15;
    init_test(depth);
    alloc_test(); //pass 1
    alloc_test(); //pass 2
    alloc_test(); //pass 3
    iter_test();

    return 0;
}

