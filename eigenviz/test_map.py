
import os

def write_graph (filename, num_verts, edge_dict):

  file = open(filename, 'w')

  file.write('GRAPH\n\n')

  file.write('%i VERTICES\n\n' % num_verts)

  file.write('%i EDGES\n' % len(edge_dict))
  for (i,j),w in edge_dict.iteritems():
    file.write('%i %i %g\n' % (i,j,w))

  file.close()

def make_torus (w = 40, h = 50):

  num_verts = w * h
  vert_index = dict(((i,j), h*i+j) for i in range(w) for j in range(h))
  edge_dict = {}

  for i in range(w):
    for j in range(h):
      edge_dict[vert_index[i,j], vert_index[(i+1)%w, j]] = 1.0
      edge_dict[vert_index[i,j], vert_index[i, (j+1)%h]] = 1.0

  write_graph('test.graph', num_verts, edge_dict)

def make_tube (w = 40, h = 50):

  num_verts = w * h
  vert_index = dict(((i,j), h*i+j) for i in range(w) for j in range(h))
  edge_dict = {}

  for i in range(w):
    for j in range(h):
      edge_dict[vert_index[i,j], vert_index[(i+1)%w, j]] = 1.0
    for j in range(h-1):
      edge_dict[vert_index[i,j], vert_index[i, j+1]] = 1.0

  write_graph('test.graph', num_verts, edge_dict)

def make_square (w = 40, h = 50):

  num_verts = w * h
  vert_index = dict(((i,j), h*i+j) for i in range(w) for j in range(h))
  edge_dict = {}

  for i in range(w-1):
    for j in range(h):
      edge_dict[vert_index[i,j], vert_index[i+1, j]] = 1.0

  for i in range(w):
    for j in range(h-1):
      edge_dict[vert_index[i,j], vert_index[i, j+1]] = 1.0

  write_graph('test.graph', num_verts, edge_dict)

def make_klein_bottle (w = 40, h = 40):

  num_verts = w * h
  vert_index = dict(((i,j), h*i+j) for i in range(w) for j in range(h))
  edge_dict = {}

  for i in range(w-1):
    for j in range(h):
      edge_dict[vert_index[i,j], vert_index[i+1, j]] = 1.0

  for j in range(h):
    edge_dict[vert_index[w-1,j], vert_index[0, h-1-j]] = 1.0

  for i in range(w):
    for j in range(h):
      edge_dict[vert_index[i,j], vert_index[i, (j+1)%h]] = 1.0

  write_graph('test.graph', num_verts, edge_dict)

def make_box (size = 20):
  num_verts = 6 * size * size
  vert_index = dict(((n,i,j), j + size * (i + size * n))
                    for n in range(6)
                    for i in range(size)
                    for j in range(size))
  edge_dict = {}
  def add_edge (v1, v2):
    edge_dict[vert_index[v1], vert_index[v2]] = 1.0
  
  def seam (face, side):
    if side == 0: return [(face, 0,               i) for i in range(size)]
    if side == 1: return [(face, i,        size-1  ) for i in range(size)]
    if side == 2: return [(face, size-1,   size-1-i) for i in range(size)]
    if side == 3: return [(face, size-1-i,        0) for i in range(size)]

  def sew (seam1, seam2, orientation = True):
    if orientation: seam1.reverse()
    for v1,v2 in zip(seam1,seam2):
      add_edge(v1,v2)

  # weave faces
  for n in range(6):
    for i in range(size):
      for j in range(size-1):
        add_edge((n, i, j), (n, i, j+1))
        add_edge((n, j, i), (n, j+1, i))

  # sew together a cylinder
  sew(seam(0,2), seam(1,0))
  sew(seam(1,2), seam(2,0))
  sew(seam(2,2), seam(3,0))
  sew(seam(3,2), seam(0,0))

  # sew on a top
  sew(seam(0,1), seam(4,0))
  sew(seam(1,1), seam(4,3))
  sew(seam(2,1), seam(4,2))
  sew(seam(3,1), seam(4,1))

  # sew on a bottom
  sew(seam(0,3), seam(5,0))
  sew(seam(1,3), seam(5,1))
  sew(seam(2,3), seam(5,2))
  sew(seam(3,3), seam(5,3))

  write_graph('test.graph', num_verts, edge_dict)

def make_pill (size = 20):
  num_faces = 10
  num_verts = num_faces * size * size
  vert_index = dict(((n,i,j), j + size * (i + size * n))
                    for n in range(num_faces)
                    for i in range(size)
                    for j in range(size))
  edge_dict = {}
  def add_edge (v1, v2):
    edge_dict[vert_index[v1], vert_index[v2]] = 1.0
  
  def seam (face, side):
    if side == 0: return [(face, 0,               i) for i in range(size)]
    if side == 1: return [(face, i,        size-1  ) for i in range(size)]
    if side == 2: return [(face, size-1,   size-1-i) for i in range(size)]
    if side == 3: return [(face, size-1-i,        0) for i in range(size)]

  def sew (seam1, seam2, orientation = True):
    if orientation: seam1.reverse()
    for v1,v2 in zip(seam1,seam2):
      add_edge(v1,v2)

  # weave faces
  for n in range(num_faces):
    for i in range(size):
      for j in range(size-1):
        add_edge((n, i, j), (n, i, j+1))
        add_edge((n, j, i), (n, j+1, i))

  # sew bottom & top cylinders
  for c in [0,4]:
    sew(seam(0+c,2), seam(1+c,0))
    sew(seam(1+c,2), seam(2+c,0))
    sew(seam(2+c,2), seam(3+c,0))
    sew(seam(3+c,2), seam(0+c,0))

  # sew cylinders together
  sew(seam(0,1), seam(4,3))
  sew(seam(1,1), seam(5,3))
  sew(seam(2,1), seam(6,3))
  sew(seam(3,1), seam(7,3))

  # sew on a bottom
  sew(seam(0,3), seam(8,0))
  sew(seam(1,3), seam(8,1))
  sew(seam(2,3), seam(8,2))
  sew(seam(3,3), seam(8,3))

  # sew on a top
  sew(seam(4,1), seam(9,0))
  sew(seam(5,1), seam(9,3))
  sew(seam(6,1), seam(9,2))
  sew(seam(7,1), seam(9,1))

  write_graph('test.graph', num_verts, edge_dict)

def make_cube (l = 9, w = 10, h = 11):

  num_verts = l * w * h
  vert_index = dict( ((i,j,k), i + l * (j + w * k))
                     for i in range(l)
                     for j in range(w)
                     for k in range(h) )
  edge_dict = {}

  for i in range(l-1):
    for j in range(w):
      for k in range(h):
        edge_dict[vert_index[i,j,k], vert_index[i+1,j,k]] = 1.0

  for i in range(l):
    for j in range(w-1):
      for k in range(h):
        edge_dict[vert_index[i,j,k], vert_index[i,j+1,k]] = 1.0

  for i in range(l):
    for j in range(w):
      for k in range(h-1):
        edge_dict[vert_index[i,j,k], vert_index[i,j,k+1]] = 1.0

  write_graph('test.graph', num_verts, edge_dict)


if __name__ == '__main__':
  #make_torus()
  #make_tube()
  #make_square()
  #make_klein_bottle()
  #make_box()
  #make_pill()
  make_cube()

