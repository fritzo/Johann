#---10---][---20---][---30---][---40---][---50---][---60---][---70---][---80---]
# generalization of boltzmann machines for GBP

#===== math tools =====
import math as M
M.inf = float('infinity')
M.nan = float('nan')
def safeSub(x,y):
  result = x - y
  if result is M.nan:  return M.inf
  else                 return result
def entropyTerm(e):
  if e < M.inf:  return e * M.exp(-e)
  else:          return 0.0
def relentropyTerm(e,e0):
  #XXX: is this right?
  if e0 < M.inf:  return (e0-e) * M.exp(-e)
  else:           return 0.0

#===== logic tools =====
def forall(X,prop):
  for x in X: if !prop(x) return False
  return True
def exists(X,prop):
  for x in X: if prop(x) return True
  return False

#===== systems of partices =====
_part_id = 0
class Particle:
  def __init__(self, X):
    global _part_id
    self.id = _part_id;  _part_id += 1
    self.X = X  #state space
 #comparison for sorting: an implementation detail
  def __cmp__(self, other):  return cmp(self.id, other.id)
  #def __le__(self, other):  return self.id <= other.id
  #def __ge__(self, other):  return self.id >= other.id

class System(tuple):
  def __add__(self, other):
    P = list(self)
    for p in other.P:
      if p not in P:
        P.append(p)
    P.sort()
    return System(P)
  def __sub__(self, other):  return System([p in self.P if p not in other.P])
  def __mul__(self, other):  return System([p in self.P if p     in other.P])
  def contains(self, other):  return forall(other, lambda p: p in self)
EmptySystem = System(())

class Atlas:
  "collection of (possibly overlapping) charts"
  def __init__(self, S):
    assert forall(S, lambda s1: forall(S, lambda s2: s1*s2 in S)),\
        "ststems are not closed under intersection"
    self.s = sum(S, EmptySystem)   #total system
    self.S = S.copy()              #sets of variables
    self.S.sort(lambda s: -len(s)) #sorted in largest-to-smallest order

class Manifold(Atlas):
  "Atlas closed under chart-intersection"
  def __init__(self, S):
    assert forall(S, lambda s1: forall(S, lambda s2: s1*s2 in S)),\
        "ststems are not closed under intersection"
    Atlas.__ini__(self, S)    count = dict([(s1,1) for s1 in self.S])
    for s1 in self.S:
      for s2 in self.S if s2.contains(s1):
        count[s1] -= count[s2]
    self.total_count = [(s,c) for s,c in count.iteritems() if c]
    self.__countCache = {}
  def count(self, s0):
    if s0 not in self.__countCache:
      count = dict([(s1,1) for s1 in self.S if s0*s1])
      for s1 in self.S:
        for s2 in self.S if s2.contains(s1):
          count[s1] -= count[s2]
      self.__countCache[s0] = [(s,c) for s,c in count.iteritems() if c]
    return self.__countCache[s0]

#===== energy charts & atlas =====
class EnergyOnChart:
  def __init__(self, s, E=None):
    if E is None:  E = dict([(_p_,0.0) for _p_ in prod_iter(s)]) #zero energy
    self.s = s  #domain
    self.E = E  #hash-table of energies
    I = range(len(s))
    assert forall(E, lambda _p_: forall(I, lambda i: _p_[i] in s[i])),\
        "invalid argument in energy function"
  def copy(self):  return EnergyOnChart(self.s, self.E)
 #info operations
  def norm(self):  return sum([M.exp(-e) for e in self.E.values()])
  def normalize(self):
    shift = M.log(self.norm())
    self.E = dict([(_p_, e + shift) for _p_,e in self.E.iteritems()])
  def entropy(self):
    return sum([entropyTerm(e) for e in self.E.values()])
  def relentropy(self, E):
    return sum([relentropyTerm(E[_p_], e) for _p_,e in self.E.iteritems()])
 #ring operations
  def __add__(self, other):
    assert self.s == other.s, "tried to multiply beliefs of different var sets"
    return EnergyOnChart(self.s, dict([(_p_, e * other.E[_p_])
                                   for _p_,mass in self.E.iteritems()]))
  def __sub__(self, other):
    assert self.s == other.s, "tried to divide beliefs of different var sets"
    return EnergyOnChart(self.s, dict([(_p_, safeSub(mass, other.E[_p_]))
                                   for _p_,mass in self.E.iteritems()]))
  def __rmul__(self, r):
    return EnergyOnChart(self.s,
                         dict([(_p_,r*e) for _p_,e in self.E.iteritems()])
  def __iadd__(self, other):
    if other.s != self.s:  other = other.__slidTo(self.s)
    for _p_,e in self.E:  self.E[_p_] = e + other.E[_p_]
  def __isub__(self, other):
    if other.s != self.s:  other = other.__slidTo(self.s)
    for _p_,e in self.E:  self.E[_p_] = safeSub(e, other.E[_p_])
 #sheaf operations
  def __restrictedTo(self, s):
    assert self.s.contains(s), "tried to restrict to non-subset"
    I = [i for i in range(len(self.s)) if self.s[i] in s]
    reindex = lambda _p_: tuple([_p_[i] for i in I])
    Z = {}
    for _p_,e in self.E.iteritems():
      ubar = reindex(_p_)
      if ubar in E:  Z[ubar] += M.exp(-e)
      else:          Z[ubar] =  M.exp(-e)
    E = dict([(_p_,-M.log(z)) for _p_,z in Z.iteritems()])
    return EnergyOnChart(s, E)
  def __extendedTo(self, s):
    assert s.contains(self.s), "tried to extend to non-superset"
    I = [i for i in range(len(s)) if s[i] in self.s]
    reindex = lambda _p_: tuple([_p_[i] for i in I])
    shift = sum([-M.log(len(_p_)) for _p_ in s - self.s]))
    E = dict([(_p_, self.E[reindex(_p_) + shift]) for _p_ in prod_iter(s)])
    return EnergyOnChart(s, E)
  def __slidTo(self, s):  return self.__restrictedTo(self.s*s).__extendedTo(s)
  def isCompatableWith(self, other, tol=1e-6):
    S = self.S * other.S
    E1 = self.__restrictedTo(S)
    E2 = other.__restrictedTo(S)
    for _p_ in E1.keys():
      if M.abs(E1[_p_] - E2[_p_]) > tol:  return False
    return True

class EnergyOnAtlas:
  def __init__(self, A, _E_):
    assert forall(_E_, lambda E1:
           forall(_E_, lambda E2: E1.isCompatibleWith(E2))),\
               "atlas contains incompatible charts"
    assert forall(A, lambda s: s in _E_),\
        "energy is not defined on entire atlas"
    self.A = A
    self.S = sum([E.s for E in _E_],EmptySystem) #charts
    self._E_ = _E_                               #local energy maps
  def __iter__(self):
    return iter(self._E_)
  def iteritems(self):
    return self._E_.iteritems()
  def __rmul__(self, r):
    _E_ = dict([(s,r*E) for s,E in self._E_.iteritems()])
    return EnergyOnAtlas(self.A, _E_)
  def __iadd__(self, _E_):
    assert _E_.A = self.A, "tried to add energies on different atlases"
    for s,E in _E_.iteritems:
      self._E_[s] += E
      self._E_[s].normalize()
 #sheaf operations
  def __refinedTo(self, S):
    assert forall(self.S, lambda s: s in S), "tried to refine to non-refinement"
    raise "LATER"
  def __relaxedTo(self, S):
    assert forall(S, lambda s: s in self.S), "tried to refine to non-refinement"
    raise "LATER"
  def __repartitionedTo(self,S):
    return self.__refinedTo(self.S + S).__relaxedTo(S)

class EnergyOnManifold(EnergyOnAtlas):
  def __init__(self, M, _E_):
    assert isinstance(M, Manifold). "energy manifold created with non-manifold"
    EnergyOnAtlas.__init__(self, M, _E_)
  def __rsub__(self, _E_):
    assert isinstance(_E_, EnergyOnAtlas),\
        "tried to subtract non-Eatlas from Emanifold"
    assert forall(_E_.S, lambda s: s in self.S),\
        "manifold is not refinement of atlas"
    diff = dict([(s,c * (self._E_[s] - _E_[s])) for s,c in self.total_count])
    return EnergyOnManifold(self.M, diff)

#===== energy manifolds =====
class EnergyManifold:
  def __init__(self, S, F):
    assert forall(S, lambda s1: forall(S, lambda s2: s1*s2 in S)),\
        "ststems are not closed under intersection"
    assert forall(F.keys(), lambda f: f.V in S),
        "factor's domain is not in sets"
    self.S = S.copy()              #sets of variables
    self.S.sort(lambda s: -len(s)) #sorted in largest-to-smallest order
    self.F = F                     #factors of variables
   #prior & posterior energy functions
    self.prior = dict([(s, sum([f for f in self.F if f.V * s],
                               EnergyOnChart(s)))
                       for s in self.S])
    for f in self.prior.values():  f.normalize()
    self.post = None
   #counting
    count = dict([(s1,1) for s1 in self.S])
    for s1 in self.S:
      for s2 in self.S if s2.contains(s1):
        count[s1] -= count[s2]
    self.__total_count = [(s,c) for s,c in count.iteritems() if c]
    self.__countCache = {}
  def propagate(self, tol=1e-4):
    self.__initProp()
    diff = 2 * tol
    while diff > tol:
      prev_post = self.post.copy()
      self.__propStep()
      diff = self.diff(prev_post)
  def __initProp(self):
    self.post = self.prior.copy()
  def __propStep(self):
    for s1 in self.S:
      prior = self.prior[s1]
      post = sum([c*(self.post[s2]-prior) for s2,c in self.__count(s1)], prior)
      post.normalize()
      self.post[s1] = post
  def __count(self, s0):
    if s0 not in self.__countCache:
      count = dict([(s1,1) for s1 in self.S if s0*s1])
      for s1 in self.S:
        for s2 in self.S if s2.contains(s1):
          count[s1] -= count[s2]
      self.__countCache[s0] = [(s,c) for s,c in count.iteritems() if c]
    return self.__countCache[s0]
 #learning methods
  def __learnStep(self, lesson, rate = 1.0):
    "gradient-descent step in Hebbian learning"
    for s,c in self.__total_count:
      self.prior[s] += c * rate * (lesson[s] - self.post[s])
      self.prior[s].normalize()
  def predict(self, input, output):
    for s in input:  self.prior[s] += input[s]
    self.propagate()
    for s in input:  self.prior[s] -= input[s]
    return self.getPMF(output)
 #info methods
  def getPMF(self, s1):
    result = sum([c*self.post[s2] for s2,c in self.__count(s1)],
                 EnergyOnChart(s1))
    result.normalize()
    return result
  def norm(self):
    return sum([c * s.norm(self.post[f]) for s,c in self.__total_count])
  def diff(self, f):
    return sum([c * s.diff(f[s], self.post[f]) for s,c in self.__total_count])

#===== examples =====



