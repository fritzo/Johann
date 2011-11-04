
class Point:
  def __init__(self):
   #measures
    self.mu0  = 0.0
    self.mu   = 0.0
    self.nu   = 0.0
    self.rho0 = 0.0;  self.rho0_prev = 0.0
    self.rho  = 0.0;  self.rho_prev  = 0.0
   #moments
    self.M1 = 0.5
    self.M2 = 0.25
class Pair:
  def __init__(self, x1, x2):
    self.x1 = x1
    self.x2 = x2
   #moments
    self.M2 = 0.0
    self.corr = 0.0

class Struct:
  def __init__(self, X, XX, XXX):
   #structure
    self.X   = X   #points
    self.XX  = XX  #pairs
    self.XXX = XXX #products
   #constant parameters
    self.N1 = len(X)
    self.N2 = len(XX)
    self.N3 = len(XXX)
    self.tau  = 1e4 #time scale
    self.H0   = 1e3 #basis entropy
   #derived variables
    self.beta = 0.5 #inverse temperature
 #interface
  def advance(self, T):
    for x in self.X:
      x.rho0_prev = x.rho0
      x.rho0 = self.epsilon * x.mu
    for x in self.X:
      if (x,T) in self.XXX:
        self.XXX[x,T].rho0 += x.rho0_prev
  def constrain(self, M, y):
    for x in self.X
      if (x,M) in self.XXX:
        y_pred = self.XXX[x,M]
        if y_pred == y :  x.rho0 *= P_correct
        else:             x.rho0 *= P_incorrect
      else:
        x.rho0 *= P_unknown
  def predict(self, M):
    result = FreeSum()
    for x in self.X:
      if (x,M) in self.XXX:
        result[XXX[x,M]] += x.mu
    return result
 #measure calculations
  def update_measures(self):
    self.calc_relevance()
    self.calc_moments()
    self.calc_complexity()
  def calc_relevance(self):
    while not converged:
      for x in self.X:
        x.rho_prev = x.rho
        x.rho = x.rho0
      for (x,y),xy in self.XXX.iteritems():
        rho = 0.5 * xy.rho_prev * x.nu * y.nu / xy.nu
        x.rho += rho
        y.rho += rho
  def calc_moments(self):
    alpha = exp(-1.0/self.tau)  #old part
    omega = 1.0 - alpha         #new part
    for x in self.X:
      x.M1 = alpha * x.M1 + omega * x.rho
      x.M2 = alpha * x.M2 + omega * x.rho * x.rho
    for p in self.XX:
      x,y = p.x1, p.x2
      p.M2 = alpha * p.M2 + omega * x.rho * y.rho
      Vxy = p.M2 - x.M1 * y.M1
      Vxx = x.M2 - x.M1**2
      Vyy = y.M2 - y.M1**2
      p.corr = Vxy / math.sqrt(Vxx * Vyy) #may be negative
  def update_beta(self):
    "adjusts beta to yield a specific constant basis entropy"
    beta = self.beta
    while not converged:
      Z = 0.0
      K = 0.0
      for x in self.X:
        z = x.M1**beta
        dz_dbeta = beta * x.M1**(beta-1) #XXX: only for beta != 1
        k = -z * math.log(z)
        dk_dbeta = -dz * math.log(z) - dz
        Z += z; dZ += dz
        K += k; dK += dk
      H  = K / Z - math.log(Z)
      dH = dK / Z - K * dZ / Z**2 - dZ / Z
      beta += (self.H0 - H) / dH #XXX: need a better functional approximation
  def update_basis(self):
    self.update_beta()
    Z = 0.0
    for x in self.X:
      z = x.M1**self.beta
      Z += z
      x.mu0 = z
    for x in self.X:
      x.mu0 /= Z
  def smooth(self):
    for x in self.X:
      x.nu = x.mu
    for p in self.XX:
      #XXX: negative correlation can lead to negative nu
      p.x1.nu += p.corr * p.x2.mu
      p.x2.nu += p.corr * p.x1.mu
  def calc_complexity(self):
    self.update_basis()
    self.smooth()
    while not converged:
      for x in self.X:
        x.mu = 0.5 * x.mu0
      for (x,y),xy in self.XXX.iteritems():
        xy.mu += x.nu * y.nu
      self.smooth()
 #structural modifications
  def update_structure(self):
    self.redistribute_points()
    self.redistribute_pairs()
    self.redistribute_products()
    self.reparse_products()
  def redistribute_points(self):
    LATER()
  def redistribute_pairs(self):
    LATER()
  def redistribute_products(self):
    LATER()
  def reparse_products(self):
    LATER()

