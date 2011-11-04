#AppTree_test.py

from exprs import *
from AppTree import *

expr = S*K*(T*F)*(S*K*(F*T))
print str(expr)+':\n'

#little app tree test
"""
at = expr.appTree()
print str(at)

fileName = 'AppTree_test_1'
at.postscript(fileName)
"""

#medium app tree test
"""
at = Omega.reduce(100).appTree()
print str(at)

fileName = 'AppTree_test_1'
at.postscript(fileName)

fileName = 'AppTree_test_2'
at.postscript(fileName)
"""

#large app tree test
two = church.numeral(2)
three = church.numeral(3)
five = church.numeral(5)
Mul = church.Mul

num = two*(Mul*(Mul*(three*two)*three)*five)

at = num.appTree()
print str(at)

fileName = 'AppTree_test'
at.postscript(fileName)

