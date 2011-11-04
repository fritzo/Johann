
from pylab import *

def propagate (V0, dV0, accel, t, dt=0.02):
  "propagates 2nd-order forward and backward in time"
  d = len(V0)

  #define concatenated system
  x0 = concatenate((V0,dV0))
  def ddx (VdV,t):
    #wrap 2nd order ode as 1st order ode
    V   = VdV[:d]
    dV  = VdV[d:]
    ddV = accel(V,dV)
    return concatenate((dV,ddV))

  #propagate forward and backward
  fwd = rk4(ddx, x0, arange(0,t,dt))
  bwd = rk4(ddx, x0, arange(0,-t,-dt))
  n = len(bwd)
  all = concatenate((bwd[n:0:-1], fwd))
  V  = all[:,:d]
  dV = all[:,d:]
  return V,dV

def test_prop():
  print "testing propagator on 2d harmonic oscillator"
  x0 = array([0,0])
  dx0 = array([1,1])
  def accel(x,dx):
    return array([-4.0*x[0], -9.0*x[1]])
  t = 2*pi
  dt = 0.1
  (X,dX) = propagate(x0,dx0, accel, t, dt)
  print X

if __name__ == "__main__":
  test_prop()

