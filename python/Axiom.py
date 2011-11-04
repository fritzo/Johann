#Axiom.py

__all__ = ['Axiom','AxiomScheme']

from Expr import *
from exprs import *

#==========[ single axiom ]==========
class Axiom:
  def __init__(ax,name,desc,lhsStr,rhsStr):
    ax.name = name
    ax.desc = desc
    ax.lhsStr = lhsStr
    ax.rhsStr = rhsStr
    ax.lhs = eval(lhsStr).reduce()
    ax.rhs = eval(rhsStr).reduce()

  def __str__(ax):
    return ('('+ax.name+'): '+ax.desc+'\n'
           +ax.lhsStr+' = '+ax.rhsStr+'\n'
           +str(ax.lhs)
           +'\n  ='+str(ax.rhs)

  
#==========[ axiom scheme ]==========
class AxiomScheme:
  def __init__(ax,name,desc,axStr):
    ax.name = name
    ax.desc = desc
    ax.axStr = axStr

  def __str__(ax):
    return ('('+ax.name+'): '+ax.desc+'\n'
           +ax.axStr+'\n')


