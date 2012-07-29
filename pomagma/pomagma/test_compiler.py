from pomagma.compiler import Constant, Variable, Sequent, EQUAL


def test_compile_ranked():
    S = Constant('S')
    APP = Constant('APP', 'binary')
    x = Variable('x')
    y = Variable('y')
    z = Variable('z')
    sequent = Sequent(
        [],
        [EQUAL(APP(APP(APP(S, x), y), z), APP(APP(x, y), APP(y, z)))])
    print 'Compiling: {}'.format(sequent)
    ranked = sequent.compile_ranked()
    for compiled in ranked:
        print '{}'.format(compiled)
