#Expr.py

__all__ = [
  'setTranslator',
  'Expr','AppExpr','AtomExpr','VarExpr',
  'S','K','Q','I','B','C','W','T','F','box',
  'I_fab','B_fab','C_fab','W_fab','T_fab','F_fab', #simple translator
  'I_abcdef','B_abcdef','C_abcdef','W_abcdef','T_abcdef','F_abcdef'
    #efficient translator
]

#[ misc ]
import pdb
from exceptions import Exception

#[ expressions ]
from AppTree import *

#[ data structures ]
from Set import *


#==========[ misc ]==========
class IrreducibleError(Exception):
  def __str__(ie):
    return "irreducible error"

#==========[ translators ]==========
knownTranslators = ['fab','abcdef']
translator = 'fab' #lambda -> <S,K,I,B,C> translator

def setTranslator(algorithm = None):
  global translator
  if algorithm is None: #default translator
    translator = 'fab'
  elif algorithm in knownTranslators:
    translator = algorithm
  else:
    raise Exception('unknownn translator: '+str(algorithm))


#==========[ combinatory expression ]==========
class Expr:
 #application
  def __mul__(lhs,rhs):
    return AppExpr(lhs,rhs)
  def __call__(fun, *args):
    result = fun
    for arg in args:
      result = AppExpr(result, arg)
    return result

 #abstraction
  def __div__(lhs,rhs):
    if not isinstance(rhs,VarExpr):
      raise Exception("only variables can be abstracted")
    if translator == 'abcdef':
      result = lhs.algorithm_abcdef(rhs)
    else: #Expr.__translator == 'fab'
      result = lhs.algorithm_fab(rhs)
    return result
    
  def algorithm_fab(lhs,rhs):
    "Curry's simple abstraction algorithm, i.e., "
    if isinstance(lhs,AppExpr):
      result = S*(lhs.lhs/rhs)*(lhs.rhs/rhs) #rule f
    elif not rhs.isFreeIn(lhs):
      result = K*lhs #rule a
    else:
      result = I #rule b
    return result 
    
  def algorithm_abcdef(lhs,rhs):
    "Curry's efficient abstraction algorithm"
    if not rhs.isFreeIn(lhs):
      result = K*lhs #rule a
    elif isinstance(lhs,AtomExpr):
        result = I #rule b
    else:
      if not rhs.isFreeIn(lhs.lhs):
        if lhs.rhs is rhs:
          result = lhs.lhs #rule c
        else:
          result = B*lhs.lhs*(lhs.rhs/rhs) #rule d
      elif not rhs.isFreeIn(lhs.rhs):
        result = C*(lhs.lhs/rhs)*lhs.rhs #rule e
      else:
        result = S*(lhs.lhs/rhs)*(lhs.rhs/rhs) #rule f
    return result
 
 #reduction
  def __reduceStep(expr):
    result = None
    if isinstance(expr,AppExpr):
      if isinstance(expr.lhs,AppExpr):
        if isinstance(expr.lhs.lhs,AppExpr):
          if expr.lhs.lhs.lhs is S:
            result = expr.lhs.lhs.rhs*expr.rhs*(expr.lhs.rhs*expr.rhs)
        elif expr.lhs.lhs is K:
          result = expr.lhs.rhs
      elif expr.lhs is I:
        result = expr.rhs
      if result is None:
        lhs_result = expr.lhs.__reduceStep()
        if lhs_result is not None:
          result = lhs_result*expr.rhs
        else:
          rhs_result = expr.rhs.__reduceStep()
          if rhs_result is not None:
            result = expr.lhs*rhs_result
    return result

  def reduce(expr,maxSteps = 1000):
    this = expr
    steps = 0
    while steps < maxSteps:
      prev = this
      this = this.__reduceStep()
      if this is None:
        this = prev
        break
      steps += 1
    if steps > 0:
      print 'reduce steps = '+str(steps)
    return this

  def printReduceSequence(expr,maxSteps = 1000):
    this = expr
    print str(this)
    steps = 0
    while steps < maxSteps:
      prev = this
      this = this.__reduceStep()
      if this is None:
        this = prev
        break
      print '--> '+str(this)
      steps += 1
    return this

  def getReduceSequence(expr,maxSteps = 1000):
    exprList = [expr]
    steps = 0
    while steps < maxSteps:
      next = exprList[-1].__reduceStep()
      if next is None:
        break
      exprList.append(next)
      steps += 1
    return exprList

 #complexity
  def __cmp__(lhs,rhs):
    "length-then-lex of polish notation form"
    result = cmp(len(lhs.name),len(rhs.name))
    if result == 0:
      result = cmp(lhs.name,rhs.name)
    return result

  def __hash__(expr):
    return hash(expr.name)


#==========[ atomic expression ]==========
class AtomExpr(Expr):
  def __init__(atom,name):
    atom.name = str(name) #in polish notation

  def __hash__(atom):
    return hash(atom.name)
    
 #display
  def __repr__(atom):
    return str(atom)

  def __str__(atom):
    return atom.name

  def latex(atom):
    return '\\'+atom.name

  def appTree(atom):
    return AppTree([[atom.name]])

 #comparison
  def __eq__(lhs,rhs):
    return lhs is rhs

#==========[ variable expression ]==========
class VarExpr(AtomExpr):
  def isFreeIn(var,expr):
    """free containment"""
    if var is expr:
      result = True
    elif isinstance(expr,AtomExpr):
      result = False
    elif isinstance(expr,AppExpr):
      result = var.isFreeIn(expr.lhs) or var.isFreeIn(expr.rhs)
    else:
      raise Exception("unknown expr")
    return result

#==========[ application expression ]==========
class AppExpr(Expr):
  def __init__(app,lhs,rhs):
    app.lhs = lhs
    app.rhs = rhs
    app.name = '*'+lhs.name+rhs.name #in polish notation
    
 #display
  def __repr__(app):
    return str(app)

  def __str__(app):
    if isinstance(app.rhs,AppExpr):
      result = str(app.lhs)+'*('+str(app.rhs)+')'
    else:
      result = str(app.lhs)+'*'+str(app.rhs)
    return result

  def latex(app):
    if isinstance(app.rhs,AppExpr):
      result = str(app.lhs)+'('+str(app.rhs)+')'
    else:
      result = str(app.lhs)+str(app.rhs)
    return result

  def appTree(app):
    return app.lhs.appTree()*app.rhs.appTree()

 #comparison
  def __eq__(lhs,rhs):
    if isinstance(rhs,AppExpr):
      result = (lhs.lhs==rhs.lhs) and (lhs.rhs==rhs.rhs)
    else:
       result = False
    return result


#==========[ basic expr definitions ]==========
#atomic basis
S = AtomExpr('S')
K = AtomExpr('K')
Q = AtomExpr('Q')

#derived obs, via combination
I = S*K*K #I = a/a
B = S*(K*S)*K #B = a*(b*c)/c/b/a
C = S*(B*B*S)*(K*K) #C = a*c*b/c/b/a
W = C*S*I #W = a*b*b/b/a
T = K #T = a/b/a
F = K*I #F = b/b/a
box = C*I
I.__str__ = lambda: 'I'
B.__str__ = lambda: 'B'
C.__str__ = lambda: 'C'
#W.__str__ = lambda: 'W'
#T.__str__ = lambda: 'T'
F.__str__ = lambda: 'F'
box.__str__ = lambda: 'box'

#derived obs, via algorithm fab
setTranslator('fab') #translator shaould already be fab
a = VarExpr('a'); b = VarExpr('b'); c = VarExpr('c')
I_fab = a/a
B_fab = a*(b*c)/c/b/a
C_fab = a*c*b/c/b/a
W_fab = a*b*b/b/a
T_fab = a/b/a
F_fab = b/b/a

#derived obs, via algorithm abcdef
setTranslator('abcdef') #translator shaould already be fab
I_abcdef = a/a
B_abcdef = a*(b*c)/c/b/a
C_abcdef = a*c*b/c/b/a
W_abcdef = a*b*b/b/a
T_abcdef = a/b/a
F_abcdef = b/b/a



