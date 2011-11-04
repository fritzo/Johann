import numpy

class Model:
  def get_param (self): raise NotImplementedError
  def set_param (self): raise NotImplementedError

  def H (self, data=None): raise NotImplementedError
  def dH (self, data=None): raise NotImplementedError
  def ddH (self, data=None): raise NotImplementedError

  def fit (self, data, sptesize=1e-3):
    t = self.get_param()
    dt = -self.ddH(data).solve(self.dH(data))

class BN (Model):
  def __init__ (self, verts, parents, phi):
    self.verts = verts
    self.parents = parents
    self.phi = phi.copy()

  def get_param (self):

