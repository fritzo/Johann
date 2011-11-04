
#include <bitset>
#include <vector>
#include <iostream>
#include <cstdlib> //for abort and div

#define Assert(cond,mess)
//{if(!(cond)) { std::cout << (mess) << std::endl; abort(); }}

template <class T> inline T min (T x, T y) { return x<y ? x : y; }
template <class T> inline T max (T x, T y) { return x<y ? y : x; }

#define VERSION 7

/** performance of various verions (2^13 items):
 * ver. alloc'd times:  no optim    -O3
 * 1    8388608         86.4s       27.2s
 * 2    4194816         38.0s        8.5s
 * 3    4193792         39.2s        8.5s
 * 4    ???             53.8s       11.1s
 * 5    ???             27.0s        6.8s+
 * 6    ???             29.3s        7.9s
 * 7    "               14.3s        4.6s  yeah baby..
 */

#if VERSION==1
//version 1: full tables
template<unsigned N> //number of items
class BinaryRelation
{
    typedef BinaryRelation<N> MyType;
    typedef std::bitset<N*N> DataType;
    DataType m_data;
    typedef typename DataType::reference BitRef;

    BitRef _at (int i, int j) { return m_data[i+N*j]; } //unsafe version
public:
    BinaryRelation () {}

    class BoolPair
    {
        MyType *m_rel;
        int m_i, m_j;
    public:
        BoolPair (MyType* rel, int i, int j)
            : m_rel(rel), m_i(i), m_j(j) {}
        operator bool () { return m_rel->_at(m_i,m_j); }
        BoolPair& operator = (bool b)
        {
            m_rel->_at(m_i,m_j) = b;
            m_rel->_at(m_j,m_i) = b;
            return *this;
        }
    };

    BoolPair operator () (int i, int j) { return BoolPair(this,i,j); }
    bool operator () (int i, int j) const { return _at(i,j); }

    void insert (int i);
    void remove (int i);
    void merge (int i, int j);
};
template<unsigned N> void BinaryRelation<N>::insert(int i)
{
    for (int j=0; j<N; ++j) _at(i,j) = false;
    for (int j=0; j<N; ++j) _at(j,i) = false;
}
template<unsigned N> void BinaryRelation<N>::remove(int i)
{
    //do nothing
}
template<unsigned N> void BinaryRelation<N>::merge(int i, int j)
{
    //Assert (not _at(i,j), "...");
    for (int k=0; k<N; ++k) { BitRef b = _at(j,k); b = b | _at(i,k); }
    for (int k=0; k<N; ++k) { BitRef b = _at(k,j); b = b | _at(k,i); }
}
#endif

#if VERSION==2
//version 2: half+ tables
template<unsigned N> //number of items
class BinaryRelation
{
    typedef BinaryRelation<N> MyType;
    typedef std::bitset<N*(N+1)/2> DataType;
    DataType m_data;
    typedef typename DataType::reference BitRef;

    BitRef _at (int i, int j)
    { return i<j ? m_data[(j*(j+1))/2 + i]
                 : m_data[(i*(i+1))/2 + j]; } //unsafe version
public:
    BinaryRelation () {}

    BitRef operator () (int i, int j)       { return _at(i,j); }
    bool   operator () (int i, int j) const { return _at(i,j); }

    void insert (int i);
    void remove (int i) {} //do nothing
    void merge (int i, int j);
};
template<unsigned N> void BinaryRelation<N>::insert(int i)
{ for (int j=0; j<N; ++j) _at(i,j) = false; }
template<unsigned N> void BinaryRelation<N>::merge(int i, int j)
{ for (int k=0; k<N; ++k) { BitRef b = _at(j,k); b = b | _at(i,k); } }
#endif

#if VERSION==3
//version 3: half- tables
template<unsigned N> //number of items
class BinaryRelation
{
    typedef BinaryRelation<N> MyType;
    typedef std::bitset<N*(N-1)/2> DataType;
    DataType m_data;
    typedef typename DataType::reference BitRef;

    BitRef _at_ord (int i, int j)
    {
        Assert (i<j, "_at_ord called with out-or-order args");
        return m_data[(j*(j-1))/2 + i];
    }
    BitRef _at (int i, int j)
    { return i<j ? _at_ord(i,j) : _at_ord(j,i); } //unsafe version

    friend BitRef& operator |= (BitRef b, BitRef a)
    { b = b|a; return b; }
public:
    BinaryRelation () {}

    BitRef operator () (int i, int j)       { return _at(i,j); }
    bool   operator () (int i, int j) const { return _at(i,j); }

    void insert (const int i);
    void remove (const int i) {} //do nothing
    void merge (const int i, const int j);
};
template<unsigned N> void BinaryRelation<N>::insert(const int i)
{
    int j;
    for (j=0; j<i; ++j) _at_ord(j,i) = false;
    for (++j; j<N; ++j) _at_ord(i,j) = false;
}
template<unsigned N> void BinaryRelation<N>::merge(const int i, const int j)
{
    Assert (j!=i, "tried to merge with self");
    Assert (j<i, "out-of-order merger");
    int k;
    for (k=0; k<j; ++k) _at_ord(k,j) |= _at_ord(k,i);
    for (++k; k<i; ++k) _at_ord(j,k) |= _at_ord(k,i);
    for (++k; k<N; ++k) _at_ord(j,k) |= _at_ord(i,k);
}
#endif

#if VERSION==4
//version 3: half- tables, vector version
template<unsigned N> //number of items
class BinaryRelation
{
    typedef BinaryRelation<N> MyType;
    typedef std::vector<bool> DataType;
    DataType m_data;
    typedef typename DataType::reference BitRef;

    BitRef _at_ord (int i, int j)
    {
        Assert (i<j, "_at_ord called with out-or-order args");
        return m_data[(j*(j-1))/2 + i];
    }
    BitRef _at (int i, int j)
    { return i<j ? _at_ord(i,j) : _at_ord(j,i); } //unsafe version

    friend BitRef& operator |= (BitRef b, BitRef a)
    { b = b|a; return b; }
public:
    BinaryRelation () : m_data((N*(N-1))/2) {}

    BitRef operator () (int i, int j)       { return _at(i,j); }
    bool   operator () (int i, int j) const { return _at(i,j); }

    void insert (const int i);
    void remove (const int i) {} //do nothing
    void merge (const int i, const int j);
};
template<unsigned N> void BinaryRelation<N>::insert(const int i)
{
    int j;
    for (j=0; j<i; ++j) _at_ord(j,i) = false;
    for (++j; j<N; ++j) _at_ord(i,j) = false;
}
template<unsigned N> void BinaryRelation<N>::merge(const int i, const int j)
{
    Assert (j!=i, "tried to merge with self");
    Assert (j<i, "out-of-order merger");
    int k;
    for (k=0; k<j; ++k) _at_ord(k,j) |= _at_ord(k,i);
    for (++k; k<i; ++k) _at_ord(j,k) |= _at_ord(k,i);
    for (++k; k<N; ++k) _at_ord(j,k) |= _at_ord(i,k);
}
#endif

#if VERSION==5
//version 3: half- tables, dynamic array version
template<unsigned N> //number of items
class BinaryRelation
{
    typedef BinaryRelation<N> MyType;
    typedef int Block;
    typedef Block* DataType;
    DataType m_data;
    enum {
        SIZE = (N*(N-1))/2,
        BLOCK_SIZE = sizeof(Block),
        NUM_BLOCKS = (SIZE + BLOCK_SIZE-1) / BLOCK_SIZE //rounds up
    };

    class BitRef
    {
        Block& m_data;
        int m_mask;
    public:
        BitRef (Block* base, int i)
            : m_data(base[i/BLOCK_SIZE]),
              m_mask(1 << (i%BLOCK_SIZE))
        {}
        operator bool () { return m_data & m_mask; }
        BitRef& operator = (bool b)
        {
            m_data |= b * m_mask;
            m_data &= ~(b * ~m_mask);
        }
        void zero () { m_data &= ~m_mask; }

        BitRef& operator |= (bool b) { m_data |= b * m_mask; }
        BitRef& operator &= (bool b) { m_data &= ~(!b * m_mask); }
        //BitRef& operator ^= (bool b) { if (b)  m_data ^= m_mask; }
    };

    BitRef _at_ord (int i, int j)
    {
        Assert (i<j, "_at_ord called with out-or-order args");
        return BitRef(m_data, (j*(j-1))/2 + i);
    }
    BitRef _at (int i, int j)
    { return i<j ? _at_ord(i,j) : _at_ord(j,i); } //unsafe version
public:
    BinaryRelation () : m_data(new Block[NUM_BLOCKS]) {}
    ~BinaryRelation () { delete m_data; }

    BitRef operator () (int i, int j)       { return _at(i,j); }
    bool   operator () (int i, int j) const { return _at(i,j); }

    void insert (const int i);
    void remove (const int i) {} //do nothing
    void merge (const int i, const int j);
};
template<unsigned N> void BinaryRelation<N>::insert(const int i)
{
    int I_ = i / BLOCK_SIZE;

    //horiz blocks
    for (int J_=0; J_<I_; ++J_) m_data[J_] = 0;

    //middle blocks
    int J = min(i+BLOCK_SIZE,static_cast<int>(N));
    for (int j=1+I_*BLOCK_SIZE; j<J; ++j) {
        _at_ord(j,i).zero();
    }

    //vertical blocks
    int _I = i % BLOCK_SIZE;
    Block mask = ~(1 << _I);
    for (int j = J; j < N; ++j) {
        m_data[((j*(j-1))/2)%BLOCK_SIZE] &= mask;
    }
    //for (j=0; j<i; ++j) _at_ord(j,i) &= false;
    //for (++j; j<N; ++j) _at_ord(i,j) &= false;
}
template<unsigned N> void BinaryRelation<N>::merge(const int i, const int j)
{
    Assert (j!=i, "tried to merge with self");
    Assert (j<i, "out-of-order merger");
    int k;
    for (k=0; k<j; ++k) _at_ord(k,j) |= _at_ord(k,i);
    for (++k; k<i; ++k) _at_ord(j,k) |= _at_ord(k,i);
    for (++k; k<N; ++k) _at_ord(j,k) |= _at_ord(i,k);
}
#endif

#if VERSION==6
//version 3: half- tables, dynamic array version, 32x32 blocks
template<unsigned N> //number of items
class BinaryRelation
{
    enum {
        SIDE = 32,
        M = (N+SIDE-1)/SIDE, //rounds up
        BLOCK_SIZE = SIDE*SIDE,
        NUM_BLOCKS = M*(M+1)/2
    };

    typedef BinaryRelation<N> MyType;
    typedef int Block[SIDE];
    Block* m_blocks;

    //bit references
    class BitRef
    {
        int& m_line;
        int m_mask;
    public:
        BitRef (Block& block, int _i, int _j)
            : m_line(block[_j]),
              m_mask(1 << _i)
        {}
        operator bool () { return m_line & m_mask; }
        BitRef& operator = (bool b)
        {
            m_line |= b * m_mask;
            m_line &= ~(b * ~m_mask);
        }
        void zero () { m_line &= ~m_mask; }

        BitRef& operator |= (bool b) { m_line |= b * m_mask; }
        BitRef& operator &= (bool b) { m_line &= ~(!b * m_mask); }
    };

    BitRef _at_ord (int i, int j)
    {
        Assert (i<j, "_at_ord called with out-or-order args");
        int i_ = i / SIDE, j_ = j / SIDE; //larger dimensions
        int _i = i % SIDE, _j = j % SIDE; //smaller dimensions
        Block& block = m_blocks[(j_*(j_+1))/2 + i_];
        return BitRef(block, _i, _j);
    }
    BitRef _at (int i, int j)
    { return i<j ? _at_ord(i,j) : _at_ord(j,i); } //unsafe version
public:
    BinaryRelation () : m_blocks(new Block[NUM_BLOCKS]) {}
    ~BinaryRelation () { delete m_blocks; }

    BitRef operator () (int i, int j)       { return _at(i,j); }
    bool   operator () (int i, int j) const { return _at(i,j); }

    void insert (const int i);
    void remove (const int i) {} //do nothing
    void merge (const int i, const int j);
};
template<unsigned N> void BinaryRelation<N>::insert(const int i)
{
    int j;
    for (j=0; j<i; ++j) _at_ord(j,i) &= false;
    for (++j; j<N; ++j) _at_ord(i,j) &= false;
}
template<unsigned N> void BinaryRelation<N>::merge(const int i, const int j)
{
    Assert (j!=i, "tried to merge with self");
    Assert (j<i, "out-of-order merger");
    int k;
    for (k=0; k<j; ++k) _at_ord(k,j) |= _at_ord(k,i);
    for (++k; k<i; ++k) _at_ord(j,k) |= _at_ord(k,i);
    for (++k; k<N; ++k) _at_ord(j,k) |= _at_ord(i,k);
}
#endif

#if VERSION==7
//version 3: half- tables, dynamic array version, 32x32 blocks, vectored
template<unsigned N> //number of items
class BinaryRelation
{
    enum {
        SIDE = 32,
        M = (N+SIDE-1)/SIDE, //rounds up
        BLOCK_SIZE = SIDE*SIDE,
        NUM_BLOCKS = M*(M+1)/2
    };

    typedef BinaryRelation<N> MyType;
    typedef int Line;
    typedef Line Block[SIDE];
    Block* m_blocks;
    Block& _block_ord (int i_, int j_)
    {
        Assert (i<j, "_block_ord called with out-or-order args");
        return m_blocks[(j_*(j_+1))/2 + i_];
    }

    //bit references
    class BitRef
    {
        Line& m_line;
        Line m_mask;
    public:
        BitRef (Block& block, int _i, int _j)
            : m_line(block[_j]),
              m_mask(1 << _i)
        {}
        operator bool () { return m_line & m_mask; }
        BitRef& operator = (bool b)
        {
            m_line |= b * m_mask;
            m_line &= ~(b * ~m_mask);
        }
        void zero () { m_line &= ~m_mask; }

        BitRef& operator |= (bool b) { m_line |= b * m_mask; }
        BitRef& operator &= (bool b) { m_line &= ~(!b * m_mask); }
    };

    BitRef _bit_ord (int i, int j)
    {
        Assert (i<j, "_bit_ord called with out-or-order args");
        int i_ = i / SIDE, j_ = j / SIDE; //larger dimensions
        int _i = i % SIDE, _j = j % SIDE; //smaller dimensions
        return BitRef(_block_ord(i_, j_), _i, _j);
    }
    BitRef _bit (int i, int j)
    { return i<j ? _bit_ord(i,j) : _bit_ord(j,i); } //unsafe version
public:
    BinaryRelation () : m_blocks(new Block[NUM_BLOCKS]) {}
    ~BinaryRelation () { delete m_blocks; }

    BitRef operator () (int i, int j)       { return _bit(i,j); }
    bool   operator () (int i, int j) const { return _bit(i,j); }

    void insert (const int i);
    void remove (const int i) {} //do nothing
    void merge (const int i, const int j);
};
template<unsigned N> void BinaryRelation<N>::insert(const int i)
{
    const int i_ = i / SIDE, _i = i % SIDE;
    
    //horiz part, wikkit fass
    for (int j_=0; j_<i_; ++j_) _block_ord(j_, i_)[_i] = 0;

    //diagonal block, const time
    Block& diag_block = _block_ord(i_,i_);
    int _j;
    for (_j=0; _j<_i; ++_j)   BitRef(diag_block, _j, _i).zero();
    for (++_j; _j<SIDE; ++_j) BitRef(diag_block, _i, _j).zero();

    //vert part, sluggish but good locality-of-ref
    Line mask = ~(1 << _i);
    for (int j_=1+i_; j_<M; ++j_) {
        Block& block = _block_ord(i_, j_);
        for (int _j=0; _j<SIDE; ++_j) {
            block[_j] &= mask;
        }
    }
}
template<unsigned N> void BinaryRelation<N>::merge(const int i, const int j)
{
    Assert (j!=i, "tried to merge with self");
    Assert (j<i, "out-of-order merger");

    const int i_ = i / SIDE, _i = i % SIDE;
    const int j_ = j / SIDE, _j = j % SIDE;

    //horiz-horiz part, wikkit fass
    for (int k_=0; k_<j_; ++k_) {
        _block_ord(k_, j_)[_j] |= _block_ord(k_, i_)[_i];
    }

    //horiz-diag part, const time
    Block& jj_block = _block_ord(j_,j_);
    int _k;
    for (_k=0; _k<_j; ++_k) {
        BitRef(jj_block, _j, _k) |= BitRef(jj_block, _k, _i);
    }
    for (++_k; _k<SIDE; ++_k) {
        BitRef(jj_block, _k, _j) |= BitRef(jj_block, _k, _i);
    }
    
    //vert-horiz part, sluggish but good locality-of-ref
    Line j_mask = 1 << _j;
    for (int k_=i_; k_<M; ++k_) {
        Block& j_block = _block_ord(j_, k_);
        Block& i_block = _block_ord(k_, i_);
        for (int _k=0; _k<SIDE; ++_k) {
            Line k_mask = 1 << _k;
            BitRef(j_block, _j, _k) |= BitRef(i_block, _k, _i);
        }
    }

    //vert-diag part, const time
    Block& ii_block = _block_ord(i_,i_);
    for (_k=0; _k<_i; ++_k) {
        BitRef(ii_block, _j, _k) |= BitRef(ii_block, _k, _i);
    }
    for (++_k; _k<SIDE; ++_k) {
        BitRef(ii_block, _j, _k) |= BitRef(ii_block, _i, _k);
    }

    //vert-vert part, asymptotically unremarkable but good locality-of-ref
    Line i_mask = 1 << _i;
    for (int k_=i_; k_<M; ++k_) {
        Block& j_block = _block_ord(j_, k_);
        Block& i_block = _block_ord(i_, k_);
        for (int _k=0; _k<SIDE; ++_k) {
            BitRef(j_block, _j, _k) |= BitRef(i_block, _i, _k);
        }
    }
}
#endif

#define N (1<<13)
//const unsigned N = 1<<10;

int main ()
{
    std::cout << "sizeof BinaryRelation<" << N << "> = "
        << sizeof(BinaryRelation<N>) << std::endl;
#if VERSION < 4
    BinaryRelation<N> &R = *(new BinaryRelation<N>); //allocate on heap
#else
    BinaryRelation<N> R; //allocate on stack
#endif

    std::cout << "inserting some items" << std::endl;
    for (int i=0; i<N; ++i) R.insert(i);

    std::cout << "setting some values" << std::endl;
    for (int i=0; i<N; ++i) {
        for (int j=i+1; j<N; ++j) {
            R(i,j) = (i % (1+j)) > 0;
        }
    }

    std::cout << "merging some items" << std::endl;
    for (int i=0; i<N/3; ++i) {
        int m=(2*i)%N, n=(2*(N-i-1)+1)%N;
        R.merge(max(m,n), min(m,n));
    }

    return 0;
}

