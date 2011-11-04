
from math import *
from numpy import *
import pyx
import os

def draw_point(canv, x, y, r):
  circ = pyx.path.circle(x, y, 0.02 * r)
  canv.fill(circ, [pyx.color.rgb.black])

def get_eigs (filename):
  #read perturbation matrix
  print "loading perturbation matrix..."
  #from perturb_out import P
  #P_size = shape(P)[0]
  P = fromfile("perturb.out", Float32)
  P_size = int(round(sqrt(shape(P)[0])))
  print "resizing %i --> %ix%i..." % (len(P),P_size,P_size)
  assert P_size*P_size == len(P), "improper size"
  P.resize((P_size,P_size))

  #calculate eigenvalues
  print "calculating eigenvalues..."
  e = linalg.eigvals(P)
  m = absolute(e)
  return (e,m)

def sqrt_eigs (e):
  "XXX: this maps to only the first quadrant"
  e2 = array([sqrt(z) for z in e if z.imag >= 0])
  m2 = absolute(e2)
  return (e2,m2)

def draw_eigs ((e,m), filename, RAD = 5.0):
  "draws eigenvalues as dots inside the unit circle"
  #set up graph
  #M = max(m)
  #canv = pyx.graph.graphxy(
  #    xpos=-RAD, ypos=-RAD,
  #    width=2*RAD, height=2*RAD,
  #    x=pyx.graph.axis.linear(min=-M, max=M),
  #    y=pyx.graph.axis.linear(min=-M, max=M))

  #set up graph
  M = 1.0
  canv = pyx.canvas.canvas()
  canv.stroke(pyx.path.circle(0,0,RAD), [pyx.color.rgb.black]) #draw unit circle

  #draw eigenvalues
  print "plotting eigenvalues..."
  for n in xrange(len(e)):
    x = e[n].real * RAD / M
    y = e[n].imag * RAD / M
    r = m[n] * RAD / M
    draw_point(canv, x, y, r)
  canv.writetofile("%s.eps" % filename)
  os.system("epstopdf %s.eps" % filename)
  os.system("kghostview %s.pdf &" % filename)
  #os.system("acroread -openInNewWindow %s.pdf &" % filename)

def run (filename = "perturb_eigs"):
  (e,m) = get_eigs(filename)
  draw_eigs((e,m), filename)

  #(e2,m2) = sqrt_eigs(e)
  #draw_eigs((e2,m2), filename+".sqrt")

if __name__ == "__main__":
  run()


