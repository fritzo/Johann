
#include "definitions.h"
#include "expressions.h"
#include "unification.h"

Logging::Logger logger("test");

namespace EX = Expressions;
namespace U = Unification;

bool unify (string lhs, string rhs)
{

    //parse inputs
    ExprHdl e1 = EX::parse(lhs);
    Assert (e1 and not e1->isBad(), "parse error on lhs: " << EX::parse_errors);
    ExprHdl e2 = EX::parse(rhs);
    Assert (e2 and not e2->isBad(), "parse error on rhs: " << EX::parse_errors);

    logger.info() << "unifying " << e1 << " = " << e2 |0;

    //convert to combinators
    e1 = e1->as_comb();
    Assert (not e1->isBad(), "lhs could not be converted to a combinator");
    e2 = e2->as_comb();
    Assert (not e2->isBad(), "rhs could not be converted to a combinator");

    //try to unify
    if (ExprHdl result = U::unify(e1,e2)) {
        //cout << result << endl;
        logger.info() << "solution: " << result |0;
        return true;
    } else {
        //cout << "no solution found" << endl;
        logger.info() << "no solution found" |0;
        return false;
    }
}
void test_unify ()
{
    logger.info() << "Testing unifiy(-,-)" |0;
    Logging::IndentBlock block;

	Assert (unify("(a,b,c,d,e,f,g)", "(b b,c c,d d,e e,f f,g g,K K)"),
            "unification failed but should have succeded");
	Assert (not unify("(a,b,c,d,e,f,g)", "(b b,c c,d d,e e,f f,g g,K a)"),
            "unification succeded but should have failed");
	Assert (not unify("(a,b,c,d,e,f,g,g)", "(b b,c c,d d,e e,f f,g g,K,S)"),
            "unification succeded but should have failed");
}

int main ()
{
    Logging::switch_to_log("test.log");
    Logging::title("Running Unification Test");

    EX::initialize("Bot Top I K F B C W S Y J V P");

    test_unify();

    EX::clear();

    return 0;
}

