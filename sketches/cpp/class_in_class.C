

class Heap
{
public:
    class Node;
    class Pos;

    class Heap::Pos
    {
        int m_offset;
    public:
        Pos (int offset=0) : m_offset(offset) {}
        Node& operator * () { return s_base[m_offset]; }
    };
    class Heap::Node
    {
        int m_data[4]; //any data works
    public:
        Pos operator & () { return this-s_base; }
    };

private:
    static Node *s_base;
};


int main ()
{
    Heap h;
    Heap::Pos p;
    Heap::Node n;

    return 0;
}
