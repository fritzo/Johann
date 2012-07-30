import itertools
from pomagma.util import TODO, union

#-----------------------------------------------------------------------------
# Syntax

class Symbol(object):
    def __init__(self, name):
        self.name = name

    def __str__(self):
        return self.name


class Variable(Symbol):
    pass


class Constant(Symbol):
    arities = ['nullary', 'unary', 'binary', 'symmetric']

    def __init__(self, name, arity='nullary'):
        assert arity in Constant.arities
        Symbol.__init__(self, name)
        self.arity = arity

    def __call__(self, *args):
        return AST(self, args)


class Relation(Symbol):
    arities = ['unary', 'binary', 'symmetric']

    def __init__(self, name, arity):
        assert arity in Relation.arities
        Symbol.__init__(self, name)
        self.arity = arity

    def __call__(self, *args):
        return AST(self, args)


EQUAL = Relation('EQUAL', 'symmetric')
LESS = Relation('LESS', 'binary')
NLESS = Relation('NLESS', 'binary')


class AST(object):

    def __init__(self, root, children=[]):
        self.root = root
        self.children = []
        for child in children:
            if isinstance(child, Variable):
                self.children.append(AST(child))
            else:
                assert isinstance(child, AST)
                self.children.append(child)

    def __str__(self):
        if isinstance(self.root, Variable):
            return str(self.root)
        else:
            return '{}({})'.format(self.root, ', '.join(map(str, self.children)))

    def get_free(self):
        result = union([child.get_free() for child in self.children])
        if isinstance(self.root, Variable):
            result.add(self.root)
        return result


    def nodes(self):
        return [self] + sum([child.nodes() for child in self.children], [])

    def internal_nodes(self):
        if self.children:
            return [self] + sum([
                child.internal_nodes() for child in self.children], [])
        else:
            return []

#-----------------------------------------------------------------------------
# Strategies

class Iter(object):
    def __init__(self, var):
        self.var = var

    def __repr__(self):
        return 'for({})'.format(self.var)


class Test(object):
    def __init__(self, ast):
        assert isinstance(ast, AST)
        self.ast = ast

    def get_free(self):
        return self.ast.get_free()


    def __repr__(self):
        return 'if({})'.format(self.ast)


class Strategy(object):
    def __init__(self, sequence):
        self.sequence = list(sequence)  # TODO generalize to trees

    def __repr__(self):
        return ' '.join(map(str, self.sequence))

    def optimized(self):
        '''
        Pull tests into preceding iterators
        '''
        TODO()

    def get_cost(self):
        TODO()


class Sequent(object):
    def __init__(self, antecedents, succedents):
        for ast in antecedents:
            if not isinstance(ast.root, Relation):
                raise ValueError('bad antecedent: {}'.format(ast))
        for ast in succedents:
            if not isinstance(ast.root, Relation):
                raise ValueError('bad succedent: {}'.format(ast))
        self.antecedents = antecedents
        self.succedents = succedents

    def __str__(self):
        return '{} |- {}'.format(
            ', '.join(map(str, self.antecedents)),
            ', '.join(map(str, self.succedents)))

    def get_free(self):
        asts = self.antecedents + self.succedents
        vars = list(union([ast.get_free() for ast in asts]))
        vars.sort(key=(lambda v: v.name))
        return vars

    def normalized(self):
        if len(self.succedents) == 1:
            return [self]
        elif self.succedents:
            TODO('allow empty succedents')
        else:
            TODO('allow multiple succedents')

    def get_tests(self):
        if len(self.succedents) != 1:
            raise NotImplementedError('only one succedent is allowed')
        succedent = self.succedents[0]
        asts = sum(map(AST.internal_nodes,
            self.antecedents + succedent.children), [])
        #asts = list(union(asts))  # TODO this requires equality comparison
        return map(Test, asts)

    def compile_ranked(self):
        # TODO keep track of which internal AST nodes have been locally named
        # (e.g. let(APP_S_x = APP(S, x)) ...),
        # and normalize tests to those local variables.
        # Then implement a compiler for incremental strategies
        # that specifies one of the internal nodes at the outset
        # (e.g. given(APP_S_x = APP(s, x)) if(s == S) ...)
        all_vars = self.get_free()
        all_tests = self.get_tests()
        strategies = []
        for ordered_vars in itertools.permutations(all_vars):
            iters = [Iter(var) for var in ordered_vars]
            bound_tests = [[] for var in ordered_vars]
            bound_vars = set()
            remaining_tests = all_tests[:]
            for iter, tests in zip(iters, bound_tests):
                bound_vars.add(iter.var)
                for test in remaining_tests:
                    if set(test.get_free()) <= bound_vars:
                        remaining_tests.remove(test)
                        tests.append(test)
            ordered_tests = itertools.product(
                    *map(itertools.permutations, bound_tests))
            for tests in ordered_tests:
                strategies.append(Strategy(
                    sum([[i] + list(ts) for i, ts in zip(iters, tests)], [])))
        return strategies

    '''
    def _iter_compiled(self, bound=[], free_vars=set(), free_asts=set()):
        ast_was_bound = False
        for ast in free_asts:
            children = free_asts
            if ast.get_free() <= free_vars and all(
                    [c in bound for c in ast.children]):
                new_bound = bound[:]
                new_bound.append(ast)
                new_free_asts = free_asts.copy()
                new_free_asts.remove(ast)
                if new_free_asts or free_vars:
                    self._iter_compiled(new_bound, free_vars, new_free_asts)
                else:
                    yield bound
                ast_was_bound = True
        if not ast_was_bound:
            for var in free_vars:
                new_bound = bound[:]
    '''


class Theory(object):
    def __init__(self, sequents):
        self.sequents = sequents

    def normalize(self):
        '''
        Transform self so that every sequent has a single succedent
        '''
        self.sequents = sum([s.normalized() for s in self.sequents], [])
