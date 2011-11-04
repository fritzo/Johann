
#include <cstdlib>
#include <iostream>
#include <vector>
#include <memory>

#define LOG(mess) std::cout << mess << std::endl;
#define PRINT(arg) LOG(#arg " = " << (arg));

template<class T>
struct Aligned
{
  void * operator new (size_t size)
  {
    void * result;
    posix_memalign(&result, 16, size);
    return result;
  }
  void * operator new[] (size_t size)
  {
    void * result;
    posix_memalign(&result, 16, size);
    return result;
  }
  void operator delete   (void * ptr) { free(ptr); }
  void operator delete[] (void * ptr) { free(ptr); }
} __attribute__ ((aligned (16)));

struct A : public Aligned<A>
{
  float data[5];
};

void assert_aligned (void * p)
{
  size_t alignment = reinterpret_cast<size_t>(p) % 16; \
  if (alignment) {
    LOG("bad alignment: " << alignment);
  }
}

// see http://www.codeguru.com/Cpp/Cpp/cpp_mfc/stl/article.php/c4079
template <class T> struct aligned_allocator
{
  typedef size_t    size_type;
  typedef ptrdiff_t difference_type;
  typedef T*        pointer;
  typedef const T*  const_pointer;
  typedef T&        reference;
  typedef const T&  const_reference;
  typedef T         value_type;
  template <class U> struct rebind { typedef aligned_allocator<U> other; };

  aligned_allocator() {}
  aligned_allocator(const aligned_allocator&) {}
  template <class U> aligned_allocator(const aligned_allocator<U>&) {}
  ~aligned_allocator() {}

  pointer address(reference x) const { return &x; }
  const_pointer address(const_reference x) const { return &x; }

  pointer allocate(size_type size,
                   std::allocator<void>::const_pointer hint = 0)
  {
    return new T[size];
  }
  void deallocate(pointer p, size_type n) { delete[] p; }
  size_type max_size() const { return -1; }

  //void construct(pointer p, const T& val) { p->T(val); }
  //void destroy(pointer p) { p->~T(p); }

  void construct(pointer p, const T& val) {}
  void destroy(pointer p) {}
};

int main ()
{
  PRINT(sizeof(Aligned<A>));
  PRINT(sizeof(A));

  LOG("checking operator new");
  for (int i = 0; i < 100; ++i) {
    assert_aligned(new A());
  }

  LOG("checking vector<A>");
  typedef std::vector<A, aligned_allocator<A> > As;
  for (int i = 0; i < 100; ++i) {
    As & as = *(new As(5));
    assert_aligned(&(as[0]));
  }

  return 0;
}

