"""(2005:01:05)
(N1) since ALR and ARL roots coincide, care must be taken in their
  multimap merging
(N2) at the top level interface (e.g., axiom enforcement),
  only obs can be merged
(N3) A single call to merge_pair can incur subsequent mergers by two methods:
  (M1) (root,key) overlap in merge_maps
  (M2) axiom scheme enforcement later in the process
(N4) no deletions should be necessary; thus free_node may be delayed until
  all mergers have taken place.
  (N1) this should be implementable as a SLL or tree within the node's
    structure
  (N2) nodes cannot be immediately freed, lest rep chains be broken.
(N5) a node may appear in a variety of buffers/queues:
    (B1) merging: when an ob is deprecated
    (B2) enforcement: when a (possibly non-ob) changes one of A,L,R
    (B3) freeing: after a node has been replaced
  In addition, there are two types of mergers:
    (M1) Ob mergesr, enqueuing map & multimap mergers, etc.
    (M2) node merger, freeing one node and shifting enforcement
      responsibility
  (N1) In the second merger, two nodes (not necc. obs) change identity.
    They must change permanence, change places in the enforcement queue, etc.
  (N2) The two merging methods are independent:
    neither need lead to the other.
(N6) multimap fast merge will be made more difficult by the sumultaneous
  change of a key and a value, i.e. in a node [Y=X*X]
"""
class Queue:
  def insert(self, item): ...
  def remove(self, item): ...
  def replace(self, old_item, new_item):
    if old_item in self:
      self.remove(old_item)
      self.insert(new_item)

def merge_pair(node1, node2):
  if node1 < node2:  merge_obs(node2, node1)
  else:              merge_obs(node1, node2)

def merge_obs(dep, rep):
  assert node1.isOb()
  assert node2.isOb()
  dep.setRep(rep)
  measures.merge(dep, rep)

def merge_nodes(dep, rep):
  if dep.isOb():
    merge_obs(dep, rep)
  enforcement_queue.replace(dep, rep)
  free_queue.replace(dep, rep)
  Ob::replace(dep, rep) #for named ob replacement

def merge_ob(dep):
  rep = dep.getRep()
  merge_maps(dep.getRoot(LRA), rep.getRoot(LRA), LRA)
  merge_maps(dep.getRoot(RLA), rep.getRoot(RLA), RLA)
  merge_multimaps(dep.getRoot(ALR), rep.getRoot(ALR), ALR)
  merge_multimaps(dep.getRoot(ARL), rep.getRoot(ARL), ARL)

def merge_multimaps(dep_root, rep_root, idx):
  deps = Set()
  for node in dep_root.iter(idx):
    deps.insert(node, idx)
  for node in deps:
    rep_root.insert_in_map(node, idx)

def merge_maps(dep_root, rep_root, idx):
  deps = Set()
  for node in dep_root.iter(idx):
    deps.insert(node, idx)
  for node in deps:
    rep_root.insert_in_map(node, idx)

def insert(root, node, idx):
  ...

def find(root, key, idx):
  ...
  return value

def insert_in_map(root, node, idx):
  value = root.find(node.key, idx)
  if value is None:
    root.insert(node, idx)
  else:
    merge_pair(value, node.value)
  enqueue_free(node)


