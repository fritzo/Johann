#ifndef JOHANN_LINALG_H
#define JOHANN_LINALG_H

#include "definitions.h"
#include <cmath>
#include <string>
#include <vector>
#include <utility>

namespace LinAlg
{

const Logging::Logger logger("linalg", Logging::DEBUG);

/** A tensor class for N by N by ... by N arrays.
 * Typically, the rank is 1, 2, or 3.
 * The dimension and rank cannot be changed after creation.
 *
 * The prototypical uses are
 *  (1) higher derivatives: Jacobians and Hessians
 *  (2) metrics, Christoffel symbols, etc.
 *
 * Tensors can be either
 *  (1) objects with data, or
 *  (2) aliases to other data
 *
 *  WARNING: linear algebra routines are implemented only for
 *    symmetric positive-definite matrices.
 */
class Eig;
class Tensor
{
    const Int m_dim, m_rank, m_size;
    Float* m_data;
    const bool _alias;
public:
    void clear ();

    //copying
    Tensor& operator= (const Tensor& other);
    Tensor& operator= (const std::vector<Float>& src);
    Tensor& operator= (const Float* data);
    Tensor& operator= (Float constant);
    void set (Float constant) { *this = constant; }

    //tensors as objects with data
    Tensor (Int dim, Int rank=1)
        : m_dim(dim), m_rank(rank), m_size(powi(dim,rank)),
          m_data(new Float[m_size]), _alias(false)
    { clear(); }
    Tensor (const Tensor& other)
        : m_dim(other.m_dim), m_rank(other.m_rank), m_size(other.m_size),
          m_data(new Float[m_size]), _alias(false)
    { *this = other; }
    ~Tensor () { if (not _alias) delete[] m_data; }

    //tensors as aliases
    Tensor (Int dim, Int rank, Float* data)
        : m_dim(dim), m_rank(rank), m_size(powi(dim,rank)),
          m_data(data), _alias(true) {}
    void set (Float* data) {
        Assert1(_alias, "tried to set an alias tensor");
        m_data = data;
    }

    //info
    bool alias () const { return _alias; }
    Int size () const { return m_size; }
    Int dim  () const { return m_dim; }
    Int rank () const { return m_rank; }
    typedef std::pair<Int,Int> Shape;
    Shape shape () const { return Shape(m_dim,m_rank); }

    //raw access
    Float*       data ()       { return m_data; }
    const Float* data () const { return m_data; }
    void copy_to (std::vector<Float>& dest) const
    {
        std::vector<Float> data(m_data, m_data+m_size);
        dest.swap(data);
    }
    Float* operator[] (Int i) const
    {//slice
        Assert1(m_rank > 0, "sliced rank-0 tensor");
        return m_data + powi(m_dim,m_rank-1) * i;
    }

    //indexing
    Float& data (Int i)
    { Assert2(i < m_size, "index out of bounds: " << i); return m_data[i]; }
    Float& operator() (Int i) { return data(i); }
    Float& operator() (Int i, Int j) { return data(m_dim*i + j); }
    Float& operator() (Int i, Int j, Int k)
        { return data(m_dim*(m_dim*i + j) + k); }
    Float data (Int i) const
    { Assert2(i < m_size, "index out of bounds: " << i); return m_data[i]; }
    Float operator() (Int i) const { return data(i); }
    Float operator() (Int i, Int j) const { return data(m_dim*i + j); }
    Float operator() (Int i, Int j, Int k) const
        { return data(m_dim*(m_dim*i + j) + k); }

    //operations
    Tensor& operator+= (const Tensor& other);
    Tensor& operator-= (const Tensor& other);
    Tensor& operator+= (Float shift);
    Tensor& operator*= (Float scalar);
    Tensor& operator-= (Float shift) { return (*this += (-shift)); }
    Tensor& operator/= (Float scalar) { return (*this *= (1.0f / scalar)); }

    Float total () const;
    Float min   () const;
    Float max   () const;
    Float norm2 () const;
    Float norm  () const { return sqrtf(norm2()); }
    Float norm2 (const Tensor& M) const;
    Float norm  (const Tensor& M) const { return sqrtf(norm2(M)); }
    void normalize (Float scale=1.0) { (*this) *= (scale / norm()); }
    void normalize (const Tensor& M) { (*this) /= norm(M); }
    void randomize ();

    Tensor transpose () const;

    //linear equation for symmetric pos-def matrices: A x = b
    Tensor multiply (const Tensor& x) const;    // A.lmult(x) = b
    Tensor solve (const Tensor& b) const;       // A.ldiv(b) = x
    Tensor inverse () const;                    // A.inverse().lmult(b) = x
    Tensor cholesky () const;
    Tensor eigenvals () const;
    std::vector<Eig> eigenvects (Int num) const;

    //output, returning true on error
    bool save_to_file (string filename) const;
};
struct Eig
{
    Float val;
    Tensor vect;
    Eig () : vect(0) { Error("default constructed an Eig"); }
    Eig (Int n, Float v, Float* V) : val(v), vect(n) { vect = V; }
};

//tensor algebra
Tensor identity (Int dim);
Tensor diagonal (const Tensor& t);
Float dot (const Tensor& t1, const Tensor& t2);
Float dot (const Tensor& t1, const Tensor& M, const Tensor& t2);
Float dist2 (const Tensor& t1, const Tensor& t2);
inline Float dist (const Tensor& t1, const Tensor& t2)
{
    return sqrtf(dist2(t1,t2));
}
Tensor exp (const Tensor& t);

//randomization
Float random_normal();
Tensor random (Int dim, Int rank=1);
Tensor random_asym (Int dim);

//types
typedef Tensor Vect;
typedef Tensor Mat;
//  neighborhoods
struct Nbhd0 { Float val; Nbhd0(Float f) : val(f) {} };
struct Nbhd1 : public Nbhd0
{
    Float deriv;
    Nbhd1 (Float f, Float d) : Nbhd0(f), deriv(d) {}
};
struct VNbhd1 : public Nbhd0
{
    Vect grad;
    VNbhd1 (Int dim) : Nbhd0(0), grad(dim) {}
    VNbhd1 (Float f, const Vect& g) : Nbhd0(f), grad(g) {}
    VNbhd1 (const VNbhd1& n) : Nbhd0(n.val), grad(n.grad) {}
    VNbhd1& operator= (const VNbhd1& n)
    { val=n.val; grad=n.grad; return *this; }
};
struct VNbhd2 : public VNbhd1
{
    Vect hess;
    VNbhd2 (Int dim) : VNbhd1(dim), hess(dim,2) {}
    VNbhd2 (Float f, const Vect& g, const Mat& h) : VNbhd1(f,g), hess(h) {}
    VNbhd2 (const VNbhd2& n) : VNbhd1(n), hess(n.hess) {}
    VNbhd2& operator= (const VNbhd2& n)
    { val=n.val; grad=n.grad; hess=n.hess; return *this; }
};

}

ostream& operator<< (ostream& os, const LinAlg::Tensor& t);

#endif
