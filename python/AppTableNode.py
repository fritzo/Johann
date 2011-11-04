#Node.py

__all__ = ['Node','makeApp']

from exceptions import Exception

LATER = Exception('unfinished code in Node.py')


#==========[ safe wrapper for node construction ]==========
def makeApp(db,lhs,rhs):
  "safely make an ob app from known {lhs, rhs}"
  assert lhs.MIO_find(rhs) is None, "cannot makeApp; app already exists"
  app = db.allocNode()
  app.lhs,app.rhs,app.app = lhs,rhs,app
  app.insert()
  return app.getRep()


#==========[ node in the sparse app table ]==========
class UnprunableNode(Exception):
  def __init__(self,*args):  pass
  def __str__(self):  return 'unprunable node found.'

class SearchTreeNode:
  "search tree node, this would be a struct in C"
  def __init__(tn):
    tn.reset()

  def reset(tn):
    tn.up    = None
    tn.left  = None
    tn.right = None
    
class Node:
  """each node corresponding to the equation app=lhs*rhs .
  the node represents an ob $x$ if $app = x$, i.e., if node.IOM_isRoot().
  combines functionality as memeber of many sparse tables' search-trees."""

  def __init__(node,db,id):
    node.db = db
    node.id = id
   #memory allocation
    node.alloc_next = None
   #maps & multimaps (Me.xxx[In]=Out)
    node.MIO = SearchTreeNode()
    node.IMO = SearchTreeNode()
    node.IOM = SearchTreeNode() #defines app
    node.OIM = SearchTreeNode()
   #obs, i.e., IOM root nodes
    node.lhs = None
    node.rhs = None
    node.app = None
   #rep structure, forming representitive trees
    node.rep = node
   #measure structure
    node.meas = 0.0
    node.parseSum = None
    node.pruneSum = None
   #permanence
    node.isPermanent = False

  def __cmp__(lhs,rhs):
    "order for binary search tree"
    from Heap import heap_cmp
    return heap_cmp(lhs.id,rhs.id)

  def __hash__(node):
    return node.id

  def __repr__(node):
    return "db.nodes[%d]" % node.id

  def __str__(node):
    "randomly converts to string, may be different upon different calls"
    lhs = node.lhs.parse()
    rhs = node.rhs.parse()
    app = node.app.parse()
    return "[%1 * %2 = %3]" % (lhs,rhs,app)

  def __mul__(lhs,rhs):
    assert lhs.isOb()
    assert rhs.isOb()
    return lhs.MIO_find(rhs)

  def isOb(node):
    return node.app is node #== node.IOM_isRoot()

 #[ node memory management ]==========
  def isFree(node):
    return node.alloc_next is not None

  def isUsed(node):
    return node.alloc_next is None

  def free(node):
    assert node.isUsed(), "cannot free node; node is already free"
    db = node.db
   #update db's free node list
    node.alloc_next = db.freeNodes
    db.freeNodes = node
    db.numFreeNodes += 1
   #update node
    node.rep = None

 #[ parsing ]==========
  def parse(ob,t=None):
    "randomly translate an ob to expr, WRT join measure"
    assert ob.isOb()
    db = ob.db
    if t is None:
      from random import uniform
      t = uniform(0,db.joinPMF[ob])
    eps = db.epsilon
    if t < eps*db.join0PMF[ob]: #atomic expr
      expr = db.basis_ob2expr[ob]
    else: #compound expr
      t = (t-eps*db.join0PMF[ob])/(1-eps)
      node = ob.parse_randomChoice(ob,t)
      expr = node.lhs.parse() * node.rhs.parse()
    return expr

  def parse_randomChoice(root,t=None):
    "chooses random node from parse tree WRT parse measure"
    assert root.IOM_isRoot(), "must choose from root"
    if t is None:
      from random import uniform
      t = uniform(0,root.parseSum)
    pos = root
    node = None
    while not node:
      left = pos.IOM_getLeft()
      right = pos.IOM_getRight()
      if left and right:
        if t < left.parseSum:
          pos = left; break
        elif t < left.parseSum + right.parseSum:
          pos = right; break
        node = pos
      elif left:
        if t < left.parseSum:
          pos = left; break
        node = pos
      elif right:
        if t < right.parseSum:
          pos = right; break
        node = pos
      else:
        node = pos
    return node

 #[ pruning ]==========
  def prune(node):
    assert node.isPrunable()
   #remove from db's PMFs:
    if node.isOb():
      db.join0PMF[ob] = 0.0
      db.joinPMF[ob] = 0.0
      db.split0PMF[ob] = 0.0
      db.splitPMF[ob] = 0.0
   #prune unsupported MIO and IMO apps
    "Note: this addresses the pruneMeas = 1/0 problem"
    "all subsequent nodes must be pruned"
    raise LATER
   #remove from {M,I,O} structure
    node.remove()

  def isPrunable(node):
    "checks whether node is globally prunable"
    result = True
    try:                    nodes = node.getPruneClosure()
    except UnprunableNode:  result = False
    return result

  def getPruneClosure(node):
    """finds all newly disconnected nodes and adds them to the prune queue;
    if any unprunable node is found, an UnprunableNode exception is raised.
    Note: this addresses the pruneMeas = 1/0 problem"""
    from Set import Set
    old_nodes = Set()
    mid_nodes = Set([node])
    new_nodes = Set()
    while len(mid_nodes) > 0:
      for node in new_nodes:
        if node.MIO_isRoot():
          raise LATER
          if not new_node.isLocallyPrunable():  raise UnprunableNode
        if node.IMO_isRoot():
          raise LATER
          if not new_node.isLocallyPrunable():  raise UnprunableNode
      new_nodes,mid_nodes,old_nodes = Set(),new_nodes,mid_nodes+old_nodes
    
  def isLocallyPrunable(node):
    "checks whether a node would be prunable if no other nodes were pruned"
    assert node.isUsed() #only full dbs should be pruned
    result = True
    if node.isPermanent:      result = False #for db axioms
    elif node.isSupport:      result = False #for db.__createExpr
    elif node.MIO_isEmpty():  result = False #node solely supports an ob
    elif node.IMO_isEmpty():  result = False #node solely supports an ob
    return result

  def prune_randomChoice(root,t=None):
    "chooses random node from prune tree WRT prune measure"
    assert root.heap_isRoot(), "must choose from root"
    if t is None:
      from random import uniform
      t = uniform(0,root.pruneSum)
    pos = root
    node = None
    while not node:
      left = pos.heap_getLeft()
      right = pos.heap_getRight()
      if left and right:
        if t < left.pruneSum:
          pos = left; break
        elif t < left.pruneSum + right.pruneSum:
          pos = right; break
        node = pos
      elif left:
        if t < left.pruneSum:
          pos = left; break
        node = pos
      elif right:
        if t < right.pruneSum:
          pos = right; break
        node = pos
      else:
        node = pos
    return node

 #[ heap traversal ]==========
  def heap_isRoot(node):
    return node.id == 1

  def heap_getUp(node):
    "returns node's parent if it exists, or None otherwise"
    if node.id == 1:  result = None
    else:             node.db.nodes[node.id/2]
    return result
  
  def heap_getLeft(node):
    "returns node's left child if it exists, or None otherwise"
    id = 2*node.id
    if id < db.numNodes:  result = db.nodes[id]
    else:                 result = None
    return result
  
  def heap_getRight(node):
    "returns node's right child if it exists, or None otherwise"
    id = 2*node.id + 1
    if id < db.numNodes:  result = db.nodes[id]
    else:                 result = None
    return result

  def heap_getMeet(lhs,rhs):
    "returns latest common ancestor"
    while rhs.heap_higherThan(lhs): lhs = lhs.heap_up
    while lhs.heap_higherThan(rhs): rhs = rhs.heap_up
    while lhs is not rhs:
      lhs = lhs.heap_up
      rhs = rhs.heap_up
    return lhs

 #[ map & multimap methods ]==========
 #root methods
  #property
  def MIO_isRoot(node):  return node.MIO.up is None
  def IMO_isRoot(node):  return node.IMO.up is None
  def IOM_isRoot(node):  return node.IOM.up is None
  def OIM_isRoot(node):  return node.OIM.up is None
 
  #lookup
  def MIO_getRoot(node):
    pos = node
    while pos.MIO.up is not None:  pos = pos.up
    return pos

  def IMO_getRoot(node):
    pos = node
    while pos.IMO.up is not None:  pos = pos.up
    return pos

  def IOM_getRoot(node):
    pos = node
    while pos.IOM.up is not None:  pos = pos.up
    return pos

  def OIM_getRoot(node):
    pos = node
    while pos.OIM.up is not None:  pos = pos.up
    return pos

  #status
  #def MIO_makeRoot(root):  pass
  #def IMO_makeRoot(root):  pass
  #def OIM_makeRoot(root):  pass
  def IOM_makeRoot(root):
    "makes a node the acknowledged root assuming it is already in place"
    assert root.app is not app
    #XXX: moving node within tree may break for root node
    for node in root.IOM_iter():
      node.app = root
    for node in root.MIO_iter():
      assert node is not root #XXX
      node.IOM_remove();  node.OIM_remove();  node.IMO_remove()
      node.lhs = root
      node.IOM_insert();  node.OIM_insert();  node.IMO_insert()
    for node in root.IMO_iter():
      assert node is not root #XXX
      node.IOM_remove();  node.OIM_remove();  node.MIO_remove()
      node.rhs = root
      node.IOM_insert();  node.OIM_insert();  node.MIO_insert()

 #isEmpty
  def MIO_isEmpty(node):
    "actually, is nearly empty, i.e., is the root a leaf?"
    assert node.MIO_isRoot()
    return (node.MIO.left is None) and (node.MIO.right is None)

  def IMO_isEmpty(node):
    "actually, is nearly empty, i.e., is the root a leaf?"
    assert node.IMO_isRoot()
    return (node.IMO.left is None) and (node.IMO.right is None)

  def IOM_isEmpty(node):
    "actually, is nearly empty, i.e., is the root a leaf?"
    assert node.IOM_isRoot()
    return (node.IOM.left is None) and (node.IOM.right is None)

  def OIM_isEmpty(node):
    "actually, is nearly empty, i.e., is the root a leaf?"
    assert node.OIM_isRoot()
    return (node.OIM.left is None) and (node.OIM.right is None)

 #comparison methods
  #vertical
  def MIO_Vcmp(lhs,rhs):
    return cmp(lhs.rhs.id,rhs.rhs.id)

  def IMO_Vcmp(lhs,rhs):
    return cmp(lhs.lhs.id,rhs.lhs.id)

  def IOM_Vcmp(lhs,rhs):
    result = cmp(lhs.lhs.id,rhs.lhs.id)
    if result == 0:
      result = cmp(lhs.rhs.id,rhs.rhs.id)
    return result

  def OIM_Vcmp(lhs,rhs):
    result = cmp(lhs.rhs.id,rhs.rhs.id)
    if result == 0:
      result = cmp(lhs.lhs.id,rhs.lhs.id)
    return result

  #horizontal
  def MIO_Hcmp(lhs,rhs):
    from Heap import heap_cmp
    return heap_cmp(lhs.rhs.id,rhs.rhs.id)

  def IMO_Hcmp(lhs,rhs):
    from Heap import heap_cmp
    return heap_cmp(lhs.lhs.id,rhs.lhs.id)

  def IOM_Hcmp(lhs,rhs):
    from Heap import heap_cmp
    result = heap_cmp(lhs.lhs.id,rhs.lhs.id)
    if result == 0:
      result = heap_cmp(lhs.rhs.id,rhs.rhs.id)
    return result

  def OIM_Hcmp(lhs,rhs):
    from Heap import heap_cmp
    result = heap_cmp(lhs.rhs.id,rhs.rhs.id)
    if result == 0:
      result = heap_cmp(lhs.lhs.id,rhs.lhs.id)
    return result

 #lookup
  def MIO_find(pos,rhs):
    """given a node pos such that pos.lhs = pos,
       finds a node pos such that pos.rhs = rhs,
       or returns None if none exists"""
    from Heap import heap_index
    assert pos.MIO_isRoot() #i.e., pos = pos.lhs
    rhs_index = heap_index(rhs.id)
    while pos:
      compare = cmp(rhs_index,heap_index(pos.rhs.id))
      if   compare > 0:  pos = pos.MIO.left
      elif compare < 0:  pos = pos.MIO.right
      else:              break #found!
    return pos
    
  def IMO_find(pos,lhs):
    """given a node pos such that pos.rhs = pos,
       finds a node pos such that pos.lhs = lhs,
       or returns None if none exists"""
    from Heap import heap_index
    assert pos.IMO_isRoot() #i.e., pos = pos.rhs
    lhs_index = heap_index(lhs.id)
    while pos:
      compare = cmp(lhs_index,heap_index(pos.lhs.id))
      if   compare > 0:  pos = pos.IMO.left
      elif compare < 0:  pos = pos.IMO.right
      else:              break #found!
    return pos

  def IOM_findKey(pos,lhs):
    """given      a      node pos such that pos.app = pos,
       finds the highest node pos such that pos.lhs = lhs,
       or returns None if none exists"""
    assert pos.IOM_isRoot() #i.e., pos == pos.app
    lhs_index = heap_index(lhs.id)
    while pos:
      compare = cmp(lhs_index,heap_index(pos.lhs.id))
      if   compare > 0:  pos = pos.IOM.left
      elif compare < 0:  pos = pos.IOM.right
      else:              break #found!
    return pos

  def OIM_findKey(pos,rhs):
    """given      a      node pos such that pos.app = pos,
       finds the highest node pos such that pos.rhs = rhs,
       or returns None if none exists"""
    assert pos.OIM_isRoot() #i.e., pos == pos.app
    rhs_index = heap_index(rhs.id)
    while pos:
      compare = cmp(rhs_index,heap_index(pos.rhs.id))
      if   compare > 0:  pos = pos.OIM.left
      elif compare < 0:  pos = pos.OIM.right
      else:              break #found!
    return pos

 #insertion methods
  #all trees
  def insert(node):
    """inserts an node in its {M,I,O} structure,
    assuming {lhs,rhs,app} have all been set
    and no node is already at the insert position"""
    assert node.lhs.isOb()
    assert node.rhs.isOb()
    assert node.app.isOb()
    if not node.MIO_isRoot():  node.MIO_insert()
    if not node.IMO_isRoot():  node.IMO_insert()
    if not node.IOM_isRoot():  node.IOM_insert()
    if not node.OIM_isRoot():  node.OIM_insert()
    node.db.enforceQueue.insert(node)

  #insert
  def MIO_insert(node):
    """insert a node in its MIO tree, respecting the sub-heap structure,
    assuming the {lhs,rhs,app} structure has been built"""
    assert not node.MIO_isRoot()
    old = node.lhs.MIO_getRoot()
    new = node
    while old:
      if new.MIO_Vcmp(old) > 0:
        new.MIO,old.MIO = old.MIO,new.MIO
        top,bottom = new,old
      elif new.MIO_Vcmp(old) < 0:
        top,bottom = old,new
      else: #need to merge nodes
        old.mergeWith(new)
        break
      new = bottom
      if top.MIO_Hcmp(bottom) > 0:
        old = top.MIO.left
        if old is None:
          top.MIO.left = bottom
          bottom.MIO.up = top
      elif top.MIO_Hcmp(bottom) < 0:
        old = top.MIO.right
        if old is None:
          top.MIO.right = bottom
          bottom.MIO.up = top
      else: #need to merge
        old.mergeWith(new)
        break
  
  def IMO_insert(node):
    """insert a node in its IMO tree, respecting the sub-heap structure,
    assuming the {lhs,rhs,app} structure has been built"""
    assert not node.IMO_isRoot()
    raise LATER #update with mergers as in MIO
    old = node.rhs.IMO_getRoot()
    new = node
    while old:
      if new.IMO_Vcmp(old) > 0:
        new.IMO,old.IMO = old.IMO,new.IMO
        top,bottom = new,old
      else: #new.IMO_Vcmp(old) < 0
        top,bottom = old,new
      new = bottom
      if top.IMO_Hcmp(bottom) > 0:
        old = top.IMO.left
        if old is None:
          top.IMO.left = bottom
          bottom.IMO.up = top
      else: #top.IMO_Hcmp(bottom) < 0:
        old = top.IMO.right
        if old is None:
          top.IMO.right = bottom
          bottom.IMO.up = top

  def IOM_insert(node):
    """insert a node in its IOM tree, respecting the sub-heap structure,
    assuming the {lhs,rhs,app} structure has been built"""
    raise LATER #update with mergers as in MIO
    assert not node.IOM_isRoot()
    old = node.app.IOM_getRoot()
    new = node
    while old:
      if new.IOM_Vcmp(old) > 0:
        new.IOM,old.IOM = old.IOM,new.IOM
        top,bottom = new,old
      else: #new.IOM_Vcmp(old) < 0
        top,bottom = old,new
      new = bottom
      if top.IOM_Hcmp(bottom) > 0:
        old = top.IOM.left
        if old is None:
          top.IOM.left = bottom
          bottom.IOM.up = top
      else: #top.IOM_Hcmp(bottom) < 0:
        old = top.IOM.right
        if old is None:
          top.IOM.right = bottom
          bottom.IOM.up = top

  def OIM_insert(node):
    """insert a node in its OIM tree, respecting the sub-heap structure,
    assuming the {lhs,rhs,app} structure has been built"""
    raise LATER #update with mergers as in MIO
    assert not node.OIM_isRoot()
    old = node.app.OIM_getRoot()
    new = node
    while old:
      if new.OIM_Vcmp(old) > 0:
        new.OIM,old.OIM = old.OIM,new.OIM
        top,bottom = new,old
      else: #new.OIM_Vcmp(old) < 0
        top,bottom = old,new
      new = bottom
      if top.OIM_Hcmp(bottom) > 0:
        old = top.OIM.left
        if old is None:
          top.OIM.left = bottom
          bottom.OIM.up = top
      else: #top.OIM_Hcmp(bottom) < 0:
        old = top.OIM.right
        if old is None:
          top.OIM.right = bottom
          bottom.OIM.up = top

  #insertBefore
  def MIO_insertBefore(new,pos):
    raise LATER

  #insertAfter
  def MIO_insertAfter(new,pos):
    raise LATER

 #removal methods
  #all trees
  def remove(node):
    """remove a node from its {M,I,O} structure,
       assuming all tree descendents have been removed"""
    rootError = Exception("cannot remove root of nonempty tree")
    assert (not node.MIO_isRoot()) or node.MIO_isEmpty(), rootError
    assert (not node.IMO_isRoot()) or node.IMO_isEmpty(), rootError
    assert (not node.IOM_isRoot()) or node.IOM_isEmpty(), rootError
    assert (not node.OIM_isRoot()) or node.OIM_isEmpty(), rootError
    node.MIO_remove()
    node.IMO_remove()
    node.IOM_remove()
    node.OIM_remove()

  #remove
  def MIO_remove(node):
    "removes a node from its MIO tree, respecting the sub-heap structure"
    up,left,right = node.MIO.up,node.MIO.left,node.MIO.right
    node.MIO.up,node.MIO.left,node.MIO.right = None,None,None
    if up is not None:
      if up.MIO.left is node:  node.MIO_fillLeftGap(up,left,right)
      else:                    node.MIO_fillRightGap(up,left,right)
    else: #up is None
      if left is not None:
        if right is not None:
          if left.MIO_Vcmp(right) > 0:
            MIO_fillRightGap(left,left.MIO.right,right)
            for node in left.MIO_iter():
              node.lhs = left #re-root lhs-trees
          else: #left.MIO_Vcmp(right) < 0
            MIO_fillLeftGap(right,right.MIO.left,left)
            for node in right.MIO_iter():
              node.lhs = right #re-root lhs-trees

  def IMO_remove(node):
    "removes a node from its IMO tree, respecting the sub-heap structure"
    up,left,right = node.IMO.up,node.IMO.left,node.IMO.right
    node.IMO.up,node.IMO.left,node.IMO.right = None,None,None
    if up is not None:
      if up.IMO.left is node:  node.IMO_fillLeftGap(up,left,right)
      else:                    node.IMO_fillRightGap(up,left,right)
    else: #up is None
      if left is not None:
        if right is not None:
          if left.IMO_Vcmp(right) > 0:
            IMO_fillRightGap(left,left.IMO.right,right)
            for node in left.IMO_iter():
              node.lhs = left #re-root lhs-trees
          else: #left.IMO_Vcmp(right) < 0
            IMO_fillLeftGap(right,right.IMO.left,left)
            for node in right.IMO_iter():
              node.lhs = right #re-root lhs-trees

  def IOM_remove(node):
    "removes a node from its IOM tree, respecting the sub-heap structure"
    up,left,right = node.IOM.up,node.IOM.left,node.IOM.right
    node.IOM.up,node.IOM.left,node.IOM.right = None,None,None
    if up is not None:
      if up.IOM.left is node:  node.IOM_fillLeftGap(up,left,right)
      else:                    node.IOM_fillRightGap(up,left,right)
    else: #up is None
      if left is not None:
        if right is not None:
          if left.IOM_Vcmp(right) > 0:
            IOM_fillRightGap(left,left.IOM.right,right)
            for node in left.IOM_iter():
              node.lhs = left #re-root lhs-trees
          else: #left.IOM_Vcmp(right) < 0
            IOM_fillLeftGap(right,right.IOM.left,left)
            for node in right.IOM_iter():
              node.lhs = right #re-root lhs-trees
        #now IOM has to do some extra stuff
        else: #right is None
          left.IOM_makeRoot()
      else: #left is None
        if right is not None:
          right.IOM_makeRoot()
        else: #right is None
          pass

  def OIM_remove(node):
    "removes a node from its OIM tree, respecting the sub-heap structure"
    up,left,right = node.OIM.up,node.OIM.left,node.OIM.right
    node.OIM.up,node.OIM.left,node.OIM.right = None,None,None
    if up is not None:
      if up.OIM.left is node:  node.OIM_fillLeftGap(up,left,right)
      else:                    node.OIM_fillRightGap(up,left,right)
    else: #up is None
      if left is not None:
        if right is not None:
          if left.OIM_Vcmp(right) > 0:
            OIM_fillRightGap(left,left.OIM.right,right)
            for node in left.OIM_iter():
              node.lhs = left #re-root lhs-trees
          else: #left.OIM_Vcmp(right) < 0
            OIM_fillLeftGap(right,right.OIM.left,left)
            for node in right.OIM_iter():
              node.lhs = right #re-root lhs-trees

  #fill left gap
  def MIO_fillLeftGap(node,up,left,right):
    if left is not None:
      if right is not None: #{U,L,R}
        if left.MIO_Vcmp(right) > 0:
          up.MIO.left,left.MIO.up,right.MIO.up = left,up,left
          MIO_fillRightGap(left,left.MIO.right,right)
        else: #left.MIO_Vcmp(right) < 0
          up.MIO.left,left.MIO.up,right.MIO.up = right,right,up
          MIO_fillLeftGap(right,right.MIO.left,left)
      else: #{U,L}
        up.MIO.left,left.MIO.up = left,up
    elif right is not None: #{U,R}
      up.MIO.left,right,MIO.up = right,up

  #fill right gap
  def MIO_fillRightGap(node,up,left,right):
    raise LATER

  #leaf removal
  def MIO_removeLeaf(node):
    if node.MIO.up is not None:
      if node.MIO.up.MIO.left is node:  node.MIO.up.MIO.left = None
      else:                             node.MIO.up.MIO.right = None
    node.MIO.up = None

  def IMO_removeLeaf(node):
    if node.IMO.up is not None:
      if node.IMO.up.IMO.left is node:  node.IMO.up.IMO.left = None
      else:                             node.IMO.up.IMO.right = None
    node.IMO.up = None

  def IOM_removeLeaf(node):
    if node.IOM.up is not None:
      if node.IOM.up.IOM.left is node:  node.IOM.up.IOM.left = None
      else:                             node.IOM.up.IOM.right = None
    node.IOM.up = None

  def OIM_removeLeaf(node):
    if node.OIM.up is not None:
      if node.OIM.up.OIM.left is node:  node.OIM.up.OIM.left = None
      else:                             node.OIM.up.OIM.right = None
    node.OIM.up = None
  
 #iteration methods
  #iter
  def MIO_iter(node):
    "iteration WRT LUR order"
    pos = ob.MIO_first()
    while pos:
      yield pos
      pos = pos.MIO_next()

  def IMO_iter(node):
    "iteration WRT LUR order"
    pos = ob.IMO_first()
    while pos:
      yield pos
      pos = pos.IMO_next()

  def IOM_iter(node):
    "iteration WRT LUR order"
    pos = ob.IOM_first()
    while pos:
      yield pos
      pos = pos.IOM_next()

  def OIM_iter(node):
    "iteration WRT LUR order"
    pos = ob.OIM_first()
    while pos:
      yield pos
      pos = pos.OIM_next()

  #first (same for all of {M,I,O})
  def MIO_first(pos):
    "initial position WRT L** order"
    while pos.MIO.left:  pos = pos.MIO.left
    return pos

  def IMO_first(pos):
    "initial position WRT L** order"
    while pos.IMO.left:  pos = pos.IMO.left
    return pos

  def IOM_first(pos):
    "initial position WRT L** order"
    while pos.IOM.left:  pos = pos.IOM.left
    return pos

  def OIM_first(pos):
    "initial position WRT L** order"
    while pos.OIM.left:  pos = pos.OIM.left
    return pos
 
  #next (same for all of {M,I,O})
  def MIO_next(pos):
    "successor WRT LUR order"
    if pos.MIO.right:
      result = pos.MIO.right.MIO_first()
    else:
      while pos: #assumes that pos.isRoot() ==> pos.MIO.u == None
        pos,old_pos = pos.MIO.up
        if pos is None:  break #XXX: this should be updated in the other versions.
        if old_pos is not pos.MIO.right:  break
    return pos
    
  def IMO_next(pos):
    "successor WRT LUR order"
    if pos.IMO.right:
      result = pos.IMO.right.IMO_first()
    else:
      while pos: #assumes that pos.isRoot() ==> pos.MIO.u == None
        if pos.IMO.up.IMO.right is pos:
          pos = pos.IMO.up
        else:
          pos = pos.IMO.up
          break
    return pos

  def IOM_next(pos):
    "successor WRT LUR order"
    if pos.IOM.right:
      result = pos.IOM.right.IOM_first()
    else:
      while pos: #assumes that pos.isRoot() ==> pos.IOM.u == None
        if pos.IOM.up.IOM.right is pos:
          pos = pos.IOM.up
        else:
          pos = pos.IOM.up
          break
    return pos
    
  def OIM_next(pos):
    "successor WRT LUR order"
    if pos.OIM.right:
      result = pos.OIM.right.OIM_first()
    else:
      while pos: #assumes that pos.isRoot() ==> pos.MIO.u == None
        if pos.OIM.up.OIM.right is pos:
          pos = pos.OIM.up
        else:
          pos = pos.OIM.up
          break
    return pos    
       
 #key iteration methods
  #iterKey
  def IOM_iterKey(app,lhs):
    "inorder iteration, given a key"
    pos = app.IOM_firstKey(lhs)
    while pos:
      if pos.lhs is not lhs:
        break
      yield pos
      pos = pos.IOM_next()

  def OIM_iterKey(app,rhs):
    "inorder iteration, given a key"
    pos = app.OIM_firstKey(rhs)
    while pos:
      if pos.rhs is not rhs:
        break
      yield pos
      pos = pos.OIM_next()

  #firstKey
  def IOM_firstKey(node,lhs):
    "inorder initial position, given a key"
    pos = node.IOM_findKey(lhs)
    if pos:
      pos = pos.IOM_first()
    return pos

  def OIM_firstKey(node,rhs):
    "inorder initial position, given a key"
    pos = node.OIM_findKey(rhs)
    if pos:
      pos = pos.OIM_first()
    return pos

 #pop methods
  #first
  def MIO_firstPop(node):
    result = None
    while result is None:
      if node.MIO.left:
        node = node.MIO.left
      elif node.MIO.right:
        node = node.MIO.right
      else: #node is leaf
        result = node
    return result

  #next
  def MIO_nextPop(old_node):
    new_node = old_node.MIO.up
    old_node.MIO_removeLeaf()
    if new_node:
      if new_node.MIO.right
    return new_node

  #iter
  def MIO_iterPop(node):
    assert node.MIO_isRoot()
    left = node.MIO_first()
    right = left.MIO_nextPop()
    while right:
      yield left
      left,right = right,right.MIO_nextPop()
    yield left

  #pop
  def MIO_pop(node):
    "pops leaves WRT LRU order, not very efficient under iteration"
    result = node.MIO_firstPop()
    result.MIO_removeLeaf()
    return result

 #merging methods, i.e., map & multimap union operations
  #disjoint union
  def MIO_disjointUnion(lhs,rhs):
    "this is inspiration for the more complicated mergeInto"
    raise LATER

  #mergeInto
  def MIO_mergeInto(dep,rep):
    "inefficient version"
    assert dep.MIO_isRoot()
    assert rep.MIO_isRoot()
    for node in dep.MIO_iterPop():
      node.lhs = rep.lhs #re-roots
      rep.MIO_insert() #should merge if necessary
      
  def IMO_mergeInto(dep,rep):
    "inefficient version"
    assert dep.IMO_isRoot()
    assert rep.IMO_isRoot()
    for node in dep.IMO_iterPop():
      node.rhs = rep.rhs #re-roots
      rep.IMO_insert() #should merge if necessary
      
  def IOM_mergeInto(dep,rep):
    "inefficient version"
    assert dep.IOM_isRoot()
    assert rep.IOM_isRoot()
    for node in dep.IOM_iterPop():
      node.app = rep.app #re-roots
      rep.IOM_insert() #should merge if necessary
      
  def OIM_mergeInto(dep,rep):
    "inefficient version"
    assert dep.OIM_isRoot()
    assert rep.OIM_isRoot()
    for node in dep.OIM_iterPop():
      node.app = rep.app #re-roots
      rep.OIM_insert() #should merge if necessary

  def MIO_mergeInto_fast(dep,rep):
    "more efficient version using insertBefore and insertAfter"
    assert dep.MIO_isRoot()
    assert rep.MIO_isRoot()
    rep_left = rep.MIO_iterFirst()
    rep_right = rep_left.IOM_iterNext()
    new_node = dep.MIO_pop()
    while dep.MIO_Hcmp(rep_left) < 0:
      old_node,new_node = new_node,dep.MIO_pop()
      old_node.lhs = rep.lhs
      old_node.MIO_insertBefore(rep_left)
      if new_node is None:  break
    while True:
      while rep_right.MIO_Hcmp(new_node) < 0:
        rep_left,rep_right = rep_right,rep_right.MIO_next()
        if rep_right is None:
          while True:
            old_node,new_node = new_node,dep.MIO_pop()
            old_node.lhs = rep.lhs
            old_node.MIO_insertAfter(rep_left)
            if new_node is dep:  break
          break
      new_node.MIO_insertAfter(rep_left)
      if new_node is dep:  break

  def MIO_insertBranch(old,new):
    "merges two branches into a single branch"
    if old.id > new.id:
      new.MIO,old.MIO = old.MIO,new.MIO
      up,down = new,old
    else:
      up,down = old,new
    compare = up.MIO_cmp(down)
    if compare < 0:
      if up.MIO.left is None:   up.MIO.left,down.MIO.up = down,up
      else:                     up.MIO.left.MIO_insertBranch(down)
    else:
      if up.MIO.right is None:  up.MIO.right,down.MIO.up = down,up
      else:                     up.MIO.right.MIO_insertBranch(down)

 #[ merging ]==========
  def mergeWith(node1,node2):
    assert node1.isOb()
    assert node2.isOb()
    rep1 = node1.getRep()
    rep2 = node2.getRep()
    if rep1.id < rep2.id:
      rep2.rep = rep1
      rep2.__enqueueMerge()
    elif rep2.id < rep1.id:
      rep1.rep = rep2
      rep1.__enqueueMerge()
    #else: rep1 = rep2, don't merge
    
  def getRep(node):
    assert node.isOb()
    if node.rep is not node:
      node.rep = node.rep.getRep()
    return node.rep

  def __enqueueMerge(node):
    node.db.mergeQueue.enqueue(node,-node.id)
    if __debug__:
      print 'merging: '+str(node)+' --> '+str(node.rep)

  def merge(dep):
    """merges a depricated node into a representative node,
    possibly incuring further mergers"""
   #find rep
    assert dep.isOb()
    rep = (dep.lhs.getRep()*dep.rhs.getRep())
    if rep is None:  rep = dep.getRep() #is this right?
    else:            rep = rep.getRep()
    assert rep.isOb()
    assert dep is not rep
   #merge maps & multipaps
    dep.MIO_mergeInto(rep)
    dep.IMO_mergeInto(rep)
    dep.IOM_mergeInto(rep)
    dep.OIM_mergeInto(rep)
    """old version:
    node.remove()
    node.lhs = node.lhs.getRep()
    node.rhs = node.rhs.getRep()
    node.app = node.app.getRep()
    rep = (node.lhs * node.rhs).getRep()
    if rep is None:
     #connect node to new context
      node.insert()
    else:
      rep = node.getRep()
    """
   #merge permanence & support
    rep.isPermanent |= node.isPermanent;  node.isPermanent = False
    rep.isSupport   |= node.isSupport;    node.isSupport = False
   #merge node measure
    rep.meas += node.meas
   #merge db measure
    db.join0PMF[re]  += db.join0PMF[node];  db.join0PMF[node]  = 0.0
    db.joinPMF[re]   += db.joinPMF[node];   db.joinPMF[node]   = 0.0
    db.split0PMF[re] += db.split0PMF[node]; db.split0PMF[node] = 0.0
    db.splitPMF[re]  += db.splitPMF[node];  db.splitPMF[node]  = 0.0
   #delete node
    dep.free()

 #[ axiom/theorem enforcement ]==========
  axiomNode = """mathematical NOTE:
    an axiom is newly enforcible only if one of its components has just come
    into existence.  The following are minimal node sets for enforcement:
    axiom scheme (K): Kxy = x
      {Kx=K*x, Kxy=Kx*y}
    axiom scheme (S): Sxyz = xz_yz
      {Sx=S*x, Sxy=Sx*y, Sxyz=Sxy*z, xz=x*z, yz=y*z, xz_yz=xz*yz}
    """

  def enforce(node):
    if node.db.K:
      node.__enforceK1()
      node.__enforceK2()
    if node.db.S:
      node.__enforceS1()
      node.__enforceS2()
      node.__enforceS3()
      node.__enforceS4()
      node.__enforceS5()
      node.__enforceS6()
    raise LATER
    if node.db.Q:
      node.__enforceQ1()
      node.__enforceQ2()
  
  def __enforceK1(node): #ob enforcement
    "(K1): K*x=Kx | Kx*y=Kxy"
    K,x,Kx = node.lhs,node.rhs,node.app #K*x=Kx
    if K is node.db.K:
      for Kxy_node in Kx.MIO_iter(): #Kx*y=Kxy
        Kxy = Kxy_node.app
        Kxy.mergeWith(x)

  def __enforceK2(node):
    "(K2): Kx*y=Kxy | K*x=Kx"
    Kx,y,Kxy = node.lhs,node.rhs,node.app #Kx*y=Kxy
    for Kx_node in Kx.IOM_iterKey(node.db.K): #K*x=Kx
      x = Kx_node.rhs
      Kxy.mergeWith(x)
        
  def __enforceS1(node):
    "(S1): S*x=Sx | Sx*y=Sxy, Sxy*z=Sxyz, x*z=xz, y*z=yz, xz*yz=xz_yz"
    S,x,Sx = node.lhs,node.rhs,node.app #S*x=Sx
    if S is node.db.S:
      for Sxy_node in Sx.MIO_iter(): #Sx*y=Sxy
        y,Sxy = Sx_node.rhs,Sx_node.app
        for Sxyz_node in Sxy.MIO_iter(): #Sxy*z=Sxyz
          z = Sxyz_node.rhs
          xz_node = x.MIO_find(z) #x*z=xz
          if xz_node:
            xz = xz_node.app
            yz_node = y.MIO_find(z) #y*z=yz
            if yz_node:
              yz = yz_node.app
              xz_yz_node = xz.MIO_find(yz) #xz*yz=xz_yz
              if xz_yz_node:
                xz_yz = xz_yz_node.app
                Sxyz.mergeWith(xz_yz)
        
  def __enforceS2(node):
    "(S2): Sx*y=Sxy | S*x=Sx, Sxy*z=Sxyz, x*z=xz, y*z=yz, xz*yz=xz_yz"
    Sx,y,Sxy = node.lhs,node.rhs,node.app #Sx*y=Sxy
    if S is node.db.S:
      for Sx_node in Sx.IOM_iterKey(node.db.S): #S*x=Sx
        for Sxyz_node in Sxy.MIO_iter(): #Sxy*z=Sxyz
          z = Sxyz_node.rhs
          xz_node = x.MIO_find(z) #x*z=xz
          if xz_node:
            xz = xz_node.app
            yz_node = y.MIO_find(z) #y*z=yz
            if yz_node:
              yz = yz_node.app
              xz_yz_node = xz.MIO_find(yz) #xz*yz=xz_yz
              if xz_yz_node:
                xz_yz = xz_yz_node.app
                Sxyz.mergeWith(xz_yz)
        
  def __enforceS3(node):
    "(S3): Sxy*z=Sxyz | Sx=S*x, Sx*y=Sxy, x*z=xz, y*z=yz, xz*yz=xz_yz"
    Sxy,z,Sxyz = node.lhs,node.rhs,node.app #Sxy*z=Sxyz
    for Sxy_node in Sxy.IOM_iter(): #Sx*y=Sxy
      Sx,Sxyz = Sxy_node.lhs,Sxy_node.app
      for Sx_node in Sx.IOM_iterKey(node.db.S): #Sx=S*x
        x = Sx_node.rhs
        xz_node = x.MIO_find(z) #x*z=xz
        if xz_node:
           yz_node = y.MIO_find(z) #y*z=yz
           if yz_node:
             yz = yz_node.app
             xz_yz_node = xz.MIO_find(yz) #xz*yz=xz_yz
             if xz_yz_node:
               xz_yz = xz_yz_node.app
               Sxyz.mergeWith(xz_yz)
        
  def __enforceS4(node):
    "(S4): x*z=xz | S*x=Sx, Sx*y=Sxy, Sxy*z=Sxyz, y*z=yz, xz*yz=xz_yz"
    x,z,xz = node.lhs,node.rhs,node.app #x*z=xz
    Sx_node = x.IMO_find(node.db.S) #S*x=Sx
    if Sx_node:
      Sx = Sx_node.app
      for Sxy_node in Sx.MIO_iter(): #Sx*y=Sxy
        y,Sxy = Sxy_node.rhs,Sxy_node.app
        Sxyz_node = Sxy.MIO_find(z) #Sxy*z=Sxyz
        if Sxyz_node:
          Sxyz = Sxyz_node.app
          yz_node = y.MIO_find(z) #y*z=yz
          if yz_node:
            yz = yz_node.app
            xz_yz_node = xz.MIO_find(yz) #xz*yz=xz_yz
            if xz_yz_node:
              xz_yz = xz_yz_node.app
              Sxyz.mergeWith(xz_yz)
        
  def __enforceS5(node):
    "(S5): y*z=yz | S*x=Sx, Sx*y=Sxy, Sxy*z=Sxyz, x*z=xz, xz*yz=xz_yz"
    y,z,yz = node.lhs,node.rhs,node.app #y*z=yz
    for xz_node in z.IMO_iter(): #x*z=xz
      x,xz = xz_node.rhs,xz_node.app
      Sx_node = x.IMO_find(node.db.S) #S*x=Sx
      if Sx_node:
        Sx = Sx_node.app
        Sxy_node = Sx.MIO_find(y) #Sx*y=Sxy
        if Sxy_node:
          Sxy = Sxy_node.app
          Sxyz_node = Sxy.MIO_find(z) #Sxy*z=Sxyz
          if Sxyz_node:
            xz_yz_node = xz.MIO_find(yz) #xz*yz=xz_yz
            if xz_yz_node:
              xz_yz = xz_yz_node.app
              Sxyz.mergeWith(xz_yz)
        
  def __enforceS6(node):
    "(S6): xz*yz=xz_yz | Sx=S*x, Sxy=Sx*y, Sxy*z=Sxyz, x*z=xz, y*z=yz"
    xz,yz,xz_yz = node.lhs,node.rhs,node.app #xz*yz=xz_yz
    for xz_node in xz.IOM_iter(): #x*z=xz
      x,z = xz_node.lhs,xz_node.rhs
      Sx_node = x.IMO_find(node.db.S) #Sx=S*x
      if Sx_node:
        Sx = Sx_node.app
        #Note: this is the only place node.OIM is used:
        for yz_node in yz.OIM_iterKey(z): #y*z=yz
          y = yz_node.lhs
          Sxy_node = Sx.MIO_find(y) #Sxy=Sx*y
          if Sxy_node:
            Sxy = Sxy_node.app
            Sxyz_node = Sxy.MIO_find(z) #Sxy*z=Sxyz
            if Sxyz_node:
              Sxyz = Sxyz_node.app
              Sxyz.mergeWith(xz_yz)
        
  def __enforceS6_alt(node):
    "(S6): xz*yz=xz_yz | Sx=S*x, Sxy=Sx*y, Sxy*z=Sxyz, x*z=xz, y*z=yz"
    #alternate less-efficient version not using node.OIM:
    xz,yz,xz_yz = node.lhs,node.rhs,node.app #xz*yz=xz_yz
    for xz_node in xz.IOM_iter(): #x*z=xz
      x,z = xz_node.lhs,xz_node.rhs
      Sx_node = x.IMO_find(node.db.S) #Sx=S*x
      if Sx_node:
        Sx = Sx_node.app
        for Sxy in Sx.MIO_iter(): #Sxy=Sx*y
          yz_node = y.MIO_find(z) #y*z=yz
          if yz_node:
            if yz_node.app = yz:
              Sxyz_node = Sxy.MIO_find(z) #Sxy*z=Sxyz
              if Sxyz_node:
                Sxyz = Sxyz_node.app
                Sxyz.mergeWith(xz_yz)

  def __enforceQ1(node):
    "(Q1): Qxy=T => x=y |"
    raise LATER
    Qx,y,Qxy = node.lhs,node.rhs,node.app

 #[ reordering ]==========
  def getLhs(node):
    if node.lhs.app is not node.lhs:
      node.lhs = node.lhs.getApp()
    return node.lhs
    
  def getRhs(node):
    if node.rhs.app is not node.rhs:
      node.rhs = node.rhs.getApp()
    return node.rhs
    
  def getApp(node):
    if node.app.app is not node.app:
      node.app = node.app.getApp()
    return node.app

  def excise(node):
    "excision from {MIO} trees, i.e., sloppy removal"
    node.MIO.reset()
    node.IMO.reset()
    node.IOM.reset()
    node.OIM.reset()

 #[ conversion to string expr ]==========
  def toStringExpr(node):
    "finds the unique string represntation of a node, if it exists and is known"
    assert node.isOb()
    PairTX_node = node.IOM_find(node.db.PairT)
    if PairTX_node is not None:
      X = PairTX_node.rhs
      result = 'T'+X.toStringExpr()
    else:
      PairFX_node = node.IOM_find(node.db.PairF)
      if PairFX_node is not None:
        X = PairFX_node.rhs
        result = 'F'+X.toStringExpr()
      else:
        result = '?'
    return result




















