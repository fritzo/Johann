import re
from pomagma.compiler import Variable, Function, Sequent, EQUAL, LESS, NLESS

APP = lambda x, y: Function('APP', x, y)
COMP = lambda x, y: Function('COMP', x, y)
JOIN = lambda x, y: Function('JOIN', x, y)
QUOTE = lambda x: Function('QUOTE', x)
f = Variable('f')
g = Variable('g')
x = Variable('x')
y = Variable('y')
z = Variable('z')

def print_compiles(compiles):
    for cost, strategy in compiles:
        print '# cost = {}'.format(cost)
        print re.sub(': ', '\n', repr(strategy))
        print

def _test_sequent(*args):
    sequent = Sequent(*args)
    print '-' * 78
    print 'Compiling full search: {}'.format(sequent)
    print_compiles(sequent.compile())

    for event in sequent.get_events():
        print 'Compiling incremental search given: {}'.format(event)
        print_compiles(sequent.compile_given(event))

def test_compile_I():
    I = Function('I')
    _test_sequent(
        [],
        [EQUAL(APP(I, x), x)])

def test_compile_K():
    K = Function('K')
    _test_sequent(
        [],
        [EQUAL(APP(APP(K, x), y), x)])


def test_compile_W():
    W = Function('W')
    _test_sequent(
        [],
        [EQUAL(APP(APP(W, x), y), APP(APP(x, y), y))])


def test_compile_B():
    B = Function('B')
    _test_sequent(
        [],
        [EQUAL(APP(APP(APP(B, x), y), z), APP(x, APP(y, z)))])


def test_compile_C():
    C = Function('C')
    _test_sequent(
        [],
        [EQUAL(APP(APP(APP(C, x), y), z), APP(APP(x, z), y))])


def test_compile_S():
    S = Function('S')
    _test_sequent(
        [],
        [EQUAL(APP(APP(APP(S, x), y), z), APP(APP(x, y), APP(y, z)))])

def test_compile_Y():
    Y = Function('Y')
    _test_sequent(
        [],
        [EQUAL(APP(Y, f), APP(f, APP(Y, f)))])

def test_compile_mono():
    _test_sequent(
        [LESS(x, y), LESS(f, g)],
        [LESS(APP(f, x), APP(g, y))])

def test_compile_mu():
    _test_sequent(
        [LESS(x, y)],
        [LESS(APP(f, x), APP(f, y))])

def test_compile_join_idem():
    _test_sequent(
        [],
        [EQUAL(JOIN(x, x), x)])

def test_compile_join_less():
    _test_sequent(
        [],
        [LESS(x, JOIN(x, y))])

def test_compile_quote():
    EVAL = Function('EVAL')
    _test_sequent(
        [],
        [EQUAL(APP(EVAL, QUOTE(x)), x)])

def test_compile_quote():
    AP = Function('AP')
    _test_sequent(
        [],
        [EQUAL(APP(APP(AP, QUOTE(x)), QUOTE(y)), QUOTE(APP(x, y)))])
