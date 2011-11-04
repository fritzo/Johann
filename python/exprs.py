#exprs.py

alphabet = 'abcdefghijklmnopqrstuvwxyz'

__all__ = (
  ['S','K','Q','I','B','C','W','T','F','stdBasis']+ #standard
  list(alphabet)+                                   #variables
  ['Y','Y1','Y2','Omega']+                                      #unsolvables
  ['And','Or','Not','Imp']+                         #logic
  ['church','sets','pmfs']                          #everything else
)


#==========[ standard exprs ]==========
from Expr import *
stdBasis = {I : 0.2,
            S : 0.2,
            B : 0.2,
            C : 0.2,
            W : 0.1,
            K : 0.1}


#==========[ variables ]==========
class vars: pass
for name in alphabet:
  setattr(vars,name,VarExpr(name))

a = vars.a
b = vars.b
c = vars.c
d = vars.d
e = vars.e
f = vars.f
g = vars.g
h = vars.h
i = vars.i
j = vars.j
k = vars.k
l = vars.l
m = vars.m
n = vars.n
o = vars.o
p = vars.p
q = vars.q
r = vars.r
s = vars.s
t = vars.t
u = vars.u
v = vars.v
w = vars.w
x = vars.x
y = vars.y
z = vars.z



#==========[ recursion in <S,K> ]==========
Y = (f*(x*x)/x)*(f*(x*x)/x)/f
Y1 = (x*x)/x*(f*(x*x)/x)/f
Y2 = S*S*K*(S*(K*(S*S*(S*(S*S*K))))*K) #whassisname's length-12 fixed point
Omega = (x*x/x)*(x*x/x)                #Barendregt's favorite unsolvable


#==========[ logic in <S,K> ]==========
And = p*(q*a*b)*b/b/a/q/p
Or = p*a*(q*a*b)/b/a/q/p
Not = p*b*a/b/a/p
Imp = p*(q*x*y)*x/y/x/q/p


#==========[ church arithmetic, using <S,K> ]==========
class Church:
  Zero = K*I
  One = I
  Two = f*(f*x)/x/f
  Succ = S*B
  Double = B*(S*B*I)
  Is_zero = K*F
  Pair = s*x*y/s/y/x
  #Pred = ???
  Add = (n*f)*(m*f*x)/x/f/n/m
  Mul = B
  Pow = n*m/n/m
  Sqr = Two
  def numeral_linear(ch,n):
    "converts python numeral to Church numeral, linear in n"
    if not (isinstance(n,int) or isinstance(n,long)):
      raise "only integers can be converted to church numerals"
    if n < 0:
      raise "only non-negative integers can be converted to church numerals"
    temp = x
    while n > 0:
      temp = f*temp
      n -= 1
    return (temp/x/f)
  def numeral(ch, n):
    "converts python numeral to Church numeral, logrithmic in n"
    if not (isinstance(n,int) or isinstance(n,long)):
      raise "only integers can be converted to church numerals"
    if n < 0:
      raise "only non-negative integers can be converted to church numerals"
    if n == 0: return ch.Zero
    this_bit = 1<<30
    while (n & this_bit) == 0:
      this_bit >>= 1
    result = ch.One
    this_bit >>= 1
    while this_bit != 0:
      result = ch.Double * result
      if (n & this_bit) != 0:
        result = ch.Succ * result
      this_bit >>= 1
    return result
    
church = Church()

#==========[ a simple set theory, using <S,K,Q> ]==========
class sets:
  "simple set theory, not zf or vngb or anything known"
  Contains = I
  Universe = K*T
  Emptyset = K*F
  Singleton = Q
  Union = Or*(Contains*x*s)*(Contains*x*t)/x/t/s
  Intersect = And*(Contains*x*s)*(Contains*x*t)/x/t/s
  #Select = Y*(p*(s*x*(Close(x)*y)/x)/y)/p/s
  #Subtract = ...
  #Apply = 
  #Close = Or*...
  Isempty = Q*Emptyset
  #Exists = Not*(Q*Emptyset*(Select*p*Universe))/p
  #Forall = Not*(Q*Universe*(Select*p*Universe))/p
  #Choose =


#==========[ probability mass functions in <S,K,Q,R> ]==========
class pmfs:
  pass
  #R = ??? #LATER: need a definition in <S,K> or <S,K,Q>
  #Close = (f*x)/f/x #encapsulation, or "lowering"
  #Bern = R*(Close*x)*(Close*y)/y/x #formation of a random pair
  #Apply = p*(Close*(f*x)/x)/p/f
  #Open = p*I/p #random sampling, or "raising"
  #Prob = Apply*(Q*x)/x #probability evaluation
  #Select = Y*(p*(s*x*(Close*x*y)/x)/y)/p/s
  #Conjoin = ...
  #Condition = ...

#(Q) is it provable that Conjoin=(Conjoin*y*x/y/x)?
#(Q) can R be derived from from S,K,Q?
  #(Q) can x!=T & x!=F -> Q*a*b!=x ?, or does this result in paradoxes?
#R = Q*x*y*x*(R*x*y)/y/x #reflexivity
#R = R*y*x/y/x #symmetry
#R = R*(x*f)*(y*f)/f/y/x #right-distributivity
#Pair = b*x*y/b/y/x #i.e., Pair*x*y*T = x, Pair*x*y*F = y
#Note: Russel's paradox provides an example of a symmetric pair in <S,K>:
#  F = Not*(f*f)/f
#  R1 = F*F
#whence
#  Not*R1 == R1
#moreover, letting
#  G = Q*x*y*x*(r*x*y)/y/x/r
#  R2 = Y*G,
#one gets the additional property
#  R2*x*X = x
#but this reqires identity <S,K,Q> for x?=y comparison.
#alternatively, let
#  G = Q*x*y*x*(r*y*x)/y/x/r


