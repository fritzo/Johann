#ifndef POMAGMA_SPLAY_H
#define POMAGMA_SPLAY_H

#include "util.hpp"

namespace pomagma
{

template<class X> class splay_forest : public X
{

private: // Shorthand for types and structure provided by X

    typedef typename X::Ob Ob;
    typedef typename X::Pos Pos;

/* requirements of X
    static inline Pos& root (Ob ob) { return X::root(ob); }
    static inline Pos& root (Pos p) { return X::root(p); }

    static inline Ob get_root (Pos p) { return X::get_root(p); }
    static inline Ob get_key  (Pos p) { return X::get_key(p); }
    static inline Ob get_val  (Pos p) { return X::get_val(p); }
*/

    static inline Pos& U (Pos p) { return X::up(p); }
    static inline Pos& L (Pos p) { return X::left(p); }
    static inline Pos& R (Pos p) { return X::right(p); }

private: // Splay tree operations

    //search tree position operations
    static inline void set_L    (Pos u, Pos l) { L(u) = l; if (l) U(l) = u; }
    static inline void set_R    (Pos u, Pos r) { R(u) = r; if (r) U(r) = u; }
    static inline void set_root (Pos p) { root(p) = p; U(p) = Pos(0); }
    static inline void clear    (Pos p) { R(p) = L(p) = U(p) = Pos(0); }

    static inline Pos UU (Pos p) { while (Pos next = U(p)) p = next; return p; }
    static inline Pos LL (Pos p) { while (Pos next = L(p)) p = next; return p; }
    static inline Pos RR (Pos p) { while (Pos next = R(p)) p = next; return p; }

    //node ordering = lexicographic key-value
    typedef Int Rank;
    static inline Rank rank (Int key, Int val) { return (key << 16) | val; }
    static inline Rank rank (Pos p) { return rank(get_key(p), get_val(p)); }

    //tree manupulation
    static inline bool is_left_of (Pos x, Pos y)
    {
        POMAGMA_ASSERT5(x == R(y) or x == L(y), "orphaned child");
        return x == L(y);
    }
    static void splay (Pos pos);
    static Pos find_pair (Ob root_ob, Ob key, Ob val, bool do_splay=true);
    static bool is_inserted (Pos eqn);
    static Pos first_key (Ob root_ob, Ob key);
    static void test_find (Pos eqn);

public: // interface

    static void insert (Pos pos);
    static void remove (Pos pos);
    static Pos find_key (Ob root_ob, Ob key);
    static void join (Pos* d, Pos u, Pos l, Pos r);
    static void test_contains (Pos eqn);
    static void test_range_contains (Pos eqn);
    static void validate_forest ();

    //an iterator
    class Iterator
    {
    protected:
        Pos m_pos;
        Ob m_root;

        //initialization
    private:
        void begin ();
    public:
        Iterator () : m_root(Ob(0)) {}
        Iterator (Ob root_ob) : m_root(root_ob) { begin(); }

        //traversal
        void begin (Ob root_ob) { m_root = root_ob; begin(); }
        operator bool () const { return m_pos; }
        bool done  () const { return not m_pos; }
        void next  ();

        //dereferencing
        Pos operator* () const { return m_pos; }
    };

    //a derived range iterator
    class RangeIterator : public Iterator
    {
        typedef Iterator Base;

        Ob m_key;

        RangeIterator (Ob) { logger.error() << "this should not be called"; }
    public:
        RangeIterator () : Iterator(), m_key(Ob(0)) {}
        RangeIterator (Ob root_ob, Ob key_ob) { begin(root_ob, key_ob); }

        //traversal
        void begin (Ob root_ob, Ob key);
        void next ();
    };

};

//tree manipulation
template<class X> void splay_forest<X>::splay (Pos x)
{
    POMAGMA_ASSERT5(is_inserted(x), "node is not inserted before splaying");

    Pos y = U(x);
    if (!y) { //quit if x is root
        POMAGMA_ASSERT5(root(x) == x, "parentless x is not root after splaying");
        return;
    }
    bool x_y = is_left_of(x,y); //get initial direction

    while (true) {
        Pos z = U(y);

        //swap x and y if y is root
        if (!z) {
            if (x_y) {              //      zig
                set_L(y,R(x));      //    y      x
                set_R(x,y);         //   x . -> . y
            }                       //  . .      . .

            else {                  //      zag
                set_R(y,L(x));      //   y        x
                set_L(x,y);         //  . x  ->  y .
            }                       //   . .    . .
            set_root(x);
            return;
        }

        //remember z's parent
        Pos new_y = U(z);

        //splay x above y,z
        if (is_left_of(y,z)) {      //zig-
            if (x_y) {              //      zig-zig
                set_L(z,R(y));      //     z        x
                set_L(y,R(x));      //    y .  ->  . y
                set_R(y,z);         //   x .        . z
                set_R(x,y);         //  . .          . .
            }
            else {                  //      zig-zag
                set_L(z,R(x));      //    z         x
                set_R(y,L(x));      //   y .  ->  y   z
                set_L(x,y);         //  . x      . . . .
                set_R(x,z);         //   . .
            }
        } else { //zag-
            if (x_y) {              //      zag-zig
                set_L(y,R(x));      //   z          x
                set_R(z,L(x));      //  . y   ->  z   y
                set_L(x,z);         //   x .     . . . .
                set_R(x,y);         //  . .
            }
            else {                  //     zag-zag
                set_R(z,L(y));      //   z          x
                set_R(y,L(x));      //  . y   ->   y .
                set_L(y,z);         //   . x      z .
                set_L(x,y);         //    . .    . .
            }
        }

        //update direction
        if ((y = new_y)) {
            x_y = is_left_of(z,y);
        } else {
            set_root(x);
            return;
        }
    }
}
template<class X> typename X::Pos splay_forest<X>::find_pair (
        Ob root_ob, Ob key, Ob val, bool do_splay)
{
    Pos p = root(root_ob); if (not p) return Pos(0);
    Rank here = rank(p);
    Rank destin = rank(key, val);

    //descend to key,val pair
    if (here == destin) return p; //p is already root
    do {
        p = (destin < here) ? L(p) : R(p);
        if (not p) return Pos(0);
    } while ((here = rank(p)) != destin);

    //splay
    if (do_splay) splay(p);
    return p;
}
template<class X> typename X::Pos splay_forest<X>::find_key (Ob root_ob, Ob key)
{//finds arbitrary pos in key range
    //find any key
    Pos p = root(root_ob); if (not p) return Pos(0);
    Int here = get_key(p);
    if (here == key) return p; //p is already root
    do {
        p = (key < here) ? L(p) : R(p);
        if (not p) return Pos(0);
    } while ((here = get_key(p)) != key);

    //splay
    splay(p);
    return p;
}
template<class X> void splay_forest<X>::insert (Pos p)
{
    POMAGMA_ASSERT5(not is_inserted(p), "pos is inserted before it should be");

    Pos& root_pos = root(p);

    //if tree is empty, insert as root
    if (not root_pos) {
        root_pos = p;
        clear(p);
        return;
    }

    //seek to bottom of tree
    Rank destin = rank(p);
    for (Pos u = root_pos;;) {
        POMAGMA_ASSERT5(destin != rank(u), "key-val pair is already inserted");
        Pos& D = (destin < rank(u)) ? L(u) : R(u);
        if (D) u = D;
        else {
            D = p; U(p) = u;
            break;
        }
    }

    //insert node and splay
     L(p) = Pos(0);
    R(p) = Pos(0);
    splay(p);

    POMAGMA_ASSERT5(is_inserted(p), "pos is not inserted when it should be");
}
template<class X> void splay_forest<X>::join (Pos* d, Pos u, Pos l, Pos r)
{//fast-and-sloppy version, biased to the left

    while (true) {

        POMAGMA_ASSERT5(get_root(l) == get_root(r), "L,R keys disagree");
        POMAGMA_ASSERT5((!u) or get_root(r) == get_root(u), "R,U keys disagree");
        POMAGMA_ASSERT5(rank(l) < rank(r), "L,R in wrong order");

        //look for space below l
        Pos x = R(l);
        if (not x) {                //                l
            *d = l; U(l) = u;       //  l  +  r  ->  w  r
            set_R(l,r);             // w     y z       y z
            return;
        }

        //look for space below r
        Pos y = L(r);
        if (not y) {                //                  r
            *d = r; U(r) = u;       //  l  +  r  ->   l  z
            set_L(r,l);             // w x     z     w x
            return;
        }

        //otherwise push down
        *d = l; U(l) = u;           //                l
        set_R(l,r);                 //  l  +  r  ->  w    r
        d = &(L(r));                // w x   y z       x+y z
        u = r; l = x; r = y;
    }
}
template<class X> void splay_forest<X>::remove (Pos p)
{
    POMAGMA_ASSERT5(is_inserted(p), "pos not inserted before removal");

    Pos u = U(p);
    Pos& d = u ? (is_left_of(p,u) ? L(u)
                                     : R(u))
               : root(p);

    if (Pos l = L(p)) {
        if (Pos r = R(p)) {     //   u      u
            join(&d,u,l,r);     //   p  -> l+r
        }                       //  l r
        else {
            d = l;              //   u      u
            U(l) = u;           //   p  ->  l
        }                       //  l
    } else {
        if (Pos r = R(p)) {     //   u      u
            d = r;              //   p  ->  r
            U(r) = u;           //    r
        }
        else d = Pos(0);        //   u  ->  u
    }                           //   p

    POMAGMA_ASSERT5(not is_inserted(p), "pos inserted after removal");
}
template<class X> bool splay_forest<X>::is_inserted (Pos p)
{
    Pos found = find_pair(get_root(p), get_key(p), get_val(p), false);
    POMAGMA_ASSERT5((!found) or (found==p), "two identical pos's have been inserted")
    return found;
}

//iteration
template<class X> void splay_forest<X>::Iterator::begin ()
{//inorder first
    Pos root_pos = root(m_root);
    m_pos = root_pos ? LL(root_pos) : Pos(0);
}
template<class X> void splay_forest<X>::Iterator::next ()
{//inorder next
    //PROFILE: Iterator<CLR>::next is very expensive: 74% of total time
    //move DR DL^*
    if (Pos r = R(m_pos)) { m_pos = LL(r); return; }

    //move UL^* UR or finish
    for (Pos d = m_pos; (m_pos = U(d)); d = m_pos) {
        if (is_left_of(d,m_pos)) break;
    }
}

//tree range iteration
template<class X> void splay_forest<X>::RangeIterator::begin(
        Ob root_ob, Ob key)
{//finds left-most pos in key range
    Base::m_root = root_ob;
    m_key = key;
    Pos& p = Base::m_pos;
    p = root(Base::m_root);
    if (not p) return;

    //find any key
    Int here;
    while ((here = get_key(p)) != key) {
        p = (key < here) ? L(p) : R(p);
        if (not p) return;
    }

    //descend left
    while (true) {
        Pos l = L(p);
        if (not l) { splay(p); return; }
        if (get_key(l) == key) { p = l; continue; }

        //descend right
        while (true) {
            Pos r = R(l);
            if (not r) { splay(l); return; }
            if (get_key(r) != key) { l = r; continue; }

            p = r; break;
        }
    }
}
template<class X> void splay_forest<X>::RangeIterator::next ()
{//for iteration through multimaps
    Base::next();
    if (Base::m_pos and get_key(Base::m_pos) != m_key) {
        splay(Base::m_pos);
        Base::m_pos = Pos(0);
    }
}

//validation
template<class X> void splay_forest<X>::test_find (Pos eqn)
{
    //test find_pair
    Pos pos = find_pair(get_root(eqn), get_key(eqn), get_val(eqn));
    POMAGMA_ASSERT (pos, "invalid: eqn not found in own " << nameof<X>() << " tree");
    POMAGMA_ASSERT (pos == eqn,
            "invalid: wrong eqn found in own " << nameof<X>() << " tree");

    //test find_key
    pos = find_key(get_root(eqn), get_key(eqn));
    POMAGMA_ASSERT (pos, "invalid: key not found in own " << nameof<X>() << " tree");
    POMAGMA_ASSERT (get_key(pos) == get_key(eqn),
            "invalid: wrong key found in own " << nameof<X>() << " tree");
}
template<class X> void splay_forest<X>::test_contains (Pos eqn)
{
    for (Iterator iter(get_root(eqn)); iter; iter.next()) {
        if (*iter == eqn) return;
    }
    Iterator iter;
    for (iter.begin(get_root(eqn)); iter; iter.next()) {
        if (*iter == eqn) return;
    }
    POMAGMA_ERROR("invalid: eqn not contained in own " << nameof<X>() << " tree");
}
template<class X> void splay_forest<X>::test_range_contains (Pos eqn)
{
    Ob my_root = get_root(eqn);
    Ob my_key = get_key(eqn);
    for (RangeIterator iter(my_root, my_key); iter; iter.next()) {
        if (*iter == eqn) return;
    }
    POMAGMA_ERROR("invalid: eqn not (range) contained in own "
          << nameof<X>() << " tree");
}
template<class X> void splay_forest<X>::validate_forest ()
{
    logger.debug() << "Validating " << nameof<X>() << " forest" |0;
    Logging::IndentBlock block;

    for (typename Pos::sparse_iterator iter=Pos::sbegin();
            iter!=Pos::send(); ++iter) {
        Pos eqn = *iter;

        //make sure eqn is inserted
        test_find(eqn);

        //check L-U agreement
        if (Pos l = L(eqn)) {
            POMAGMA_ASSERT (rank(l) < rank(eqn), "L-U out of order");
            POMAGMA_ASSERT (U(l) == eqn, "invalid: runaway L-child");
        }

        //check R-U agreement
        if (Pos r = R(eqn)) {
            POMAGMA_ASSERT (rank(eqn) < rank(r), "R-U out of order");
            POMAGMA_ASSERT (U(r) == eqn, "invalid: runaway R-child");
        }

        //check U-_ agreement
        if (Pos u = U(eqn)) {
            if (rank(eqn) < rank(u)) {
                POMAGMA_ASSERT (L(u) == eqn, "invalid: neglected L-child");
            } else {
                POMAGMA_ASSERT (R(u) == eqn, "invalid: neglected R-child");
            }
        } else {
            POMAGMA_ASSERT (root(eqn) == eqn, "invalid: root mismatch");
        }
    }
}

}

#endif
