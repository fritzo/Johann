__doc__ = """Interpreter and tools for
a simple intensional stochastic lazy functional programming language

TODO:
  * add parser, pretty-printer (see parser.py)
  * add type checker
  * add execution paths, or at least subterm tagging
  * define perturbation objects for lower-bounding perturbed probabilites
    using perturbed exection paths

ISSUES:
  * Can evaluation and sampling really be identified as done here?
    * the problem is in sampling, say, head lists from streams...
"""

verbose = True

import random
from math import exp,log
from exceptions import OverflowError,Exception
from sys import argv

LATER = Exception("LATER")
WORKING = Exception("WORKING")

inf = float("inf")

def geomvariate (p):
  """
  implemented by sampling floor(expovariate):
    expo(t;mu) = exp(-t/mu)/mu
    int(expo(t;mu), t in [0,1] = 1-exp(-1/mu) =: 1-p
  whence
    p = exp(-1/mu)
    mu = -1/log(p)
  with entropy
    h(geom(-;p)) = -(1-p)log(1-p) + p(log(h(...)) - log(p))
                 = -log(1-p) + p log(p)/(1-p)
    p = ???
  """
  mu = -1/log(p)
  return int(floor(random.expovariate(1/mu)))

class PMF (dict):
  "multinomial distribution-as-dict"
  def __str__ (self):
    result = "PMF({\n"
    items = list(self.iteritems())
    items.sort(key = lambda (_,y): -y)
    for keyval in items:
      result += "  %s : %s,\n" % keyval
    return result + "})"
  def __missing__ (self, key): return 0.0
  def normalize (self):
    total = sum(self.values(), 0.0)
    assert total > 0, ValueError("zero mass in normalize")
    for key in self.keys():
      self[key] /= total
  def sample (self):
    total = 0.0
    intervals = []
    for key,val in self.iteritems():
      total += val
      intervals.append((key,total))
    assert total > 0, ValueError("zero mass in sample")
    point = random.uniform(0,total)
    for key,val in intervals:
      if point < val:
        return key
    
  def __rmul__ (self, scale):
    return PMF([(key, scale*val) for key,val in self.iteritems()])
  def __add__ (self, other):
    result = PMF(self)
    for key,val in other.iteritems():
      result[key] += val
    return result
  def likelihood (self, other):
    "Kullback-Liebler likelihood"
    try:
      return exp(sum([val * log(other[key]) for key,val in self.iteritems()]))
    except OverflowError:
      return 0

def choose2 (n):
  for k in range(n+1):
    yield k, n-k
def choose3 (n):
  for k1 in range(n+1):
    for k2 in range(n-k1+1):
      yield k1, k2, n-k1-k2

#abstract terms
STEPS = 100
class Term:
  def __str__ (self):
    try: return self.__str
    except AttributeError:
      self.__str = self.str()
      return self.__str
  def __repr__ (self): return str(self)
  def __hash__ (self): return hash(str(self))
  def __eq__ (self, other): return str(self) == str(other)
  def __ne__ (self, other): return str(self) != str(other)
  def __call__ (self, arg): return App(self, arg)
  def subs (self, var, term): return self
  def hist (self, samples=1000, steps=100):
    "generate historgam via monte carlo sampling"
    histogram = PMF()
    for _ in range(samples):
      usedsteps,sample = self.sample(steps)
      if usedsteps > 0: histogram[sample] += 1
    histogram.normalize()
    return histogram
  def perturb (self, dist):
    "yields perturbations with EXACTLY dist changes"
    if dist == 0: yield self
  def perturbs (self, dist):
    "yields perturbations with AT MOST dist changes"
    for d in range(dist + 1):
      for term in self.perturb(d):
        yield term
  def random ():
    global signature
    signature.choose().random()

numvars = 0
class Var (Term):
  """
  x var
  ------ term-var
  x term
  """
  def __init__ (self, name=None):
    if name is None:
      global numvars
      name = "x%i" % numvars
      numvars += 1
    else:
      assert isinstance(name, str), TypeError("bad variable name")
    self.name = name
  def copy (self): return Var(self.name)
  def str (self): return self.name
  def __contains__ (self, var): return self == var
  def subs (self, var, term):
    if var == self: return term
    else:           return self
  def sample (self, steps=STEPS, lazy=False):
    """
    ---- val-var
    x=>x
    """
    return steps,self
  def denotes (self):
    "[x] = x"
    return self
  def random ():
    p = 0.5 #arbitrary
    return Var("r%i") % int(geomvariate(p))
x = Var("x")
y = Var("y")
z = Var("z")
f = Var("f")
g = Var("g")
_ = Var("_")

def ensure_term (t):
  if isinstance(t, str): return Var(t)
  assert isinstance(t, Term), TypeError("expected Term, got %s" % t.__class__)
  return t
def ensure_var (v):
  if isinstance(v, str): return Var(v)
  assert isinstance(v, Var), TypeError("expected Var, got %s" % v.__class__)
  return v

#functions
class App (Term):
  """
  e:a->b   e':a
  ------------- term-app
     e(e'):b
  """
  def __init__ (self, fun, arg):
    self.fun = ensure_term(fun)
    self.arg = ensure_term(arg)
  def str (self): return "%s(%s)" % (self.fun, self.arg)
  def __contains__ (self, var): return var in self.fun or var in self.arg
  def subs (self, var, term):
    return App(self.fun.subs(var,term),
               self.arg.subs(var,term))
  def sample (self, steps=STEPS, lazy=False):
    """
    e1=>Abs(x,e1')   [x:=e2]e1'=>e1''
    --------------------------------- red-app
             e1(e2) ~-> e1''
    """
    if steps == 0: return 0,self
    steps,fun = self.fun.sample(steps,lazy=True)
    if isinstance(fun, Abs):
      if verbose: print "  red-app"
      return fun.term.subs(fun.var, self.arg).sample(steps - 1)
    return steps,App(fun, arg)
  def perturb (self, dist):
    if dist == 0: yield self
    else:
      for d1,d2 in choose2(dist):
        for fun in self.fun.perturb(d1):
          for arg in self.arg.perturb(d2):
            yield App(fun,arg)
  def denotes (self):
    "[e1(e2)] = [e1]([e2])"
    return self.fun.denotes()( self.arg.denotes() )
  def random ():
    fun = Term.random()
    arg = Term.random()
    return App(fun,arg)

class Abs (Term):
  """
     x:a|-e:b
  ------------- term-abs
  Abs(x,e):a->b
  """
  def __init__ (self, var, term):
    self.var = ensure_var(var)
    self.term = ensure_term(term)
  def str (self): return "Abs(%s, %s)" % (self.var, self.term)
  def __contains__ (self, var): return var != self.var and var in self.term
  def subs (self, var, term):
    if self.var == var: return self
    if self.var in term:
      fresh = Var()
      return Abs(fresh, self.term.subs(self.var, fresh).subs(var, term))
    return Abs(self.var, self.term.subs(var, term))
  def sample (self, steps=STEPS, lazy=False):
    """
    ------------------ red-abs-lazy
    Abs(x,e)=>Abs(x,e)

            e=>e'
    ------------------- red-abs-eager
    Abs(x,e)=>Abs(x,e')
    """
    if lazy: return steps,self
    if steps == 0: return 0,self
    steps,term = self.term.sample(steps)
    return Abs(self.var,term)
  def perturb (self, dist):
    if dist == 0: yield self
    else:
      for term in self.term.perturb(dist):
        yield Abs(self.var, term)
  def denotes (self):
    "[Abs(x,e1,e2)] = Abs(x,[e1],[e2])"
    return Abs(self.var, self.term.denotes())
  def random ():
    var = Var.random()
    term = Term.random()
    return Abs(var,term)
Y = Abs(y, Abs(x, y(x(x)))
         ( Abs(x, y(x(x))) ))

class Fix (Term):
  """
   x:a|-e:a
  ---------- term-fix
  Fix(x,e):a
  """
  def __init__ (self, var, term):
    self.var = ensure_var(var)
    self.term = ensure_term(term)
  def str (self): return "Fix(%s, %s)" % (self.var, self.term)
  def __contains__ (self, var): return var != self.var and var in self.term
  def subs (self, var, term):
    if self.var == var: return self
    if self.var in term:
      fresh = Var()
      return Fix(fresh, self.term.subs(self.var, fresh).subs(var, term))
    return Fix(self.var, self.term.subs(var, term))
  def sample (self, steps=STEPS, lazy=False):
    """
    [x:=Fix x.e]e=>e'
    ----------------- red-fix
       Fix x.e=>e'
    """
    if steps == 0: return 0,self
    if verbose: print "  red-fix"
    return self.term.subs(self.var, self).sample(steps - 1, lazy)
  def perturb (self, dist):
    if dist == 0: yield self
    else:
      for term in self.term.perturb(dist):
        yield Fix(self.var, term)
  def denotes (self):
    "[Fix(x,e)] = Y(Abs(x,[e]))"
    return Y(Abs(self.var, self.term.denotes()))
  def random ():
    var = Var.random()
    term = Term.random()
    return Abs(var,term)
id = Abs(x,x)

#sampling monad
class Sgtn (Term):
  """
        e:a
  --------------- term-sgtn
  Sgtn(e):Rand(a)
  """
  def __init__ (self, term): self.term = ensure_term(term)
  def str (self): return "Sgtn(%s)" % self.term
  def __contains__ (self, var): return var in self.term
  def subs (self, var, term): return Sgtn(self.term.subs(var,term))
  def sample (self, steps=STEPS, lazy=False):
    """
    ---------------- val-sgtn
    Sgtn(e)=>Sgtn(e)
    """
    return steps,self
  def perturb (self, dist):
    if dist == 0: yield self
    else:
      for term in self.term.perturb(dist):
        yield Sgtn(term)
  def denotes (self):
    "[Sgtn(e)] = Abs(f,f([e]))"
    term = self.term.denotes()
    return Abs(f, f(term))
  def random (): return Sgtn(Term.random())

class Rand (Term):
  """
  e1:Rand(a)   e2:Rand(a)
  ----------------------- term-rand
    Rand(e1,e2):Rand(a)
  """
  def __init__ (self, lhs, rhs):
    self.lhs = ensure_term(lhs)
    self.rhs = ensure_term(rhs)
  def str (self): return "Rand(%s, %s)" % (self.lhs, self.rhs)
  def __contains__ (self, var): return var in self.lhs or var in self.rhs
  def subs (self, var, term):
    return Rand(self.lhs.subs(var,term),
                self.rhs.subs(var,term))
  def sample (self, steps=STEPS, lazy=False):
    """
         e1=>e1'
    ---------------- red-rand-lhs @ 1/2
    Rand(e1,e2)=>e1'

         e2=>e2'
    ---------------- red-rand-rhs @ 1/2
    Rand(e1,e2)=>e2'
    """
    if steps == 0: return 0,self
    if verbose: print "  red-rand"
    return random.choice([self.lhs,self.rhs]).sample(steps - 1)
  def perturb(self, dist):
    if dist == 0: yield self
    else:
      for d1,d2 in choose2(dist):
        for lhs in self.lhs.perturb(d1):
          for rhs in self.rhs.perturb(d2):
            yield Rand(lhs,rhs)
      for lhs in self.lhs.perturb(dist - 1): yield lhs  #R --> K
      for rhs in self.rhs.perturb(dist - 1): yield rhs  #R --> F
      for d1,d2 in choose2(dist - 1):
        for lhs in self.lhs.perturb(d1):
          for rhs in self.rhs.perturb(d2):
            yield Rand31(lhs,rhs)                       #R --> R31
            yield Rand13(lhs,rhs)                       #R --> R13
  def denotes (self):
    "[Rand(e1,e2)] = Rand([e1],[e2])"
    lhs = self.lhs.denotes()
    rhs = self.rhs.denotes()
    return Rand(lhs, rhs)
  def random ():
    lhs = Term.random()
    rhs = Term.random()
    return Rand(lhs,rhs)

class Samp (Term):
  """
  x var   e term   e' term
  ------------------------ samp-term
      Samp(x,e,e') term
  """
  def __init__ (self, var, defn, term):
    self.var = ensure_var(var)
    self.defn = ensure_term(defn)
    self.term = ensure_term(term)
  def str (self): return "Samp(%s, %s, %s)" % (self.var, self.defn, self.term)
  def __contains__ (self, var):
    return (  (var != self.var and var in self.term)
           or (var in self.defn and self.var in self.term) )
  def subs (self, var, term):
    defn = self.defn.subs(var, term)
    if self.var == var:
      return Samp(self.var, defn, self.term)
    myvar,myterm = self.var,self.term
    if myvar in term:
      myvar = Var()
      myterm = myterm.subs(self.var, myvar)
    return Samp(myvar, defn, myterm.subs(var, term))
  def sample (self, steps=STEPS, lazy=False):
    """
    e1=>e1'   [x:=e1']e2=>e2'
    ------------------------- red-samp
        Samp(x,e1,e2)=>e2'
    """
    if steps == 0: return 0,self
    if verbose: print "  red-samp"
    steps,defn = self.defn.sample(steps)
    if steps > 0: return defn(Abs(self.var, self.term)).sample(steps - 1)
    else:         return 0,Samp(self.var, defn, self.term)
  def denotes (self):
    "[Samp(x,e1,e2)] = [e1](Abs(x,[e2]))"
    defn = self.defn.denotes()
    term = self.term.denotes()
    return defn(Abs(self.var, term))
  def random ():
    var = Var.random()
    defn = Term.random()
    term = Term.random()
    return Samp(var,defn,term)

def Up (x): return Abs(f, f(x))
def Down (rrx): return rrx(id)
def Lift (f): return Sgtn(Abs(y, Sgtn(f(y))))

def Def (var, defn, term):
  "lazy substitution"
  return Abs(var, term)(defn)
def Let (var, defn, term):
  "eager substitution"
  return Samp(var, Sgtn(defn), term) #XXX Sgtn(-) prevents evaluation

def DefRec (var, defn, term):
  return Def(var, Fix(var, defn), term)
def LetRec (var, defn, term):
  return Let(var, Fix(var, defn), term)

#finite products
class Unit (Term):
  """
  --------- term-unit
  unit:Unit
  """
  def __str__ (self): return "unit"
  def __contains__ (self, var): return False
  def subs (self, var, term): return self
  def sample (self, steps=STEPS, lazy=False):
    """
    ---------- val-unit
    unit=>unit
    """
    return steps,self
  def pertrub (self, dist):
    if dist == 0:
      yield self
  def denotes (self):
    "[unit] = Abs(x,x)"
    return id
  def random (): return Unit()
unit = Unit()

class Pair (Term):
  """
  e1 term   e2 term
  ----------------- term-pair
  Pair(e1,e2) term
  """
  def __init__ (self, lhs, rhs):
    self.lhs = ensure_term(lhs)
    self.rhs = ensure_term(rhs)
  def str (self): return "Pair(%s, %s)" % (self.lhs, self.rhs)
  def __contains__ (self, var):
    return var in self.lhs or var in self.rhs
  def subs (self, var, term):
    return Pair(self.lhs.subs(var,term),
                self.rhs.subs(var,term))
  def sample (self, steps=STEPS, lazy=False):
    """
    ---------------------- val-pair
    Pair(e,e')=>Pair(e,e')
    """
    return steps,self
  def perturb (self, dist):
    if dist == 0: yield self
    else:
      for d1,d2 in choose2(dist):
        for lhs in self.lhs.perturb(d1):
          for rhs in self.rhs.perturb(d2):
            yield Pair(lhs,rhs)
  def denotes (self):
    "[Pair(e1,e2)] = Abs(f,f([e1])([e2]))"
    lhs = self.lhs.denotes()
    rhs = self.rhs.denotes()
    return Abs(x, x(lhs)(rhs))
  def random ():
    lhs = Term.random()
    rhs = Term.random()
    return Pair(lhs,rhs)

class Proj_ (Term):
  """
  """
  def __init__ (self, prod): self.prod = ensure_term(prod)
  def __contains__ (self, var): return self.prod.contains(var)
  def subs (self, var, term): return self.__class__(self.prod.subs(var,term))
  def perturb (self, dist):
    if dist == 0: yield self
    else:
      for prod in self.prod.perturb(dist):
        yield self.__class__(prod)

class Proj1 (Proj_):
  """
  e:Prod(a,b)
  ----------- term-proj1
  Proj1(e):a
  """
  def __str__ (self): return "Proj1(%s)" % self.prod
  def sample (self, steps=STEPS, lazy=False):
    """
    e=>Pair(e1,e2)   e1=>e1'
    ------------------------ red-proj1
          Proj1(e)=>e1'
    """
    if steps == 0: return 0,self
    steps,prod = self.prod.sample(steps)
    if steps > 0 and isinstance(prod, Pair):
      if verbose: print "  red-proj1"
      return prod.lhs.sample(steps - 1)
    return steps,Proj1(prod)
  def denotes (self):
    "[Proj1(e)] = [e](Abs(x,Abs(_,x)))"
    prod = self.prod.denotes()
    return prod(Abs(x, Abs(_, x)))
  def random (): return Proj1(Term.random())

class Proj2 (Proj_):
  """
  e:Prod(a,b)
  ----------- term-proj2
  Proj2(e):b
  """
  def __str__ (self): return "Proj2(%s)" % self.prod
  def sample (self, steps=STEPS, lazy=False):
    """
    e=><e1,e2>   e2=>e2'
    ------------------- red-proj2
       Proj2(e)=>e2'
    """
    if steps == 0: return 0,self
    steps,prod = self.prod.sample(steps)
    if steps > 0 and isinstance(prod, Pair):
      if verbose: print "  red-proj1"
      return prod.rhs.sample(steps - 1)
    return steps,Proj2(prod)
  def denotes (self):
    "[Proj2(e)] = [e](Abs(_,Abs(y,y)))"
    prod = self.prod.denotes()
    return prod(Abs(_, Abs(y, y)))
  def random (): return Proj2(Term.random())

#finite nonempty sums
class In_ (Term):
  def __init__ (self, val): self.val = ensure_term(val)
  def __contains__ (self, var): return var in self.val
  def subs (self, var, term): return self.__class__(self.val.subs(var, term))
  def perturb (self, dist):
    for val in self.val.perturb(dist):
      yield self.__class__(val)
class In1 (In_):
  """
    e term
  ----------- term-in1
  In1(e) term
  """
  which = True
  def str (self): return "In1(%s)" % self.val
  def sample (self, steps=STEPS, lazy=False):
    """
    -------------- val-in1
    In1(e)=>In1(e)
    """
    return steps,self
  def denotes (self):
    "[In1(e)] = Abs(f,Abs(_,f([e])))"
    val = self.val.denotes()
    return Abs(f,Abs(_,f(val)))
  def random(): return In1(Term.random())
class In2 (In_):
  """
    e term
  ----------- term-in2
  In2(e) term
  """
  wich = False
  def str (self): return "In2(%s)" % self.val
  def sample (self, steps=STEPS, lazy=False):
    """
    -------------- val-in2
    In2(e)=>In2(e)
    """
    return steps,self
  def denotes (self):
    "[In2(e)] = Abs(_,Abs(g,g([e])))"
    val = self.val.denotes()
    return Abs(_,Abs(g,g(val)))
  def random(): return In2(Term.random())

class Case (Term):
  """
  e term   e1 term   e2 term
  -------------------------- term-case
       Case(e,e1,e2) term
  """
  def __init__ (self, sum, if1, if2):
    self.sum = ensure_term(sum)
    self.if1 = ensure_term(if1)
    self.if2 = ensure_term(if2)
  def str (self):
    return "Case(%s, %s, %s)" % (self.sum, self.if1, self.if2)
  def __contains__ (self, var):
    return var in self.sum or var in self.if1 or var in self.if2
  def subs (self, var, term):
    return Case(self.sum.subs(var,term),
                self.if1.subs(var,term),
                self.if2.subs(var,term))
  def sample (self, steps=STEPS, lazy=False):
    """
    e=>In1(e')   e1 e'=>e1'
    ----------------------- red-case-1
       Case(e,e1,e2)=>e1'

    e=>In2(e')   e2 e'=>e2'
    ----------------------- red-case-2
       Case(e,e1,e2)=>e2'

          e=>x        x var
    ---------------------------- red-case-var
    Case(e,e1,e2)=>Case(x,e1,e2)
    """
    if steps == 0: return 0,self
    steps,sum = self.sum.sample(steps)
    if isinstance(sum, In1):
      if verbose: print "  red-case-1"
      return self.if1(sum.val).sample(steps - 1)
    if isinstance(sum, In2):
      if verbose: print "  red-case-2"
      return self.if2(sum.val).sample(steps - 1)
    #lazy cases
    #steps,if1 = self.if1.sample(steps)
    #steps,if2 = self.if2.sample(steps)
    return steps,Case(sum, if1, if2)
  def perturb (self, dist):
    if dist == 0: yield self
    else:
      for d1,d2,d3 in choose3(dist):
        for sum in self.sum.perturb(d1):
          for if1 in self.if1.perturb(d2):
            for if2 in self.if2.perturb(d3):
              yield Case(sum, if1, if2)
  def denotes (self):
    "[In_(e1,e2)] = Abs(f,f([e1])([e2]))"
    sum = self.sum.denotes()
    if1 = self.if1.denotes()
    if2 = self.if2.denotes()
    return sum(if1)(if2)
  def random():
    sum = Term.random()
    if1 = Term.random()
    if2 = Term.random()
    return Case(sum,inf1,if2)

#bool = In_ unit unit
true = In1(unit)
false = In2(unit)
def If (test, iftrue, iffalse):
  return Case(test, Abs("u", iftrue), Abs("u", iffalse))
def Not (u): return If(u, false, true)
def Eq_bool (u,v): return If(u, v, Not(v))

#nat = mu a. In_ unit a
zero = In1(unit)
def Succ (pred):
  return In2(pred)
one = Succ(zero)
two = Succ(one)
three = Succ(two)
def Pred (num, ifzero, ifsucc):
  return Case(num, Abs("u", ifzero), ifsucc)
def Iszero(num):
  return Pred(num, true, Abs(_, false))

#lists
def Cons (head, tail): return Pair(head, tail)
nil = unit

#streams
def ConstStream (val):
  return Fix(x, Pair(val, x))
def Evolve (start, next):
  return Fix(x, Abs(y, Pair(y, x(next(y)))))(start)
Heads = (
    #returns list of first n heads
    Fix("heads",
    Abs("stream",
    Abs("num",
      Pred("num",
           nil,
           Abs("pred", Pair(Proj1("stream"),
                            App("heads", Proj2("stream")) ("pred")
                       )
           )
      )
    )))
  )

#pmfs
def Rand31 (lhs, rhs):
  return Def(x, lhs, Rand(x, Rand(x, rhs)))
def Rand13 (lhs, rhs):
  return Def(x, rhs, Rand(Rand(lhs, x), x))
R11 = Abs(x, Abs(y, Rand(x,y)))
R31 = Abs(x, Abs(y, Rand31(x,y)))
R13 = Abs(x, Abs(y, Rand13(x,y)))

def Expovariate (p):
  return Fix(x, Abs(z, p (Sgtn(z)) (x(Succ(z))) )) (zero)
def RandStream (p):
  raise LATER
  #XXX: this is impossible to define without join
  return Fix(f,
         p(Abs(x,
         f(Abs(y,
         Sgtn(Cons(x,y))
         )))))          # = Bot :(

#language tools
signature = [
    App,Var,Abs,Fix,  #exponentials   (-,lazy,lazy,lazy) ???
    Rand,Sgtn,Samp,   #random         (lazy)
    Unit,             #nullary prods  (-)
    Pair,Proj1,Proj2, #binary prods   (lazy,eager,eager)
    In1,In2,Case,     #sums           (eager,eager,eager)
    ]

#documentation
def align_at (lines, symb, column):
  "aligns lines so that symb appears in given column"
  def align (line):
    pos = line.index(symb)
    shift = max(0, column - pos)
    return " "*shift + line
  return "\n".join([ align(line) for line in lines.split("\n") ])
def help_term ():
  return "".join([term.__doc__ for term in signature])
def help_red ():
  return "".join([term.sample.__doc__ for term in signature])
def help_denote():
  message = "\n".join([term.denotes.__doc__ for term in signature])
  return align_at(message, "=", 20)

#learning
def learn_static (prior, age, observe, data,
                  dist=1, samples=1000, verbose=False):
  """bayes learning step, perturbing a hypothesis WRT data
  no complexity term yet.
  INPUTS:
    prior - the prior R-Sgtn-pmf
    age - the extent to which the prior matters
    observe - the stochastic observation function
    data - an R-Sgtn-pmf of observed data
  PARAMS: dist, samples, verbose
  OUTPUTS: perturbed hypothesis
  """
  #get training histogram
  if age == 0:
    hist = Down(data).hist(samples)
  else:
    prior_hist = Down(observe(prior)).hist(age * samples)
    data_hist = Down(data).hist(samples)
    hist = age * prior_hist + data_hist

  #evaluate posterior probabilitites
  hyps = prior.perturbs(dist)
  def likelihood (h):
    return hist.likelihood( Down(observe(h)).hist(samples) )
  posterior = PMF([(h, likelihood(h)) for h in hyps])
  posterior.normalize()
  if verbose: print "sampling from %s" % posterior

  #randomly jump to alternative hypothesis
  return posterior.sample()

def learn_dynamic (prior, age, next, observe, data,
                   hipness=R11, dist=1, samples=1000, verbose=False):
  characterize = LATER
  raise LATER
  WORKING

#tests
def sample_test (term, maxsteps=100):
  print term
  steps,sample = term.sample(maxsteps)
  print " --%i-->" % (maxsteps - steps)
  print sample

def hist_test (term): print "term = %s" % term.hist()

def perturb_test (term, dist=1):
  print "%s ~~>" % term
  for perturbed in term.perturb(dist):
    print perturbed

def red_seq (term, maxsteps=20):
  seq = [term]
  for steps in range(1,1+maxsteps):
    remaining,red = term.sample(steps)
    if remaining > 0: break
    seq.append(red)
  return seq
def print_red_seq (term):
  seq = red_seq(term)
  print seq[0]
  for term in seq[1:]:
    print "==>"
    print term

#unit tests
class TestError (Exception): pass
def asserteq (ans, truth):
  if ans != truth:
    raise TestError("expected %s,\n  got %s" % (truth,ans))

def test_Let (): #XXX this fails
  print "Testing Let(-,-,-)"
  term = Let(x,id(id),Abs(z,x))
  _,ans = term.sample()
  truth = Abs(z,id)
  asserteq(ans,truth)

def test_Let2 (): #XXX this fails
  print "Testing Let(-,-,-)"
  three_units = DefRec("units",
                  Abs("num",
                  Pred("num", nil, Abs("pred",
                  Let("tail", App("units", "pred"),
                  Cons(unit, "tail")
                  )))),
                App("units", three))

  if True: print_red_seq(three_units) #DEBUG

  _,ans = three_units.sample()
  truth = Cons(unit, Cons(unit, Cons(unit, nil)))
  asserteq( ans, truth )
  raise WORKING
  r"""What should happen:
  u := (
      Fix u.
      \n. pred n <> \n'.
      let tail = u n' in <unit,tail>
  ).
  u 3
  => (Fix u. \n. pred n <> \n'. let tail = u n' in <unit,tail>) 3   #app
  => (\n. pred n <> \n'. let tail = (...) n' in <unit,tail>) 3      #fix
  => pred 3 <> \n'. let tail = (...) n' in <unit,tail>              #case
  => (\n'. let tail = (...) n' in <unit,tail>) 2                    #app
  => let tail = (...) 2 in <unit,tail>)                             #samp
  ... # app,fix,case,app,samp,
  ... # app,fix,case,app,samp,
  ... # app,fix,case
  """

def test_Lam ():
  print "Testing Abs(-,-)"
  p1 = Rand13(true, false)
  p2 = Rand31(true, false)
  partial = Rand(Sgtn(p1), Sgtn(p2))
  ans = set(partial.hist().keys())
  truth = set([Sgtn(p1), Sgtn(p2)])
  asserteq( ans, truth )

def test_Pair ():
  print "Testing Pair(-,-)"
  p = Rand(Sgtn(true), Sgtn(false))
  #test for laziness
  pp1 = Def("p", p,
        p(Abs("u",
        p(Abs("v",
        Pair("u","v")
        )))))
  _,ans = pp1.sample()
  print ans
  #test for eagerness
  pp2 = Def("p", p,
        Samp("u", "p",
        Samp("v", "p",
        Pair("u","v")
        )))
  _,ans = pp2.sample()
  print ans

def test_denotes ():
  print "Testing denotes()"
  #this should denote Bot
  bern_stream = Def("p", Rand(Sgtn(true), Sgtn(false)),
                Fix("ps",
                Samp("x", "p",
                Samp("xs", "ps",
                Sgtn(Cons("x","xs"))
                ))))
  print bern_stream.denotes()

def unit_tests ():
  test_Let()
  test_Lam()
  test_Pair()

#misc tests
def test1 ():
  sample_test(Iszero(two))

def test2 (trials = 20):
  rand_number = Rand(Rand(zero,one),two)
  for _ in range(trials):
    sample_test(Iszero(rand_number))

def test3 ():
  p = Rand31(Rand(zero,one),two)
  hist_test(p)

def test4 ():
  p = Rand(Sgtn(true),Sgtn(false))(Abs(x, Sgtn(Pair(x,x))))
  f = Abs(x, Eq_bool(Proj1(x), Proj2(x)))
  hist_test(p(f))

def test5 ():
  p = Rand(Rand(Sgtn(zero), Sgtn(one)), Sgtn(two))(
        Abs(x, Rand(Pair(x,one),Pair(x,x)))
      )
  perturb_test(p)

def test6 ():
  print "testing expovariate distribution"
  hist_test(Down(Expovariate(R11)))

def test7 ():
  print "testing static learning"
  prior = Rand13(Sgtn(true), Sgtn(false))
  print "prior = %s" % prior
  print "prior hist = %s" % prior.hist()
  print "----"
  age = 0
  observe = id
  data = Rand31(Sgtn(true), Sgtn(false))
  post = learn_static(prior, age, observe, data, verbose = True)
  print "----"
  print "post = %s" % post
  print "post hist = %s" % post.hist()

def test8 ():
  "testing getting the head of a random stream"
  bern = Rand(Sgtn(true), Sgtn(false))
  bern_stream = Fix("s", Pair(bern,s))
  h = Var("h")
  xs = Var("xs")
  ys = Var("ys")
  n = Var("n")
  n2 = Var("n'")
  head = Fix(h,
         Abs(xs,
         Abs(n,
         Pred(n, nil,
         Abs(n2,
         Samp(y, Proj1(xs),
         Let(ys, h (Proj2(xs)) (n2),
         Cons(y, ys)
         )))))))

def test9 ():
  #XXX learning with infinite support will not work until lower-bounds
  #  of probabilities can be generated from perturbed execution paths 
  print "testing static learning with infinite support"
  prior = Expovariate(R11)
  print "prior = %s" % prior
  age = 0
  observe = id
  data = Expovariate(R13)
  post = learn_static(prior, age, observe, data, verbose = True)
  print "post = %s" % post

#command-line interface
commands = """Commands:
    help - prints this message
    term - prints term formation rules
     red - prints random reductoin rules
  denote - prints translation to pure lambda-R calculus
    test - all unit tests"""
if __name__ == "__main__":
  argv = argv[1:]
  if not argv or argv == ['']:
    print commands
  for arg in argv:
    if   arg  == "help":    print commands
    elif arg == "term":     print help_term()
    elif arg == "red":      print help_red()
    elif arg == "denote":   print help_denote()
    elif arg == "test":     unit_tests()
    else:
      print "unknown command: " + arg
      print commands

