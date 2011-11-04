#---10---][---20---][---30---][---40---][---50---][---60---][---70---][---80---]
# Belief propagation algorithms for
#   similarity graphs of magmas (groupoids) under a random equational theory.
# (see notes/ideas/belief_prop.text)

import pdb

#===== math tools =====
from sets import Set
from math import *
inf = float('infinity')
nan = float('nan')
def argmin(f):
  val = inf
  arg = None
  for x,y in f.iteritems():
    if y < val:
      val = y
      arg = x
  return arg
def argmax(f):
  val = -inf
  arg = None
  for x,y in f.iteritems():
    if y > val:
      val = y
      arg = x
  return arg
sqr = lambda x: x*x

#===== stats tools =====
def BernVar(p):
  q = 1-p
  return p*q*(p+q)
def e2p(e):
  "converts energies to probabilities"
  l = exp(-e)
  return l / (1.0 + l)
def p2e(p):
  "converts probabilities to energies"
  l = p / (1.0 - p)
  return -log(l)

#===== logic tools =====
def forall(X,prop):
  for x in X:
    if not prop(x): return False
  return True
def exists(X,prop):
  for x in X:
    if prop(x): return True
  return False

#===== algebras =====
class ParitalMagma:
  "an partially enumerated magma with a random free carrier and inequalities"
  def __init__(self, A, B, D):
    assert abs(sum(A.values()) - 1.0) < 1e-6, "carrier PMf is not normalized"
    assert forall(B, lambda (a,b): a in A and b in A), "B contains a not in A"
    assert forall(B.values(), lambda a: a in A), "B contains value not in A"
    assert forall(D, lambda (a,b): a in A and b in A), "D contains a not in A"
    assert forall(D, lambda (a,b): a<b), "D contains out-of-order pair"
    self.A = A #carrier, a PMF
    self.B = B #a single binary operation
    self.D = D #distinctions
   #close distinctions (bad complexity here; C++ code is written better)
    changed = True
    while changed:
      changed = False
      for ((l,r),a) in B.iteritems():
        for l2 in A:
          XX = (min(l,l2), max(l,l2))
          if XX in D:  continue
          a2 = self(l2,r)
          if a2 is None:  continue
          YY = (min(a,a2), max(a,a2))
          if YY in D:
            D.append(XX)
            changed = True
        for r2 in A:
          XX = (min(r,r2), max(r,r2))
          if XX in D:  continue
          a2 = self(l,r2)
          if a2 is None:  continue
          YY = (min(a,a2), max(a,a2))
          if YY in D:
            D.append(XX)
            changed = True
  def __call__(self,a,b):
    "a partial application table"
    if (a,b) in self.B:  return self.B[a,b]
    else:                return None

class SimilarityGraph(ParitalMagma):
  def __init__(self, A, B, D, S):
    ParitalMagma.__init__(self, A, B, D)
    D = self.D
    assert forall(S, lambda (a,b): a in A and b in A), "S contains a not in A"
    assert forall(S, lambda (a,b): a<b), "S contains out-of-order pair"
    S = [s for s in S if s not in D]
    self.S = S
   #complexity data
    self.m = A.copy() #initial mass
    self.h = dict([(a,-p*log(p)) for a,p in A.iteritems()]) #initial energy
   #triple iterator
    SSS = [(a,b,c) for a,c in S for b in A if (a,b) in S and (b,c) in S]
    SSD = [(a,b,c) for a,c in S for b in A if (a,b) in S and (b,c) in D]
    SDS = [(a,b,c) for a,c in D for b in A if (a,b) in S and (b,c) in S]
    DSS = [(a,b,c) for a,c in S for b in A if (a,b) in D and (b,c) in S]
    self.AAA = SSS + SSD + SDS + DSS
   #forcing iterator
    XXYY = Set()
    XXYYZZ = Set()
    for ((l,r),a) in B.iteritems():
     #defining XXYY
      for l2 in A:
        XX = (min(l,l2), max(l,l2))
        if XX not in S:  continue
        a2 = self(l2,r)
        if a2 is None:  continue
        YY = (min(a,a2), max(a,a2))
        assert YY not in D, "unenforced distinction axiom"
        if YY in S:  XXYY.add((XX,YY))
      for r2 in A:
        XX = (min(r,r2), max(r,r2))
        if XX not in S:  continue
        a2 = self(l,r2)
        if a2 is None:  continue
        YY = (min(a,a2), max(a,a2))
        assert YY not in D, "unenforced distinction axiom"
        if YY in S:  XXYY.add((XX,YY))
     #defining XXYYZZ
      for l2 in A:
        XX = (min(l,l2), max(l,l2))
        if XX not in S:  continue
        for r2 in A:
          YY = (min(r,r2), max(r,r2))
          if YY not in S:  continue
          a2 = self(l2,r2)
          if a2 is None:  continue
          ZZ = (min(a,a2), max(a,a2))
          #assert ZZ not in D, "unenforced distinction axiom"
          if ZZ in S:  XXYYZZ.add((XX,YY,ZZ))
    self.XXYY   = list(XXYY)
    self.XXYYZZ = list(XXYYZZ)
 #entropy term calculations
  def __init_mass(self):
    print "    initializing expected complexity"
    self.M1 = self.m.copy()
    self.M2 = dict([(s,0.0) for s in self.S])
  def __calc_mass(self):
    print "    calculating expected complexity"
    self.M1 = self.m.copy()
    for a,b in self.S:
      self.M1[a] += self.rho[a,b] * self.m[b]
      self.M1[b] += self.rho[a,b] * self.m[a]
   #LATER: average entropy term
    #V = dict([(a,0) for a in self.A]) #variances
    #for a,b in self.S:
    #  V[a] += self.m[b] * BernVar(self.rho[a,b])
    #  V[b] += self.m[a] * BernVar(self.rho[a,b])
    #for a in self.A:
    #  self.H[a] += -0.5 * V[a] / self.M[a]
 #pair-wise energy calculation
  def __init_energy(self):
    print "    initializing prior pairwise energies"
    e = {}
    l = dict([(d,0) for d in self.D])
    def ent(p): return -p*log(p)
    for (a,b) in self.S:
      ma = self.m[a]
      mb = self.m[b]
      e_ab = e[a,b] = ent(ma + mb) - ent(ma) - ent(mb)
      l[a,b] = exp(-e_ab)
    self.e = e
    self.l = l
  def __calc_energy(self):
    print "    calculating prior pairwise energies"
    e = {}
    l = dict([(d,0) for d in self.D])
    def ent(p): return -p*log(p)
    for (a,b),r in self.rho.iteritems():
      ma = self.M1[a] - r*self.m[b]
      mb = self.M1[b] - r*self.m[a]
      e_ab = e[a,b] = ent(ma + mb) - ent(ma) - ent(mb)
      l[a,b] = exp(-e_ab)
    self.e = e
    self.l = l
 #pair-wise similarity calculation 
  def __init_rho(self):
    print "    initializing similarity"
    self.rho = self.l.copy()
  def __calc_rho(self):
    print "    calculating similarity"
    e = self.e
    l = self.l
    E = e.copy()
   #iterating over triple regions, marginalize
    for a,b,c in self.AAA:
     #pair-wise likelihoods
      l_ab = l[a,b]
      l_ac = l[a,c]
      l_bc = l[b,c]
      l_abc = l_ab*l_ac*l_bc
     #update posterior energies
      if l_ab:  E[a,b] += -e[a,b]  -log(l_ab + l_abc)  +log(1.0 + l_ac + l_bc)
      if l_ac:  E[a,c] += -e[a,c]  -log(l_ac + l_abc)  +log(1.0 + l_ab + l_bc)
      if l_bc:  E[b,c] += -e[b,c]  -log(l_bc + l_abc)  +log(1.0 + l_ab + l_ac)
   #iterating over app regions, marginalize
    for XX,YY in self.XXYY:
     #pair-wise likelihoods
      l_x = l[XX]
      l_y = l[YY]
      l_xy = l_x * l_y
     #update posterior energies
      E[XX] += -e[XX]  -log(l_xy)        +log(1.0 + l_y)
      E[YY] += -e[YY]  -log(l_y + l_xy)  +log(1.0)
    for XX,YY,ZZ in self.XXYYZZ:
     #pair-wise likelihoods
      l_x = l[XX]
      l_y = l[YY]
      l_z = l[ZZ]
      l_xz = l_x * l_z
      l_yz = l_y * l_z
      l_xyz = l_x * l_y * l_z
     #update posterior energies
      E[XX] += -e[XX] -log(l_x +l_xz +l_xyz)       +log(1.0 +l_y +l_z +l_yz)
      E[YY] += -e[YY] -log(l_y +l_yz +l_xyz)       +log(1.0 +l_x +l_z +l_xz)
      E[ZZ] += -e[ZZ] -log(l_z +l_xz +l_yz +l_xyz) +log(1.0 +l_x +l_y)
   #combine
    for a,b in self.S:
      self.rho[a,b] = e2p(E[a,b])
  def __calc_rho_alt(self):
    print "    calculating similarity"
    e = self.e
    l = self.l
    E = e.copy()
   #iterating over triple regions, marginalize
    for a,b,c in self.AAA:
     #mass components
      ma=M1[a];  mb=M1[B];  mc=M1[c];  mab=M2[a,b];  mac=M2[a,c];  mbc=M1[b,c]
      m_abc = pow(sqr(mab * mac * mbc)/(ma * mb * mc), 1.0/3.0)
      m_a = ma - mab - mac + mabc
      m_b = mb - mab - mbc + mabc
      m_c = mc - mac - mbc + mabc
      m_ab = mab - mabc
      m_ac = mac - mabc
      m_bc = mbc - mabc
     #hypothesis energies
      e_a_b_c = "LATER"
      e_ab_c  = l[a,b]
      e_ac    = l[a,c]
      e_bc    = l[b,c]
      e_abc   = l_ab * l_ac * l_bc
     #hypothesis probabilities
      l2p = 1.0/(1.0 + l_ab + l_ac + l_bc + l_abc)
      p_a_b_c = l2p;
      p_ab_c = l2p * l_ab;
      p_ac_b = l2p * l_ac;
      p_a_bc = l2p * l_bc;
      p_abc = l2p * l_abc;
     #pair-wise probabilities
      p_ab = rho[a,b]
      p_ac = rho[a,c]
      p_bc = rho[b,c]
     #update posterior energies
      if l_ab:  E[a,b] += -e[a,b]  -log(l_ab + l_abc)  +log(1.0 + l_ac + l_bc)
      if l_ac:  E[a,c] += -e[a,c]  -log(l_ac + l_abc)  +log(1.0 + l_ab + l_bc)
      if l_bc:  E[b,c] += -e[b,c]  -log(l_bc + l_abc)  +log(1.0 + l_ab + l_ac)
     #update mass
      ma = m[a];   mb=m[b];   mc=m[c]
      M1[a] += (p_ab_c+p_abc-p_ab) * mb + (p_ac_b+p_abc-p_ac) * mc
      M2[a,b] += p_abc * mc
      M2[a,c] += p_abc * mb
      M2[b,c] += p_abc * ma
   #combine
    for a,b in self.S:
      self.rho[a,b] = e2p(E[a,b])
 #iterative propatation algorithm
  def __initProp(self):
    print "  initializing propagation"
    self.__init_mass()
    self.__init_energy()
    self.__init_rho()
    #self.E = dict([() for ])
  def __propStep(self):
    print "  propagating"
    rho = self.rho.copy()
    self.__calc_mass()
    self.__calc_energy()
    self.__calc_rho()
    diff = sum([sqr(rho[s]-r) for s,r in self.rho.iteritems()])
    print "    diff = %g" % diff
    return diff
  def prop(self, tol=1e-4):
    print "iteratively propagating"
    self.__initProp()
    diff = 2*tol
    while diff > tol:
      diff = self.__propStep()
  def enumerate(self):
    print "exhaustively enumerating"
    D = self.D
   #build list of feasible partitions
    _X_ = [()] #set of sets of subsets of A
    for a in self.A:
      _X_ = [tuple([y for y in X if y is not x])+(x+(a,),)
             for X in _X_ for x in X if forall(x, lambda b: (b,a) not in D)]\
          + [X+((a,),) for X in _X_]
               
    print "  found %i feasible theories" % len(_X_)
    #for X in _X_:
    #  print "    %s" % X
   #evaluate likelihood of each theory
    mass = lambda x: sum([self.m[a] for a in x])
    ent  = lambda m: -m*log(m)
    Ent  = lambda X: sum([ent(mass(x)) for x in X])
    like = lambda X: exp(-Ent(X))
    L = dict([(X, like(X)) for X in _X_])
    tot = sum(L.values())
    P = dict([(X, L[X]/tot) for X in _X_])
   #calculate expected masses
    M = self.m.copy()
    for X,p in P.iteritems():
      for x in X:
        m = p*mass(x)
        for a in x:
          M[a] += m
    self.M1 = M
   #calculate pairwise marginals
    rho = dict([((a,b),0.0) for a in self.A for b in self.A
        if a<b if (a,b) not in D])
    for X,p in P.iteritems():
      for x in X:
        for a in x:
          for b in x:
            if a<b:
              rho[a,b] += p
    self.rho = rho
 #statistics
  def entropy(self):
    "expected entropy"
    return sum([-m*log(self.M1[a]) for a,m in self.A.iteritems()])
  def print_stats(self):
    print "A = %s" % self.A
    print "B = %s" % self.B
    print "D = %s" % self.D
    print "S = %s" % self.S
    print "m = %s" % self.m
    print "M1 = %s" % self.M1
    print "rho = %s" % self.rho
    print "expected entropy = %s" % self.entropy()
 #estimation
  def guess(self, x):
    sim = {x : self.m[x] / self.M1[x]}
    for (a,b) in self.S:
      if a is x:   sim[b] = self.rho[a,b] * self.m[b] / self.M1[b]
      elif b is x: sim[a] = self.rho[a,b] * self.m[a] / self.M1[a]
    return argmax(sim)

#===== examples =====
def example1(size = 5):
  print "===== a line segment of %i vertices =====" % size
  assert size >= 2
  N = range(size)
  m = 1.0 / size
  A = dict([(n,m) for n in N])
  B = {}
  D = [(0,size-1)]
  S = [(n,n+1) for n in N[:-1]]
  G = SimilarityGraph(A,B,D,S)
  G.prop()
  G.print_stats()
  G.enumerate()
  G.print_stats()

  #estimation
  for n in N:
    print "  %i --> %i" % (n, G.guess(n))

def example2(size = 5):
  print "===== a complete graph on %i vertices =====" % size
  assert size >= 2
  N = range(size)
  m = 1.0 / size
  A = dict([(n,m) for n in N])
  B = {}
  D = []
  S = [(n1,n2) for n1 in N for n2 in N if n1<n2]
  G = SimilarityGraph(A,B,D,S)
  G.prop()
  G.print_stats()
  G.enumerate()
  G.print_stats()

def example3(size = 5):
  print "===== a first-order ring on %i vertices =====" % size
  assert size >= 2
  N = range(size)
  m = 1.0 / size
  A = dict([(n,m) for n in N])
  B = {}
  D = [(n,n+1) for n in N[:-1]] + [(0,size-1)]
  def mod_dist(x,y):
    return min(y-x,size-(y-x))
  S = [(n1,n2) for n1 in N for n2 in N if n1<n2 and mod_dist(n1,n2) > 1]
  G = SimilarityGraph(A,B,D,S)
  G.prop()
  G.print_stats()
  G.enumerate()
  G.print_stats()

def example4(size = 7):
  print "===== a second-order ring on %i vertices =====" % size
  assert size >= 2
  N = range(size)
  m = 1.0 / size
  A = dict([(n,m) for n in N])
  B = {}
  D = [(n,n+1) for n in N[:-1]] + [(0,size-1)]\
    + [(n,n+2) for n in N[:-2]] + [(0,size-2),(1,size-1)]
  def mod_dist(x,y):
    return min(y-x,size-(y-x))
  S = [(n1,n2) for n1 in N for n2 in N if n1<n2 and mod_dist(n1,n2) > 2]
  G = SimilarityGraph(A,B,D,S)
  G.prop()
  G.print_stats()
  G.enumerate()
  G.print_stats()

def example5(size = 8):
  print "===== additive group Z/%i =====" % size
  assert size >= 2
  N = range(size)
  m = 1.0 / size
  A = dict([(n,m) for n in N])
  B = dict([((a,b),(a+b)%size) for a in A for b in A])
  D = [(0,1)]
  S = [(n1,n2) for n1 in N for n2 in N if n1<n2]
  G = SimilarityGraph(A,B,D,S)
  G.prop()
  G.print_stats()
  G.enumerate()
  G.print_stats()
  

#===== test script =====

def run():
  #example1()
  #example2()
  #example3()
  #example4()
  example5()

if __name__ == '__main__':
  run()


