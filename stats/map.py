
from math import *
from numpy import *
import pyx
import os

def draw_point(canv, x, y, r, c):
  circ = pyx.path.circle(x, y, r)
  canv.fill(circ, [pyx.color.rgb(c,c,c)])

def get_points (filename):
  #read perturbation coords
  print "loading perturbation coords..."
  P = fromfile(filename + ".coords", float32)
  P_size = int(round(shape(P)[0] / 5)) #5 columns
  print "resizing %i --> 5x%i..." % (len(P),P_size)
  assert 5*P_size == len(P), "improper size"
  P.resize((P_size,5))
  return P

def draw_points (P, filename, RAD = 20.0):
  "draws as dots"
  #set up graph
  #M = max(m)
  #canv = pyx.graph.graphxy(
  #    xpos=-RAD, ypos=-RAD,
  #    width=2*RAD, height=2*RAD,
  #    x=pyx.graph.axis.linear(min=-M, max=M),
  #    y=pyx.graph.axis.linear(min=-M, max=M))

  #scale points
  N = shape(P)[0]
  x_size = sqrt(max(P[:,2]**2))
  y_size = sqrt(max(P[:,3]**2))
  scale = RAD / max(x_size, y_size)
  print "scaling by %g" % scale
  P *= scale
  x_size *= scale
  y_size *= scale
  r_size = 4 * sqrt(x_size * y_size) / N #radius scale

  #set up graph
  canv = pyx.canvas.canvas()
  x_box = 1.05*x_size
  y_box = 1.05*y_size
  canv.stroke(pyx.path.rect(-x_box, -y_box, 2*x_box, 2*y_box),
              [pyx.color.rgb.black]) #border

  #draw points
  print "plotting eigenvalues..."
  c_max = max(P[:,0])
  for n in xrange(N):
    x = P[n,2]
    y = P[n,3]
    r = 0.003 * RAD #sqrt(P[n,0]) * r_size
    c = 1 - P[n,0] / c_max
    c = 0.8 * c
    draw_point(canv, x, y, r, c)
  canv.writetofile("%s.eps" % filename)
  os.system("epstopdf %s.eps" % filename)
  os.system("kghostview %s.pdf &" % filename)
  #os.system("acroread -openInNewWindow %s.pdf &" % filename)

def run (filename = "map"):
  P = get_points(filename)
  draw_points(P, filename)

if __name__ == "__main__":
  run()


