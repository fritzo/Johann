from PriorityQueue import *

PQ = PriorityQueue
pq = PQ()
pq.insert('a',0)
pq.insert('b',1)
pq.insert('c',2)
pq.insert('d',10)
pq.insert('e',9)
pq.insert('f',8)

while len(pq) > 0:
  print pq.remove()
