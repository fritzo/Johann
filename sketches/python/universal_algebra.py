#wilfred hodges says: universal algebra = model theory - logic
class Algebra:
  def __init__(self, carrier, signature):
    self.carrier = carrier     #lo:wenheim-skolem says carrier is moot
    self.signature = signature
    self.constraints = constraints
class Construct:
  def __init__(self, arity):
    self.arity = arity
  def __call__(self, *args):
    assert len(args) = self.arity
    raise NotImplementedError
class Atom(Construct):
  def __init__(self):
    Construct.__init__(self,0)

#wilfred hodges says: model theory = universal algebra + (f.o.) logic
class Language:
  def __init__(self, atoms, grammar):
    self.atoms = atoms
    self.grammar = grammar
class Theory:
  def __init__(self, language, propositions):
    self.language = language
    self.propositions = propositions
#the pure fragment of the untyped lambda calculus (SK-terms), and its
#  associated lattice of theories, seems to be the most interesting variety
#  of algebraic structures, balancing simplicity with verbosity/universality.
class EquationalTheory(Theory):
  raise NotImplementedError
#the combinators seem to be a "unit sphere" in the information lattice,
#  separating Top from Bottom and generating all the other interesting
#  structure.

#¡use model theory to show H^* is "optimal" among models,
#  based on effectiveness of finite sub-models!
#¿what is (model theory / ZFC) [intuitionistic / classical],
#  i.e., on an arbitrary topos?
#¿what are these operations on (non-tech) theories,
#  as in the Vaughbohn kitchen crepe epiphany over physics?

