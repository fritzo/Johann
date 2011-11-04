#Set.py

__doc__ = """finite sets implemented as dicts, v2003_05_26"""

__all__ = ["Set", "union", "intersection"]

from copy import copy

LATER = 'unfinished code in Set.py'

#[ a finite set class ]----------
def union(*args):
  if len(args) == 1:
    sets = args[0]
  else:
    sets = args
  union = Set()
  for set in sets:
    union += set
  return union

def intersection(*args):
  if len(args) == 1:
    sets = args[0]
  else:
    sets = args
  intersection = sets.chooseElement()
  for set in sets:
    intersection*= set
  return intersection

class Set:
  def __init__(self,arg = None):
    self.elements = {}
    if arg is not None:
      if isinstance(arg,Set):
        self.elements = copy(arg.elements)
      else:
        for element in arg:
          self.elements[element] = None

  def __str__(self):
    elementlist = [str(element) for element in self.elements]
    elementlist.sort() #lex, then length
    return '{'+','.join(elementlist)+'}'

  def __repr__(self):
    return 'Set('+repr(self.elements.keys())+')'

  def __copy__(set):
    other = Set([])
    other.elements = copy(set.elements)
    return other

 #single element functions
  def __contains__(set,element):
    return element in set.elements

  def insert(set,element):
    set.elements[element] = None
    return set

  def remove(set,element):
    if element in set.elements:
      del set.elements[element]
    return set

  def chooseElement(self):
   #as per axiom of choice
    if len(self.elements)==0:
      exit('cannot choose element from empty set')
    return self.elements.keys()[0]

 #multiple element funcitons
  def __len__(self):
    return len(self.elements)

  def items(self): # return list of elements
    return self.elements.keys()

  def __iter__(self):
    return iter(self.elements)

 #operations
  def __add__(lhs,rhs): #union
    sum = copy(lhs)
    for element in rhs:
      sum.insert(element)
    return sum

  def __iadd__(lhs,rhs):
    for element in rhs:
      lhs.insert(element)
    return lhs

  def __radd__(rhs,lhs):
    """this works for lists"""
    if isinstance(lhs,Set):
      sum = NotImplemented
    else:
      sum = copy(lhs)
      for element in rhs:
        if element not in lhs:
          lhs.append(element)
    return sum

  def __sub__(lhs,rhs): #set difference
    difference = Set()
    for element in lhs:
      if not (element in rhs):
        difference.elements[element] = None
    return difference

  def __isub__(lhs,rhs):
    for element in rhs:
      if element in lhs.elements:
        del lhs.elements[element]
    return lhs

  def __rsub__(rhs,lhs):
    """this works for lists"""
    if isinstance(lhs,Set):
      difference = NotImplemented
    else: 
      difference = copy(lhs)
      for element in rhs:
        while element in difference:
          difference.remove(element)
    return difference

  def __mul__(lhs,rhs): #intersect
    product = Set()
    for element in lhs:
      if element in rhs:
        product.insert(element)
    return product

  def __imul__(lhs,rhs): #intersect
    for element in lhs:
      if element not in rhs:
        lhs.remove(element) 

 #relations
  def __le__(lhs,rhs):
    return logical_and([element in rhs for element in lhs])

  def __ge__(lhs,rhs):
    return logical_and([element in lhs for element in rhs])

  def __lt__(lhs,rhs):
    return (lhs <= rhs) and (not rhs <= lhs)

  def __gt__(lhs,rhs):
    return (lhs >= rhs) and (not rhs <= lhs)

  def __eq__(lhs,rhs):
    return (lhs <= rhs) and (rhs <= lhs)
    
  def __ne__(lhs,rhs):
    return not (lhs == rhs)

 #more complicated algorithms
  def underMultiOp(set,op):
    return Set.Union(Set([map(op,element) for element in set]))

  def closedUnderMultiOp(self,op,radius = None):
   #look for options
    if radius is None:
      radius = float('inf')
    closure = Set()
    boundary = copy(self)
    r = -1
    while len(boundary)>0 and r < radius:
      closure += boundary
      boundary = boundary.underMultioOp(op) - closure
      r += 1
    return closure

  def restrictedByCondition(self,condition):
    restriction= Set()
    for element in self:
      if condition(element):
        restriction.insert(element)
    return restriction
    
  def powerSet(self):
    elements = list(self.elements)
    powerSet = Set([Set()])
    for element in elements:
      singleton = Set([element])
      for subset in copy(powerSet):
        powerSet.insert(subset+singleton)
    return powerSet

  def cartesianProduct(lhs,rhs):
    product = Set()
    for x in lhs:
      for y in rhs:
        product.insert((x,y))
    return product

  def elWiseSum(lhs,rhs):
    product = Set()
    for x in lhs:
      for y in rhs:
        product.insert(x+y)
    return product

  def elWiseProduct(lhs,rhs):
    product = Set()
    for x in lhs:
      for y in rhs:
        product.insert(x*y)
    return product
