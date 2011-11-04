

def heap_idx1 (i,size):
  idx = (i<<1) + 1
  while not (idx & size):
    idx <<= 1
  return idx

def heap_idx2 (i,size):
  "reverses bits"
  idx = i & 1
  for n in range(4):
    idx <<= 1
    i >>= 1
    idx |= i & 1
  return idx

if __name__ == '__main__':
  size = 32
  for i in range(size):
    print "%i\t%i" % (i, heap_idx2(i,size))
  print "-----"
  #for i in range(size):
  #  for j in range(size)

