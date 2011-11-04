#axioms.py

__all__ = ['axioms','axiomSchemes','theorems']

#[ expressions ]
from Axiom import *
from exprs import *

#[ data structures ]
from Set import *

#==========[ single axioms ]==========
axioms = Set()
axioms.insert(Axiom('xi.K','K-reduction',
  'K*(a*x)*(b*x)/x/b/a','a*x/x/b/a'))
axioms.insert(Axiom('xi.S','S-reduction',
  'S*(a*x)*(b*x)*(c*x)/x/c/b/a','a*x*(c*x)*(b*x*(c*x))/x/c/b/a'))


#==========[ axiom schemes ]==========
axiomSchemes = Set()
axiomSchemes.insert(AxiomScheme('xi',
  'extensionality, replaced by a finite axiom set',
  'for a,b in xexprs, x in vars: a = b -> a/x = b/x'))
axiomSchemes.insert(AxiomScheme('K','K-reduction',
  'forall a,b: K*a*b = a'))
axiomSchemes.insert(AxiomScheme('S','S-reduction',
  'forall a,b,c: S*a*b*c = a*c*(b*c)'))


#==========[ single theorems ]==========
theorems = Set()
theorems.insert(Axiom('eta','eta-equivalence',
  'S*K','K*I'))
#WORKING:  there is a bug somewhere in here:===================================
#theorems.insert(Axiom('A.1','hindley-lurcher-seldin',
#  'S*(K*x)*(K*y)/y/x','K'))
#theorems.insert(Axiom('A.2','hindley-lurcher-seldin',
#  'S*(K*x)*I/x','I'))
#theorems.insert(Axiom('A.3','hindley-lurcher-seldin',
#  'S*(K*I)','I'))
#theorems.insert(Axiom('A.4','hindley-lurcher-seldin',
#  'S*(S*(K*K)*x)/x','K'))
#theorems.insert(Axiom('A.5','hindley-lurcher-seldin',
#  'S*(S*(S*(K*S)*x)*y)/y/x','S*(S*x*z)*(S*y*z)/z/y/x'))

