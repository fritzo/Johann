

class Term:
  def __mul__(self,rhs):
    return App(self, rhs)
  def parse(self): raise NotImplementedError
class Atom(Term):
  def __init__(self,name):
    self.name = name
  def __str__(self):
    return self.name
  def __parse__(self):
    return ("atom", 
class Type(Atom): pass
class App(Term):
  def __init__(self, lhs,rhs):
    self.lhs = lhs
    self.rhs = rhs
  def __str__(self):
    return "(%s %s)" % (self.lhs, self.rhs)

terms = [
  "close",
  "prod","mk_pair","pi1","pi2",
  "sum","inl","inr",
  "maybe","none","some",
  "bool","true","false",
  "nat","zero","succ","pred","add","mul",
  "fix",
]
def init_terms ():
  global terms
  for name in terms:
    Term.__dict__[name] = Atom(name)

def eval (term):
  "returns normalized term if it exists; None if already normalized"
  (a0,v0) = term.parse()
  if t0 == "atom":
    return None
  (l0,r0) = v0


def test1

if __name__ == "__main__":

