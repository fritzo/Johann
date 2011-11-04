#PMF.py

__doc__ = """Probability Mass Function (multiset):
version 2003_04_22"""

__all__ = ["PMF"]


#==========[ misc ]==========
from exceptions import *
from Numeric import sum
from Shortcuts import LATER
from copy import copy

def _isProb(p):
  return (0<=p) and (p<=1)
  
LATER = "unfinished code in PMF.py"
class InvalidError(Exception):
  pass


#==========[ probability mass funciton class (or multiset) ]==========
class PMF:
  def __init__(self,probs=None):
    if not probs:
      self.probs = {}
    else:
      self.probs = copy(probs)
      for item in self.probs:
        self.probs[item] = float(self.probs[item])

  def __copy__(self):
    return PMF(self.probs)

  def __str__(self):
    return 'PMF('+str(self.probs)+')'

  def __repr__(self):
    return str(self)

  def __getitem__(self,item):
    value = 0.0
    if item in self.probs:
      value = self.probs[item]
    return value

  def __setitem__(self,item,prob):
    self.probs[item] = float(prob)

  def __delitem__(self,item):
    del self.probs[item]

  def keys(self):
    return self.probs.keys()

  def values(self):
    return self.probs.values()

  def insert(self,item):
    self.probs[item] = 1.0

  def __contains__(self,item):
    return (item in self.probs)

  def __iter__(self):
    return iter(self.probs)

  def __len__(self):
    return len(self.probs)

 #math operations
  def __iadd__(lhs,rhs):
    for item in rhs.probs:
      if item in lhs.probs:
        lhs.probs[item] += rhs.probs[item]
      else:
        lhs.probs[item] = rhs.probs[item]
    return lhs

  def __add__(lhs,rhs):
    sum = copy(lhs)
    sum += rhs
    return sum

  def __imul__(lhs,rhs):
    if isinstance(rhs,float) or isinstance(rhs,int):
      for item in lhs.probs:
        lhs.probs[item] *= rhs
    else: #isinstance(rhs,PMF):
      for item in lhs.probs:
        if item in rhs.probs:
          lhs.probs[item] *= rhs.probs[item]
        else:
          del lhs.probs[item]
    return lhs

  def __mul__(lhs,rhs):
    product = copy(lhs)
    product *= rhs
    return product

  def __rmul__(rhs,lhs):
    if isinstance(lhs,PMF):
      product = NotImplemented
    else:
      product = rhs*lhs
    return product

  def __idiv__(lhs,rhs):
    if isinstance(rhs,float) or isinstance(rhs,int):
      for item in lhs.probs:
        lhs.probs[item]/=rhs
    else: #isinstance(rhs,PMF):
      for item in lhs.probs:
        if item in rhs.probs:
          lhs.probs[item]/=rhs.probs[item]
        else:
          lhs.probs[item]=inf
    return lhs

  def __div__(lhs,rhs):
    ratio = copy(lhs)
    ratio /= rhs
    return ratio

 #math stuff
  def getMaxProb(self):
    maxProb = 0.0
    for item in self.probs:
      if self.probs[item]>maxProb:
        maxProb = self.probs[item]
    return maxProb

  def getTotal(self):
    return sum(self.probs.values())

  def normalize(self):
    total = self.getTotal()
    if total > 0:
      for item in self.probs:
        self.probs[item] /= total
    else:
      p = 1.0/len(self.probs)
      for item in self.probs:
        self.probs[item] = p

  def mean(self):
    return sum([item*self[item] for item in self])

  def validate(self,tol=1e-5):
    for item in self.probs:
      if not _isProb(self.probs[item]):
        raise InvalidError, 'probability nor in [0,1]'
    if abs(1-self.getTotal) > tol:
      raise InvalidError, 'probabilities do not sum to 1.0'

  def randomChoice(self,t=None):
    if t is None:
      from random import uniform
      t = uniform(0,self.getTotal())
    items = list(self.items)
    choice = None
    for item in items:
      t -= self[item]
      if t < 0:
        choice = item
        break
    return choice

 #pruning
  def pruneBelow(self,minProb=0):
    pruned = []
    for item in copy(self.probs):
      if self.probs[item] <= minProb:
        del self.probs[item]
        pruned.append(item)
    return pruned

  def prune(self,item):
    if item in self.probs:
      del self.probs[item]
      
      
