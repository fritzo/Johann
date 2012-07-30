from pomagma.compiler import Constant, Variable, Sequent, EQUAL, LESS


def _test_sequent(sequent):
    print 'Compiling: {}'.format(sequent)
    ranked = sequent.compile_ranked()
    for compiled in ranked:
        print '{}'.format(compiled)


def test_compile_S():
    APP = Constant('APP', 'binary')
    S = Constant('S')
    x = Variable('x')
    y = Variable('y')
    z = Variable('z')
    _test_sequent(Sequent(
        [],
        [EQUAL(APP(APP(APP(S(), x), y), z), APP(APP(x, y), APP(y, z)))]))

def test_compile_Y():
    APP = Constant('APP', 'binary')
    Y = Constant('Y')
    f = Variable('f')
    _test_sequent(Sequent(
        [],
        [EQUAL(APP(Y(), f), APP(f, APP(Y(), f)))]))

def test_compile_mu():
    APP = Constant('APP', 'binary')
    f = Variable('f')
    x = Variable('x')
    y = Variable('y')
    _test_sequent(Sequent(
        [LESS(x, y)],
        [LESS(APP(f, x), APP(f, y))]))
