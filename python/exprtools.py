
from Expr import *
from exprs import *

def term_iter(basis,length):
  if length < 1:   return []
  elif length == 1:  return basis[:]
  else:
    sub_terms = [term_iter(basis,i) for i in range(length)]
    result = []
    for i in range(1,length):
      for lhs in sub_terms[i]:
        for rhs in sub_terms[length-i]:
          result.append(lhs*rhs)
    return result

def term_enumerate(basis):
  "iterates through all terms"
  i = 1
  while True:
    level = term_iter(basis,i)
    for term in level:
      yield term
    i += 1

def find_omega(thresh = 100):
  "finds smallest irreducible ob"
  for term in term_enumerate([S,K]):
    print term
    reduced = term.reduce(thresh)
    if reduced is not reduced.reduce(1):
      print "Omega(%i) = %s" % (thresh,term)
      return term
    
if __name__=="__main__":
  find_omega()

