#ifndef JOHANN_SMALL_PTR_H
#define JOHANN_SMALL_PTR_H

// Johann was originally implemented on a 32-bit system,
// and some of the original code relies on small 32-bit pointers.
// This class is an add-only lookup table
// that compresses longer pointers to 32-bits
// (kind of like bit.ly).

#include <vector>
#include <unordered_map>
#include <stdint.h>

template<class T>
class SmallPtr
{
    static std::vector<T *> s_fwd;
    static std::unordered_map<T *, uint32_t> s_bwd;
    static const SmallPtr<T> null; // ensures NULL has offset 0

    uint32_t m_offset;

public:

    SmallPtr (T * ptr) {
        auto i = s_bwd.find(ptr);
        if (i != s_bwd.end()) {
            m_offset = i->second;
        } else {
            uint32_t offset = s_fwd.size();
            s_fwd.push_back(ptr);
            m_offset = s_bwd[ptr] = offset;
        }
    }
    operator T * () const {
        static SmallPtr<T> null(NULL); // HACK since SmallPtr<T>::null is not being initialized
        return s_fwd[m_offset];
    }
    operator bool () const { return m_offset; }
    T & operator *  () const { return *(s_fwd[m_offset]); }
    T * operator -> () const { return s_fwd[m_offset]; }
};

template<class T> std::vector<T*> SmallPtr<T>::s_fwd;
template<class T> std::unordered_map<T*,uint32_t> SmallPtr<T>::s_bwd;
template<class T> const SmallPtr<T> SmallPtr<T>::null(NULL); // XXX FIXME this is not being initialized

#endif // JOHANN_SMALL_PTR_H
