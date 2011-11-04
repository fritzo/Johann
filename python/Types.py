__doc__ = """A Typed Combinatory System:
  this project is being postponed until a type theory can be found
  which does not require alpha-conversion."""

#==========[ types ]==========
def mergeContexts(lhs,rhs):
  "contexts are dictionaries from VarTypes to Types"
  result = lhs.copy()
  for key in rhs:
    if key in lhs:
      if lhs[key] is not rhs[key]:
        return None
    else:
      result[key] = rhs[key]
  return result

class Type:
  def __mul__(lhs,rhs):
    return makeMapType(lhs,rhs)

class AtomType(Type):
  def __init__(atom,name):
    atom.name = str(name)

  def solveForContext(self,other):
    if self is other:  return {}
    else:              return None
unit = AtomType('D')

class VarType(AtomType):
  def solveForContext(self,other):
    return {self:other}
U = VarType('U')
V = VarType('V')
W = VarType('W')
X = VarType('X')
Y = VarType('Y')
Z = VarType('Z')

class MapType(Type):
  def __init__(dom,cod):
    self.dom = dom
    self.cod = cod

  def solveForContext(self,other):
    if isinstance(other,MapType):
      dom_context = self.dom.solveAppType(other.dom)
      cod_context = self.cod.solveAppType(other.cod)
      context = mergeContexts(dom_context, cod_context) #may also be None
    else:
      return None

  def getAppType(lhs,rhs):

def makeMapType(lhs,rhs):


#==========[ typed combinators ]==========
class Term:
  def __mul__(lhs,rhs):
    return makeAppTerm(lhs,rhs)

  def __div__(lhs,rhs):
    assert isinstance(rhs,VarTerm), "only variables can be abstracted"
    return lhs.algorithm_abcdef(rhs)

  def algorithm_fab(lhs,rhs):
    "Curry's simple abstraction algorithm, i.e., "
    if isinstance(lhs,AppTerm):
      temp = S*(lhs.lhs/rhs)
      if temp is None:
        return None
      else:
        return temp*(lhs.rhs/rhs) #rule f
    elif not rhs.isFreeIn(lhs):
      return K*lhs #rule a
    else:
      return = I #rule b

class AtomTerm(Term):
  def __init__(atom,name,type):
    atom.name = str(name)
S = AtomTerm('S',(U*(V*W))*((U*V)*(U*W)))
K = AtomTerm('K',X*(Y*X))

class VarTerm(AtomTerm):
u = VarTerm('u')
v = VarTerm('v')
w = VarTerm('w')
x = VarTerm('x')
y = VarTerm('y')
z = VarTerm('z')

class AppTerm(Term):
  def __init__(app,lhs,rhs,type):
    atom.lhs = lhs
    atom.rhs = rhs
    atom.type = type

def makeAppTerm(lhs,rhs):
  context = solveForContext(lhs,rhs)
  if context is None:
    return None
  else:
    type = applyContext
    term = AppTerm(lhs,rhs,type)

