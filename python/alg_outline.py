
class Theorist:
  "an applicative structure db corresponding to a single theory"
  def __init__(depth,axioms):
    "sets up a full db, ready to work"
  def __copy__():
  def think():
    "performs a think cycle of contraction and expansion"
  def sleep():
    "optimizes internal structures"
  def focus(focal_pmf):
    "sets focal measure to some given pmf"
  def defocus():
    "resets focal measure"
  def compare(term1,term2):
    "evaluates correlation of two terms"
  def simplify(term):
    "finds an equivalent term that is shorter (possibly shortest)"
  def approximate(term):
    """finds a similar term that is shorter
    (Q1) how to define 'best'? how to weigh error against complexity?"""
  def suggestAssumption():
    """finds a pair of terms whose equality is highly likely and
    also highly collapsing
    (Q1) how to relate the two?  that is, how to define expected utility?"""
  def assumeEquiv(term1,term2):
    "adds an axiom which identifies two terms"
  def assumeDistinct(term1,term2):
    """adds a theorem which distinguishes two terms
    (N1) this is used after proofs by contradiction"""

class SubTheory:
  "represents a r.e. sub-theory of a universal complete theory"
  def __init__(term1,term2):
    "defines a subtheory of a single identity"
  def simplify(theory):
    """simplifies a theory by finding an equivalent small theory
    (Q1) what could be a measure on theories?"""
  def meet(theory1,theory2):
    "tries to find an intersection of two subtheories"
  def join(theory1,theory2):
    "returns the union of two subtheories"

class Genealogist:
  """manages a population of Theorists over an extension system.
  (Q1) is it possible to communicate conditional equivalence?, i.e.
    (a=b |- c=d)?"""
  def __init__(core):
    "sets up a single individual"
  def think():
    "breeds theorists while they think & sleep"

class Numerologist:
  def __init__():
    "initializes working data"
  def read(article):
    "reads an article or general character string"
  def think():
    "processes known information"
  def rankNumbers(length=10):
    "returns list of most common numbers"
  def approximateNumber(gaussian):
    "finds a real number within maximum posterior likelihood"

class Literate:
  def __init__():
    "initializes working data"
  def read(article):
    "reads an article or general character string"
  def think():
    "processes known information"
  def writeAbout(topic,size):
    "provides formatted information about a topic"
  def assimilate(criticism):
    "asjusts writing style based on criticism"

