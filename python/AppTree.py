#AppTree.py

#__all__ = ['AppTree']

#[ graphics ]
#from pyx import *


"""
#==========[ pyx stuff ]==========
alphabet = 'abcdefghijklmnopqrstuvwxyz'
Alphabet = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'

lineScale = 0.5

center = path.path(path.moveto(0,lineScale/2.0),
                   path.lineto(lineScale/2.0,0),
                   path.lineto(lineScale,lineScale/2.0),
                   path.lineto(lineScale/2.0,lineScale),
                   path.closepath())
vbar = path.line(lineScale/2.0,0,
                 lineScale/2.0,lineScale)
hbar = path.line(0,lineScale/2.0,
                 lineScale,lineScale/2.0)
def drawChar(ch,canv,x,y):
  if ch in alphabet+Alphabet:
    canv.text(x+lineScale/2.0, y+lineScale/2.0,
              ch, text.halign.center, text.valign.center)
    canv.stroke(path.circle(x+lineScale/2.0, y+lineScale/2.0,
                            lineScale/2.0), canvas.linewidth.thin)
  elif ch == '*':
    canv.stroke(center.transformed(trafo.translate(x,y)), canvas.linewidth.thin)
  elif ch == '|':
    canv.stroke(vbar.transformed(trafo.translate(x,y)), canvas.linewidth.thin)
  elif ch == '-':
    canv.stroke(hbar.transformed(trafo.translate(x,y)), canvas.linewidth.thin)
  #else:
    #draw nothing

def drawArray(Ch, fileName=None):
  if fileName is None:
    fileName = "appTree"
  print "writing appTree to file "+fileName
  canv = canvas.canvas()
  for i in range(len(Ch)):
    for j in range(len(Ch[i])):
      drawChar(Ch[i][j], canv, lineScale*j, -lineScale*i)
  canv.writetofile(fileName)
"""

#==========[ merging calculations ]==========
def getSize(height,width):
 #perimeter
  #return height + width
 #area
  #return (2*height-1)*(2*width-1)
 #mixed
  x = 2.0*width+1.0
  y = 2.0*height+1.0
  return x*x + y*y #roughly the area of the bounding circle

def getLwidth(row):
  j = len(row)
  while row[j-1] == 0:
    j -= 1
  return j

def getRwidth(row):
  j = len(row)
  while row[-j] == 0:
    j -= 1
  return j


#==========[ app tree class ]==========
class AppTree:
  def __init__(at,cells = None):
    if cells is None:
      cells = [[]]
    at.cells = cells

 #display
  def __str__(at):
    return '\n'.join([''.join(at.cells[i]) for i in range(2*at.height()-1)])

  def postscript(at,fileName):
    raise "LATER"
    #drawArray(at.cells,fileName)

 #measurements
  def height(at):
    return (len(at.cells)+1)/2

  def width(at):
    return (len(at.cells[0])+1)/2

 #merging
  def __mul__(lhs,rhs):
    height1 = lhs.height()
    height2 = rhs.height()
    width1 = lhs.width()
    width2 = rhs.width()
   #find min area
    sh1 = lhs.silhouette()
    sh2 = rhs.silhouette()
    minArea = getSize(height1+height2, width1+width2) + 1
    for delta_h in range(1,height1+1):
     #at which offest delta_h the area minimized:
      maxDelta_w = 1
      for i in range(delta_h,min(height1,height2+delta_h)):
       #on which row does the worst conflict appear:
        delta_w = getLwidth(sh1[i]) + getRwidth(sh2[i-delta_h]) - width1
        if delta_w > maxDelta_w:
          maxDelta_w = delta_w
      delta_w = maxDelta_w
      height = max(height1, height2+delta_h)
      width = max(width1+delta_w,width2)
      area  = getSize(height, width)
      if area < minArea:
        minArea = area
        minDelta_h = delta_h
        minDelta_w = delta_w
    delta_h = minDelta_h
    delta_w = minDelta_w
   #build new cells
    height = max(height1,height2+delta_h)
    width = max(width1+delta_w,width2)
    cells = [[' ' for j in range(2*width-1)] for i in range(2*height-1)]
   #draw existing trees within new cells
    i0 = 0
    j0 = 2*(width-width1-delta_w)
    for i in range(2*height1-1):
      for j in range(2*width1-1):
        if lhs.cells[i][j] != ' ':
          cells[i0+i][j0+j] = lhs.cells[i][j]
    i0 = 2*delta_h
    j0 = 2*(width - width2)
    for i in range(2*height2-1):
      for j in range(2*width2-1):
        if rhs.cells[i][j] != ' ':
          cells[i0+i][j0+j] = rhs.cells[i][j]
   #connect the trees
    cells[0][-1] = '*'
    for i in range(1,2*delta_h):
      cells[i][-1] = '|'
    for j in range(-2*delta_w,-1):
      cells[0][j] = '-'
    return AppTree(cells)

  def silhouette(at):
   #make a full matrix
    height = at.height()
    width = at.width()
    shadow = [[1 for j in range(width)] for i in range(height)]
   #remove UL corner
    for i in range(height):
      for j in range(width):
        if at.cells[2*i][2*j] != ' ':
          break
        shadow[i][j] = 0
   #remove DR corner
    for i in range(height):
      for j in range(-1,-width-1):
        if at.cells[2*i][2*j] != ' ':
          break
        shadow[i][j] = 0
    return shadow



