


"""
(N1) need to move entirely away from the ob framework and use only apps.
  (N1) define an app-based probability algorithm
"""
def updateMeasure1():
  while ...:
    for node in db.nodesAnyOrder():
      node.meas.app = node.app.meas.split * node.lhs.meas.join * node.rhs.meas.join
    for node in db.rootsAnyOrder():
      node.meas.ob = 0.0
    for node in db.nodesAnyOrder():
      node.app.meas.ob += node.meas.app
"""
  (N2) the point is to calculate measures for each node.
"""
def updateMeasure2():
  """Note:
  join and split measures are only retained for obs, not general nodes.
  """
 #initialize working data
  for ob in db.ob_iter():
    ob.meas.temp = 0.0
 #update join measure
  while ...:
    for node in db.node_iter():
      node.app.meas.temp += node.lhs.meas.join * node.rhs.meas.join
    for ob in db.ob_iter():
      ob.meas.join = eps*ob.meas.join0 + (1-eps)*ob.meas.temp
      ob.meas.temp = 0.0
 #update split measure
  while ...:
    for node in db.node_iter():
      part = ( (1-eps)*(node.lhs.meas.join * node.rhs.meas.join)
               /(node.app.meas.join - eps*node.app.meas.join0) )
      temp = part * node.app.meas.split / 2.0
      node.lhs.meas.temp += temp
      node.rhs.meas.temp += temp
    for ob in db.ob_iter():
      ob.meas.split = eps*ob.meas.split0 + (1-eps)*ob.meas.temp
      ob.meas.temp = 0.0


def updateMeasure3():
 #define data representation
  num2root = list(root_iter())
  Nroots = len(num2root)
  root2num = dict([(root2num[n],n) for n in range(Nroots)])
 #initialize join measure
  join0 = 0.0*zeros(Nroots)
  for root in root_iter():
    join0[root2num[root]] = joinPMF[root]
  join = 0.0*zeros(Nroots)
  for node in node_iter():
    join[root2num[node.app]] += node.join
  joinSum = 0.0*zeros(Nroots)
 #update join measure
  while ...:
    for node in node_iter():
      joinSum[root2num[node.app]] += join[root2num[node.lhs]] * join[root2num[node.rhs]]
    for root in root_iter():
      join[root2num[root]] = eps*join0[root2num[root]] + (1-eps)*joinSum[root2num[root]]
 #initialize split measure
  split0 = 0.0*zeros(Nroots)
  for root in root_iter():
    split0[root2num[root]] = splitPMF[root]
  split = 0.0*zeros(Nroots)
  for node in node_iter():
    split[root2num[node.app]] += node.split
  splitSum = 0.0*zeros(Nroots)
 #update split measure
  while ...:
    for node in node_iter():
      part = join[root2num[node.lhs]] * join[root2num[node.rhs]] / joinSum[root2num[node.app]]
      temp = part * split[root2num[node.app]] / 2.0
      splitSum[root2num[node.lhs]] += temp
      splitSum[root2num[node.rhs]] += temp
    for root in root_iter():
      split[root2num[root]] = eps*split0[root2num[root]] + (1-eps)*splitSum[root2num[root]]
 #calculate final node measure
  for node in node_iter():
    node.meas = split[root2num[node.app]] * join[root2num[node.lhs]] * join[root2num[node.rhs]]


def updateMeasure4():
  """this version uses only one floating point per node.
  
  Note: in the iterative calculations, node.meas is used as node.joinTerm,
    since the meas is unnecessary.
  """
 #define data representation
  num2root = list(root_iter())
  Nroots = len(num2root)
  root2num = dict([(root2num[n],n) for n in range(Nroots)])
  zero = 0.0*zeros(Nroots)
 #iteratively calculate join measure
  join0  = copy(zero)
  for root in root_iter():
    n = root2num[root]
    join0[n] = joinPMF[root]
  join   = copy(join0)
  while ...:
    joinSum = copy(zero)
    for node in node_iter():
      n_app,n_lhs,n_rhs = root2num[node.app],root2num[node.lhs],root2num[node.rhs]
      joinTerm = join[n_lhs] * join[n_rhs]
      node.meas = joinTerm
      joinSum[n_app] += joinTerm
    for root in root_iter():
      n = root2num[root]
      join[n] = eps*join0[n] + (1-eps)*joinSum[n]
  del join0
 #iteratively calculate split measure
  split0  = copy(zero)
  for root in root_iter():
    n = root2num[root]
    split0[n] = splitPMF[root]
  split = copy(split0)
  while ...:
    splitSum = copy(zero)
    for node in node_iter():
      n_app,n_lhs,n_rhs = root2num[node.app],root2num[node.lhs],root2num[node.rhs]
      splitTerm = split[n_app] * (node.meas / joinSum[n_app])
      splitSum[n_lhs] += splitTerm / 2.0
      splitSum[n_rhs] += splitTerm / 2.0
    for root in root_iter():
      n = root2num[root]
      split[n] = eps*split0[n] + (1-eps)*splitSum[n]
  del joinSum
  del split0
  del splitSum
  rename nods
 #calculate final node measure
  for node in node_iter():
    n_app,n_lhs,n_rhs = root2num[node.app],root2num[node.lhs],root2num[node.rhs]
    node.meas = split[n_app] * join[n_lhs] * join[n_rhs]
  del join
  del split

"""
(N2) MCMC creation might do some literal graph traversal, e.g., {u,l,r} in the MIO tree,
  but some drgree-counting correction needs to be made to node.meas's for jump probabilities.
(N3) we still need a difference-queue local update algorithm.
  before that, we need a progressive update algorithm
"""
def updateMeasure5(db):
  """this version uses previous join & split calculations for initialization,
  thus speeding convergence.
  
  Note: the persistent data is
    db.join0PMF
    db.joinPMF
    db.split0PMF
    db.splitPMF
    [node.meas for node in db.node_iter()]
  """
  node_iter = db.node_iter
  root_iter = db.root_iter
 #define data representation
  num2root = list(root_iter())
  Nroots = len(num2root)
  root2num = dict([(root2num[n],n) for n in range(Nroots)])
  zero = 0.0*zeros(Nroots)
 #convert join & split PMFs to arrays
  join0  = copy(zero)
  join   = copy(zero)
  split0 = copy(zero)
  split  = copy(zero)
  for root in root_iter():
    n = root2num[root]
    join0[n]  = db.join0PMF[root]
    join[n]   = db.joinPMF[root]
    split0[n] = db.split0PMF[root]
    split[n]  = db.splitPMF[root]
 #iteratively calculate join measure
  while ...:
    joinSum = copy(zero)
    for node in node_iter():
      n_app,n_lhs,n_rhs = root2num[node.app],root2num[node.lhs],root2num[node.rhs]
      joinTerm = join[n_lhs] * join[n_rhs]
      node.meas = joinTerm
      joinSum[n_app] += joinTerm
    for root in root_iter():
      n = root2num[root]
      join[n] = eps*join0[n] + (1-eps)*joinSum[n]
  del join0
 #iteratively calculate split measure
  while ...:
    splitSum = copy(zero)
    for node in node_iter():
      n_app,n_lhs,n_rhs = root2num[node.app],root2num[node.lhs],root2num[node.rhs]
      splitTerm = split[n_app] * (node.meas / joinSum[n_app]) #node.meas is joinTerm
      splitSum[n_lhs] += splitTerm / 2.0
      splitSum[n_rhs] += splitTerm / 2.0
    for root in root_iter():
      n = root2num[root]
      split[n] = eps*split0[n] + (1-eps)*splitSum[n]
  del joinSum
  del split0
  del splitSum
  rename nods
 #calculate final node measure
  for node in node_iter():
    n_app,n_lhs,n_rhs = root2num[node.app],root2num[node.lhs],root2num[node.rhs]
    node.meas = split[n_app] * join[n_lhs] * join[n_rhs]
 #convert join & split arrays back to PMFs
  for root in root_iter():
    n = root2num[root]
    db.join[root] = join[n]
    db.split[root] = split[n]
  del join
  del split
"""
  now a crippled version to do only join measure
"""
def updateMeasure5_join(db):
  """this version uses previous join calculations for initialization,
  thus speeding convergence.
  
  Note: the persistent data is
    db.join0PMF
    db.joinPMF
    [node.meas for node in db.node_iter()]
  """
  node_iter = db.node_iter
  root_iter = db.root_iter
 #define data representation
  num2root = list(root_iter())
  Nroots = len(num2root)
  root2num = dict([(root2num[n],n) for n in range(Nroots)])
  zero = 0.0*zeros(Nroots)
 #convert the join PMF to arrays
  join0  = copy(zero)
  join   = copy(zero)
  for root in root_iter():
    n = root2num[root]
    join0[n]  = db.join0PMF[root]
    join[n]   = db.joinPMF[root]
 #iteratively calculate join measure
  while ...:
    joinSum = copy(zero)
    for node in node_iter():
      n_app,n_lhs,n_rhs = root2num[node.app],root2num[node.lhs],root2num[node.rhs]
      joinSum[n_app] += join[n_lhs] * join[n_rhs]
    for root in root_iter():
      n = root2num[root]
      join[n] = eps*join0[n] + (1-eps)*joinSum[n]
  del join0
  del joinSum
 #calculate final node measure
  for node in node_iter():
    n_app,n_lhs,n_rhs = root2num[node.app],root2num[node.lhs],root2num[node.rhs]
    node.meas = eps*join0[n_app] + (1-eps)*join[n_lhs]*join[n_rhs]
    "Note: join0 is nonzero only for root nodes"
 #convert join arrays back to PMFs
  for root in root_iter():
    n = root2num[root]
    db.join[root] = join[n]
  del join
"""
  now for the difference queue version:
"""
class DiffQueue:
  def __init__(dq):
    dq.items = []
    dq.diffs = {}
  def __getitem__(dq,item):
    if item in dq.items:  result = dq.diffs[item]
    else:                 result = 0.0
    return result
  def __setitem__(dq,item,diff):
    if item not in dq.items:
      items.insert(item)
    dq.diffs[item] = value
    dq.items.sort(lambda x,y: -cmp(abs(dq.diffs[x]),abs(dq.diffs[y]))
  def remove(dq,item = None):
    if item is None:
      item = dq[0]
      del dq.items[0]
      del dq.diffs[item]
    else:
      raise LATER #only used in pruning or node deletion
    return item
  def tol(dq):
    if len(dq.items) > 0:  result = abs(dq.diffs[dq.items[0]])
    else:                  result = 0.0
    return result
def makeNodeDiff_join(db,app,lhs,rhs):
  node = makeNode(db,app,lhs,rhs)
  db.diffQueue[app] += (1-eps)*db.join[lhs]*db.join[rhs]
  db.updateMeasure6()
def updateMeasure6_join(db,tol=1e-4):
  while db.diffQueue.tol() > tol:
    root = db.diffQueue.remove() #moves root to back of queue, sets diff to zero
   #recalculate new measure
    joinSum = sum([db.join[node.lhs]*db.join[node.rhs] for node in root.IOM_iter()])
    join = eps*db.join0[root] + (1-eps)*joinSum
    diff = join - db.join[root]
    db.join[root] = join
   #add neighbors to difference queue
    for node in root.MIO_iter():
      db.diffQueue[node.app] += (1-eps)*diff*join[app.rhs]
    for node in root.IOO_iter():
      db.diffQueue[node.app] += (1-eps)*join[app.lhs]*diff
"""
  now adding in the split components...
  (N1) in terms of derivatives, one wants to estimate the effect of, say, node1.d_joinTerm
    on node2.meas . thus from the calculations:
      node.joinTerm = join[node.lhs] * join[node.rhs]
      joinSum[root] = sum([node.joinTerm for node in root.IOM_iter()])
      join[root] = eps*join0[root] + (1-eps)*joinSum[root]
      node.splitTerm = split[node.app]*node.joinTerm/joinSum[node.app]
      splitSum[root] = 0.5*(
        sum([node.splitTerm for node in root.MIO_iter()])
        +sum([node.splitTerm for node in root.IMO_iter()])
      )
      split[root] = eps*split0[root] + (1-eps)*splitSum[root]
      node.meas = split[node.app] * join[node.lhs] * join[node.rhs]
    come the derivatives:
      node.d_joinTerm = d_join[node.lhs]*join[node.rhs] + join[node.lhs]*d_join[node.rhs]
      d_joinSum[root] = sum([node.d_joinTerm for node in root.IOM_iter()])
      d_join[root] = eps*d_join0[root] + (1-eps)*d_joinSum[root]
      node.d_splitTerm = (
        d_split[node.app]*node.joinTerm/joinSum[node.app]
        +split[node.app]*d_node.joinTerm/joinSum[node.app]
        +split[node.app]*node.joinTerm*d_joinSum[node.app]/(joinSum[node.app]**2)
      )
      d_splitSum[root] = 0.5*(
        sum([node.d_splitTerm for node in root.MIO_iter()])
        +sum([node.d_splitTerm for node in root.IMO_iter()])
      )
      d_split[root] = eps*d_split0[root] + (1-eps)*d_splitSum[root]
      node.d_meas = (
        d_split[node.app] * join[node.lhs] * join[node.rhs]
        +split[node.app] * d_join[node.lhs] * join[node.rhs]
        +split[node.app] * join[node.lhs] * d_join[node.rhs]
      )
    now each time one quantity changes, it will affect others as well.  the point is to
    postpone calculating the resulting effects until other things have also happened,
    which also need to calculate the effects.
    Reverse dependency (i.e. effect) graph:
      (source --[effects]--> destination)
      join[root] --> node.joinTerm, node.meas for node in root.IOM_iter()
      node.joinTerm --> joinSum[node.app], node.splitTerm
      joinSum[root] --> join[root], node.splitTerm for node in root.MIO_iter() ???
      node.splitTerm --> splitSum[node.lhs], splitTerm[node.rhs]
      split[root] --> node.splitTerm, node.meas for node in root.MIO_iter() ???
      node.meas --| (nothing)
    Forward dependency graph:
      (source --[depends on]--> destination)
"""
def updateMeasure7(db,tol=1e-4):
  while true: #better dependent queue flow control
    if db.d_join.tol() > tol:
      db.processJoinDiff(); continue
    if db.d_split.tol() > tol:
      db.processSplitDiff(); continue
    break
def processJoinDiff(db):
  root = db.d_join.remove() #moves root to back of queue, sets diff to zero
 #recalculate new measure
  joinSum = sum([db.join[node.lhs]*db.join[node.rhs] for node in root.IOM_iter()])
  join = eps*db.join0[root] + (1-eps)*joinSum
  diff = join - db.join[root]
  db.join[root] = join
 #add neighbors to difference queue
  for node in root.MIO_iter():
    db.d_join[node.app] += (1-eps)*diff*join[app.rhs]
  for node in root.IOO_iter():
    db.d_join[node.app] += (1-eps)*join[app.lhs]*diff
def processSplitDiff(db):
  root = db.d_split.remove() #moves root to back of queue, sets diff to zero
 #recalculate new measure
  joinSum = sum([db.join[node.lhs]*db.join[node.rhs] for node in root.IOM_iter()])
  join = eps*db.join0[root] + (1-eps)*joinSum
  diff = join - db.join[root]
  db.join[root] = join
 #add neighbors to difference queue
  for node in root.MIO_iter():
    db.d_split[node.app] += (1-eps)*diff*join[app.rhs]
  for node in root.IOO_iter():
    db.d_split[node.app] += (1-eps)*join[app.lhs]*diff
"""
(N4) the new node paradigm does not require that x=[I*x=x], and allows different
  root nodes for each {M,I,O} tree.  thus, the probability calculation needs to
  be reformulated.
"""
def relentropy(P,Q,thresh=1e-8):
  def re_term(p,q,thresh):
    if p < thresh:  result = 0.0
    else:           result = p*log(p/q)
  return sum([re_term(P[n],Q[n]) for n in range(len(P))])
def updateMeasure8_join(db,tol=1e-4):
  nodeIter = db.nodeIter
  MIO_rootIter = db.MIO_rootIter
  IMO_rootIter = db.IMO_rootIter
  IOM_rootIter = db.IOM_rootIter
 #define data representation
  MIO_num2root = list(MIO_rootIter())
  IMO_num2root = [node.IMO.root for node in MIO_num2root]
  IOM_num2root = [node.IOM.root for node in MIO_num2root]
  N = len(MIO_num2root)
  MIO_root2num = dict([(MIO_num2root[n],n) for n in range(N)])
  IMO_root2num = dict([(IMO_num2root[n],n) for n in range(N)])
  IOM_root2num = dict([(IOM_num2root[n],n) for n in range(N)])
 #convert the join PMF to arrays
  zero = 0.0*zeros(Nroots)
  join0  = copy(zero)
  join   = copy(zero)
  for n in range(N):
    MIO_root = MIO_num2root[n]
    join0[n]  = db.MIO_join0PMF[MIO_root]
    join[n]   = db.MIO_joinPMF[MIO_root]
 #iteratively calculate join measure
  diff = 2*tol
  while diff >= tol:
    joinSum = copy(zero)
    for node in nodeIter():
      n_lhs = MIO_root2num[node.MIO.root]
      n_rhs = IMO_root2num[node.IMO.root]
      n_app = IOM_root2num[node.IOM.root]
      joinSum[n_app] += join[n_lhs] * join[n_rhs]
    newJoin = eps*join0 + (1-eps)*joinSum #a vector calculation
    diff = relentropy(join,newJoin)
    join = newJoin
    del newJoin
  del join0
  del joinSum
 #calculate final node measure
  for node in nodeIter():
    n_lhs = MIO_root2num[node.MIO.root]
    n_rhs = IMO_root2num[node.IMO.root]
    node.meas = (1-eps)*join[n_lhs]*join[n_rhs]
    if node.IOM.root is node:
      n_app = IOM_root2num[node.IOM.root]
      node.meas += eps*join0[n_app]
 #convert join arrays back to PMFs
  for n in range(n):
    db.MIO_joinPMF[MIO_num2root[n]] = join[n]
  del join
"""  
(N5) the persistent data is:
  db.MIO_join0PMF
  db.MIO_joinPMF
  [node.meas for node in db.nodeIter()]
(Q1) does this work?
  (Q1) why is N the same for all {M,I,O}?
    (Q1) does node.MIO.root.IMO.root = node.IMO.root ?
      (N1) node.MIO.root represents the node's lhs.  similarly,
        node.IMO.root and node.IOM.root represent the root's rhs and app.
      (A1) no.
    (Q2) what is the root, in terms of {left,right,up}?
      (A1) node.xxx.root satisfies calcRoot() below:
"""
class SearchTreeNode:
  def __init__(node):
    node.up    = None
    node.left  = None
    node.right = None
    node.root  = None
  def calcRoot(node):
    pos = node
    while pos.up is not None:
      pos = pos.up
    node.root = pos
    return pos
"""
    (N1) each node is an MIO node (as well as an xxx node, etc.).
      each node has an MIO root, representing its lhs.  to use an lhs
      node as an rhs, one must be able to convert:
"""
def lhs2rhs(lhs):
  lhs = lhs.MIO.root #just for safety
  rhs = ???
"""
      this cannot be calculated, and is a reason to want to store
      conversions between all roots in {M,I,O}, or to want all roots to
      be the same node.
    (Q3) what is required for all roots to be the same node?
      * when an app is created, it is its own IOM node, and has no MIO or IMO roots.
      * in fact some of the {IMO} trees may be empty, and so app --> lhs conversion
        may not be able to yield a result
(N6) some simple example systems
  (E1) minimal
    I   = [I*I=I],  IOM.root = I,  MIO.root = I,   IMO.root = I, OIM.root = I
    S   = [I*S=S],  IOM.root = S,  MIO.root = I,   IMO.root = S, OIM.root = S
    K   = [I*K=K],  IOM.root = K,  MIO.root = I,   IMO.root = K, OIM.root = K
    SK  = [S*K=SK], IOM.root = SK, MIO.root = SK,  IMO.root = K, OIM.root = SK
    SKK = [SK*K=I], IOM.root = I,  MIO.root = SKK, IMO.root = K, OIM.root = I
  (E2) some additional structure
     = [*=], IOM.root = , MIO.root = , IMO.root = , OIM.root = 
table-------------------------------------------------------------------------------------
id--    relatives----------     roots----------------------------------------   properties
name    lhs     rhs     app === IOM     OIM     MIO     (MOI)   IMO     (OMI)   isOb
I       I       I       I       I       I       I       I       I       I       T
K=IK    I       K       K       K       K       I       I       K       KI_K*   T
S=IS    I       S       S       S       S       I               S       KI_S*   T
KI      K       I       I_KI*   I_KI*   KI      KI              K               T
KII     KI      I       I       I       I       KII             I               F
KIK     KI      K       I       I       I       KII             K               F
KIS     KI      S       I       I       I       KII             S               F
KK      K       K       I_KK*   I_KK*   KK      KI              K               T
KKI     KK      I       K       K       KKI     KKI             I               F
KKK     KK      K       K       K       KKI     KKI             I               F
KKS     KK      S       K       K       KKI     KKI             I               F
KS      K       S       I_KS*   I_KS*   KS      KI              S               T
KKI     KK      I       S       S       KSI     KSI             I               F
KKK     KK      K       S       S       KSI     KSI             I               F
KKS     KK      S       S       S       KSI     KSI             I               F
SI      S       I       I_SI*   I_SI*   SI      SI              I               T
SII     SI      I       SII     SII     SII     SII             I               T
SIK     SI      K       SIK     SIK     SIK     SII             K               T
SIS     SI      S       SIS     SIS     SIS     SII             S               T
SK      S       K       I_KI*   I_KI*   KI      SI              K               F
SS      S       S       I_SS*   I_SS*   SS      SI              S               F
SSI     SS      I       SSI     SSI     SSI     SSI             I               T
SSK     SS      K       SSK     SSK     SSK     SSI             K               T
SSS     SS      S       SSS     SSS     SSS     SSI             S               T
I_KI    I       KI      I_KI    I_KI    KI      I               I_KI            F
I_KK    I       KK      I_KK    I_KK    KK      I               I_KK            F
I_KS    I       KS      I_KS    I_KS    KS      I               I_KS            F
I_SI    I       SI      I_SI    I_SI    SI      I               I_SI            F
I_SS    I       SI      I_SI    I_SI    SI      I               I_SI            F
K_...
S_...
KI_I    KI      I       I       
...

  (Q1) now what do lhs, rhs, or app point to?
    (A1) in this problem, the original application which made a node.
      each node (even if empirical or unknown) can always bootstrap its existence
      through the application x=[I*x=x].
    (N1) more importantly, if every ob's IOM tree is nonempty (containing at least I*x=x)
      then obs can be represented by their IOM roots.
(Q2) exactly what structure is needed to
  (P1) calculate the join measure
"""
def isOb(node):
  return node is node.IOM.root
for node in db.obIter():
  assert isOb(node)
  node.joinSum = 0.0
for node in db.nodeIter():
  app = node.IOM.root; assert isOb(app)
  lhs = node.lhs; assert isOb(lhs)
  rhs = node.rhs; assert isOb(rhs)
 #find
  assert node is lhs.MIO_find(rhs)
  assert node is lhs.MOI_find(app,rhs)
  assert node is rhs.IMO_find(lhs)
  assert node is rhs.OMI_find(app,lhs)
  assert node is app.IOM_find(lhs,rhs)
  assert node is app.OIM_find(rhs,lhs)
 #measure
  node.joinTerm = lhs.join * rhs.join
  app.joinSum += node.joinTerm
"""
    thus, only app=IOM.root, lhs, and rhs are needed
  (P2) calculate ob2expr
"""
def expr2ob(db,ob):
  assert isOb(ob)
  from random import uniform
  t = uniform(0,ob.join)
  eps = db.epsilon
  if t < eps*ob.join0: #atomic expr
    expr = db.ob2expr_basis[ob]
  else: #compound expr
    t = (t-eps*ob.join0)/(1-eps)
    node = ob.IOM_choose(t) #need to define this
    lhs = expr2ob(db,node.lhs)
    rhs = expr2ob(db,node.rhs)
    expr = lhs*rhs
  return expr
"""
    this time, only lhs and rhs are needed.
  (P3) calculate the split measure
"""
for node in db.obIter():
  assert isOb(node)
  node.splitSum = 0.0
for node in db.nodeIter():
  app = node.IOM.root; assert isOb(app)
  lhs = node.lhs; assert isOb(lhs)
  rhs = node.rhs; assert isOb(rhs)
 #find
  assert node is lhs.MIO_find(rhs)
  assert node is lhs.MOI_find(app,rhs)
  assert node is rhs.IMO_find(lhs)
  assert node is rhs.OMI_find(app,lhs)
  assert node is app.IOM_find(lhs,rhs)
  assert node is app.OIM_find(rhs,lhs)
 #measure
  node.splitTerm = app.split*(node.joinTerm/app.joinSum)
  lhs.splitSum += node.splitTerm / 2.0
  rhs.splitSum += node.splitTerm / 2.0
"""
    again, need only app=IOM.root, lhs, and rhs.
  (Q1) are roots (as opposed to app, lhs, and rhs) ever needed?
(Q3) how to make random nodes?
  (P1) WRT the join measure
"""
def getRandomOb_join(db):
  eps = db.epsilon
  from random import uniform
  t = uniform(0,1)
  if t < eps:
    t = t/eps #rescale
    ob = db.join0PMF.choose(t)
  else:
    node = None
    while not ob:
      lhs = db.getRandomOb_join()
      rhs = db.getRandomOb_join()
      node lhs.MIO_find(rhs)
    ob = node.app
  return ob
def makeRandomOb_join(db):
  exists = True
  while exists:
    lhs = db.getRandomOb_join()
    rhs = db.getRandomOb_join()
    exists =  lhs.MIO_contains(rhs)
  return makeApp(db,lhs,rhs)
def pruneRandomOb_join(db):
  db.getRandomOb_joinInv().prune()
"""
  (P2) WRT the split measure
"""
def getRandomOb_split(db):
  eps = db.epsilon
  from random import uniform
  t = uniform(0,1)
  if t < eps:
    t = t/eps #rescale
    ob = db.split0PMF.choose(t)
  else:
    t = (1-t)/(1-eps) #rescale
    app = db.getRandomOb_split()
    node = app.IOM_choose(t) #WRT join measure
    t = uniform(0,1)
    if t < 0.5:  ob = node.lhs
    else:        ob = node.rhs
  return ob
"""
  (N1) if nodes are created WRT a given measure and pruned WRT the same
    measure (inverted), then s.s. ob existence will correspond to the
    measure squared.  thus,
    (Q1) should obs be uniformly created, and specifically deleted?
      (A1) no; that would be slow to respond to new mass, and deletion WRT
        specific (inverted) measures is difficult
    (Q2) should obs be specifically created and uniformly deleted?
      (A1) no; that would accidentally delete many useful obs.
    (N1) maybe existence corresponding to the measure squared is ok.
    (Q3) should obs be created WRT the join measure and pruned WRT the
      split measure (inverted)?
      (N1) that's surely reminiscent of the search procedure flavor of
        innovative human thought.  it's also fairly straight-forward to
        implement, in that obs cannot be created WRT the split measure.
    (Q4) what if the initial split and join measures are the same?  does
      that simplify things signifigantly?
      (N1) that starts to look like random traversal on {app,lhs,rhs} edges.
        it also has a nice round aesthetic appeal for shape criteria
    (Q5) what if i'm interested in how a particular ob behaves (estimation)?
      or in optimizing something (control)?  
      (N1) eventually i want to write an expr and have the db simplify it.
        that's entirely covered under the join-split framework.
  (P1) create WRT join, prune WRT split.
    (N1) creating random obs WRT join is easy.  this can even be done
      temperature-independently.
    (Q1) how to delete obs WRT split?
      (A1) do a tree-choose from the inverse PMF (floating point)
        (N1) hard to maintain the floating point structure.  this really
          requires a local probability update calculation.
    (Q2) how to create nodes other than obs?
      (A1) no need to; use axioms to decide whether a new app makes a new ob
        or is just equivalent to an existing ob.
    (Q3) how to prune nodes other than apps?
      (A1) prune not the ob but a node in ob.IOM_choose(t),
        rerooting if the root node is deleted.
    (P1) maybe do this in batches of like 100, and only re-update the split
      prob before pruning.  maybe even create in batchs of like Nnodes/10.
      expand & collapse.  very organic.  only update probs when sleeping.
"""











  
