#ExprDB.py

__all__ = ["ExprDB"]

#[ misc ]
from exceptions import Exception
from copy import copy

#[ exprs ]
import Expr
import exprs

LATER = Exception("unfinished code in ExprDB.py")

#==========[ probability functions ]==========
def entropy(P,thresh):
  "safely calculates a entropy of a distribution"
  def e_term(p,q):
    if p < thresh:  result = 0.0
    else:           result = -p*log(p)
  return sum([e_term(P[n]) for n in range(len(P))])

def relentropy(P,Q,thresh):
  "safely calculates a relative entropy between two distributions"
  def re_term(p,q):
    if p < thresh:  result = 0.0
    else:           result = p*log(p/q)
  return sum([re_term(P[n],Q[n]) for n in range(len(P))])


#==========[ expression database ]==========
class ExprDB:
  """Expression Database, a manager for maintaining a well-shaped
  sparse app table for expr equiv classes.
  The exprDB maintains structure the structure and measure of the app table.
  """
  def __init__(db,numNodes=256):
   #initialize node heap
    from Heap import Heap
    from Node import Node
    db.numNodes = numNodes
    db.nodes = Heap(numNodes) #the node heap
    for id in range(1,numNodes+1):  nodes[id] = Node(id)
    db.rootNode = db.nodes[1]
   #initialize the node allocator
    for id in range(1,numNodes):
      db.nodes[id].alloc_next = db.nodes[id+1]
    db.freeNodes = db.rootNode #doubly-linked list
    db.numFreeNodes = numNodes
    db.numNodesToPrune = db.numNodes/256
    """Note: choose numNodesToPrune so that updateMeasure takes about as long as
      the rest of the prune cycle"""
    db.nodesToPrune = Heap(int,db.numNodesToPrune)
   #initialize processing queues
    from PriorityQueue import PriorityQueue
    db.mergeQueue = PriorityQueue()
    db.enforceQueue = PriorityQueue()
   #initialize null values for node axiom/theorem enforcement
    db.K = None
    db.S = None
    db.Q = None
   #make Identity, Konstant, and Substitution obs (requires 4 nodes)
    from Node import makeApp
    I = db.allocNode();  I.lhs,I.rhs,I.app = I,I,I
    K = db.allocNode();  K.lhs,K.rhs,K.app = I,K,K
    S = db.allocNode();  S.lhs,S.rhs,S.app = I,S,S
    SK = makeApp(db,S,K)
    SKK = makeApp(db,SK,K)
    SKK.mergeWith(I) #define I=S*K*K
    db.S,db.K = S.getRep(),K.getRep() #the db's only known basis
   #define combinatory basis
    db.basis_ob2expr = {S:exprs.S, K:exprs.K}
    db.basis_expr2ob = {exprs.S:S, exprs.K:K}
   #define measure parameters
    from PMF import PMF
    db.epsilon = 0.2
    db.join0PMF = PMF({S:0.5, K:0.5})
    db.joinPMF = PMF()
    db.split0PMF = PMF({S:0.5, K:0.5})
    db.splitPMF = PMF()
   #enforce axioms & theorems
    from axioms import axioms
    for axiom in axioms:
      print 'enforcing theorem ('+thm.name+')...'
      lhs = db.__expr2ob(axiom.lhs,create=True)
      rhs = db.__expr2ob(axiom.rhs,create=True)
      lhs.mergeWith(rhs);  db.__updateStructure()
   #mark existing nodes as permanent
    for node in db.node_iter():
      node.isPermanent = True
   #fill out the db
    db.__expand()
    db.__updateObBasis()
    db.__updateJoinMeasure()
    db.__resetFocus()
    db.__updateSplitMeasure()
    db.__updateNodeMeasures()

  def __len__(db):
    return db.numNodes

  def isFull(db):
    return db.numFreeNodes == 0

 #[ top level interface ]==========
  #interactive states
  def predict(db,input,output):
    "randomly try to predict a bitstream"
    raise LATER
    x = naivePredictor
    db.focusOn(x)
    for mx in input.iterGet():
      x &= m\mx
      y = t*x
      db.thinkAbout(y)
      my = db.chooseFrom(m*y)
      y &= m\my
      output.set(my)
      x = t*y

  def control(db,input,output,control):
    "randomized optimal control of a bitstream"
    db.focusOn(naiveController)
    raise LATER
  
  #db control
  def think(db):
    from math import ceil
    for n in range(ceil(1.0/db.epsilon)):
      db.__thinkCycle()
    
  def thinkAbout(db,expr):
    db.focusOn(expr)
    db.think()

  def meditate(db):
    from math import ceil
    for n in range(ceil(1.0/db.epsilon)):
      db.defocus()
      db.__thinkCycle()

  def sleep(db):
    "occasional batch maintenance"
    db.enforceTheorems()
    db.__expand()
    db.reorder()
    db.__updateNodeMeasures() #'cause sum trees have changed

  def assume(db,expr):
    "propositional assumtion"
    ob = db.makeExpr(expr)
    if ob is not db.K: #db.K == T
      ob.mergeWith(db.K)
      db.__updateStructure()
      db.__updateMeasure()

  #expr commands
  def norm(db,expr):
    "returns an expr's estimated norm in nats"
    from math import log
    ob = db.makeExpr(expr)
    return -log(db.joinPMF[ob])

  def simplify(db,expr):
    "random simplification, i.e., parsing"
    ob = db.makeExpr(expr)
    return ob.parse()

  def reduce(db,expr):
    "random reduction"
    raise LATER

  def chooseFrom(db,pmfExpr):
    "random choice from an R-pmf"
    raise LATER

  def evalProb(db,pmfExpr,itemExpr):
    "evaluation of probability"
    #XXX: is this necessary?
    raise LATER

  def expr2prob(db,probExpr):
    "converts an expr to a real number in [0,1]"
    return db.makeExpr(intExpr).toProb()

  def expr2int(db,intExpr):
    "converts an expr to natural number in {0,1,2,...}"
    return db.makeExpr(intExpr).toInt()

  def expr2string(db,stringExpr):
    "converts an expr to a string in {0,1}^*"
    return db.makeExpr(stringExpr).toString()

 #[ information printing ]==========
  def printNodeStats(db):
    from math import exp
    db.__updateStats()
    print '/----------{ node statistics }----------'
    print '| total number nodes:  '+str(db.numNodes)
    print '| number used:         '+str(db.numNodes - db.numFreeNodes)
    print '| number free:         '+str(db.numFreeNodes)
    print '| effective number:    '+str(exp(db.nodeEntropy))
    print '\\----------'

  def printMeasureParams(db):
    print '/----------{ measure parameters }----------'
    print '| epsilon: '+str(db.epsilon)
    print '| join0PMF: {'
    for root in db.join0PMF:
      print '|   %.5e: %s' % db.join0PMF[root], root
    print '| }'

    print '| split0PMF: {'
    for root in db.split0PMF:
      print '|   %.5e: %s' % db.split0PMF[root], root
    print '| }'
    print '\\----------'

 #[ focus control ]==========
  def __resetFocus(db):
    db.split0PMF = copy(db.joinPMF)
 
  def focusOn(db,ob):
    "add an ob to the geometrically decaying focus queue"
    eps = db.epsilon
    db.split0PMF *= (1-eps)
    db.split0PMF[ob] += eps
    db.__updateSplitMeasure()

  def defocus(db,ob):
    "progressively enter the meditative state of mind"
    eps = db.epsilon
    db.split0PMF *= (1-eps)
    db.split0PMF[ob] += eps * db.joinPMF
    db.__updateSplitMeasure()

 #[ random creation & deletion ]==========
  def __thinkCycle(db):
    """randomly prunes and creates nodes,
      creating no more nodes than are already in the db"""
    db.__contract()
    db.__expand()
    db.__updateMeasure()

  def __expand(db):
    "make random app WRT the join measure"
    while db.numFreeNodes > 0:
      app = True
      lhs = db.__getRandomNode_join()
      while app:
        rhs = lhs
        lhs = db.__getRandomNode_join()
        app = lhs * rhs
      makeApp(db,lhs,rhs) #this should enforce axioms
      db.__updateStructure()

  def __getRandomNode_join(db,t=None):
    "generates a random ob WRT the join measure"
    eps = db.epsilon
    if t is None:
      from random import uniform
      t = uniform(0,1)
    if t < eps:
      t = t/eps #rescale
      ob = db.join0PMF.randomchoice(t)
    else:
      app = None
      lhs = db.__getRandomOb_join()
      while not app:
        rhs = lhs
        lhs = db.__getRandomOb_join()
        app = lhs * rhs
      ob = app
    assert ob.isOb()
    return ob

  def __contract(db,numNodes = None):
    "prune existing applications randomly WRT calculated complexity"
    if numNodes is None:  numNodes = int(db.epsilon * db.numNodes)
    from Set import Set
    nodes = Set()
    while len(nodes) < numNodes:
      nodes += db.__getRandomNode_prune() #must be a set due to disconnection pruning
    for node in nodes:
      node.prune()

  def __getRandomNode_prune():
    "generates a random ob WRT the pruning measure, i.e., the inverse split measure"
    return db.rootNode.prune_randomChoice()

 #[ expr lookup ]==========
  def makeExpr(db,expr):
    """safely makes an ob representing a given expr,
       creating all nodes necessary to make the ob"""
    expr = db.__getExpr(expr)
    if expr is None:
      cost = db.__getExprCost(expr)
      db.__holdExprSupport(expr)
      db.__pruneRandomNodes(cost)
      db.__releaseExprSupport(expr)
      expr = db.__createExpr(db,expr)
      db.__expand() #in case expr requires fewer nodes than expected
      db.__updateMeasure()
    return expr

  def __getExpr(db,expr):
    "looks an expr up in the db"
    from Expr import AtomExpr
    if isinstance(expr,AtomExpr):
      if expr in db.basis_expr2ob:
        result = db.basis_expr2ob[expr]
      else:
        raise Exception('unknown atomic ob: '+str(expr))
    else: #isinstance(expr,AppExpr)
      lhs = db.__getExpr(expr.lhs)
      rhs = db.__getExpr(expr.rhs)
      if (lhs is None) or (rhs is None):
        result = None
      else:
        result = lhs * rhs #may also = None
    return result

  def __createExpr(db,expr):
    "creates an expr in the db"
    from Expr import AtomExpr
    from Node import makeApp
    if isinstance(expr,AtomExpr):
      if expr in db.basis_expr2ob:
        result = db.basis_expr2ob[expr]
      else:
        raise Exception('unknown atomic ob: '+str(expr))
    else: #isinstance(expr,AppExpr)
      lhs = db.__makeExpr(expr.lhs,create)
      if lhs is None:  lhs = db.__createExpr(expr.lhs)
      rhs = db.__expr2ob(expr.rhs,create)
      if rhs is None:  rhs = db.__createExpr(expr.rhs)
      result = lhs * rhs
      if result is None:
        result = makeApp(lhs,rhs)
        db.__updateStructure()
        result = result.getRep()
    return result

  def __getExprCost(db,expr):
    "counts the number of new nodes necessary to represent the given expr"
    expr = db.__getExpr(expr)
    if expr is None:
      result = 1 + db.__getExprCost(expr.lhs) + db.__getExprCost(expr.rhs)
    else:
      result = 0
    return result

 #[ structural maintenance ]==========
  def __updateStructure(db):
    "processes the merge and enforcement queue."
   #complete mergers
    while True:
      if   db.mergeQueue:    db.mergeQueue.dequeue().merge()
      elif db.enforceQueue:  db.enforceQueue.dequeue().enforce()
      else:                           break
   #rename basic ob K
    K_dep,K_rep = dbK,db.K.getRep()
    if K_dep is not K_rep:
      import Expr
     #update basis
      db.K = K_rep
      db.basis_ob2expr[K_rep] = db.basis_ob2expr[K_dep]
      del db.basis_ob2expr[K_dep]
      db.basis_expr2ob[Expr.K] = K_rep
   #rename basic ob S
    S_dep,S_rep = dbS,db.S.getRep()
    if S_dep is not S_rep:
      import Expr
     #update basis
      db.S = S_rep
      db.basis_ob2expr[S_rep] = db.basis_ob2expr[S_dep]
      del db.basis_ob2expr[S_dep]
      db.basis_expr2ob[Expr.S] = S_rep

 #[ measure maintenance ]==========
  def __updateMeasure(db,tol=1e-3,thresh=1e-8):
    """performs a global measure calculation,
      should only be done occasionally due to expense.
    iteratively claculates {join, split, total, prune, parse} measures,
      using previous join & split calculations for initialization.
    Note: the persistent data is
      db.join0PMF
      db.joinPMF
      db.split0PMF
      db.splitPMF
      [node.meas for node in db.node_iter()]
      [node.pruneMeas for node in db.node_iter()]
      [node.parseMeas for node in db.node_iter()]
    """
    db.__updateObBasis()
    db.__updateJoinMeasure()
    db.__updateSplitMeasure()
    db.__updateNodeMeasures()

  def __updateObBasis(db):
    "update basis for ob measures"
    db.num2ob = list(db.ob_iter())
    db.Nobs = len(db.num2ob)
    db.ob2num = dict([(db.num2ob[n],n) for n in range(db.Nobs)])

  def __updateJoinMeasure(db):
    assert db.isFull()
    import Numeric
    eps = db.epsilon
    zero = 0.0*Numeric.zeros(db.Nobs)
   #convert PMF to array
    join0  = copy(zero)
    join   = copy(zero)
    for ob in db.ob_iter():
      n = db.ob2num[ob]
      join0[n]  = db.join0PMF[ob]
      join[n]   = db.joinPMF[ob]
   #iteratively calculate join measure
    while diff >= tol:
      joinSum = copy(zero)
      for node in db.node_iter():
        joinSum[db.ob2num[node.app]] += join[db.ob2num[node.lhs]] * join[db.ob2num[node.rhs]]
      newJoin = eps*join0 + (1-eps)*joinSum
      diff = relentropy(join,newJoin,thresh)
      join = newJoin
    del join0
   #precalculate the join ratio
    for node in db.node_iter():
      node.meas = join[db.ob2num[node.lhs]]*join[db.ob2num[node.rhs]]/joinSum[db.ob2num[node.app]]
    del joinSum
   #convert array back to PMF
    for ob in num2ob:
      n = db.ob2num[ob]
      db.joinPMF[ob] = join[n]
    del join

  def __updateSplitMeasure(db):
    assert db.isFull()
    import Numeric
    eps = db.epsilon
    zero = 0.0*Numeric.zeros(db.Nobs)
   #convert PMF to array
    split0 = copy(zero)
    split  = copy(zero)
    for ob in num2ob:
      n = db.ob2num[ob]
      split0[n] = db.split0PMF[ob]
      split[n]  = db.splitPMF[ob]
   #iteratively calculate split measure
    while diff >= tol:
      splitSum = copy(zero)
      for node in db.node_iter():
        splitTerm = split[db.ob2num[node.app]] * node.meas / 2.0 #node.meas is joinTerm
        splitSum[db.ob2num[node.lhs]] += splitTerm
        splitSum[db.ob2num[node.rhs]] += splitTerm
      newSplit = eps*split0 + (1-eps)*splitSum
      diff = relentropy(split,newSplit,thresh)
      split = newSplit
    del split0
    del splitSum
   #convert arrays back to PMFs
    for ob in num2ob:
      n = db.ob2num[ob]
      db.splitPMF[ob] = split[n]
    del split

  def __updateNodeMeasures(db):
    "update parse and prune measures"
    for node in db.node_revIter():
     #calculate parse measure
      node.parseSum = node.meas #=joinTerm
      left = node.parse_getLeft()
      if left:  node.parseSum += left.parseSum
      right = node.parse_getRight()
      if right:  node.parseSum += right.parseSum
     #calculate combined node measure
      node.meas *= split[db.ob2num[node.app]]
     #calculate prune measurea
      #XXX: this should use splitTerm, not joinTerm.
      if node.isPrunable():  node.pruneSum = 1.0/node.meas
      else:                  node.pruneSum = 0.0
      left = node.prune_getLeft()
      if left:  node.pruneSum += left.pruneSum
      right = node.parse_getRight()
      if right:  node.pruneSum += right.pruneSum

  def __updateStats(db):
    "update node stats: entropy"
    nodeProbs = Numeric.array([node.meas for node in db.node_iter()])
    #nodeProbs /= sum(nodeProbs) #LATER: is this necessary?
    db.nodeEntropy = entropy(nodeProbs,thresh)

 #[ node memory management ]==========
  def allocNode(db):
    from exceptions import MemoryError
   #get node from free node list
    if db.freeNodes is None:
      raise MemoryError, "node allocator is out of nodes"
    node = db.freeNodes
   #update free node list
    db.freeNodes = node.alloc_next
    db.numFreeNodes -= 1
   #update node
    node.rep = node

 #[ iteration ]==========
  def node_iter(db):
    "iterate WRT the heap's inorder"
    for n in range(1,1+db.numNodes):
      node = db.nodes[n]
      if node.isUsed():
        yield node

  def node_revIter(db):
    "iterate WRT the heap's reverse inorder"
    for n in range(db.numNodes,0,-1):
      node = db.nodes[n]
      if node.isUsed():
        yield node

  def ob_iter(db):
    "slow, but does it matter?"
    return iter(db.num2ob)

 #[ reordering ]==========
  def __reorder(db):
    "reorders the db WRT the app measure"
    assert db.isFull()
   #define new orders
    new2old,old2new = db.__getOrders()
    ids = range(1,1+db.numNodes)
   #rebuild {lhs,rhs,app} structure, choosing new obs
    old_obs = list(db.ob_iter())
    for ob in old_obs:
     #find new ob, i.e. compute argmin WRT new IOM_cmp()
      getIndex = labmda node: (old2new[node.lhs.id],old2new[node.rhs.id])
      index,new_ob = (-1,-1),None
      for node in ob.IOM_iter():
        node_index = getIndex(node)
        if node_index < index:
          index,new_ob = node_index,node
     #set new app, lhs, and rhs
      for node in ob.MIO_iter():  node.lhs = new_ob
      for node in ob.IMO_iter():  node.rhs = new_ob
      for node in ob.IOM_iter():  node.app = new_ob
   #move nodes around, in place
    moved = [None]+[False for id in ids]
    for id in db.ids:
      old_id = id
      new_id = old2new[old_id]
      while not moved[new_id]:
       #the keep-booting-the-next-guy algorithm
        node,db.nodes[old_id] = db.nodes[old_id],None
        if node is None:  break
        db.nodes[new_id],node = node,db.nodes[new_id]
        moved[new_id] = True
        old_id = new_id
        new_id = old2new[old_id]
   #rebuild {M,I,O} trees
    for node in db.nodes:  node.excise()
    for node in db.nodes:  node.insert()
   #reset ob basis
    db.S = db.S.app
    db.K = db.K.app

  def __getOrders(db):
    ids = range(1,1+db.numNodes)
    new2old = copy(ids)
    new2old.sort(lambda x,y: -cmp(x.meas,y.meas))
    old2new = copy(ids)
    new2old,old2new = [0]+new2old,[0]+old2new #'cause ids start at 1
    for new_id in ids:
      old_id = new2old[new_id]
      old2new[old_id] = new_id
      db.nodes[old_id].id = new_id
    return new2old,old2new

  def __getReorderBenefit(db):
    oldCost = db.getOrderCost()
    new2old,old2new = db.__getOrders()
    newCost = db.getOrderCost(old2new)
    return oldCost/newCost #>= 0

  def __getOrderCost(db,old2new = None):
    if old2new is None: old2new = range(db.numNodes+1)
    return sum([db.nodes[old_id].meas*log(2*old2new[old_id]+1)
                for old_id in range(1,1+db.nomNodes)])

 #[ conversion ]==========
  def toTable(db):
    """LATER: db needs a small persistent data format for backup.
    need to store:
      for node in db.odes:
        node.lhs, node.rhs, node.app
        node.isPermanent
      db.S, db.K
    """
    ids = range(1,1+db.numNodes)
    lhs,rhs,app = [0]+ids,[0]+ids,[0]+ids
    permanence = [0]+ids
    for id in ids:
      node = db.nodes[id]
      lhs[id] = ndoe.lhs.id
      rhs[id] = node.rhs.id
      app[id] = node.app.id
      permanence[id] = node.isPermanent
    S = db.S.id
    K = db.K.id
    return (S,K),(lhs,rhs,app),permanence #LATER: make a DBImage class or something

 #[ theorem enforcement ]==========
  def enforceTheorems(db):
    "batch theorem enforcement"
    db.__enforceI()
    db.__enforceB()
    db.__enforceC()
    db.__enforceW()
    db.__enforceQ()
    db.__enforceR()
    db.__enforceNumbers()
    db.__enforceStrings()

  def enforceTheorems_alt(db):
    assert db.Thms is not None
    for ThmX_node in db.Thms.MIO_iter():
      X,ThmX = ThmX_node.rhs,ThmX_node.app
      for ThmXY_node in ThmX.MIO_iter():
        Y,ThmXY = ThmXY_node.rhs,ThmXY_node.app
        x,y = X.parseAsString(),Y.parseAsString()
        raise LATER
        #now do something to use X,Y as expressions, loop through their free variables



