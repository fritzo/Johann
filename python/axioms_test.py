#axioms_test.py

from axioms import *

def listCombinatoryAxioms():
  print '==========[ The Combinatory Axioms ]=========='
  print '[ Axiom Schemes ]'
  for axs in axiomSchemes:
    print str(axs)+'\n'
  print '[ Single Axioms ]'
  for ax in axioms:
    print str(ax)+'\n'

listCombinatoryAxioms()

def drawAxioms():
  for axiom in axioms:
    print '[ '+axiom.name+' ]=========='
    print str(axiom.lhs)
    print str(axiom.lhs.appTree())
    print ' = '
    print str(axiom.rhs)
    print str(axiom.rhs.appTree())+'\n'

drawAxioms()
