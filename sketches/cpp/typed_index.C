#include <string>
#include <iostream>

class AssertionError {};

void Assert(bool condition, char* message)
{
    if (!condition) {
        std::cout << "Assertion Error: " <<  message << std::endl;
        throw AssertionError();
    }
}

// %%%%%%%%%%%%%%%%%% begin snip
// this can be moved to heap.h, and then included in equiv. and distinc.
// the stuff in heap.[Ch] can be moved to database.[Ch]

//type-conscious array index
template<class ValueType>
class TypedIndex
{
    int m_value;
public:
    //acts like an integer
    TypedIndex (int value) : m_value(value) {}
    TypedIndex& operator = (int value) { m_value = value; return *this; }
    operator int () { return m_value; }
    //operator int& () { return m_value; }
    
    //but knows some typing information
    typedef ValueType valueType;
    enum {size = sizeof(valueType)};
};

//multi-purpose memory block using reiterpret_cast instead of union
template<int size_in_bytes, class default_type=int>
class MemoryBlock
{
    char m_data[size_in_bytes];
public:
    inline void reset () { std::memset(m_data, 0, size_in_bytes); }

    //unsafe indexing
    template <class typedIndex>
    inline typename typedIndex::valueType& operator [] (typedIndex index)
    { return reinterpret_cast<typename typedIndex::valueType*>(m_data)[index]; }
    inline default_type& operator [] (int index)
    { return reinterpret_cast<default_type*>(m_data)[index]; }

    //safe indexing
    template <class typedIndex>
    typename typedIndex::valueType& operator () (typedIndex index)
    {
        static const int min_index = 0;
        static const int max_index = size_in_bytes / typedIndex::size;
        Assert(index >= min_index, "index out of bounds, too low");
        Assert(index <  max_index, "index out of bounds, too high");
        return reinterpret_cast<typename typedIndex::valueType*>(m_data)[index];
    }
    default_type& operator () (int index)
    {
        static const int min_index = 0;
        static const int max_index = size_in_bytes / sizeof(default_type);
        Assert(index >= min_index, "index out of bounds, too low");
        Assert(index <  max_index, "index out of bounds, too high");
        return reinterpret_cast<default_type*>(m_data)[index];
    }
};

// %%%%%%%%%%%%%%%%%% end snip



// %%%%%%%%%%%%%%%%%% begin snip
// this can be moved to heap_test.h,

const TypedIndex<int>    i1(0), i2(1), i3(2), i4(3);
const TypedIndex<float>  f1(0), f2(1), f3(2), f4(3);
const TypedIndex<double> d1(0), d2(1);

int main ()
{
    typedef Block<64> CacheLine;
    
    Assert(sizeof(CacheLine) == 16*sizeof(int), "invalid block size"); 
    
    CacheLine x;
    x.reset();
    
    x(i1) = 666;
    x(f2) = 3.141592653;
    x(d2) = 2.718281828;

    std::cout << "i1 = " << x[i1] << std::endl;
    std::cout << "f2 = " << x[f2] << std::endl;
    std::cout << "d2 = " << x[d2] << std::endl;

    int out_of_bounds = 16;
    x(out_of_bounds); //should throw an assertion error

    return 0;
}

// %%%%%%%%%%%%%%%%%% end snip


