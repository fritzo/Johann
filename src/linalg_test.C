
#include "definitions.h"
#include "linalg.h"

namespace LA = LinAlg;

namespace Testing
{

Logging::Logger logger("test");

void test_solve ()
{
    logger.info () << "Testing linear equation solver" |0;
    Logging::IndentBlock block;

    Float M_data[4*4] = {
        1, 0, 2, 0,
        0, 2, 1, 0,
        2, 1, 9, 1,
        0, 0, 1, 4
    };
    LA::Tensor M(4,2,M_data);
    logger.debug () << "matrix: M =\n" << M |0;

    Float x_data[4] = {-1,2,-3,4};
    LA::Tensor x(4,1,x_data);
    logger.debug() << "vector: x = " << x |0;

    LA::Tensor b = M.multiply(x);
    logger.debug() << "under M: M x = " << b |0;

    LA::Tensor y = M.solve(b);
    logger.debug() << "under M: M\\(M x) = " << y |0;

    Float error = LA::dist(x, y);
    logger.info() << "error = " << error |0;
    AssertW(error < 1e-6, "error = " << error);
}

void test_inverse ()
{
    logger.info () << "Testing matrix inverse" |0;
    Logging::IndentBlock block;

    Float M_data[4*4] = {
        1, 0, 0, 0,
        0, 2, 0, 0,
        0, 0, 4, 1,
        0, 0, 1, 4
    };
    LA::Tensor M(4,2,M_data);
    logger.debug () << "original matrix: M =\n" << M |0;

    LA::Tensor Mc = M.cholesky();
    logger.debug () << "Cholesky: Mc =\n" << Mc |0;

    LA::Tensor Mi = M.inverse();
    logger.debug () << "inverse: Mi =\n" << Mi |0;

    LA::Tensor Mii = Mi.inverse();
    logger.debug () << "inverse inverse: Mii =\n" << Mii |0;

    Float error = LA::dist(Mii, Mi);
    logger.info() << "error = " << error |0;
    AssertW(error < 1e-6, "error = " << error);
}

void test_eigenvals (Int dim=6)
{
    logger.info () << "Testing eigenvalues" |0;
    Logging::IndentBlock block;

    //construct random eivenvalues
    LA::Tensor eigs = LA::random(dim);
    logger.info() << "eigs = " << eigs |0;

    //construct randomly rotated matrix with given eigenvalues
    LA::Tensor A = LA::diagonal(eigs);
    LA::Tensor asym = LA::random_asym(dim);
    LA::Tensor rotate = LA::exp(asym);
    A = A.multiply(rotate);
    A = rotate.transpose().multiply(A);
    logger.info() << "A = " << A |0;

    //find eigenvalues
    LA::Tensor eigs2 = A.eigenvals();
    logger.info() << "eigs2 = " << eigs2 |0;

    //AssertW(LA::dist(eigs, eigs2) < 1e-8, "eigenvales do not match");
}

void test_eigenvects (Int dim=6, Int num=3)
{
    logger.info () << "Testing eigenvects" |0;
    Logging::IndentBlock block;

    //construct random eivenvalues
    LA::Tensor eigs = LA::random(dim);
    logger.info() << "eigs = " << eigs |0;

    //construct randomly rotated matrix with given eigenvalues
    LA::Tensor A = LA::diagonal(eigs);
    LA::Tensor asym = LA::random_asym(dim);
    LA::Tensor rotate = LA::exp(asym);
    A = A.multiply(rotate);
    A = rotate.transpose().multiply(A);
    logger.info() << "A = " << A |0;

    //find eigenvalues
    std::vector<LA::Eig> eigs2 = A.eigenvects(num);
    const Logging::fake_ostream& log = logger.info() << "eigs2:";
    for (Int i=0; i<eigs2.size(); ++i) {
        log << "\n" << eigs2[i].val << ":\t " << eigs2[i].vect;
    }
    log |0;
}

void test_random (Int N = 10000)
{//passes with high probability
    double mu = 0,  mu2 = 0;
    for (Int i=0; i<N; ++i) {
        Float t = LA::random_normal();
        mu += t;
        mu2 += sqr(t);
    }
    mu /= N;
    mu2 /= N;
    double V = mu2 - sqr(mu);

    AssertW(fabs(mu) < 3.0f / sqrt(N), "bad mean: mu = " << mu);
    AssertW(fabs(V-1) < 3.0f / sqrt(N), "bad variance: V = " << V);
}

}

int main ()
{
    using namespace Testing;

    Logging::switch_to_log("test.log");
    Logging::title("Running Linear Algebra Test");

    test_solve();
    test_inverse();
    test_eigenvals();
    //test_eigenvects();

    test_random();

    return 0;
}

