
def argmin(items, objFun):
  minVal = inf
  minItem = None
  for item in items:
    val = objFun(item)
    if val < minVal:
      minVal = val
      minItem = item
  return minItem

class Lattice:
  def __init__(self): ???
  def insert(self, item): ???
  def remove(self, item): ???
  def merge(self, dep, rep): ???

  #algebraic operations
  def bot(self): ???
  def top(self): ???
  def getInterval(self, LB, UB): ???
  def getLowerSet(self, UB): ???
  def getUpperSet(self, LB): ???

class Type:
  lattice = Lattice()
  def __init__(self, down, up, ):
    #order structure
    assert isinstance(down, set), "down must be a set"
    assert isinstance(up, set), "down must be a set"
    self.down = down
    self.up = up
    #applicative structure
    self.LRA = {}
    self.RLA = {}
    self.LRA = {}
    self.LRA = {}

  #structural operations
  def apply(self,arg):
    "constructs an app type; possibly fails; notation: F{X}"

  def abstract(self,arg):
    "constructs an abstraction type, possibly empty (failing); notation: DR"
  def mapsto(self,range):
    "the traditional dom->rng notation"
    return arg.abstract(self)

  def equateWith(self,other):
    "merging operation; allows recursion"
    ???

  #approximate operations
  def __and__(self,other):
    "intersection operation, returns a minimal set"
    """This is the trick: finding a reasonable set of types.
    The set of types is an order-theoretic lattice, but the algebraic
      meet operation is not representable, and in a finite approximation
      must thus be approximated.
    """

class Estimator:
  def __init__(self, numTypes):
    self.numTypes = numTypes
    #define type structure
    self.Bot = ???
    self.Top = ???
    #define basic types
    self.B = ??? #booleans, two inhabitants
    self.S = ??? #(o)stream type, i.e., infinite binary sequences
    self.T = ??? #true,  a singleton type
    self.F = ??? #false, a singleton type
    #initialize state
    self.X = S

  def advance(self, output):
    "advances estimator by one bit"
    assert output <= self.B, "invalid output type"
    self.X &= self.F.mapsto(output)
    self.X = self.state.applyto(self.T)

  def estimate(self):
    "finds maximum likelihood estimate at current time"
    support = self.__getLatticeInterval(self.Bot, self.X)
    points = [type in support if type.isSingleton()]
    result = argmin(points, (lambda p: p.getCompMeas()))
    return result

  def compress(self):
    "conservatively compresses current state"
    support = self.__getLatticeInterval(self.X, self.S)
    objFun = ???
    self.X = argmin(support, objFun)

  def reshape(self):
    "reshapes type enumeration structure based on current data"

  #========== internal functions ==========

  def __createRandomApp(self):
    while True:
      lhs = random_choice(self.types)
      rhs = random_choice(self.types)
      if (lhs,rhs) not in self.appTable: break
    app = self.__makeApp(lhs,rhs)

  def __makeApp(self, lhs, rhs):
    app = Type()
    self.appTable[lhs,rhs] = app
    for iter in lhs.LRA():
      r2 = iter.rhs
      raise LATER

  def __getLatticeInterval(self, LB, UB): ???


"""
(N1) what if there is no compression;
  what if 'meet' is included as an algebraic operation?
  Then a string of words supporting X will be created,
    and rather than compress, one can simple erase part of the string,
    keeping only partial information about X.
  Information can be retained based on its relevance.
  What _will_ be known of X is:
    * its order position WRT other points,
    * its app position WRT other points
  (N1) The new update step would be
"""
  def advance(self,output):
    self.X.applyTo(self.F).equateTo(output)
    self.X = self.X.applyTo(T)
"""
    and the following rules axiomatize app-order interaction:
      t1 < t2  <==>  (s t1) < (s t2)  #covariant
      s1 < s2  <==>  (s1 t) > (s2 t)  #contravariant
(N2) A more general database might have the following operations:
"""
class InferenceSystem:
  def buildApp(self,lhs,rhs): ...
  def assumeEquiv(self,lhs,rhs,app): ...
  def estimate(self,type): ...
  def reshape(self): ...
  def getPMF(self):
    "returns pmf over type's inhabitants, modulo some theory"
"""
  together with some relevance calculation tools based on the current
  working terms.
  This system could transcend the simple applications of dynamic
    estimation & control, and support an entire style of programming.
  In control applications, one specifies an objective function phi(x,y), and
    tries to minimize the expected value bar{phi}(y) = E_x phi(x,y).
    Estimation is thus the special case of phi(x,y) = delta(x,y) (dirac).
  The general system supports the following programming constructs
    * data (X,Y) is in the form of types
    * define constants, e.g., B, S, T, F
    * define the state X (possibly in terms of its previous value)
    * assume equations (or inequalities <=, >=)
    * define the response Y (")
    * define the joint likelihood phi(x,y) (the objective function)
  More generally, one could simply define a prior likelihood function
    phi(x) and let X represent a (world,controller,response) triple.
    (Q1) what form should the likelihood function take?
      (N1) The control should be a "corrective intervention" whose complexity
        is to be minimized. (this idea was mentioned months-years ago).
      (A1) 
"""
