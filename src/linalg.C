
#include "linalg.h"
#include <iomanip>
#include <cstring>
#include <cstdlib> //for drand48
#include <fstream>

//lapack declarations
typedef int* CHAR;
typedef int* INT;
typedef int* INTS;
typedef Float* REAL;
typedef Float* REALS;

//Hint: careful with scalars vs arrays
#ifdef SINGLE_PRECISION_FLOATS

extern "C" void spotrf_ (CHAR UPLO, INT N, REALS A, INT LDA, INT INFO);
extern "C" void spotri_ (CHAR UPLO, INT N, REALS A, INT LDA, INT INFO);
extern "C" void spotrs_ (
        CHAR UPLO, INT N, INT NRHS, REALS A, INT LDA,
        REALS B, INT LDB, INT INFO);
extern "C" void ssytrd_ (
        CHAR UPLO, INT N, REALS A, INT LDA, REALS D,
        REALS E, REALS TAU, REALS WORK, INT LWORK, INT INFO);
extern "C" void sstegr_ (
        CHAR JOBZ, CHAR RANGE, INT N, REALS D, REALS E,
        REAL VL, REAL VU, INT IL, INT IU, REAL ABSTOL, INT M,
        REALS W, REALS Z, INT LDZ, INTS ISUPPZ,
        REAL WORK, INT LWORK, INTS IWORK, INT LIWORK, INT INFO);
extern "C" void sstevx_ (
        CHAR JOBZ, CHAR RANGE, INT N, REALS D, REALS E, REAL VL, REAL VU,
        INT IL, INT IU, REAL ABSTOL, INT M, REALS W, REALS Z, INT LDZ,
        REALS WORK, INTS IWORK, INTS IFAIL, INT INFO);

#else

extern "C" void dpotrf_ (CHAR UPLO, INT N, REALS A, INT LDA, INT INFO);
extern "C" void dpotri_ (CHAR UPLO, INT N, REALS A, INT LDA, INT INFO);
extern "C" void dpotrs_ (
        CHAR UPLO, INT N, INT NRHS, REALS A, INT LDA,
        REALS B, INT LDB, INT INFO);
extern "C" void dsytrd_ (
        CHAR UPLO, INT N, REALS A, INT LDA, REALS D,
        REALS E, REALS TAU, REALS WORK, INT LWORK, INT INFO);
extern "C" void dstegr_ (
        CHAR JOBZ, CHAR RANGE, INT N, REALS D, REALS E,
        REAL VL, REAL VU, INT IL, INT IU, REAL ABSTOL, INT M,
        REALS W, REALS Z, INT LDZ, INTS ISUPPZ,
        REAL WORK, INT LWORK, INTS IWORK, INT LIWORK, INT INFO);
extern "C" void dstevx_ (
        CHAR JOBZ, CHAR RANGE, INT N, REALS D, REALS E, REAL VL, REAL VU,
        INT IL, INT IU, REAL ABSTOL, INT M, REALS W, REALS Z, INT LDZ,
        REALS WORK, INTS IWORK, INTS IFAIL, INT INFO);
#define spotrf_ dpotrf_
#define spotri_ dpotri_
#define spotrs_ dpotrs_
#define ssytrd_ dsytrd_
#define sstegr_ dstegr_
#define sstevx_ dstevx_

#endif

namespace LinAlg
{

void Tensor::clear () { std::memset(m_data, 0, m_size * sizeof(Float)); }

//copying
Tensor& Tensor::operator= (const Tensor& other)
{
    Assert1(shape() == other.shape(), "copied tensors of different shape");
    std::memcpy(m_data, other.m_data, m_size * sizeof(Float));
    return *this;
}
Tensor& Tensor::operator= (const std::vector<Float>& src)
{
    Assert1(m_size == src.size(), "copied from improperly-sized source");
    for (Int i=0; i<m_size; ++i) {
        m_data[i] = src[i];
    }
    return *this;
}
Tensor& Tensor::operator= (const Float* data)
{
    std::memcpy(m_data, data, m_size * sizeof(Float));
    return *this;
}
Tensor& Tensor::operator= (Float constant)
{
    for (Int i=0; i<m_size; ++i) {
        m_data[i] = constant;
    }
    return *this;
}
Tensor& Tensor::operator+= (const Tensor& other)
{
    Assert1(shape() == other.shape(), "added tensors of different shape");
    for (Int i=0; i<m_size; ++i) {
        m_data[i] += other.m_data[i];
    }
    return *this;
}

//tensor algebra
Tensor& Tensor::operator-= (const Tensor& other)
{
    Assert1(shape() == other.shape(), "added tensors of different shape");
    for (Int i=0; i<m_size; ++i) {
        m_data[i] -= other.m_data[i];
    }
    return *this;
}
Tensor& Tensor::operator+= (Float shift)
{
    for (Int i=0; i<m_size; ++i) {
        m_data[i] += shift;
    }
    return *this;
}
Tensor& Tensor::operator*= (Float scalar)
{
    for (Int i=0; i<m_size; ++i) {
        m_data[i] *= scalar;
    }
    return *this;
}

Float Tensor::total () const
{
    Float result = 0;
    for (Int i=0; i<m_size; ++i) {
        result += m_data[i];
    }
    return result;
}
Float Tensor::min () const
{
    Float result = INFINITY;
    for (Int i=0; i<m_size; ++i) {
        if (m_data[i] < result) result = m_data[i];
    }
    return result;
}
Float Tensor::max () const
{
    Float result = -INFINITY;
    for (Int i=0; i<m_size; ++i) {
        if (m_data[i] > result) result = m_data[i];
    }
    return result;
}
Float Tensor::norm2 () const
{
    Float result = 0;
    for (Int i=0; i<m_size; ++i) {
        result += sqr(m_data[i]);
    }
    return result;
}

Tensor identity (Int dim)
{
    Tensor result(dim, 2);
    for (Int i=0; i<dim; ++i) {
        result(i,i) = 1.0f;
    }
    return result;
}
Tensor diagonal (const Tensor& t)
{
    Assert1(t.rank() == 1, "construced diagonal from non-vector");
    Tensor result(t.dim(), 2);
    for (Int i=0; i<t.dim(); ++i) {
        result(i,i) = t(i);
    }
    return result;
}

Float dot (const Tensor& t1, const Tensor& t2)
{
    Assert1(t1.shape() == t2.shape(), "tensors have different shapes");
    Float result = 0;
    for (Int i=0; i<t1.size(); ++i) {
        result += t1(i) * t2(i);
    }
    return result;
}
Float dot (const Tensor& t1, const Tensor& M, const Tensor& t2)
{
    Int dim = M.dim();
    Assert1(t1.dim() == dim, "t1 has wrong dimension");
    Assert1(t2.dim() == dim, "t2 has wrong dimension");
    Assert1(t1.rank() == 1, "t1 has wrong rank");
    Assert1(t2.rank() == 1, "t2 has wrong rank");
    Assert1(M.rank() == 2, "M has wrong rank");

    Float result = 0;
    for (Int i=0; i<dim; ++i) {
    for (Int j=0; j<dim; ++j) {
        result += t1(i) * M(i,j) * t2(j);
    } }
    return result;
}
Float Tensor::norm2 (const Tensor& M) const
{
    Assert1(M.dim() == m_dim, "M has wrong dimension");
    Assert1(M.rank() == 2, "M has wrong rank");
    Assert1(m_rank == 1, "self has wrong rank");

    Float result = 0;
    for (Int i=0; i<m_size; ++i) {
    for (Int j=0; j<m_size; ++j) {
        result += M(i,j) * m_data[i] * m_data[j];
    } }
    return result;
}
Float dist2 (const Tensor& t1, const Tensor& t2)
{
    Assert1(t1.shape() == t2.shape(), "tensors have different shapes");
    Float result = 0;
    for (Int i=0; i<t1.size(); ++i) {
        result += sqr(t1(i) - t1(i));
    }
    return result;
}
Tensor exp (const Tensor& t)
{//matrix exponential by taylor series
    Assert1(t.rank() == 2, "exp'd non-square tensor");
    Int dim = t.dim();
    Tensor result = identity(dim);
    Tensor power = t;
    result += power;
    for (Int n=2; n<20; ++n) {
        power = power.multiply(t);
        power /= n;
        result += power;
        if (power.norm() < 1e-10) break;
    }
    return result;
}
Tensor Tensor::transpose () const
{
    Assert1(m_rank == 2, "transposed a non-square tensor");
    Tensor result (m_dim, m_rank);
    for (Int i=0; i<m_dim; ++i) {
    for (Int j=0; j<m_dim; ++j) {
        result(i,j) = (*this)(j,i);
    } }
    return result;
}

//linear algebra
Tensor Tensor::multiply (const Tensor& x) const
{//matrix-vector and matrix-matrix multiplication
    Assert1(m_rank == 2, "A has wrong rank " << m_rank);
    Assert1(m_dim == x.dim(), "A and b have different dimensions");

    const Tensor &A = *this;
    Tensor result(x.dim(), x.rank());
    switch (x.rank()) {
        case 1: //matrix-vector
            for (Int i=0; i<m_dim; ++i) {
            for (Int j=0; j<m_dim; ++j) {
                result(i) += A(i,j) * x(j);
            } }
            break;

        case 2: //matrix-matrix
            for (Int i=0; i<m_dim; ++i) {
            for (Int j=0; j<m_dim; ++j) {
            for (Int k=0; k<m_dim; ++k) {
                result(i,k) += A(i,j) * x(j,k);
            } } }
            break;

        default: Error("multiplied tensor of wrong rank: " << x.rank());
    }
    return result;
}
Tensor Tensor::cholesky () const
{//cholesky decomposition of symmetric pos-def matrix
    Assert (m_rank == 2, "inverted tensor of rank != 2");
    Tensor result = *this;

    //Cholesky decomposition
    int dim = m_dim;
    int uplo = 'U'; //this is lower-diag in C indexing
    int leading = m_dim;
    int info;
    spotrf_ (&uplo, &dim, result.data(), &leading, &info);
    if (info) {
        if (info > 0) {
            Tensor eigs = eigenvals();
            logger.error() << "matrix not pos def: eigs = " << eigs |0;
        }
        Error ("spotrf_ error, info = " << info );
    }

    //zero-out lower-diagonal
    for (Int i=0; i<m_dim; ++i) {
    for (Int j=0; j<i; ++j) {
        result(j,i) = 0;
    } }

    return result;
}
Tensor Tensor::solve (const Tensor& b) const
{
    Assert (m_rank == 2, "A has wrong rank " << m_rank);
    Assert (b.rank() == 1, "b has wrong rank " << b.rank());
    Assert (m_dim == b.dim(), "A and b have different dimensions");

    Tensor chol_A = cholesky();
    Tensor x = b;

    //linear solver from cholesky
    int dim = m_dim;
    int uplo = 'U'; //this is lower-diag in C indexing
    int nrhs = 1;
    int leading = m_dim;
    int info;
    spotrs_ (&uplo, &dim, &nrhs, chol_A.data(), &leading,
                                 x.data(),      &leading, &info);
    Assert (info==0, "spotrs_ error, info = " << info);

    return x;
}
Tensor Tensor::inverse () const
{//inverse of symmetric positive-definite matrix
    Assert (m_rank == 2, "inverted tensor of rank != 2");
    Tensor result = cholesky();

    //inverse from Cholesky
    int dim = m_dim;
    int uplo = 'U'; //this is lower-diag in C indexing
    int leading = m_dim;
    int info;
    spotri_ (&uplo, &dim, result.data(), &leading, &info);
    Assert (info==0, "spotri_ error, info = " << info);

    //symmetrize result
    for (Int i=0; i<m_dim; ++i) {
    for (Int j=0; j<i; ++j) {
        result(j,i) = result(i,j);
    } }

    return result;
}
Tensor Tensor::eigenvals () const
{//eigenvalues of symmetric matrix
    Assert (m_rank == 2, "got eigenvals of tensor of rank != 2");
    Tensor eigs(m_dim); //W

    //rotate matrix to tridiagonal form
    int dim = m_dim;
    Tensor diag(dim), off_diag(dim-1);
    {
        Tensor tri = *this; //input: matrix, output: elem. refl. representation
        Tensor tau(dim-1);  //output: scalar factors of elementary reflectors
        int uplo = 'U';     //this is lower-diag in C indexing
        int lda = m_dim;
        int lwork = -1;     //query on first call
        Float lworkf;       //for querying only
        int info;

        //query for work size
        ssytrd_(&uplo, &dim, NULL, &lda, NULL, NULL,
                 NULL, &lworkf, &lwork, &info);
        lwork = static_cast<int>(lworkf);
        Tensor work(lwork);

        ssytrd_(&uplo, &dim, tri.data(), &lda, diag.data(), off_diag.data(),
                 tau.data(), work.data(), &lwork, &info);
        Assert (info==0, "ssytrd_ error, info = " << info);
    }

    //compute eigenvalues
    {
        int jobz = 'N';     //eigenvalues only
        int range = 'A';    //find all eigenvalues
        int il, iu;         //not used
        Float vl, vu;       //not used
        Float abstol;       //not used
        int m;              //num_eigs = m_dim, not used
        Tensor z(dim,2);    //eigenvects, not used
        int ldz = dim;      //not used
        int* isuppz = NULL; //not used
        int lwork = -1;     //query on first call
        Float lworkf;       //for querying only
        int liwork = -1;
        int info;

        //query for work size
        sstegr_(&jobz, &range, &dim, diag.data(), off_diag.data(),
                &vl, &vu, &il, &iu, &abstol, &m, eigs.data(),
                z.data(), &ldz, isuppz,
                &lworkf, &lwork, &liwork, &liwork, &info);
        lwork = static_cast<int>(lworkf);
        Tensor work(lwork);
        int* iwork = new int[liwork];

        sstegr_(&jobz, &range, &dim, diag.data(), off_diag.data(),
                &vl, &vu, &il, &iu, &abstol, &m, eigs.data(),
                z.data(), &ldz, isuppz,
                work.data(), &lwork, iwork, &liwork, &info);
        Assert (info==0, "sstegr_ error, info = " << info);
        delete[] iwork;
    }
    return eigs;
}
std::vector<Eig> Tensor::eigenvects (Int num) const
{//largest few eigenvectors and eigenvalues
    TODO();
    /*
    Assert (m_rank == 2, "got eigenvals of tensor of rank != 2");

    if (num == 0) num = m_dim;

    //rotate matrix to tridiagonal form
    int dim = m_dim;
    Tensor diag(dim), off_diag(dim-1); //D,E
    Tensor refl = *this;    //input: matrix, output: elem. refl. representation
    Tensor tau(dim-1);      //scalar factors of elementary reflectors
    {
        int uplo = 'U';     //this is lower-diag in C indexing
        int lda = m_dim;
        int lwork = -1;     //query on first call
        Float lworkf;       //for querying only
        int info;

        //query for work size
        ssytrd_(&uplo, &dim, NULL, &lda, NULL, NULL,
                NULL, &lworkf, &lwork, &info);
        lwork = static_cast<int>(lworkf);
        Tensor work(lwork);

        ssytrd_(&uplo, &dim, tri.data(), &lda, diag.data(), off_diag.data(),
                refl.data(), work.data(), &lwork, &info);
        Assert (info==0, "ssytrd_ error, info = " << info);
    }

    //compute eigenvectors
    Float* evals  = new Float[num];     //W
    Float* evects = new Float[dim*num]; //Z
    {
        int jobz = 'V';         //eigenvalues and eigenvectors
        int range = 'I';        //first few
        int il = 1, iu = num;   //range of eigenvalues
        Float vl, vu;           //not used
        Float abstol = 0;       //eigenvector tolerance, default
        int m;                  //num_eigs = dim, not used
        int ldz = dim;          //data dimension
        Float* work = new Float[5*dim];
        int* iwork = new int[5*dim];
        int* ifail = new int[dim];
        int info;

        sstevx_(&jobz, &range, &dim, diag.data(), off_diag.data(),
                &vl, &vu, &il, &iu, &abstol, &m, evals, evects, &ldz,
                work, iwork, ifail, &info);
        Assert (info==0, "sstevx_ error, info = " << info);
        delete[] work;
        delete[] iwork;
        delete[] ifail;
    }

    //transform and repack eigenvectors
    std::vector<Eig> result;
    {
        char side = 'L';        //for column vectors
        char uplo = 'U';        //matching ssytrd_
        char trans = 'T';       //correct?
        int m = num;
        int n = dim;
        Float* a = tri.data();  //from ssytrd_
        int lda = dim;
        Float* c = evects;      //input and output
        int ldc = dim;
        int lwork = -1;         //query on first call
        Float lworkf;           //for querying only

        //query for work size
        sormtr_(&side, &uplo, &trans, &m, &n, a, &lda, tau, c, &ldc,
                &lworkf, &lwork, info);
        Float* work = new Float[lwork];

        sormtr_(&side, &uplo, &trans, &m, &n, a, &lda, tau, c, &ldc,
                work, &lwork, info);
        Assert (info==0, "sormtr_ error, info = " << info);
        delete[] work;

        for (Int i=0; i<num; ++i) {
            result.push_back(Eig(dim, evals[i], evects + dim*i));
        }

        delete[] evals;
        delete[] evects;
    }

    return result;
    */
}

//random
inline Float random_unif () { return drand48(); }
Float random_normal ()
{//box-muller
    static bool available = true;
    available = not available;

    static Float y = 0.0f;
    if (available) return y;

    Float theta = 2.0f * M_PI * random_unif();
    Float r = sqrtf(-2.0f * logf(1.0f - random_unif()));
    Float x = r * cosf(theta);
    y = r * sinf(theta);
    return x;
}
void Tensor::randomize ()
{
    for (Int i=0; i<m_size; ++i) data(i) = random_normal();
}
Tensor random (Int dim, Int rank)
{
    Tensor result(dim, rank);
    result.randomize();
    return result;
}
Tensor random_asym (Int dim)
{
    Tensor result(dim, 2);
    for (Int i=0; i<dim; ++i) {
    for (Int j=0; j<i; ++j) {
        Float t = M_SQRT2 * random_normal();
        result(i,j) = t;
        result(j,i) = -t;
    } }
    return result;
}

//output
bool Tensor::save_to_file (string filename) const
{//returns true on error
    logger.info() << "Writing Tensor to file " << filename << " ..."|0;
    Logging::IndentBlock block;
    FILE* file = fopen(filename.c_str(), "wb");
    if (!file) {
        logger.error() << "failed to open file " << filename |0;
        return true;
    }
    safe_fwrite(m_data, sizeof(Float), m_size, file);
    fclose(file);
    logger.info() << "...done" |0;
    return false;
}

}

ostream& operator<< (ostream& os, const LinAlg::Tensor& t)
{
    if (t.size() == 0 or t.rank() == 0) return os << "[]";

    //print vectors as rows
    if (t.rank() == 1) {
        os << "[" << std::setw(10) << t(0);
        for (Int i=1; i<t.dim(); ++i) {
            os << ", " << std::setw(10) << t(i);
        }
        return os << "]";
    }

    //otherwise print list of slices
    LinAlg::Tensor s(t.dim(), t.rank()-1, t[0]);
    os << "[\n\t" << s;
    for (Int i=1; i<t.dim(); ++i) {
        s.set(t[i]);
        os <<",\n\t" << s;
    }
    return os << "\n]";
}

