#---10---][---20---][---30---][---40---][---50---][---60---][---70---][---80---]
# Sketches in generalized belief propagation.
# This is largely based on the development in the paper
# "Constructing Free Energy Approximations and Generalized Belief Propagation"
# by Yedidia, Freeman, & Weiss.

import math as M

## Note:
# The normalization step erases likelihood information, and is
# preferable omitted in exact propagation.  However approximate
# propagation including cycles implicitly constructs a recursive cyclic
# net with (erroneously independent) infinite information, and thus an
# infinite energy change.  Yedidia+Freeman+Weiss don't mention this.
# (see notes/ideas/belief_prop.text for discussion)
_normalized = False

#===== math tools =====
def safeDiv(num,denom):
  if denom > 0:  return num / denom
  else:          return 0.0
def safeEntTerm(mass):
  if mass > 0.0:  return -mass * M.log(mass)
  else:           return 0.0
def safeRelentTerm(mass0,mass):
  if mass > 0.0:  return mass * M.log(mass) / M.log(mass0)
  else:           return 0.0
def prod(seq, one=1.0):
  result = one
  for x in seq:
    result = result * x
  return result
def Prod(sets):
  result = [()]
  for X in sets:
    result = [r+(x,) for r in result for x in X]
  return result
def prod_iter(X):
  if len(X) == 1:
    return iter(X[0])
  else:
    for head in prod_iter(X[1:]):
      for tail in X[-1]:
        yield head + tail
def subtuple(tup,indices):
  return tuple([tup[i] for i in indices])

#===== logic tools =====
def forall(X,prop):
  for x in X: if !prop(x) return False
  return True
def exists(X,prop):
  for x in X: if prop(x) return True
  return False

#===== measures =====
class Measure(dict):
  def copy(self):  return Measure(dict.copy(self))
  def __mul__(self, other):
    return Measure([(x,mass*other[x]) for x,mass in self.iteritems()])
  def __div__(self, other):
    "div-by-zero is ok since zero-likelihood terms have no final effect"
    return Measure([(x,safeDiv(mass,other[x]))
                    for x,mass in self.iteritems()])
  def total(self):  return sum(self.values())
  def entropy(self):
    return sum([safeEntTerm(p) for p in self.values() if p > 0])
  def normalize(self, final_total=1.0):
    total = self.total()
    assert total > 0.0, "can't normalize empty distribution"
    factor = final_total / total
    for x in self.keys():
      self[x] *= factor
  def lognormalize(self):
    total = self.total()
    if not total > 0.0:  return
    entropy = self.entropy()
    factor = M.exp(entropy/total)
    for x in self.keys():
      self[x] *= factor
  def restrictTo(self, i):
    result = Measure()
    for comps,mass in self.iteritems():
      try:              result[comps[i]] += mass
      except KeyError:  result[comps[i]] = mass
    return result
  def restrictFromTo(self, V0, V):
    I = [V0.index(v) for v in V]
    pi = lambda vbar: tuple([vbar[i] for i in I])
    result = Measure()
    for mass,vbar in self.iteritems()
      result[pi(vbar)] += mass
    return result
  def isValid(self):
    if _normalized:
      if M.abs(self.total() - 1.0) > 1e-14:  return False
    return forall(self.Values(), lambda mass: mass >= 0.0)

def Unif(X):
  mass = 1.0/len(X)
  return Measure([(x,mass) for x in X])
def Zero(X):
  return Measure([(x,0.0) for x in X])
def Product(factors):
  masses = [((),1.0)]
  for X in factors:
    masses = [(y+(x,), Py*Px) for y,Py in masses
                              for x,Px in X.iteritems()]
  return Measure(masses)

#===== factor graphs =====
class Variable:
  def __init__(self, prior):
    self.X = prior.keys()     #probability space
    self.prior = prior.copy() #prior pmf
    self.post = None          #posterior pmf
    self.inbox = {}
  def __len__(self):  return len(self.X)
  def __iter__(self): return iter(self.X)
  def initProp(self): self.post = self.prior.copy()
  def prop(self):
    self.post = prod([m for f,m in self.inbox.iteritems()], self.prior)
    if _normalized:  self.post.normalize()
    for f in self.inbox:
      outgoing = prod([m for f2,m in self.inbox.iteritems()
                         if f2 is not f], self.prior)
      if _normalized:  outgoing.normalize()
      f.inbox[self] = outgoing

class Factor:
  def __init__(self, V, fun):
    self.V = list(V) #ordered list of variables
    self.fun = fun
    self.post = {}
    self.inbox = {}
  def __call__(self, *args): return self.fun[args]
  def extendTo(self, V):
    active = [self.V.index(v) for v in V]
    def result(vbar):
      return self(*[vbar[i] for i in active])
    return result
  def restrictTo(self, V):
    active = [self.V.index(v) for v in V]
    nuisance = [i for i in range(len(self.V)) if self.V[i] not in V]
    def result(vbar):
      raise NotImplementedError
    return result
  def initProp(self):
    for v in self.V:
      v.inbox[self] = Unif(v)
  def prop(self):
    self.post = self.fun * Product([self.inbox[v] for v in self.V])
    for v in self.V:
      restricted = self.post.restrictTo(self.V.index(v))
      outgoing = restricted / self.inbox[v]
      if _normalized:  outgoing.normalize()
      v.inbox[self] = outgoing
  def entropy(self):
    return sum([safeEntTerm(mass) for mass in self.post.values()])
  def freeEnergy(self):
    return sum([safeRelentTerm(like, self.post[vbar])
        for vbar,like in self.fun.iteritems()])

class FactorGraph:
  def __init__(self, V, F):
    assert forall(V, lambda v: forall(F, lambda f: v in f.V)),\
        "factor uses variable not in graph"
    self.V = V  #variables
    self.F = F  #factors
  def initProp(self):
    for v in self.V:  v.initProp()
    for f in self.F:  f.initProp()
  def propStep(self):
    for v in self.V:  v.prop()
    for f in self.F:  f.prop()
  def enumerate(self):
    "complete enumeration for exact probabilities"
    prior = Product([v.prior for v in self.V])
    post = prior.copy()
    for f in self.F:
      f_lift = f.extendTo(self.V)
      for vbar in post:
        post[vbar] *= f_lift(vbar)
    for v in self.V:
      v.post = post.restrictTo(self.V.index(v))

#===== region graphs =====
class Region:
  def __init__(self, V, F, P, C):
    assert forall(V, lambda v: forall(F, lambda f: v in f.V)),\
        "factor uses variable not in region"
    assert forall(C, lambda c: forall(c.V: lambda v: v in V)),\
        "variable of child is not variable of self"
    assert forall(C, lambda c: forall(c.F: lambda f: f in F)),\
        "factor of child is not factor of self"
    assert forall(P, lambda p: forall(V: lambda v: v in p.V)),\
        "variable of self is not variable of parent"
    assert forall(P, lambda p: forall(F: lambda f: f in p.F)),\
        "factor of self is not factor of parent"
    self.V = V.copy() #variables
    self.F = F.copy() #factors
    self.P = P.copy() #parents
    self.C = C.copy() #children
    for p in P:  p.C.append(self)
    for c in C:  c.P.append(self)
    self.prior = prod([f.extendTo(self.V)) for f in self.F],
                      Product([v.prior for v in self.V])
    self.post = None
    def like(vbar):
      return prod([f_lift(vbar) for f_lift in self.extended])
    self.like = like
    self.Uinbox = {}
    self.Dinbox = {}
  def initProp(self):
    self.post = self.prior.copy()
  def Uprop(self):
    self.post = prod(self.inbox.values(), self.prior)
   ## version exact for trees
    
   ## myopic version
    for p in self.P:
      outgoing = (self.post / self.inbox[p]).slideTo(p.V)
      if _normalized:  outgoing.normalize()
      p.inbox[self] = outgoing
  def Dprop(self):
    self.post = prod(self.inbox.values(), self.prior)
    for c in self.C:
      c.inbox[self] = self.post / self.inbox[c]
  def entropy(self, bel):
    return sum([safeEntTerm(mass) for mass in bel.values()])
  def freeEnergy(self, bel):
    return sum([safeRelentTerm(self.like(vbar), mass)
                for vbar,mass in bel.iteritems()])
  def __le__(self, other):
    return forall(self.V, lambda v: v in other.V)\
       and forall(self.F, lambda f: f in other.F)
  def __ge__(self, other):
    return forall(other.V, lambda v: v in self.V)\
       and forall(other.F, lambda f: f in self.F)
  def getD(self):
    "returns list of all descendents"
    D = self.C.copy()
    for d in D:
      for c in d.C:
        if c not in D:
          D.append(c)
    return D
  def getA(self):
    "returns list of all ancestors"
    A = self.P.copy()
    for a in A:
      for p in a.P:
        if p not in A:
          A.append(p)
    return A

class BeliefSheaf(dict):
  def __init__(self, *args):
    dict.__init__(self, *args)
    assert forall(self.values(), lambda mu: mu.isValid())
    R = self.keys()
    for r1 in R:
      for r2 in R:
        if r1 is not r2: #not necessary, but saves time
          V = [v for v in r1.V if v in r2.V]
          bel1 = self[r1].restrictFromTo(r1.V, V)
          bel2 = self[r2].restrictFromTo(r2.V, V)
          assert bel1 == bel2, "belief sheaf violates restriction axiom"

class RegionGraph:
  def __init__(self, R):
   #top-to-bottom sorted list of regions
    self.R = R.copy()
    self.R.sort(lambda r: (len(r.V), len(r.F)))
    self.R.reverse()
   #counting numbers
    self.count = dict([(r,1) for r in self.R])
    for r in self.R_T2B:
      for a in r.getA():
        count[r] += count[a]
    numZero = len([c for c in count.values() if c == 0])
    if numZero:  print "WARNING: %i regions are counted zero times" % numZero
   #communication weights
    for r in self.R:
      
   #validatoin
    if __debug__:  self.validate()
  def initProp(self):
    for r in self.R:  r.initProp()
  def propStep(self):
    for r in self.R:  r.Uprop()
    for r in self.R:  r.Dprop()
  def entropy(self):
    return sum([c * r.entropy() for r,c in self.count])
  def freeEnergy(self):
    return sum([c * r.freeEnergy() for r,c in self.count])
  def validate(self):
   #each vertex & factor is counted once
    Vcount = {}
    Fcount = {}
    for r,c in self.count.iteritems():
      for v in r.V:
        if v in Vcount: Vcount[v] += c
        else:           Vcount[v] =  c
      for f in r.F:
        if f in Fcount: Fcount[v] += c
        else:           Fcount[v] =  c
    assert forall(Vcount.values(), lambda c: c == 1),\
        "variable is not counted exactly once"
    assert forall(Fcount.values(), lambda c: c == 1),\
        "factor is not counted exactly once"
   #each variable's stalk is connected
    V = Vcount.keys()
    for v in V:
      G = [r for r in self.R if v in r.V]
      component = [G[0]]
      for r in component:
        for r2 in r.P + r.C:
          if r2 not in component:
            if v in r2.V:
              component.append(r2)
      assert len(component) = len(G), "vertex's stalk is not connected"

class RegionGraph_old:
  def __init__(self, V, F, R, count):
    assert forall(V, lambda v: exists(R, lambda r: v in r.V)),\
        "variable appears in no region"
    assert forall(F, lambda f: exists(R, lambda r: f in r.F)),\
        "factor appears in no region"
    assert forall(V, lambda v: (sum([count[r] for r in R if v in r.V]) == 1)),\
        "variable is not counted exactly once"
    assert forall(F, lambda f: (sum([count[r] for r in R if f in r.F]) == 1)),\
        "factor is not counted exactly once"
    self.V = V         #variables
    self.F = F         #factors
    self.R = R         #regions
    self.count = count #counting numbers

#===== energy manifold =====
## moved to energy_manifold.py

#===== association chains =====
class Vertex(Variable):
  def __init__(self, X, prior=None):
    Variable.__init__(self, X, prior)
    self.L = [] #left-edges
    self.R = [] #right-edges
  def prop(self):
    Lprior = sum([self.inbox[l] for l in self.L], Zero(X))
    Rprior = sum([self.inbox[r] for r in self.R], Zero(X))
    self.post = Lprior * self.prior * Rprior
    if _normalized:  self.post.normalize()
   #send messages left
    outgoing = self.prior * Rprior
    if _normalized:  outgoing.normalize()
    for l in self.L:
      l.inbox[self] = outgoing
   #send messages right
    outgoing = Lprior * self.prior
    if _normalized:  outgoing.normalize()
    for r in self.R:
      r.inbox[self] = outgoing

class Edge(Factor):
  def __init__(self, L, R, fun):
    Factor.__init__(self, [L,R], fun)
    self.L = L;  L.R.append(self) #left vertex
    self.R = R;  R.L.append(self) #right vertex
  def initProp(self):
    self.L.inbox[self] = Unif(L)
    self.R.inbox[self] = Unif(R)
  def prop(self):
    
  def prop_L2R(self, bel_L):
    joint = bel_L.extendTo(self.R.V,1)
    joint *= self.fun
    bel_R = joint.restrictTo(2)
    return bel_R
  def prop_R2L(self, bel_R):
    joint = bel_R.extendTo(self.L.V,0)
    joint *= self.fun
    bel_L = joint.restrictTo(1)
    return bel_L

class AssocChain:
  def __init__(self, V, E, F):
    assert forall(V, lambda v: forall(v.L: lambda e: e in E),\
        "left-edge of vertex is not in edges"
    assert forall(V, lambda v: forall(v.R: lambda e: e in E),\
        "right-edge of vertex is not in edges"
    assert forall(E, lambda e: e.L in V),\
        "left-vertex of edge is not in vertices"
    assert forall(E, lambda e: e.R in V),\
        "right-vertex of edge is not in vertices"
    #assert forall(F, lambda f: forall(f.V, lambda v: v in V)),\
    #    "factor vertex not in vertices"
    self.V = V.copy() #vertices
    self.E = E.copy() #directed edges
    #self.F = F.copy()
  def initProp(self, flow):
    for v in self.V:  v.initProp()
    for e in self.E:  e.initProp(flow[e])
  def propStep(self, flow):
    for v in self.V:  v.prop()
    for e in self.E:  e.prop(flow[e])
    ## OLD
    #L,C,R = None,None,None
    #for v in self.V:
    #  if v.L.empty():  L = v.prior
    #  else:            L = sum
  #def propagate(self):
  #  L = dict([self. ...
  def propNorm(self, like): #like cost2flow
    Z_L = dict[(v,0.0) for v in self.V]) #left-expectation
    Z_R = dict[(v,0.0) for v in self.V]) #right-expectation
    for v in self.V:
      if v.L.empty():  Z_L[v] = 1.0
      if v.R.empty():  Z_R[v] = 1.0
    for e in self.E_L2R:  Z_L[e.R] += like[e] * Z_L[e.L]
    for e in self.E_R2L:  Z_R[e.L] += like[e] * Z_R[e.R]
    return dict([(e, Z_L[e.L] * like[e] * Z_R[e.R]) for e in self.E])
  def initPropNorm(self):
    for 
  def propNormStep(self):
    for v in self.V:  v.propStep()
    for e in self.E:  e.propStep()
  def entropy(self, flow):
    H_L = dict([(v,0.0) for v in self.V]) #left-expectation
    for e in self.E_L2R:
      mass = flow[e]
      H_L[e.R] += mass * H_L[e.R]
    return sum([h for h,v in H_L.iteritems() if v.R.empty()])
  def freeEnergy(self, flow):
    raise NotImplementedError

class AssocNet(FactorGraph):
  def __init__(self, V, F, G):
    assert forall(G.keys(), lambda (v,f): v in V and f in F),\
        "groups contains vertex/factor not in vertices/factors"
    assert forall(F, lambda f: forall(f.V, lambda v: (v,f) in G)),\
        "vertex-factor pair does not occur in groups"
    FactorGraph.__init__(self, V, F)
    self.G = G
  def propStep(self, a):
    #flow constraints?
    for v in self.V:  v.inbox = {}
    for f in self.F:
      local = ...
      for v in f.V:
        outgoing = local / v.post.extendTo(f)
        g = self.G[v,f]
        if g in v.inbox:  v.inbox[g] += a[g] * outgoing
        else:             v.inbox[g] =  a[g] * outgoing
    for v in self.V:
      v.post = prod([mu for mu in v.inbox.values()], v.prior)

#===== types of factor graphs =====
class HMM(FactorGraph):
  def __init__(self, X, Z, f, h, N):
    K = range(N)
    self.X_ = [Variable(X) for k in K]                         #states
    self.Z_ = [Variable(Z) for k in K]                         #observations
    self.f_ = [Factor(self.X_[k:k+2], f) for k in K[:-1]]      #state transforms
    self.h_ = [Factor([self.X_[k], self.Z_[k]], h) for k in K] #sensors
   #treatment as network
    V = self.X_ + self.Z_
    F = self.f_ + self.h_
    FactorGraph.__init__(self, V, F)

class Cycle(FactorGraph):
  def __init__(self, X, f, N):
    V = [Variable(X) for n in range(N)]
    F = [Factor(V[n:n+2], f) for n in range(N-1)]
    F.append(Factor([V[-1],V[0]], f))
    FactorGraph.__init__(self, V, F)

class Komplete(FactorGraph):
  def __init__(self, X, f, N):
    V = [Variable(X) for n in range(N)]
    F = [Factor([V[n],V[m]], f) for n in range(N) for m in range(n)]
    FactorGraph.__init__(self, V, F)

#===== types of region graphs =====
class BetheGraph(RegionGraph):
  "builds a RegionGraph equivalent to a FactorGraph"
  def __init__(self, V, F):
    R_large = [Region(f.V, [F]) for f in F]
    R_small = [Region([v], []) for v in V]
    R = R_large + R_small
    count = {}
    for r in R_large: count[r] = 1
    for r in R_small: count[r] = 1 - len([r2 for r2 in R_large if r <= r2])
    RegionGraph.__init__(self, V, F, R, count)

class JunctionGraph(RegionGraph):
  raise NotImplementedError

class ClusterVariationGraph(RegionGraph):
  raise NotImplementedError

#===== types of association chains =====
class MarkovAssocChain(AssocChain):
  def __init__(self, X, priors, ):
    VX = [Vertex([Variable(

#===== example tools =====
def Bernoulli(p):
  return Measure({True: p, False: 1.0-p})
def Flip(p):
  return Measure({(True,  True): 1.0-p, (True,  False): p,
                  (False, True): p,     (False, False): 1.0-p})

#===== examples =====
def example1():
  print "==========[ Example 1: a short HMM ]=========="
  print "_normalized = %s" % _normalized
  net = HMM(Bernoulli(0.5), Bernoulli(0.9), Flip(0.1), Flip(0.4), 7)
  net.initProp()
  value = lambda x: x.post[True] / x.post.total()
  for i in range(12):
    print "----- iteration %i -----" % i
    net.propStep()
    print ', '.join(['%g' % value(x) for x in net.X_])
  net.initProp()
  print "----- exact enumeration -----"
  net.enumerate()
  print ', '.join(['%g' % value(x) for x in net.X_])

def example2():
  print "==========[ Example 2: a small cycle ]=========="
  net = Cycle(Bernoulli(0.4), Flip(0.1), 5)
  net.initProp()
  def getTotal():
    return net.V[0].post.entropy()
  for i in range(50):
    old_total = getTotal()
    net.propStep()
    new_total = getTotal()
    energy = -M.log(new_total / old_total)
    print "extra energy = %s" % energy

def example3():
  print "==========[ Example 3: a larger cycle ]=========="
  net = Cycle(Bernoulli(0.5), Flip(0.9), 11)
  net.V[0].prior = Bernoulli(0.9)
  net.initProp()
  value = lambda x: x.post[True] / x.post.total()
  for i in range(24):
    print "----- iteration %i -----" % i
    net.propStep()
    print ', '.join(['%g' % value(v) for v in net.V])
  net.initProp()
  print "----- exact enumeration -----"
  net.enumerate()
  print ', '.join(['%g' % value(v) for v in net.V])

def example4():
  print "==========[ Example 4: K_5 Boltzmann machine ]=========="
  net = Komplete(Bernoulli(0.4), Flip(0.2), 5)
  net.initProp()
  net.initProp()
  value = lambda x: safeDiv(x.post[True], x.post.total())
  for i in range(12):
    print "----- iteration %i -----" % i
    net.propStep()
    print ', '.join(['%g' % value(x) for x in net.V])
  net.initProp()
  print "----- exact enumeration -----"
  net.enumerate()
  print ', '.join(['%g' % value(x) for x in net.V])

#===== execution script =====
from pdb import pm
def run():
  #example1()
  #example2()
  #example3()
  example4()
if __name__ == '__main__':
  run()

