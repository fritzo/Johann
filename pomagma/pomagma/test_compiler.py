import re
from pomagma.compiler import (
    Variable,
    Function,
    Sequent,
    EQUAL,
    LESS,
    NLESS,
    add_costs,
    )

CO = lambda x: Function('CO', x)
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
    compiles = sequent.compile()
    print_compiles(compiles)
    full_cost = add_costs(*[cost for cost, _ in compiles])

    incremental_cost = None
    for event in sequent.get_events():
        print 'Compiling incremental search given: {}'.format(event)
        compiles = sequent.compile_given(event)
        print_compiles(compiles)
        if event.children:
            cost = add_costs(*[cost for cost, _ in compiles])
            if incremental_cost:
                incremental_cost = add_costs(incremental_cost, cost)
            else:
                incremental_cost = cost

    print '# full cost =', full_cost, 'incremental cost =', incremental_cost

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

def test_compile_B_app():
    B = Function('B')
    _test_sequent(
        [],
        [EQUAL(APP(APP(APP(B, x), y), z), APP(x, APP(y, z)))])

def test_compile_B_comp():
    B = Function('B')
    _test_sequent(
        [],
        [EQUAL(APP(APP(B, x), y), COMP(x, y))])

def test_compile_app_comp():
    _test_sequent(
        [],
        [EQUAL(APP(COMP(x, y), z), APP(x, APP(y, z)))])

def test_compile_comp_assoc():
    _test_sequent(
        [],
        [EQUAL(COMP(COMP(x, y), z), COMP(x, COMP(y, z)))])

def test_compile_C():
    C = Function('C')
    _test_sequent(
        [],
        [EQUAL(APP(APP(APP(C, x), y), z), APP(APP(x, z), y))])

def test_compile_S():
    # Compiling incremental search given: APP APP_x_y APP_y_z
    # cost = 2.70067540518
    # if S
    # for x let APP_S_x
    # for y if APP x y let APP_APP_S_x_y
    # for z if APP y z let APP_APP_APP_S_x_y_z
    # ensure EQUAL APP_APP_APP_S_x_y_z APP_APP_x_y_APP_y_z
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

def test_compile_co():
    # TODO use inverse iteration for unary_function::inverse_iterator
    # Compiling incremental search given: APP CO_x y
    # cost = 0.926425846749
    # for x if CO x let APP_y_x
    # ensure LESS APP_CO_x_y APP_y_x
    _test_sequent(
        [],
        [LESS(APP(CO(x), y), APP(y, x))])

def test_compile_eval():
    EVAL = Function('EVAL')
    _test_sequent(
        [],
        [EQUAL(APP(EVAL, QUOTE(x)), x)])

def test_compile_qt_quote():
    QT = Function('QT')
    _test_sequent(
        [],
        [EQUAL(APP(QT, QUOTE(x)), QUOTE(QUOTE(x)))])

def test_compile_ap_quote():
    AP = Function('AP')
    _test_sequent(
        [],
        [EQUAL(APP(APP(AP, QUOTE(x)), QUOTE(y)), QUOTE(APP(x, y)))])
