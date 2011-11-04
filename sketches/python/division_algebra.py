
global id
id = 0
global ID #max id
ID = 1000

class Ob:
  def __init__(self, name=None):
    global id
    self.id = id
    id += 1
    #properties
    self.name = name
    self.con = False
    #measure
    self.z = 0.0
    self.z0 = 0.0

class Eqn:
  def __init__(self, F, X, Y, type="Y"):
    global ID
    offset = self.F.id + ID*(self.X.id + ID*self.Y.id)
    self.__hash_val = {"Y":0,"F":1,"X":2}[self.type] + 3*offset
    #components
    self.F = F
    self.X = X
    self.Y = Y
    assert type in ["Y","F","X"], "bad Eqn type"
    self.type="Y" #which ob is defined by this eqn
    if self.type == "Y":
      self.L = F
      self.R = X
      self.A = Y
    elif self.type == "F":
      self.L = Y
      self.R = X
      self.A = F
    elif self.type == "X":
      self.L = F
      self.R = Y
      self.A = X
  def __eq__(self, other):
    return self.type == other.type and self.L == other.L and self.R == other.R
  def __hash__(self):
    return self.__hash_val

class Rel:
  def __init__(self, L, U):
    global ID
    self.__hash_val self.L.id + ID*self.U.id
    #components
    self.L = L
    self.U = U
  def __hash__(self):
    return self.__hash_val

class CombStruct:
  def __init__(self):
    self.O = Set() #obs
    self.E = Set() #eqns
    self.R = Set() #order relations
    #poset
    Bot = self.Bot = Ob("Bot"); self.O.insert(Bot)
    Top = self.Top = Ob("Top"); self.O.insert(Top)
    self.__make_rel(Bot,Bot)
    self.__make_rel(Bot,Top)
    self.__make_rel(Top,Top)
    #lattice operations
    J = self.J = self.__make_op("J")
    M = self.M = self.__make_op("M")
    #combinatory atoms
    S = self.S = self.__make_atom("S")
    K = self.K = self.__make_atom("K")
    I = self.K = self.__make_atom("I")
    #consistency
    Bot.con = True
    S.con   = True
    K.con   = True
    I.con   = True
    M.con   = True
    #complexity
    self.Z0 = {S:0.2, K:0.2, I:0.2, J:0.2, M:0.2}
    self.eps = 0.5

  #constructions
  def make_app (self, f, x):
    if ???: return #don't add duplicates
    y = self.__make_ob()
    self.__make_eqn(f, x, y, "Y")
    self.enforce_axioms()
  def make_rdiv(self, y, x):
    f = self.__make_ob()
    self.__make_eqn(f, x, y, "F")
    self.enforce_axioms()
  def make_ldiv(self, f, y):
    x = self.__make_ob()
    self.__make_eqn(f, x, y, "X")
    self.enforce_axioms()

  #complexity tools
  def update_complexity(self, eps):
    "a single iteration"
    for o in self.O:
      o.z0 = o.z
      o.z  = 0.0
    for e in self.E:
      e.A.z += eps * e.L.z * e.R.z
    for b,z in self.Z0.iteritems():
      b.z += (1-e) * z

  #factories
  def __make_rel(self, L,U):
    self.R.insert(R(L,U))
  def __make_atom(self, name):
    A = Ob(name)
    self.O.insert(A)
    self.__make_rel(Bot, A )
    self.__make_rel( A , A )
    self.__make_rel( A ,Top)
    return A
  def __make_ob(self):
    A = Ob()
    self.O.insert(A)
    self.__make_rel(self.Bot, A)
    self.__make_rel(A, A)
    self.__make_rel(A, self.Top)
    self.__make_app(self.I, A, A, "F")
    #XXX what if A=Top?
    #self.__make_app(self.Bot, A, self.Bot, "F")
    #XXX what if A=Bot?
    #self.__make_app(self.Top, A, self.Top, "F")
    return A


  #queries
  def __are_rel(L,U):
    return Rel(L,U) in self.R

  #ensurance
  def __ensure_rel(L, U):
    if not self__are_rel(L,U):
      self.__make_rel(L,U)
  def __ensure_app(F,X,Y, type):
    if Eqn(F,X,Y) not in self

  #axiom enforcement
  def enforce_axioms(self):
    "global, inefficient"
    new_identities = Set()
    new_relns = Set()
    while ???:
      #check transitivity
      for r1 in self.R:
        for r2 in self.R:
          if r1.U is not r2.L: continue
          self.__ensure_rel(r1.L, r2.U)
      #check monotonicity
      for e1 in self.E:
        if e1.type == "Y":
          for e2 in self.E:
            if e2.F != e1.F: continue
            if e2.X != e1.X: continue
            self.__ensure_rel(e1.Y, e2.Y)
        elif e1.type == "F":
          for e2 in self.E:
            if e2.Y != e1.Y: continue
            if e2.X != e1.X: continue
            self.__ensure_rel(e2.Y, e1.Y)
        elif e1.type == "X":
          for e2 in self.E:
            if e2.F != e1.F: continue
            if e2.Y != e1.Y: continue
            self.__ensure_rel(e2.Y, e1.Y)
      #check consistency of terms
      for e in self.E:
        if e.type == "Y":
          e.Y.con |= e.F.con and e.X.con:
        
  def __merge_eqns(e1,e2): LATER
  def __merge_obs(o1,o2): LATER
        

