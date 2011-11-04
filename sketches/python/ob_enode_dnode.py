"""(2005:01:05)

(N1) There will be three large data structures:
  (information units: 1W = 4B, 1B = 8b)
  (D1) Obs: N x 8W?
    1: rep,
    5: roots to 4 E (actually 3 roots) and 2 D trees, and
    2: total measures (dense)
  (D2) Enodes: N^2 x 16W
    3: root, key, & value
    8: 4x2 children, and
    2: split & join term measures
    1: pruning measures
  (D3) Dnodes: (N^2)/2 x 8W?
    4: 2x2 children, and
    1: similarity measure
  (Q1) how many of each should be allocated?
    (N1) Nodes will be allocated in random creation events.  At each
      creation, at most one Ob and one Enode will be created, however, the
      number of new Dnodes is unbounded.
      This suggests that Dnodes should be plentiful, limited by Enode and Ob
        supply.  Moreover, since the Dnode similarity measures approximate a
        sparse matrix, It may be beneficial to free approximating Dnodes when
        exact Dnodes are required, thus achieving graceful similarity
        degradation.
    (N2) Perhaps a shared memory pool is best, where the Enode and Dnode
      depth and Nobs can be optimally tuned according to shape
      (N1) this poses a slight problem as Enodes are double-sized.
(N2) In addition, there are a variety of smaller data structures:
  (D1) merging queues & buffers
  (D2) naming maps
  (D3) existing Expr maps
"""
class Ob:
  def __init__(self):
    #merging representative
    self.rep = None
    #equivalence roots
    self.alr_root = None
    self.arl_root = None
    self.lra_root = None
    self.rla_root = None
    #distinction roots
    self.fs_root = None
    self.sf_root = None
class Enode:
  def __init__(self):
    #roots, keys, & values
    self.app = None
    self.lhs = None
    self.rhs = None
    #sub-heap children
    self.alr_left = None
    self.alr_right = None
    self.arl_left = None
    self.arl_right = None
    self.lra_left = None
    self.lra_right = None
    self.rla_left = None
    self.rla_right = None
#this comes later
class Dnode:
  def __init__(self):
    #roots & keys
    self.first = None
    self.second = None
    #sub-heap children
    self.fs_left = None
    self.fs_right = None
    self.sf_left = None
    self.sf_right = None
