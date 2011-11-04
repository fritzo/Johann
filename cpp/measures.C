
#include "measures.h"
#include "aligned_alloc.h"
#include <algorithm>

namespace Measures
{

//ob polynomials
//XXX adding is slow
void ObPolynomial::add_ob (Ob ob)
{
    for (Int i=0, e=size(); i<e; ++i) {
        Pair &p = m_obs[i];
        if (*(p.first) == ob) {
            ++p.second;
            return;
        }
    }
    m_obs.push_back(Pair(ObHdl(ob), 1));
}
Int ObPolynomial::total () const
{
    Int result = 0;
    for (Int i=0; i<size(); ++i) result += num(i);
    return result;
}
void ObPolynomial::write_to (ostream& os) const
{
    os << m_apps << " apps + "
       << m_comps << " compositionss + "
       << m_joins << " joins";
    for (Int i=0; i<size(); ++i) {
        os << " + [" << ob(i) << "] @ " << num(i);
    }
}

//measure manager
Int Manager::s_static = 0;
Int Manager::s_size = 0;
Int Manager::s_obs = 0;
char* Manager::s_raw = NULL;    //zero-based
void* Manager::s_data = NULL;   //one-based
std::vector<bool> Manager::s_used(0);

inline char* alloc_bytes (Int size)
{//mediates between zero- and one- based
    Assert(((size >>6) <<6) == size, "please allocate a multiple of 64");
    return static_cast<char*>(nonstd::alloc_blocks(1<<6, size>>6));
}
inline void free_bytes (char* raw) { nonstd::free_blocks(raw); }
Int tidy_size (Int size)
{
    Int result = size + 1;              //always keep room for one
    result = nonstd::roundUp(result);   //powers of two only
    result = max(result, Int(1<<6));    //allocate cache lines
    return result;
}
void Manager::initialize (Int static_size)
{
    Assert (s_data == NULL, "initialized measures twice");

    Int size = tidy_size(static_size);
    logger.info() << "initializing measures to " << size << "B" |0;

    s_static = static_size;
    s_size = size;
    s_obs = Ob::capacity();
    s_raw = alloc_bytes(s_size * s_obs);    //zero-based
    s_data = s_raw - s_size;                //one-based
    std::vector<bool> new_used(s_size,false);
    s_used.swap(new_used);

    //clear static data
    for (Ob::iterator iter=Ob::begin(); iter!=Ob::end(); ++iter) {
        nonstd::clear_block(Manager::data(*iter), s_static);
    }
}
void Manager::clear ()
{
    Assert (s_data != NULL, "cleared measure manager twice");
    Int still_used = num_used();
    AssertW(still_used == 0, still_used << " measures still allocated");

    free_bytes(s_raw);
    s_static = 0;
    s_size = 0;
    s_obs = 0;
    s_raw = NULL;
    s_data = NULL;
    s_used.clear();
}
void Manager::resize (Int new_obs, const Int* new2old)
{
    logger.debug() << "resizing measures to " << new_obs << " obs" |0;
    char* new_raw = alloc_bytes(s_size * new_obs);

    //copy old data
    if (new2old) { //when compacting obs
        for (Int pos = 1; pos <= new_obs; ++pos) {
            nonstd::copy_blocks(new_raw + s_size * (        pos  - 1),
                                  s_raw + s_size * (new2old[pos] - 1),
                                sizeof(char), s_size);
        }
    } else { //when growing obs
        nonstd::copy_blocks(new_raw, s_raw,
                            sizeof(char), s_size * min(new_obs, s_obs));
    }

    //clear new data
    int more_obs = new_obs - s_obs;
    if (more_obs > 0) {
        nonstd::clear_block(new_raw + s_size * s_obs, s_size * more_obs);
    }

    //update
    free_bytes(s_raw);
    s_obs = new_obs;
    s_raw = new_raw;            //zero-based
    s_data = s_raw - s_size;    //one-based
}
void Manager::_resize_bytes (Int new_size)
{
    logger.info() << "resizing measures to " << new_size << "B" |0;

    //copy old data
    char* new_raw = alloc_bytes(new_size * s_obs);
    for (Int pos = 1; pos <= s_obs; ++pos) {
        nonstd::copy_blocks(new_raw + new_size * (pos - 1),
                              s_raw +   s_size * (pos - 1),
                            sizeof(char), min(new_size, s_size));
    }

    //update
    free_bytes(s_raw);
    s_size = new_size;
    s_raw = new_raw;            //zero-based
    s_data = s_raw - s_size;    //one-based
    s_used.resize(s_size, false);
}
void Manager::grow ()
{
    Assert (s_data, "grew measures before initializing");
    Int new_size = s_size * 2;
    _resize_bytes(new_size);
}
void Manager::shrink ()
{
    Assert (s_data, "shrunk measures before initializing");
    Int last = s_size - 1;
    while (last > s_static and not s_used[last]) --last;
    Int new_size = tidy_size(last + 1);
    _resize_bytes(new_size);
}
bool Manager::get_used (Int beg, Int end)
{//returns whether entire range is free
    for (Int i=beg; i<end; ++i) {
        if (s_used[i]) return true;
    }
    return false;
}
void Manager::set_used (Int beg, Int end, bool value)
{//returns whether entire range is free
    Assert (s_static <= beg and end <= s_size, "tried to free a bad block");
    for (Int i=beg; i<end; ++i) {
        Assert (s_used[i] != value,
                "tried to set used[" << i << "] twice to " << value);
        s_used[i] = value;
    }
}
Int Manager::num_used ()
{
    Int result = 0;
    for (Int i=s_static; i<s_size; ++i) {
        if (s_used[i]) ++result;
    }
    return result;
}
Int Manager::_alloc (Int block, Int num)
{
    Int i = (s_static + block - 1) / block;
    while (block * (i + num) > s_size) grow();
    while (get_used(block * i, block * (i + num))) {
        ++i;
        while (block * (i + num) > s_size) grow();
    }
    set_used(block * i, block * (i + num), true);
    return i;
}
void Manager::_free (Int block, Int offset, Int num)
{
    set_used(block * offset, block * (offset + num), false);
}
void Manager::create_ob (Ob ob) const
{//zeros out all measures as floats
    size_t block = sizeof(Float);
    Float* data_ob = data<Float>(ob);
    for (Int i=(s_static+block-1)/block, I=s_size/block; i<I; ++i) {
        if (s_used[block * i]) data_ob[i] = 0.0;
    }
}
void Manager::merge_obs (Ob dep, Ob rep) const
{//adds all measures as floats
    size_t block = sizeof(Float);
    Float* data_dep = data<Float>(dep);
    Float* data_rep = data<Float>(rep);
    for (Int i=(s_static+block-1)/block, I=s_size/block; i<I; ++i) {
        if (s_used[block * i]) data_rep[i] += data_dep[i];
    }
}

//vector measure functions
#define sparse_iter_over_Obs (Ob::sparse_iterator iter=Ob::sbegin(); iter!=Ob::send(); ++iter)

void VectMeas::set (Float m) const
{
    for sparse_iter_over_Obs { Ob ob = *iter;
        Float *v = (*this)(ob);
        for (Int i=0; i<m_size; ++i) v[i] = m;
    }
}
const VectMeas& VectMeas::operator*= (Float m) const
{
    for sparse_iter_over_Obs { Ob ob = *iter;
        Float *v = (*this)(ob);
        for (Int i=0; i<m_size; ++i) v[i] *= m;
    }
    return *this;
}
const VectMeas& VectMeas::operator*= (const Vect& m) const
{
    for sparse_iter_over_Obs { Ob ob = *iter;
        Float *v = (*this)(ob);
        for (Int i=0; i<m_size; ++i) v[i] *= m(i);
    }
    return *this;
}

#undef sparse_iter_over_Obs

}

