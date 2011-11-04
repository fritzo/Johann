
from pylab import *
from scipy.linalg import cholesky,inv
from scipy import outer
from ode import propagate

LATER = "LATER"

#coord transforms
def bb2rect (b1,b2):
  B1 = b1**2
  B2 = b2**2
  A = 1 - B1 - B2
  return (B2/(1-A), 2*A)

def V2pa (V):
  b1 = V[0]**2
  b2 = V[1]**2
  a = 1 - b1 - b2
  p2 = b2 / (b1+b2)
  return [p2,a]

def pa2V (pa):
  p2 = pa[0]
  p1 = 1 - p2
  a = pa[1]
  A = 1-a
  b1 = A * p1
  b2 = A * p2
  return [sqrt(b1), sqrt(b2)]

def pa2V_diff (pa,dpa):
  p2 = pa[0];     dp2 = dpa[0]
  p1 = 1 - p2;    dp1 = -dp2
  a = pa[1];      da = dpa[1]
  A = 1-a;        dA = -da
  b1 = A * p1;    db1 = dA * p1 + A * dp1
  b2 = A * p2;    db2 = dA * p2 + A * dp2
  v1 = sqrt(b1);  dv1 = 0.5 * db1 / v1
  v2 = sqrt(b2);  dv2 = 0.5 * db2 / v2
  return [v1,v2], [dv1,dv2]

#geometry and dynamics
def metric (V): #XXX WRONG
  V_V = dot(V,V)
  u2 = 1 - V_V
  M = 1/(1.0 - 2 * u2)
  W = outer(V,V) / u2
  I = identity(len(V))
  return M * (I + W)

def accel (V,dV): #XXX WRONG
  "constrained acceleration"
  V_V = dot(V,V)
  V_dV = dot(V,dV)
  dV_dV = dot(dV,dV)

  u = sqrt(1 - V_V)
  du = -V_dV/u

  return (-V_dV) * dV + (-1 + u/2*( dV_dV + du**2 )) * V

#plotting
def plot_paths (paths):
  figure()
  for p in paths:
    X = [x for (x,y) in p]
    Y = [y for (x,y) in p]
    plot(X,Y,color='black')
  axis([0,1,0,0.5])
  show()

def get_star (XY,time=1,n=4):
  "returns paths eminating from given point"
  paths = []
  V = pa2V(XY)
  for t in arange(0,pi,pi/n):
    phi = cholesky(inv(metric(V)))
    dV = dot(phi, array([cos(t),sin(t)]))
    Vs,dVs = propagate(V, dV, accel, time, 0.005)
    paths.append([V2pa(v) for v in Vs])
  return paths

def plot_stars (n=3):
  "plots paths eminating from many stars"
  dx = 1.0/(2*n)
  time = 0.5/n
  paths = []
  for x in arange(0.5*dx, 1.0+0.5*dx, dx):
    for y in arange(0.5*dx, 0.5+0.5*dx, dx):
      paths += get_star((x,y),time)
  plot_paths(paths)

def test_plot ():
  plot_stars()

if __name__ == "__main__":
  test_plot()
  

