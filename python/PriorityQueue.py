#PriorityQueue.py


class PriorityQueue:
  def __init__(pq):
    pq.items = []
    pq.priorities = []

  def __len__(pq):
    return len(pq.items)

  def __nonzero__(pq):
    return len(pq.items) != 0

  def enqueue(pq,item,priority):
    pq.remove(item)
    i = 0
    while i < len(pq.items):
      if pq.priorities[i] > priority:
        break
      else:
        i += 1
    pq.items.insert(i,item)
    pq.priorities.insert(i,priority)

  def dequeue(pq):
    if pq:
      result = pq.items[0]
      del pq.items[0]
      del pq.priorities[0]
    else:
      result = None
    return result

  def remove(pq,item): 
    if item in pq.items: #then remove it
      i = pq.items.index(item)
      del pq.items[i]
      del pq.priorities[i]


