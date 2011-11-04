
#include <iostream>
#include <utility>

namespace nonstd
{

//set implemented as splay tree
template<class Key> class Set
{
    typedef Set<Key> SetType;

    struct Node
    {
        Key key;
        Node *U, *L, *R; //up,left,right

        Node (Key k, Node* u=NULL, Node* l=NULL, Node* r=NULL)
            : key(k), U(u), L(l), R(r)
        {}
        ~Node () { if (L) delete L; if (R) delete R; }

        void clear () { L = R = NULL; }
        void set_L (Node* child) { L = child; child->U = this; }
        void set_R (Node* child) { R = child; child->U = this; }

        //traversal
        Node* Lstar () {
            Node *node = this;
            while (node->L) node = node->L;
            return node;
        }
        Node* Unext () {
            Node *prev = this, *node = U;
            while (node and node->L == prev) {
                prev = node;
                node = node->U;
            }
            return node ? node->Lstar() : NULL;
        }
        Node* next () { return R ? R->Lstar() : Unext(); }

        //joining
        Node* join_small (Node* L, Node* R) {//returns new root
            if (L->R == NULL) { L->set_R(R); return L; }
            if (R->L == NULL) { R->set_L(L); return R; }
            R->set_L(join(L->R, R->L));
            L->set_R(R);
            return L;
        }
        void join_fast (Node**D, Node* U, Node* L, Node* R) {
            while (true) {
                if (L->R == NULL) { D = L; L->U = U; L->set_R(R); return; }
                if (R->L == NULL) { D = R; R->U = U; R->set_L(L); return; }
                L->set_R(R);
                U = R; D = &(R->L); L = R->L; R = R->R;
            }
        }
    };

    Node* m_root;
    void set_root (Node* root) { m_root = root; root.U = NULL; }
public:
    Set () : m_root(NULL) {}
    ~Set () { if (m_root) delete m_root; }

private:
    void splay (Node* node);
    Node* upper_bound ();
    Node* upper_bound (Key key); //only on non-empty trees
public:

    //global access
    bool empty () { return !m_root; }
    void clear () { if (!m_root) return; delete m_root; m_root = NULL; }

    //element access
    bool insert (Key key);  //returns whether key was found
    bool remove (Key key);  //returns whether key was found
    bool remove (Key key);  //returns whether key was found

    //iteration
    class Iterator
    {
    protected:
        Node* m_pos;
        Iterator (Node* pos) : m_pos(pos) {}
    public:
        Iterator (SetType& set) : m_pos(set.upper_bound()) {}
        void next () { m_pos = m_pos->next(); }
        bool done () { return m_pos==NULL; }
        const Node* operator* () const { return m_pos; }
        const Key& key () const { return m_pos->key; }
    };
    Iterator iter () { return Iterator(*this); }

    //range iteration
    class RangeIterator : public Iterator
    {
        Key m_begin, m_end;
    public:
        RangeIterator (SetType& set, Key begin, Key end)
            : Iterator(set.upper_bound(begin)),
              m_begin(begin), m_end(end)
        {}
        bool done () { return m_pos and m_pos->key <= m_end; }
    };
    RangeIterator iter (Key begin, Key end)
    { return RangeIterator(*this, begin, end); }
};
template<class Key>
void Set<Key>::splay (Node* node)
{
    Node *x = node;

    while (true) {
        Node *y = x->U;
        if (!y) return;
        Node *z = y->U;
        if (!z) {
            if (x < y) {            //      zig
                y->set_L(x->R);     //    y      x
                x->set_R(y);        //   x . -> . y
            }                       //  . .      . .

            else {                  //      zag
                y->set_R(x->L);     //   y        x
                x->set_L(y);        //  . x  ->  y .
            }                       //   . .    . .
            set_root(x);
            return;
        } if (y < z) { //zig-
            if (x < y) {            //      zig-zig
                z->set_L(y->R);     //     z        x
                y->set_L(x->R);     //    y .  ->  . y
                y->set_R(z);        //   x .        . z
                x->set_R(y);        //  . .          . .
            }
            else {                  //      zig-zag
                z->set_L(x->R);     //    z         x
                y->set_R(x->L);     //   y .  ->  y   z
                x->set_L(y);        //  . x      . . . .
                x->set_R(z);        //   . .
            }
        } else { //zag-
            if (x < y) {            //      zag-zig
                y->set_L(x->R);     //   z          x
                z->set_R(x->L);     //  . y   ->  z   y
                x->set_L(z);        //   x .     . . . .
                x->set_R(y);        //  . .
            }
            else {                  //     zag-zag
                z->set_R(y->L);     //   z          x 
                y->set_R(x->L);     //  . y   ->   y .
                y->set_L(z);        //   . x      z .
                x->set_L(y);        //    . .    . .
            }
        }
    }
}
template<class Key>
typename Set<Key>::Node* Set<Key>::upper_bound ()
{
    if (!m_root) return NULL;
    Node *node = m_root, *next = node->L;
    while (next) {
        node = next;
        next = next->L;
    }
    return node;
}
template<class Key>
typename Set<Key>::Node* Set<Key>::upper_bound (Key key)
{
    //XXX this doesn't work: ub requires backtracking
    //e.g. finding UB(1)=1 vs UB(2)=3 in the tree
    //    3
    //  0
    //    1
    LATER();

    if (!m_root) return NULL;
    Node *node = m_root;
    while (true) {
        Key node_key = node->key;
        if (node_key < key) {
            if (node->R == NULL) return node->Unext();
            node = node->R;
        }
        if (node_key > key) {
            if (node->L == NULL) break;
            node = node->L;
        }
        break; //since node_key == key
    }
    return node;
}
template<class Key>
bool Set<Key>::insert (Key key)
{
    if (!m_root) {
        m_root = new Node(key);
        return false;
    }

    Node *UB = upper_bound(key);
    if (UB->key == key) {
        splay(UB);
        return true;
    }

    Node *node = new Node(key);
    Node *R = UB->R;
    if (R) node.set_R(R);
    UB->set_R(node);

    splay(node);
    return false;
}
template<class Key>
bool Set<Key>::remove (Key key)
{
    if (!m_root) return false;
    Node *node = upper_bound(key);
    if (node->key != key) return false;
    Node *U = node->U;
    Node **D = (U==NULL) ? &m_root
                         : (U->L==node) ? &(U->L)
                                        : &(U->R);
    if (node->L) {
        if (node->R)    Node::join_fast(D, U, node->L, node->R);
        else            { *D = node->L;  node->L->U = U; }
    } else {
        if (node->R)    { *D = node->R;  node->R->U = U; }
        else            *D = NULL;
    }
    node->clear();
    delete node;
    return true;

}

template <class Key, class Val>
class MMap : public Set<std::pair<Key,Val> >
{
    typedef std::pair<Key,Val> Pair;
public:
    inline bool insert (Key k, Val v) { return insert(Pair(k,v)); }
    inline bool remove (Key k, Val v) { return remove(Pair(k,v)); }
    void remove_key (Key key) { LATER(); }
    void remove_val (Val val) { LATER(); }
};

}

int main (void)
{
    using namespace nonstd;
    typedef uint32_t Int;
    typedef MMap<Int,Int> Reln;

    Reln r;

    //points on a circle
    r.insert(0,5);
    r.insert(3,4);
    r.insert(4,3);
    r.insert(5,0);
    r.insert(4,-3);
    r.insert(3,-4);
    r.insert(0,-5);
    r.insert(-3,-4);
    r.insert(-4,-3);
    r.insert(-5,0);
    r.insert(-4,-3);
    r.insert(-3,-4);

    for (Reln::Iterator i = i.begin(); not i.done(); i.next()) {
        std::cout << i->key().first << " , " << i->key().second << std::endl;
    }

    return 0;
}

