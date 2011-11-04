#ifndef JOHANN_MOMENTS_H
#define JOHANN_MOMENTS_H

#include <utility>
#include <cmath>

namespace Moments
{

const Logging::Logger logger("moment", Logging::DEBUG);

//================================ (x,x',x'') ================================
//quadratic function for taking second derivatives
class Poly2
{
    typedef Poly2  MyType;
    Float m_x0;
    Float m_x1;
    Float m_x2;
    Float _unused_filler_space; //needed for correct alignment
public:
    Poly2 () : m_x0(0), m_x1(0), m_x2(0) {}

    //access
    inline MyType& operator = (Float rhs);
    Float& value  () { return m_x0; }
    Float& deriv1 () { return m_x1; }
    Float& deriv2 () { return m_x2; }
    Float getValue  () const { return m_x0; }
    Float getDeriv1 () const { return m_x1; }
    Float getDeriv2 () const { return m_x2; }

    //residual
    inline Float norm2 (Float h=1.0f) const;
    inline Float norm  (Float h=1.0f) const;

//---------- generic math operations ----------

    //negation
    inline MyType operator - () const;

    //addition
    inline MyType operator += (Float rhs);
    inline MyType operator +  (Float rhs) const;
    friend inline MyType operator + (Float lhs, const MyType& rhs);
    inline MyType operator += (const MyType& rhs);
    inline MyType operator +  (const MyType& rhs) const;

    //subtraction
    inline MyType operator -= (Float rhs);
    inline MyType operator -  (Float rhs) const;
    friend inline MyType operator - (Float lhs, const MyType& rhs);
    inline MyType operator -= (const MyType& rhs);
    inline MyType operator -  (const MyType& rhs) const;

    //multiplication
    inline MyType operator *= (Float rhs);
    inline MyType operator *  (Float rhs) const;
    friend inline MyType operator * (Float lhs, const MyType& rhs);
    inline MyType operator *= (const MyType& rhs);
    inline MyType operator *  (const MyType& rhs) const;

    //division
    inline MyType operator /= (Float rhs);
    inline MyType operator /  (Float rhs) const;
    friend inline MyType operator / (Float lhs, const MyType& rhs);
    inline MyType operator /= (const MyType& rhs);
    inline MyType operator /  (const MyType& rhs) const;

    //functions
    friend inline MyType exp (const MyType& val);
    friend inline MyType log (const MyType& val);
};

//access
inline Poly2& Poly2::operator = (Float rhs)
{
    m_x0 = rhs;
    m_x1 = 0;
    m_x2 = 0;
    return *this;
}

//negation
inline Poly2 Poly2::operator - () const
{
    return -1 * (*this);
}

//addition
inline Poly2 Poly2::operator += (Float rhs)
{
    m_x0 += rhs;
    return *this;
}
inline Poly2 Poly2::operator + (Float rhs) const
{
    Poly2 result(*this);
    return result += rhs;
}
inline Poly2 operator + (Float lhs, const Poly2& rhs)
{
    Poly2 result(rhs);
    return result += lhs;
}
inline Poly2 Poly2::operator += (const Poly2& rhs)
{
    m_x0 += rhs.m_x0;
    m_x1 += rhs.m_x1;
    m_x2 += rhs.m_x2;
    return *this;
}
inline Poly2 Poly2::operator + (const Poly2& rhs) const
{
    Poly2 result(*this);
    return result += rhs;
}

//subtraction
inline Poly2 Poly2::operator -= (Float rhs)
{
    m_x0 -= rhs;
    return *this;
}
inline Poly2 Poly2::operator - (Float rhs) const
{
    Poly2 result(*this);
    return result -= rhs;
}
inline Poly2 operator - (Float lhs, const Poly2& rhs)
{
    Poly2 result(-rhs);
    return result += lhs;
}
inline Poly2 Poly2::operator -= (const Poly2& rhs)
{
    m_x0 -= rhs.m_x0;
    m_x1 -= rhs.m_x1;
    m_x2 -= rhs.m_x2;
    return *this;
}
inline Poly2 Poly2::operator - (const Poly2& rhs) const
{
    Poly2 result(*this);
    return result -= rhs;
}

//multiplication
inline Poly2 Poly2::operator *= (Float rhs)
{
    m_x0 *= rhs;
    m_x1 *= rhs;
    m_x2 *= rhs;
    return *this;
}
inline Poly2 Poly2::operator * (Float rhs) const
{
    Poly2 result(*this);
    return result *= rhs;
}
inline Poly2 operator * (Float lhs, const Poly2& rhs)
{
    Poly2 result(rhs);
    return result *= lhs;
}
inline Poly2 Poly2::operator *= (const Poly2& rhs)
{
    m_x2 *= rhs.m_x0;
    m_x2 += 2.0f * m_x1 * rhs.m_x1;
    m_x2 += m_x0 * rhs.m_x2;
    m_x1 *= rhs.m_x0;
    m_x1 += m_x0 * rhs.m_x1;
    m_x0 *= rhs.m_x0;
    return *this;
}
inline Poly2 Poly2::operator * (const Poly2& rhs) const
{
    Poly2 result(*this);
    return result *= rhs;
}

//division
inline Poly2 Poly2::operator /= (Float rhs)
{
    m_x0 /= rhs;
    m_x1 /= rhs;
    m_x1 /= rhs;
    return *this;
}
inline Poly2 Poly2::operator / (Float rhs) const
{
    Poly2 result(*this);
    return result /= rhs;
}
inline Poly2 operator / (Float lhs, const Poly2& rhs)
{
    Float inv = 1.0f / rhs.m_x0;
    Poly2 result;
    result.m_x0 = lhs * inv;
    result.m_x1 = -result.m_x0 * rhs.m_x0 * inv;
    result.m_x2 = 2.0f * result.m_x0 * rhs.m_x1 * rhs.m_x1;
    result.m_x2 -= lhs * rhs.m_x2;
    result.m_x2 *= inv * inv;
    return result;
}
inline Poly2 Poly2::operator / (const Poly2& rhs) const
{
    Float inv = 1.0f / rhs.m_x0;
    Poly2 result = inv * (*this);
    result.m_x1 -= result.m_x0 * rhs.m_x1 * inv;
    Float x2 = 2.0f * result.m_x1 * rhs.m_x1 + result.m_x0 * rhs.m_x2;
    result.m_x2 -= x2 * inv;
    return result;
}
inline Poly2 Poly2::operator /= (const Poly2& rhs)
{
    return (*this) = (*this) / rhs;
}

//functions
inline Poly2 exp (const Poly2& val)
{
    Poly2 result;
    result.m_x0 = expf(val.m_x0);
    result.m_x1 = result.m_x0 * val.m_x1;
    result.m_x2 = result.m_x0 * (sqr(val.m_x1) + val.m_x2);
    return result;
}
inline Poly2 log (const Poly2& val)
{
    Poly2 result;
    result.m_x0 = logf(val.m_x0);
    Float inv = 1.0f/val.m_x0;
    result.m_x1 = val.m_x1 * inv;
    result.m_x2 = -result.m_x1 * val.m_x1;
    result.m_x2 += val.m_x2;
    result.m_x2 *= inv;
    return result;
}

//residuals
inline Float Poly2::norm2 (Float h) const
{
    Float h2 = sqr(h);
    return sqr(m_x0) + h2*(sqr(m_x1) + h2*sqr(m_x2));
}
inline Float Poly2::norm (Float h) const
{ return sqrt(norm2(h)); }

//finite-difference approximation
template<class Function>
inline Poly2 FiniteDifference (Function function, Float x=0.0f, Float h=1e-2f)
{
    Float f_x = function(x);
    Float f_p = function(x+h);
    Float f_n = function(x-h);

    Poly2 result;
    result.value() = f_x;
    Float h_inv = 1.0/h;
    result.deriv1() = (f_p - f_n) * (0.5 * h_inv);
    result.deriv2() = (f_p - 2.0f*f_x + f_n) * sqr(h_inv);
    return result;
}

//================================ (Z,H) ================================
inline Float h_fun (Float z)
{
    Assert3(z>=0.0f, "in h_fun: z < 0");
    return z == 0.0f ? 0.0f : -z * logf(z);
}
inline Float zh_mul (Float s, Float z, Float h)
{
    Assert3(s>=0.0f, "in zh_mul: s < 0");
    Assert3(z>=0.0f, "in zh_mul: z < 0");
    if (s == 0.0f) return 0.0f;
    if (z == 0.0f) return s * h;
    return s * (h - z*logf(s));
}

class Z_H
{
    typedef Z_H MyType;
    Float m_z, m_h;
public:
    Z_H () : m_z(0), m_h(0) {}
    Z_H (Float z) : m_z(z), m_h(h_fun(z)) {}
    Z_H (Float z, Float h) : m_z(z), m_h(h) {}

    //access
    inline MyType& operator = (Float rhs)
    { m_z=rhs; m_h=h_fun(rhs); return *this; }
    Float& total   () { return m_z; }
    Float& entropy () { return m_h; }
    Float getTotal   () const { return m_z; }
    Float getEntropy () const { return m_h; }

    //residual
    inline Float norm2 (Float h=1.0f) const;
    inline Float norm  (Float h=1.0f) const;

//---------- generic math operations ----------

    //sum & difference
    inline MyType operator += (const MyType& rhs);
    inline MyType operator +  (const MyType& rhs) const;
    inline MyType operator -= (const MyType& rhs);
    inline MyType operator -  (const MyType& rhs) const;
    inline MyType operator - () const;

    //product
    inline MyType operator *= (const MyType& rhs);
    inline MyType operator *  (const MyType& rhs) const;

    //scalar multiplication & division
    inline MyType operator *= (Float rhs);
    inline MyType operator *  (Float rhs) const;
    friend inline MyType operator * (Float lhs, const MyType& rhs);
    inline MyType operator /= (Float rhs);
    inline MyType operator /  (Float rhs) const;
};

//sum & difference
inline Z_H Z_H::operator += (const Z_H& rhs)
{
    m_z += rhs.m_z;
    m_h += rhs.m_h;
    return *this;
}
inline Z_H Z_H::operator + (const Z_H& rhs) const
{
    Z_H result(*this);
    return result += rhs;
}
inline Z_H Z_H::operator -= (const Z_H& rhs)
{
    m_z -= rhs.m_z;
    m_h -= rhs.m_h;
    return *this;
}
inline Z_H Z_H::operator - (const Z_H& rhs) const
{
    Z_H result(*this);
    return result -= rhs;
}
inline Z_H Z_H::operator - () const
{
    return -1 * (*this);
}

//product
inline Z_H Z_H::operator *= (const Z_H& rhs)
{
    m_h = zh_mul(rhs.m_z, m_z, m_h)
        + zh_mul(m_z, rhs.m_z, rhs.m_h);
    m_z *= rhs.m_z;
    return *this;
}
inline Z_H Z_H::operator * (const Z_H& rhs) const
{
    Z_H result(*this);
    return result *= rhs;
}

//scalar multiplication & division
inline Z_H Z_H::operator *= (Float rhs)
{
    m_h = zh_mul(rhs, m_z, m_h);
    m_z *= rhs;
    return *this;
}
inline Z_H Z_H::operator * (Float rhs) const
{
    Z_H result(*this);
    return result *= rhs;
}
inline Z_H operator * (Float lhs, const Z_H& rhs)
{
    Z_H result(rhs);
    return result *= lhs;
}
inline Z_H Z_H::operator /= (Float rhs)
{
    return (*this) *= (1.0f/rhs);
}
inline Z_H Z_H::operator / (Float rhs) const
{
    Z_H result(*this);
    return result *= (1.0f/rhs);
}

//residuals
inline Float Z_H::norm2 (Float h) const
{
    return sqr(m_z) + sqr(h * m_h);
}
inline Float Z_H::norm (Float h) const
{
    return sqrt(norm2(h));
}

//================================ (Z0,Z,D) ================================
inline Float d_fun(Float z0, Float z)
{
    Assert3(z0>=0.0f, "in d_fun: z0 < 0");
    Assert3(z >=0.0f, "in d_fun: z < 0");
    return z0 == 0.0f ? 0.0f : z0 * logf(z0/z);
}

class Z_Z_D
{
    typedef Z_Z_D MyType;
    Float m_z0, m_z, m_d; //d(z0|z)
public:
    Z_Z_D () : m_z0(0), m_z(0), m_d(0) {}
    Z_Z_D (Float z0, Float z) : m_z0(z0), m_z(z), m_d(d_fun(z0,z)) {}
    Z_Z_D (Float z0, Float z, Float d) : m_z0(z0), m_z(z), m_d(d) {}

    //access
    Float& total0     () { return m_z0; }
    Float& total      () { return m_z; }
    Float& relentropy () { return m_d; }
    Float getTotal0     () const { return m_z0; }
    Float getTotal      () const { return m_z; }
    Float getRelentropy () const { return m_d; }

    //residual
    inline Float norm2 (Float h=1.0f) const;
    inline Float norm  (Float h=1.0f) const;

//---------- generic math operations ----------

    //sum & difference
    inline MyType operator += (const MyType& rhs);
    inline MyType operator +  (const MyType& rhs) const;
    inline MyType operator -= (const MyType& rhs);
    inline MyType operator -  (const MyType& rhs) const;
    inline MyType operator - () const;

    //product
    inline MyType operator *= (const MyType& rhs);
    inline MyType operator *  (const MyType& rhs) const;

    //scalar multiplication & division
    inline MyType operator *= (Float rhs);
    inline MyType operator *  (Float rhs) const;
    friend inline MyType operator * (Float lhs, const MyType& rhs);
    inline MyType operator /= (Float rhs);
    inline MyType operator /  (Float rhs) const;
};

//sum & difference
inline Z_Z_D Z_Z_D::operator += (const Z_Z_D& rhs)
{
    m_z0 += rhs.m_z0;
    m_z  += rhs.m_z;
    m_d  += rhs.m_d;
    return *this;
}
inline Z_Z_D Z_Z_D::operator + (const Z_Z_D& rhs) const
{
    Z_Z_D result(*this);
    return result += rhs;
}
inline Z_Z_D Z_Z_D::operator -= (const Z_Z_D& rhs)
{
    m_z0 -= rhs.m_z0;
    m_z  -= rhs.m_z;
    m_d  -= rhs.m_d;
    return *this;
}
inline Z_Z_D Z_Z_D::operator - (const Z_Z_D& rhs) const
{
    Z_Z_D result(*this);
    return result -= rhs;
}
inline Z_Z_D Z_Z_D::operator - () const
{
    return -1 * (*this);
}

//product
inline Z_Z_D Z_Z_D::operator *= (const Z_Z_D& rhs)
{
    m_d *= rhs.m_z0;
    m_d += m_z0 * rhs.m_d;
    m_z0 *= rhs.m_z0;
    m_z  *= rhs.m_z;
    return *this;
}
inline Z_Z_D Z_Z_D::operator * (const Z_Z_D& rhs) const
{
    Z_Z_D result(*this);
    return result *= rhs;
}

//scalar multiplication & division
inline Z_Z_D Z_Z_D::operator *= (Float rhs)
{
    m_z0 *= rhs;
    m_z  *= rhs;
    m_d  *= rhs;
    return *this;
}
inline Z_Z_D Z_Z_D::operator * (Float rhs) const
{
    Z_Z_D result(*this);
    return result *= rhs;
}
inline Z_Z_D operator * (Float lhs, const Z_Z_D& rhs)
{
    Z_Z_D result(rhs);
    return result *= lhs;
}
inline Z_Z_D Z_Z_D::operator /= (Float rhs)
{
    return (*this) *= (1.0f/rhs);
}
inline Z_Z_D Z_Z_D::operator / (Float rhs) const
{
    Z_Z_D result(*this);
    return result *= (1.0f/rhs);
}

//residuals
inline Float Z_Z_D::norm2 (Float h) const
{
    return sqr(m_z0) + sqr(m_z) + sqr(h * m_d);
}
inline Float Z_Z_D::norm (Float h) const
{
    return sqrt(norm2(h));
}

}

#endif
