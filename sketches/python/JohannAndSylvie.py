# JohannAndSylvie.py

__doc__ = """In search of Johann & Sylvie.

==========[ Overview ]==========
The main idea here is to define the language only in terms of 
lambda expressions, and then define a basis of other combinators for
use in complexity measures.  By including impure combinators (i.e.,
those with unbound variables), one can do quite well in moding out 
by extentionality.  Eventually, I'd like to incorporate more math
with the basis changing.  It may also be possible to include more
complicated combinators in the basis, soas to implement a small
version of lisp.

==========[ Behavioral Details ]==========
Unless a normal reduction standard is found for classes, Exprs
store (in sparse form) normal reduction trees.  see (Q1)

ExprClasses store only application and abstraction tables.  They
also know their MLrep and MErep.

Constructive equivalence is stored in two forms: proven equivalence
by classes, and proven distinction by explicit links between classes.

==========[ Interface ]==========
(Example 1)
  m = Vars.m
  n = Vars.n
  f = Vars.f
  x = Vars.x
  two = ChurchNumeral(2)
  three = ChurchNumeral(2)
  add = ((((m*f)*((n*f)*x)/x)/f)/n)/m #builds Exprs and ExprClasses
 #returns simplest equivalent expression known
  add.simplify()
 #thinks for 10 seconds, then returns simplest equivalent expression known
  add.simplify(10)
 #returns most reduced equivalent expression known
  ((add*two)*three).evaluate() 
 #thinks for an hour, then returns most reduced equivalent expression known
  ((add*two)*three).evaluate(3600)

  
==========[ Questions, Answers, & Notes ]==========
(ordered in decreasing importance and in similarity neighborhoods)

(Q1) how to do normal form reduction on classes?  or any reduction?
(A1) instead, do normal form reduction on random elements of the
  class and probably add them to the class, based on their
  complexity.
(A2) somehow use reduction information from reduction of resulting
  combinators after application and then abstraction of some 
  variables.
(N1) normal form reduction depends on the basis.
(A3) one might drop entirely the idea of normal form reduction, and
  simply use random reduction order.
==========
(Q3) how to define complexity based on a PMF basis and some 
  applicative temperature?
(A1) use prodFlow.Papp, prodFlow.Pabs, and an atomic basis.
==========
(Q8) what are the random production and reduction flows?
(N1) they are sort of duals to each other, production being a
  space flow, reduction being a time flow.  Somehow, each
  production flow should define an optimal reduction flow.
(N2) I think one should break this into
  *initial distribution (expr.initProb, exprClass.initProb)
  *production flow, resulting in (expr.prodProb, exprClass.prodProb)
  *reduction flow, resulting in (expr.redProb, exprClass.redProb)
  each of which is a PMF, in exprs, transitions, and transitions, resp.
(Q8.1) might one define an optimal structure (what is tabulted) by
  maximizing the mean NF reduction distance over the produced Exprs?
==========
(Q4) how to simplify expressions WRT a given PMF basis & temperature?
==========
(Q2) should the probabilistic disjunction R be implemented as a
  combinator (only in ExprClass), or as a construct (and thus in
  Expr)?
(N1)  An eventual bin implementation in ExprClass requires
  that it be implemented as a construct.
==========
(Q5) how to implement the mergeInto function?
==========
(Q7) how is the language embedded in itself?  see ninth attempt in
  Johann.py
==========
(Q6) how to efficiently order the Expr.calcProb() function so that not
  all subsexpressions need to be evaluated?
(A1) store the computed value (implemented).
(N1) this also works for non-atomic bases because of the ordering on
  compound expressions
==========
(Q9) regardless of the Expr basis (here implemented as the
  lambda calculus, but SK works), the entire ExprClass structure should
  follow from any sufficient ExprClass basis, where productions and
  reductions occur only between ExprClasses.  In this way, the Exprs
  function only as a translator between machine-think (ITO ExprClasses)
  and something legible (lambda expressions).  Thus, as in (Q1), how do
  ExprClasses reduce, based only on ExprClasses?
(A1) if c1=c3*c5 and c2=c4*c5 and c3=c4 then c1=c2
     if c1=c3*c4 and c2=c3*c5 and c4=c5 then c1=c2
     if c1=((x*c2)/x)*c3) then c1 = c3*c2
(N1) if ExprClasses need no set of Exprs, then translation can work
  using only mappings VarExprs (--> ExprClasses.
(N2) one reduction operation uses substitution:
    (x/y)*z --> "x.subs(y,z)".
  this can be broken into further subs steps as
    if x==a*b: a*b/y*z -->   (a/y*z) * (b/y*z)
    if x==a/b: a/b/y*z --> {  a/y*z / b  if y!=b and (b not in z)
                           {    a / b    if y==b
                           { a/b*c/y*z/c if y!=b and (Var c not in z)
(N3) extensionality can then take the form:
    x --> (x/y)*y
    x --> (x*y)/y   if y does not appear freely in x
  but the second condition may be troublesome
==========
(Q10) if ExprClasses can be reduced, then one can generalize abstraction
  from variables to any ExprClass, variables being distinguished only by
  their initial state of being known distinct from each other.  Thus,
  the equality operator E (or Q) could be implemented within the
  abtraction-generalized lambda calculus.  How does this work?
(A1) replacement of an expr x by an expr y is conditioned on the equality
  of y to another expr z, that is replace x by y only if y != z.
(N1) this does not seem obviously useful.
(N2) the probabilistic AND operator ^ may be able to be constructed this
  way, however:
    ^*X*Y --> [(^*X*Y)/(^*X*Y)/x*y*(^*X*Y),
                 for x in X, for y in Y]
(N3) but what's so special about variables?:
  * they're in NF (whatever that means for an ExprClass)
  * they're known distinct
==========
(Q11) if ExprClasses are basis-free (other than variables), then can one
  do some simpler statistics in making a well-shaped brain?
(A1) the pertinance of an ExprClass can be calculated by finding the
  steady-state distribution based on a given flow and the empirical
  uniform existence PMF.  the uncertainty of an ExprClass has something
  to do with its known relatives.  ???
(A2) certainly, a well-shaped brain will have fairly uniform ss probs
  (no very low prob exprs)  Thus, simply prune the min-likelihood exprs
  from the graph, randomly add new exprs, and contantly re-calculate ss
  probs using the current brain as the basis (maybe while dreaming)?
==========
(Q12) how does Sylvie fit in?  (the eventual control-theoretic program)?
(N1) if Sylvie is a program, she needs input and output streams.  These
  can be special combinator streams whose values are, say, zero or one.
  The output stream can be, e.g, the result of _M_easuring sylvie, and
  the input stream can be feeding bits to Sylvie through _T_ime = 0 or 1.
  The problem with this is that _M_easuring sylvie may not be immediately
  evaluated and thus not be really measurable.
(N2) Even if sylvie outputs probabilistic bits, the probability may not
  be known in time to randomly sample from.
(N3) Alternatively, Sylvie can be incorporated into the system from the
  start, where each expr's "score" can be composed of both the ss prob
  (as in (Q11)) component and a current-correctness component.
(N4) in an (N3) scheme, look-ahead evaluation can be based on prediction
  and eventually be sampled to find the actual input.
(N5) if the system can be self-embedded, then there will be no difference
  between a program implementation and a built-in implementation of
  Sylvie.
==========
(Q13) how does the simpler estimation-theoretic program work?
(A1) score based on the estimated PMF, where the predicted state is
  either something in the permissible set (e.g., {T,F}) or something
  that's not known to be (at the current eval step), which is _then_
  counted against the predictor.
(N1) This requires, however, a notion of the _currency_ of knowledge,
  since decisions are made at the current time step.
(N2) Alternatively, the point of the program can be thought of as to
  "find the _provably_ most likely program which models the input".
(N3) Can an exact version exist, where somehow the uncomputed information
  is stored in at most the space required to store the input stream, or
  a compressed version of it?
==========
(Q14) how does the similar pure input-compression program work, where at
  any time, Johann must know everything he ever heard?
(A1) in this case, Johann tries to determine the identity of an unknown
  input-stream expr.  Each expr has (as in (Q12)) a correctness prob.
  The difference is that in compression, the initial state, not just the
  final (current) state must be retained.  That's a silly requirement
  unless one wants to implicitly store the input for later evaluation
  (or digestion: chew cud).
(N1) This may be necessary for initially data-hungry implementations
  (prior to video straming, etc.).
(A2) A better option than exact compression is to retain as much info as
  possible at any one time, using some estimated importance measure.
  This permits later higher data rates, while still allowing the
  possibility of postponed processing (dreaming) during sensory sleep.
==========
(Q15) if abstraction is generalized to exprs, then how does free
  containment generalize?
(A1) containment can be recursively defined by the axioms:
    x in x
    x not in y/x
    y*x irreducible => x in y*x ? (needs reducibility def, nonconstant)
    ???others???
  In general, if the NF Expr in ExprClass x is free of the NF Expr in
  ExprClass y, then x is free of y.  This is squirrely.
==========
(Q16) 
==========
(Q17)
==========
(Q18)
==========
(Q19)

Fritz Obermeyer
version 2003:11:17:22:44"""

__all__ = ['Vars','Consts']

#==========[ misc ]==========
from exceptions import *
from Set import *
from PMF import *
from Linear import *

_Map = dict #LATER: use balanced tree implementation
_Set = Set
_PMF = PMF

AbstractError = "abstract function called"
LATER = "unfinished code in MultiLang.py"
class InvalidError(Exception):
  pass
class IrreducibleError(Exception):
  pass

_exprDB = None #needs to be set later

#==========[ stochastic expression flows ]==========
class ExprFlow(PMF):
  def __init__(f):
    f.dirs = PMF()

  def 
        
#==========[ stochastic production flow ]==========
class ProdFlow:
  """recursively defined expression measure, i.e.
    stochastic production flow
    (formerly ExprMeas)"""
  def __init__(f):
    f.basis = PMF() #initial distribution
    f.comp = PMF() #flow
    f.comp['stay'] = 0.25 #do nothing
    f.comp['app'] = 0.25 #apply with another term
    f.comp['abs'] = 0.25 #abstract with another term
    f.comp['or'] = 0.25 #or with another term

  def normalize(m):
    f.basis.normalize()
    f.comp.normalize()

  def validate(f):
    f.basis.validate()
    f.comp.validate()

#==========[ stochastic reduction flow ]==========
class RedFlow:
  """recursively defined stochastic reduction flow, which
    acts only within equivalence classes
    (formerly ExprFlow)"""
  def __init__(f):
    """the maximal subset of applicable flow directions is used at
      any one step"""
    f.comp = PMF()
    f.comp['stay'] = 0.1 #do nothing
    f.comp['ext'] = 0.1 #extentional equivalence
    f.comp['lhs'] = 0.2 #lhs in a CompExpr
    f.comp['rhs'] = 0.2 #rhs in a CompExpr
    f.comp['subs'] = 0.4 #substitution in an app(abs(.,.),.) expression

  def normalize(f):
    f.comp.normalize()

  def validate(f):
    f.comp.validate()

#==========[ lambda expressions ]==========
class Expr:
  """lambda calculus expression"""
  def __init__(e):
    e.prob = None

  def __copy__(e):
    return e

  #construction
  def __mul__(lhs,rhs):
    """functional application"""
    return _exprDB.getAppExpr(lhs,rhs)

  def __div__(lhs,rhs):
    """functional abstraction"""
   return _exprDB.getAbsExpr(lhs,rhs)

  #equivalence
  def __eq__(lhs,rhs):
    """has lhs been proven to be equivalent to rhs?"""
    return lhs.exprClass == rhs.exprClass

  def __ne__(lhs,rhs):
    """has lhs been proven not to be equivalent to rhs?"""
    return lhs.exprClass != rhs.exprClass

  #flow
  def subs(e,v_old,e_new):
    raise AbstractError

  def randomlyReduce(e):
    raise AbstractError

  def randomlyProduce(e)
    raise AbstractError

  #measure
  def calcProb(e):
    "calculates a term's complexity from the exprDB's measure"
    if e.prob is None:
      e.prob = _exprDB.prodFlow.basis[e]
    return e.prob

  #simplification
  def simplify(e):
    "returns most likely equivalent expression"
    return e.exprClass.MLrep

  def evaluate(e):
    "returns most evaluated equivalent expression"
    return e.exprClass.MErep

#atomic expressions
class AtomExpr(Expr):
  """atomic lambda expression"""
  def __init__(e,name):
    Expr.__init__(e)
    e.name = name

class ConstExpr(AtomExpr):
  """constant lambda expression, 
    used in an extended lambda calculus"""
    
  def subs(e,v_old,e_new):
    return e

class VarExpr(AtomExpr):
  """variable lambda expression"""
  
  def subs(e,v_old,e_new):
    if e is v_old:
      result = e_new
    else:
      result = e
    return result

#compound expressions
class CompExpr(Expr):
  """abstract compound expression"""
  def __init__(e,lhs,rhs):
    Expr.__init__(e)
    e.lhs = lhs
    e.rhs = rhs

  def subs(e,v_old,e_new):
    return e.__class__(e.lhs.subs(v_old,e_new),e.rhs.subs(v_old,e_new))

class AppExpr(CompExpr):
  """functional application expression"""

  def randomlyReduce(e):
    if isinstance(e.lhs,AbsExpr):
      redType = _exprDB.redFlow.comp.randChoice()
      raise LATER
    else
      raise LATER
    return reduced

  def calcProb(e):
    if e.prob is None:
      e.prob = Expr.calcProb(e)
      e.prob += _exprDB.prodFlow.Papp * e.lhs.calcProb() * e.rhs.calcProb()
    return e.prob

class AbsExpr(CompExpr):
  """functional abstraction expression"""
  def __init__(e,lhs,rhs):
    if not isinstance(rhs,VarExpr):
      raise TypeError, 'only variables can be abstracted'
    CompExpr.__init__(e,lhs,rhs)

  def calcProb(e):
    if e.prob is None:
      e.prob = Expr.calcProb(e)
      e.prob += _exprDB.prodFlow.Pabs * e.lhs.calcProb() * e.rhs.calcProb()
    return e.prob

  def subs(e,v_old,e_new):
    if e.rhs is v_old:
      result = e
    else:
      result = e.lhs.subs(v_old,e_new) / e.rhs
    return result

""" alternate version?  see (Q2)
class OrExpr(CompExpr):
  "probabilistic disjunction expression"
"""
#==========[ expression classes ]==========
class ExprClass:
  """equivalence class of expressions modulo extentionality & reduction"""
  def __init__(c,initType,*args):
   #equivalence
    c.exprs = _Set()
    c.distinctFrom = _Set()
   #relative classes
    c.Lapp = _Map()
    c.Rapp = _Map()
    c.Labs = _Map() #empty for non-variables
    c.Rabs = _Map()
   #probability
    c.MLrep = None
   #build type; can't subclass because type is not known
    if initType == 'atom':
      c.__initFromExpr(*args)
    elif initType == 'app':
      c.__initFromApp(*args)
    elif initType == 'abs':
      c.__initFromAbs(*args)

  def __copy__(c):
    return c
  
  def __initFromAtom(c,e):
    """build class from an atom"""
    c.exprs.insert(e) #an Expr
    e.exprClass = c
    c.isVar = isinstance(e,VarExpr):
   #equivalence
    for c2 in _exprDB.atomExprClasses:
      c.assumeDistinctFrom(c2)
    

  def __initFromApp(c,lhs,rhs):
    """build class from application"""
    c.isVar == False #acutally unknown
    lhs.Rapp[rhs] = c
    rhs.Lapp[lhs] = c
   #equivalence
    for rhs2 in rhs.distinctFrom:
      if rhs2 in lhs.Rapp:
        c.assumeDistinctFrom(lhs*rhs2)
    for lhs2 in lhs.distinctFrom:
      if lhs2 in rhs.Lapp:
        c.assumeDistinctFrom(lhs2*rhs)
   #probability
    c.prob = lhs.prob * rhs.prob * _exprDB.prodFlow.Papp
    
  def __initFromAbs(c,lhs,rhs):
    """build class from abstraction"""
    c.isVar == False
    lhs.Rabs[rhs] = c
    rhs.Lbs[lhs] = c
   #equivalence
    for rhs2 in rhs.distinctFrom:
      if rhs2 in lhs.Rabs:
        c.assumeDistinctFrom(lhs/rhs2)
    for lhs2 in lhs.distinctFrom:
      if lhs2 in rhs.Labs:
        c.assumeDistinctFrom(lhs2/rhs)
   #probability
    c.prob = lhs.prob * rhs.prob * _exprDB.prodFlow.Pabs

  def insert(c,e):
    if e.exprClass is not None:
      raise 'expr already belongs to an exprClass'
    c.exprs.insert(e)
    e.exprClass = c
   #probability
    if e.prob > c.MLrep.prob:
      c.MLrep = e

  #construction
  def __mul__(lhs,rhs):
    """functional application"""
    return exprDB.getAppExprClass(lhs,rhs)

  def __div__(lhs,rhs):
    """functional abstraction""" 
    return exprDB.getAbsExprClass(lhs,rhs)

  #equivalence
  def assumeDistinctFrom(lhs,rhs):
    lhs.distinctFrom.insert(rhs)
    rhs.distinctFrom.insert(lhs)

  def assumeEqualTo(lhs,rhs):
    lhs.__mergeInto(rhs)

  def __eq__(lhs,rhs):
    return lhs is rhs
    
  def __ne__(lhs,rhs):
    return rhs in lhs.distinctFrom

  #simplification
  def __mergeInto(old,new):
    """merges old class into new class and then merges neighbors"""
    raise LATER

  def __expand(c):
    """looks for simple expressions equivalent and near to
      existing expressions"""
    raise LATER

#==========[ expression class graph ]==========
class _ExprDB:
  """expression database, recording constructive equivalence
    and complexity measure"""
  def __init__(db):
   #exprs and classes
    db.exprs = _Set()
    db.exprClasses = _Set()
    db.varNames = {}
    db.constNames = {}
   #measure
    db.prodInit = None #initial production distribution
    db.prodFlow = None #production flow
    db.redInit = None #initial reduction distribution
    db.redFlow = None #reduction flow
  
  #Expr construction
  def getVarExpr(db,name):
    if name in db.varNames:
      var = db.varNames[name]
    else:
     #make new VarExpr
      varClass = db.getVarExprClass()
      var = VarExpr(name)
      varClass.insert(var)
      db.exprs.insert(var)
      db.varNames[name] = var
    return var

  def getConstExpr(db,name):
    if name in db.constNames:
      const = db.constNames[name]
    else:
     #make new VarExpr
      constClass = db.getVarExprClass()
      const = VarExpr(name)
      constClass.insert(const)
      db.exprs.insert(const)
      db.constNames[name] = const
    return const

  def getAppExpr(db,lhs,rhs):
    if rhs in lhs.Rapp:
      app = lhs.Rapp[rhs]
    else:
     #make new Expr
      appClass = db.getAppExprClass(lhs.exprClass,rhs.exprClass)
      app = AppExpr(lhs,rhs)
      appClass.insert(app)
      db.exprs.insert(app)
    return app
    
  def getAbsExpr(db,lhs,rhs):
    if not isinstance(lhs,VarExpr):
      raise TypeError, 'only variables can be abstracted'
    if rhs in lhs.Rabs:
      abs = lhs.Rabs[rhs]
    else:
     #make new Expr
      absClass = db.getAbsExprClass(lhs.exprClass,rhs.exprClass)
      abs = AbsExpr(lhs,rhs)
      absClass.insert(abs)
      db.exprs.insert(abs)
    return abs

  #ExprClass construction
  def getAtomExprClass(db,name):
    if name in db.varNames:
      atom = db.varNames[name].exprClass
    elif name in db.constNames:
      atom = db.constNames[name].exprClass
    else:
     #make new ExprClass
      atom = ExprClass('atom',name)
      db.exprClasses.insert(atom)
    return var

  def getAppExprClass(db,lhs,rhs):
    if rhs in lhs.Rapp:
      app = lhs.Rapp[rhs]
    else:
     #make new ExprClass
      app = ExprClass('app',lhs,rhs)
      db.exprClasses.insert(app)
    return app

  def getAbsExprClass(db,lhs,rhs):
    if not lhs.isVar:
      raise TypeError, 'only variable classes can be abstracted'
    if rhs in lhs.Rabs:
      abs = lhs.Rabs[rhs]
    else:
     #make new ExprClass
      abs = ExprClass('abs',lhs,rhs)
      db.exprClasses.insert(abs)
    return abs

  #measure calculations    
  def calcExprProbs(db,tol=1e-5):
    """calculates Expr probabilities using the PMF basis
      and the probabilities of application and abstraction"""
    for e in db.exprs:
      e.prob = None
    for e in db.exprs:
      e.calcProb()
  
  def calcExprClassProbs(db,tol=1e-8):
    """estimates ExprClass probabilities using the PMF prodFlow.basis
      and the probabilities of application and abstraction"""
   #integer parametrization
    C = db.exprClasses
    N = len(C)
    N2C = list(C)
    C2N = dict([(N2C[n],n) for n in range(N)])
   #initial distribution
    P0 = Mzeros(N)
    for c in db.prodFlow.basis:
      P0[C2N][c] = db.prodFlow.basis[c]
    P0 /= sum(P0)
   #try to find steady state distribution
    Pstay = (1-db.prodFlow.Papp-db.prodFlow.Pabs)
    res = 2*tol
    while res > tol:
      P1 = Pstay * P0
      for lhs in C:
        Plhs = P0[C2N[lhs]]
       #application
        for rhs in lhs.Rapp:
          Prhs = P0[C2N[rhs]]
          P1[C2N[lhs.Rapp[rhs]]] += db.prodFlow.Papp * Plhs * Prhs
       #abstraction
        for rhs in lhs.Rabs:
          Prhs = P0[C2N[rhs]]
          P1[C2N[lhs.Rapp[rhs]]] += db.prodFlow.Pabs * Plhs * Prhs
      P1 /= sum(P1) #ignores lost probability
      res = norm(P0-P1)
      P0 = P1
   #assign probabilities
    for n in range(N):
      N2C[n].prob = P0[n]

  #inverse measure calculations
  def calcEmpiricalMeasure(db):
    "finds the min relentropy measure WRT the database's 0-1 expr entries"
    raise LATER
    
  #simplification
  def calcMLreps(db):
    for c in db.exprClasses:
      MLrep = None
      MLprob = -1
      for e in c:
        if e.prob > MLprob:
          MLrep = e
          MLprob = e.prob
      c.MLrep = MLrep

_exprDB = _ExprDB()

#==========[ constants ]==========
class _ConstDB():
  """constant server"""
  raise LATER

Consts = _ConstDB

S = None #substitution
K = None #constant
#I = None #identity
#B = None #substitution
#C = None #substitution

R = None #random pair
L = None #lowering
U = None #raising
A = None #function application
C = None #random choice

T = None #true
F = None #false
Q = None #equals
P = None #probability evaluation

#==========[variables ]==========
class _VarDB:
  """variable server"""
  raise LATER

Vars = _VarDB()


