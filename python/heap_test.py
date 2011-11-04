
depth = 32
maxPos = 2**depth

def heap_up(pos):
  return pos/2

def heap_left(pos):
  return 2*pos

def heap_right(pos):
  return 2*pos + 1

def heap_hasLchild(pos):
  return heap_isValid(heap_left(pos))

def heap_hasRchild(pos):
  return heap_isValid(heap_right(pos))

def heap_isLchild(pos):
  return (pos+1) % 2

def heap_isRchild(pos):
  return pos % 2

def heap_isValid(pos):
  return (0 < pos) and (pos < maxPos)

def inorder_first(pos=1):
  while heap_hasLchild(pos):
    pos = heap_left(pos)
  return pos

def inorder_next(pos):
  if heap_hasRchild(pos):
    pos = inorder_first(heap_right(pos))
  elif heap_isLchild(pos):
    pos = heap_up(pos)
  else:
    while heap_isRchild(pos):
      pos = heap_up(pos)
    pos = heap_up(pos)
  return pos

def inorder_pos(pos):
  "the big function, floating-point version"
  from math import fmod,log
  return fmod(log(2*pos+1),log(2))

def inorder_cmp(lhs,rhs):
  return cmp(inorder_pos(lhs),inorder_pos(rhs))

def inorder_iter():
  pos = inorder_first()
  while heap_isValid(pos):
    yield pos
    pos = inorder_next(pos)

def inorder_test(maxSteps = 100):
  step = 1
  prevPos = 0
  for pos in inorder_iter():
    diff = inorder_pos(pos) - inorder_pos(prevPos)
    print str(pos)+':\t'+str(diff)
    prevPos = pos
    step += 1
    if step > maxSteps:
      break

if __name__ == '__main__':
  print 'running test...'
  inorder_test()
    
