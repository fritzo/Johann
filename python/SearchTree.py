



#==========[ example code for tree searching ]==========
# soon to be deprecated
class SearchTreeNode:
  def __init__(tn,item = None,u = None):
    tn.u = None
    tn.l = None
    tn.r = None
    tn.item = item
    
class SearchTree:
  def __init__(st):
    st.root = None

 #lookup
  def find_traverse(item):
    "returns the items position if found or None, otherwise"
    pos = st.root
    while pos is not None:
      if item < pos.item:
        pos = pos.l
      elif item > pos.item
        pos = pos.r
      else: #item is pos.item
        break
    return pos

  def findNearest_traverse(item):
    pos = st.root
    while pos is not None:
      if item < pos.item:
        nextPos = pos.l
      elif item > pos.item
        nextPos = pos.r
      else: #item is pos.item
        break
      if nextPos is None:
        break
      pos = nextPos
    return pos

 #insertion/removal
  def insert_traverse(st,item):
    pos = st.findNearest(item)
    if pos is None:
      st.root = TreeNode(item)
    elif item < pos.item:
      pos.l = TreeNode(item,pos)
    elif item > pos.item:
      pos.r = TreeNode(item,pos)

  def remove_traverse(item):
    raise LATER

 #iteration
  def inorder_funStack(item):
    if item.l:  L = item.l.inorder_funStack()
    else:       L = []
    if item.r:  R = item.r.inorder_funStack()
    else:       R = []
    result = L + [item] + R

  def getFirst_traverse(st):
    pos = st.root
    prevPos = pos
    while pos is not None:
      prevPos = pos
      pos = pos.l
    return pos
      
  def getNext_traverse(st,pos):
    if pos is not None:
      while pos.u is not None:
        if pos.u.r is pos:
          raise LATER
