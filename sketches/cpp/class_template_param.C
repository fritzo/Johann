
class Node
{
    int m_data[32];
};

template<class Node, class Pos> class Heap
{
    Node* m_base;
public:
    inline Node& operator [] (int offset) { return m_base[offset]; }
};

class MyPos;
typedef Node MyNode;
typedef Heap<MyNode, MyPos> MyHeap;
MyHeap myHeap;

class MyPos
{
    int m_offset;
public:
    MyPos (int offset) : m_offset(offset) {}
    inline Node& operator * () { return myHeap[m_offset]; }
};

int main ()
{
    MyPos pos(0);

    return 0;
}
