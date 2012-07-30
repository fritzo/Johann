import re
import itertools
from pomagma.util import TODO, union, set_with, set_without

#-----------------------------------------------------------------------------
# Syntax

class Expression:
    def __init__(self, _repr):
        self._repr = _repr
        self._hash = hash(_repr)

    def __repr__(self):
        return self._repr

    def __hash__(self):
        return self._hash

    def __eq__(self, other):
        return self._repr == other._repr


class Variable(Expression):
    def __init__(self, name):
        if isinstance(name, str):
            assert not re.match('[A-Z]', name[-1])
        else:
            name = re.sub('[(), ]+', '_', repr(name))
        self.name = name
        Expression.__init__(self, name)

    def get_vars(self):
        return set([self])

    def get_constants(self):
        return set()

    def get_tests(self):
        return set()


class Compound(Expression):
    def __init__(self, name, *children):
        assert re.match('[A-Z]', name[-1])
        self.name = name
        self.children = list(children)
        Expression.__init__(self, ' '.join([name] + map(repr, children)))

    def get_vars(self):
        return union([child.get_vars() for child in self.children])

    def get_constants(self):
        if self.children:
            return union([c.get_constants() for c in self.children])
        else:
            assert isinstance(self, Function)
            return set([self])

    def get_atom(self):
        return self.__class__(self.name, *map(Variable, self.children))

    def get_tests(self):
        result = set([self.get_atom()])
        for c in self.children:
            result |= c.get_tests()
        return result


class Function(Compound):
    pass


class Relation(Compound):
    pass


APP = lambda x, y: Function('APP', x, y)
EQUAL = lambda x, y: Relation('EQUAL', x, y)
LESS = lambda x, y: Relation('LESS', x, y)
NLESS = lambda x, y: Relation('NLESS', x, y)


#-----------------------------------------------------------------------------
# Strategies

class Iter(object):
    def __init__(self, var):
        assert isinstance(var, Variable)
        self.var = var

    def __repr__(self):
        return 'for({})'.format(self.var)

class Let(object):
    def __init__(self, expr):
        assert isinstance(expr, Function)
        self.var = Variable(expr)
        self.expr = expr

    def __repr__(self):
        return 'let({})'.format(self.var)


class Test(object):
    def __init__(self, expr):
        assert isinstance(expr, Expression)
        self.expr = expr

    def __repr__(self):
        return 'if({})'.format(self.expr)


class Ensure(object):
    def __init__(self, expr):
        assert isinstance(expr, Compound)
        self.expr = expr

    def __repr__(self):
        return 'ensure({})'.format(self.expr)


class Strategy(object):
    def __init__(self, sequence, succedent):
        self.sequence = list(sequence)  # TODO generalize to trees
        self.succedent = Ensure(succedent)

    def __repr__(self):
        return ' '.join(map(str, self.sequence + [self.succedent]))

    def optimized(self):
        '''
        Pull tests into preceding iterators
        '''
        TODO()

    def get_cost(self):
        TODO()


class Sequent(object):
    def __init__(self, antecedents, succedents):
        antecedents = set(antecedents)
        succedents = set(succedents)
        for expr in antecedents | succedents:
            assert isinstance(expr, Compound)
        self.antecedents = antecedents
        self.succedents = succedents

    def __str__(self):
        return '{} |- {}'.format(
            ', '.join(map(str, self.antecedents)),
            ', '.join(map(str, self.succedents)))

    def get_vars(self):
        return union([e.get_vars() for e in self.antecedents | self.succedents])

    def get_constants(self):
        return union([e.get_constants()
                      for e in self.antecedents | self.succedents])

    def _is_normal(self):
        '''
        A sequent is normalized if it has a single consequent of the form
            Relation Variable Variable
        and each antecedent is of one of the minimal forms
            Function Variable Variable  (which abbreviates Var = Fun Var Var)
            Relation Variable Variable
        '''
        if len(self.succedents) != 1:
            return False
        for node in self.antecedents | self.succedents:
            for child in node.children:
                if isinstance(child, Compound):
                    return False
        return True

    def _normalized(self):
        '''
        Return a list of normalized sequents.
        '''
        if not self.succedents:
            TODO('allow multiple succedents')
        elif len(self.succedents) > 1:
            TODO('allow empty succedents')
        succedent = self.succedents.copy().pop()
        succedents = set([succedent.get_atom()])
        antecedents = union([r.get_tests() for r in self.antecedents] +
                            [e.get_tests() for e in succedent.children])
        return [Sequent(antecedents, succedents)]

    # TODO deal with EQUAL succedent where one side need not exist
    def compile(self):
        free = self.get_vars()

        # HEURISTIC test for constants first
        constants = self.get_constants()
        pre = map(Let, constants)
        context = set(constants)
        bound = set(map(Variable, constants))

        results = []
        for part in self._normalized():
            result = []
            for v in free:
                pre_v = pre + [Iter(v)]
                bound_v = set_with(bound, v)
                result += list(part._compile(pre_v, context, bound_v))
            results.append(result)
        return results

    def compile_given(self, atom):
        assert isinstance(atom, Compound)
        context = set([atom])
        bound = atom.get_vars()
        if isinstance(atom, Function):
            bound.add(Variable(atom))

        # HEURISTIC test for constants first
        constants = self.get_constants()
        pre = map(Test, constants)
        context |= set(constants)
        bound |= set(map(Variable, constants))

        results = []
        for part in self._normalized():
            if atom in part.antecedents:
                results.append(list(part._compile(pre, context, bound)))
        return results

    def _compile(self, pre, context, bound):
        assert self._is_normal()
        free = self.get_vars() - bound
        antecedents = self.antecedents - context
        succedent = self.succedents.copy().pop()
        for s in iter_compiled(antecedents, bound, free):
            yield Strategy(pre + s, succedent)


def iter_compiled(antecedents, bound, free):
    #print 'DEBUG', list(antecedents), list(bound), list(free)
    assert bound
    if not (free or antecedents):
        yield []
        return

    # HEURISTIC test eagerly
    testable = False
    for a in antecedents:
        if isinstance(a, Relation):
            if a.get_vars() <= bound:
                antecedents_a = set_without(antecedents, a)
                pre = [Test(a)]
                for s in iter_compiled(antecedents_a, bound, free):
                    yield pre + s
                testable = True
        elif isinstance(a, Function):
            if a.get_vars() <= bound and Variable(a) in bound:
                antecedents_a = set_without(antecedents, a)
                pre = [Test(a)]
                for s in iter_compiled(antecedents_a, bound, free):
                    yield pre + s
                testable = True
    if testable:
        return

    # HEURISTIC bind eagerly
    bindable = False
    for a in antecedents:
        if isinstance(a, Function):
            if a.get_vars() <= bound:
                var_a = Variable(a)
                assert var_a not in bound
                antecedents_a = set_without(antecedents, a)
                bound_a = set_with(bound, var_a)
                free_a = set_without(free, var_a)
                pre = [Let(a)]
                for s in iter_compiled(antecedents_a, bound_a, free_a):
                    yield pre + s
                bindable = True
    if bindable:
        return

    # HEURISTIC iterate forward eagerly
    forward_iterable = False
    for a in antecedents:
        if isinstance(a, Function):
            if a.get_vars() & bound:
                for v in a.get_vars() - bound:
                    free_v = set_without(free, v)
                    bound_v = set_with(bound, v)
                    pre = [Iter(v)]
                    for s in iter_compiled(antecedents, bound_v, free_v):
                        yield pre + s
                    forward_iterable = True
    if forward_iterable:
        return

    # iterate backward
    for a in antecedents:
        if isinstance(a, Function):
            if Variable(a) in bound:
                for v in a.get_vars() - bound:
                    free_v = set_without(free, v)
                    bound_v = set_with(bound, v)
                    pre = [Iter(v)]
                    for s in iter_compiled(antecedents, bound_v, free_v):
                        yield pre + s


class Theory(object):
    def __init__(self, sequents):
        self.sequents = sequents

    def compile(self):
        TODO()
