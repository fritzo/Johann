
#include "measures.h"
#include "random_choice.h"
//#define _ISOC99_SOURCE //is this necessary?
#include <cmath>

namespace M = Measures;
namespace RC = RandomChoice;

Logging::Logger logger("test");

//node definitions for heap
namespace Nodes
{
class PosSignature_
{
public:
    typedef PosSignature_ MySig;
    enum { size_in_bytes = 64, is_used_field = 15 };
    typedef Heap_::Node_   <MySig> Node;
    typedef Heap_::Heap_   <MySig> Heap;
    typedef Heap_::Pos_    <MySig> Pos;
    typedef Heap_::Name_   <MySig> Name;
    typedef Heap_::Handle_ <MySig> Handle;
};
}

typedef Nodes::PosSignature_::Pos Pos;

Pos::Heap Pos::s_heap;

const Heap::TypedIndex<float>
    LENGTH(3);

const M::Measure<Pos, float>
    complexity(0),
    relevance(1), irrelevance(2);
const M::Measure<Pos, Int>
    create(4), create_l(5), create_r(6),
    prune(7), prune_l(8), prune_r(9);

double Z, H;

//testing utilities
void define_relevance (double lambda = 1e-1f,
                       int num_singular=100,
                       int num_permanent=100)
{
    logger.info() << "Defining relevance measure" |0;
    Logging::IndentBlock block;

    double C = 0;
    Z = 0;
    int count = Pos::size() - num_singular ; //number nonsingular
    Pos::iterator iter=Pos::begin();
    for (; count; ++iter, --count) {
        Pos pos = *iter;
        double length = lambda * log(double(Int(pos)));
        pos(LENGTH) = length;
        double mass = exp(-length);
        complexity(pos) = mass;
        relevance(pos) = mass;
        Z += mass;
        C += mass * length;
    }

    H = C/Z + log(Z);
    logger.info() << "total mass = " << Z |0;
    logger.info() << "entropy = " << H |0;

    logger.info() << "defining " << num_singular << " singular nodes" |0;
    for (; iter!=Pos::end(); ++iter) {
        Pos pos = *iter;
        pos(LENGTH) = INFINITY;
        complexity(pos) = 0.0f;
        relevance(pos) = 0.0f;
    }

    logger.info() << "declaring " << num_permanent << " permanent nodes" |0;
    count = num_permanent;
    for (iter=Pos::begin(); count; ++iter, --count) {
        const Pos pos = *iter;
        relevance(pos) = INFINITY;
    }
}
void measure_test (int num_choices = 1<<14)
{
    logger.info() << "Testing measure calculations" |0;
    Logging::IndentBlock block;

    logger.info() << "defining creation measure" |0;
    RC::Generator<Pos> creation(complexity, create, prune_l);

    logger.info() << "initializing creation measure" |0;
    creation.update_all();
    creation.validate(create_r);

    logger.info() << "making " << num_choices << " random choices"|0;
    double C = 0; //empirical versions
    int N = 0;
    for (int i=0; i<num_choices; ++i) {
        Pos pos = creation.choose();
        Assert (complexity(pos) > 0.0f, "creation: massless node chosen");
        float _length = pos(LENGTH);
        if (!isinf(_length)) {
            C += _length;
            ++N;
        }
    }

    double H_emp = C/N + log(Z);
    logger.info() << "empirical entropy = " << H_emp |0;
}
void inverse_measure_test (int num_choices = 1<<14)
{
    logger.info() << "Testing inverse measure calculations" |0;
    Logging::IndentBlock block;

    logger.info() << "defining pruning measure" |0;
    RC::InverseGenerator<Pos> pruning(relevance, irrelevance, prune, prune_l);

    logger.info() << "initializing pruning measure" |0;
    pruning.update_all();
    pruning.validate(prune_r);

    logger.info() << "making " << num_choices << " random choices"|0;
    for (int i=0; i<num_choices; ++i) {
        if (pruning.empty()) {
            logger.info() << "ran out of choices" |0;
            break;
        }
        Pos pos = pruning.choose();
        Assert (!isinf(relevance(pos)), "permanent node pruned");
        pruning.remove(pos);
    }
}

int main ()
{
    Logging::switch_to_log("test.log");
    Logging::title("Running Random Choice Test");

    //set up heap for testing
    Pos::init((1<<13) - 1);
    while (!Pos::full()) Pos::alloc();
    define_relevance();

    measure_test();
    inverse_measure_test();

    return 0;
}

