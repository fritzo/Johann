#Heap.py

__all__ = [
  'heap_up','heap_left','heap_right',
  'heap_index','heap_cmp',
  'Heap'
]

from exceptions import Exception

LATER = Exception('unfinished code in Heap.py')

#==========[ traversal functions ]==========
def heap_up(pos):
  return pos/2

def heap_left(pos):
  return 2*pos

def heap_right(pos):
  return 2*pos+1

def heap_index(pos):
  "floating-point heap index for inorder comparison"
  from math import log,fmod
  return fmod(log(2*pos+1),log(2))

def heap_cmp(lhs,rhs):
  "comparison for inorder binary search tree (sub-heap)"
  return cmp(heap_index(lhs),heap_index(rhs))


#==========[ heap class ]==========
class Heap:
  "Heap, a bounded-size search-tree set implementation"
  def __init__(h,itemType,numItems):
    h.numItems = numItems
    h.items = [None for n in range(numItems())]

  def __getitem__(h,pos):
    "returns item with given pos, or None if it is not in the heap"
    if pos > h.numItems:  result = None
    else:                 result = h.items[pos+1]
    return result

  def __setitem__(h,pos,value):
    assert pos <= h.numItems
    h.items[pos+1] = value
    return value

 #[ sorted access ]==========
  def insert(h,item):
    "inserts an item in the heap, rebalancing if space is needed"
    raise LATER

  def remove(h,item):
    "left-heavy recursive removal"
    pos = h.__find(item)
    if pos is not None:
      right_pos = heap_right(pos)
      right = h[heap_right(pos)]
      if right is not None:
        h[pos] = right
        h.remove(right_pos)
      else:
        left_pos = heap_left(pos)
        left = h[left_pos]
        if left is not None:
          h[pos] = left
          h.remove(left_pos)

  def contains(h,item):
    return h.__find(pos) is not None
    
  def __find(h,item):
    pos = 1
    while h[pos] is not None:
      if item < h[pos]:    pos = heap_left(pos)
      elif item > h[pos]:  pos = heap_right(pos)
      else:                break #item == h[pos]
    return pos
      
 #[ sorting ]==========
  "using the notation of _Intro. to Alg's_ by Cormen et al."
  def sort(h):
    "classic heap-sort"
    numItems = h.numItems
    for n in range(numItems,1,-1):
      h.numItems = n
      h.__ensure_max_heap(h)
      h[1],h[n] = h[n],h[1]
    h.numItems = numItems

  def push(h,item):
    "insert an item with given priority"
    h.numItems += 1
    if len(h.items) < h.numItems:
      h.items += [None]
    h[h.numItems] = item
    h.__decreaseValue
    

  def pop(h):
    "extract the min item from the heap"
    item = h[1]
    h[1] = h[h.numItems]
    h.numItems -= 1
    h.__increaseValue(item)
    
  def __ensure_min_heap(h):
    "ensures that each child is less than its parent"
    for pos in range(h.numItems/2,0,-1):
      h.__decreaseValue(pos)

  def __decreaseValue(h,pos):
    "restores min-heap property after value at pos decreases"
    while pos > 1:
      up = heap_up(pos)
      posVal,upVal = h[pos],h[up]
      if upVal < posVal:
        h[pos],h[up] = h[up],h[pos]
        pos = up
      else:
        break

  def __increaseVal(h,pos):
    "restores min-heap property after value at pos increases"
    while pos < h.numItems/2: #i.e., while pos is not a leaf
      posVal,leftVal,rightVal = h[pos],h[heap_left(pos)],h[heap_right(pos)]
      minPos,minVal = pos,posVal
      if leftVal < minVal:   minPos,minVal = left,leftVal
      if rightVal is not None:
        if rightVal < minVal:  minPos,minVal = right,rightVal
      if minPos is not Pos:
        h[pos],h[minPos] = h[minPos],h[pos]
        pos = minPos
      else:
        break





