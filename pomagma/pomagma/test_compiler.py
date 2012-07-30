from pomagma.compiler import Variable, Function, Sequent, APP, EQUAL, LESS


def _test_sequent(sequent):
    print 'Compiling: {}'.format(sequent)
    for part in sequent.compile():
        print len(part), 'versions:'
        for version in part:
            print '{}'.format(version)


def test_compile_K():
    K = Function('K')
    x = Variable('x')
    y = Variable('y')
    _test_sequent(Sequent(
        [],
        [EQUAL(APP(APP(K, x), y), x)]))


def test_compile_S():
    S = Function('S')
    x = Variable('x')
    y = Variable('y')
    z = Variable('z')
    _test_sequent(Sequent(
        [],
        [EQUAL(APP(APP(APP(S, x), y), z), APP(APP(x, y), APP(y, z)))]))

def test_compile_Y():
    Y = Function('Y')
    f = Variable('f')
    _test_sequent(Sequent(
        [],
        [EQUAL(APP(Y, f), APP(f, APP(Y, f)))]))

def test_compile_mu():
    f = Variable('f')
    x = Variable('x')
    y = Variable('y')
    _test_sequent(Sequent(
        [LESS(x, y)],
        [LESS(APP(f, x), APP(f, y))]))
