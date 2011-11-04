
Y = (x*x/x)*(f*(x*x)/x)/f
# define the random choice
R = ??? # in <S,K>
R*a*a = a
R*a*b = R*b*a
R*(R*a*b)*(R*c*d) = R*(R*a*d)*(R*c*b)
R*a*b*c = R*(a*c)*(b*c)
a*(R*b*c) = R*(a*b)*(a*c) #this one is non-standard, but ok for now
# a naive distribution in <S,K>
N = Y*(R*(n*n)*(R*S*K)/n)*(R*S*K)
  = (x*x/x)*((R*(n*n)*(R*S*K)/n)*(x*x)/x)*(R*S*K)
  = ((R*(n*n)*(R*S*K)/n)*(x*x)/x)*((R*(n*n)*(R*S*K)/n)*(x*x)/x)*(R*S*K)
  = (R*(n*n)*(R*S*K)/n)*(((R*(n*n)*(R*S*K)/n)*(x*x)/x)*((R*(n*n)*(R*S*K)/n)*(x*x)/x))*(R*S*K)
  = (R*(n*n)*(R*S*K)*(R*S*K)/n)*(((R*(n*n)*(R*S*K)/n)*(x*x)/x)*((R*(n*n)*(R*S*K)/n)*(x*x)/x))
  = (R*(n*n*(R*S*K))*(R*S*K*(R*S*K))/n)*(((R*(n*n)*(R*S*K)/n)*(x*x)/x)*((R*(n*n)*(R*S*K)/n)*(x*x)/x))
  = (R*(n*n*(R*S*K))*(R*(S*(R*S*K))*(K*(R*S*K)))/n)*(((R*(n*n)*(R*S*K)/n)*(x*x)/x)*((R*(n*n)*(R*S*K)/n)*(x*x)/x))
  = (R*(n*n*(R*S*K))*(R*(R*(S*S)*(S*K))*(R*(K*S)*(K*K)))/n)*(((R*(n*n)*(R*S*K)/n)*(x*x)/x)*((R*(n*n)*(R*S*K)/n)*(x*x)/x))
  = (R*(R*(R*(S*S)*(S*K))*(R*(K*S)*(K*K)))*(n*n*(R*S*K))/n)*(((R*(n*n)*(R*S*K)/n)*(x*x)/x)*((R*(n*n)*(R*S*K)/n)*(x*x)/x))
  = R*(R*(R*(S*S)*(S*K))*(R*(K*S)*(K*K)))*((n*n*(R*S*K))/n*(((R*(n*n)*(R*S*K)/n)*(x*x)/x)*((R*(n*n)*(R*S*K)/n)*(x*x)/x)))
  = R*(R*(R*(S*S)*(S*K))*(R*(K*S)*(K*K)))*((n*n)/n*(((R*(n*n)/n)*(x*x)/x)*((R*(n*n)*(R*S*K)/n)*(x*x)/x))*(R*S*K))
  = ???
# a naive distribution in <K> ?
N = R*x*x/x*(R*(R*x*x)*K/x)*K
  = R*x*x*K/x*(R*(R*x*x)*K/x)
  = R*(x*K)*(x*K)/x*(R*(R*x*x)*K/x)
  = R*x*x/x*(R*(R*x*x)*K/x*K)
  = R*x*x/x*(R*(R*K*K)*K)
  = R*x*x/x*(R*K*K)
  = R*x*x/x*K
  = R*K*K
  = K
# oops!  try again:
N = x*x/x*(R*(R*x*x)*K/x)*K
  = x*x*K/x*(R*(R*x*x)*K/x)
  = R*(R*x*x)*K/x*(R*(R*x*x)*K/x)*K
  = R*(R*x*x)*K*K/x*(R*(R*x*x)*K/x)
  = R*(R*x*x*K)*(K*K)/x*(R*(R*x*x)*K/x)
  = R*(R*x*x*K/x*(R*(R*x*x)*K/x))*(K*K)
  = R*(K*K)*(R*x*x*K/x*(R*(R*x*x)*K/x))
  = R*(K*K)*(R*(x*K)*(x*K)/x*(R*(R*x*x)*K/x))
  = R*(K*K)*(R*x*x/x*(R*(R*x*x)*K/x*K))
  = R*(K*K)*(R*x*x/x*(R*(R*K*K)*K))
  = R*(K*K)*(R*x*x/x*(R*K*K))
  = R*(K*K)*(R*x*x/x*K)
  = R*(K*K)*(R*K*K)
  = R*(K*K)*K
# damn, wrong again...
N = x*x/x*(R*(x*x)*K/x)*K
  = x*x*K/x*(R*(x*x)*K/x)
  = R*(x*x)*K/x*(R*(x*x)*K/x)*K #(1)
  = R*(x*x)*K*K/x*(R*(x*x)*K/x)
  = R*((x*x)*K)*(K*K)/x*(R*(x*x)*K/x)
  = R*((R*(x*x)*K/x*(R*(x*x)*K/x))*K)*(K*K) #(2)
  = R*((R*(x*x)*K*K/x)*(R*(x*x)*K/x))*(K*K)
  = R*((R*((x*x)*K)*(K*K)/x)*(R*(x*x)*K/x))*(K*K)
# now from (1) and (2), N = R*N*(K*K) -> N = K*K; wrong again
# this suggests another R-axiom
R*x*y=R*x*z -> y=z
#or in single axiom form:
Q*(R*x*y)*(R*x*z)/z/y/x = K*Q
# another try:
N = x*x/x*(R*K*(n*n)/n*(x*x)/x)
  = R*K*(n*n)/n*(x*x)/x*(R*K*(n*n)/n*(x*x)/x)
  = R*K*((x*x)*(x*x))/x*(R*K*(n*n)/n*(x*x)/x)
  = R*K*((R*K*(n*n)/n*(x*x)/x*(R*K*(n*n)/n*(x*x)/x))*(R*K*(n*n)/n*(x*x)/x*(R*K*(n*n)/n*(x*x)/x)))
  = R*K*((R*K*(n*n)/n*(x*x)/x*(R*K*(n*n)/n*(x*x)/x))*(R*K*(n*n)/n*(x*x)/x*(R*K*(n*n)/n*(x*x)/x)))
# better notation:
Y = x*x/x*(f*(x*x)/x)/f
  = f*(x*x)/x*(f*(x*x)/x)/f
  = f*(x*x/x*(f*(x*x)/x))/f
# so that 
Y*f = f*(Y*f) #for each f
#now define the generator G
G = R*K*(g*g)/g
N = Y*G
  = G*(Y*G)
  = G*N
  = R*K*(N*N)
  = R*K*(R*K*(N*N)*N)
  = R*K*(R*(K*N)*(N*N*N))
  = R*K*(R*(K*(R*K*(N*N)))*(N*N*N))
  = R*K
     *(R*(R*(K*K)
           *(K*(N*N)))
        *(N*N*N))
  = R*K
     *(R*(R*(K*K)
           *(K*(R*K*(N*N)*(R*K*(N*N)))))
        *(N*N*N))
  = R*K
     *(R*(R*(K*K)
           *(K*(R*(K*(R*K*(N*N)))
                 *((N*N)*(R*K*(N*N))))))
        *(N*N*N))
  = R*K
     *(R*(R*(K*K)
           *(K*(R*((R*(K*K)
                     *(K*(N*N)))
                 *(R*((N*N)*K)
                    *((N*N)*(N*N))))))
        *(N*N*N))
"""
Thus R-free leaves can be enumerated.
The point should be to enumerate the most leaves (WRT the R-measure)
  with the smallest expression set.
In a graph reduction setting, cycles can give very much higher probability to
  often reused leaves than the smallest derivation would suggest.
"""
#define the entropy
def H(nodes):
  h = lambda x: -log(x)*x
  return sum([h(node.prob) for node in nodes])
#define the storage cost
n0 = sizeof(Ob); a0 = sizeof(App)
def N(nodes):
  return len(nodes)
def A(nodes):
  return sum([len(node.apps) for node in nodes])
def cost(nodes):
  return n0*N(nodes) + a0*A(nodes)
#calculate the probability, using floating point math
probs = {N:1.0}
for i in range(numIterations):
  new_probs = zeros(len(nodes))
  for (R*a*b) in ...
"""
Since each ob may be simulteneously equal to R*a*b and R*c*d for a!=c and b!=d,
there is freedom in where mass is moved in R-parsing. 
The only requirement in this flexible setting is that mass is moved equally in R-splits.
Maybe mass can move randomly in the network, subject to
 * an accumulation energy and
 * a descent energy
so that mass wants to accumulate in one place that's far down the R-tree.
"""
#calculate the probability, using floating point math
probs = {N:1.0}
for i in range(numIterations):
  ob in obs:
    childTot = sum([??? for l,r in obs x obs if ob is R*l*r])
    ???
"""
The steady-state conditions should be something like
(1) "twice as much prob should be in the children as in the parent"
    i.e., a parent should have the same mass as each of its children
(2) "between different children, entropy is minimized"?
There is a problem with (1) in that there are different ways to write R(Rab)(Rcd).
(1')"each child should have the same mass as _its_share_ of its parent.
Assume first that the entire R-tree is leaved, i.e., there are no massive cycles.
Then all mass should go to the leaves, and there is no freedom of weighting.
In a finite approximation, there will be mass left unterminated.
This mass can go in a number of places, and thus introduces freedom.

Alternatively, it is is impossible to determine whether a node is a leaf, or
just the root of another tree.
From this perspective, those paths should be followed which lead to an eventually
higher node-entropy, i.e., with more enumerated leaves.

The major questions at this point (2004:02:12) are:
(Q1) how to define the ob R in <S,K> or <S,K,Q>?
  (N1) R-axioms:
    R*a*a = a
    R*a*b = R*b*a
    R*(R*a*b)*(R*c*d) = R*(R*a*d)*(R*c*b)
    R*a*b*c = R*(a*c)*(b*c)
    R*a*b = R*a*c  <-->  b = c
  (N2) axiom-wise construction
    (N1) R*a*a = a
      G1 = Q*a*b*a*(g*a*b)/b/a/g
      R1 = Y*G
    (N2) R*a*b = R*b*a
      G2 = g*b*a/b/a/g
      R2 = Y*G/b/a
    (N3) R*(R*a*b)*(R*c*d) = R*(R*a*d)*(R*c*b)
      ???
    (N4) R*a*b*c = R*(a*c)*(b*c)
      G4 = g*(a*c)*(b*c)/c/b/a/g
      R4 = Y*G
    (N5) R*a*b = R*a*c  <-->  b = c
      ...this should follow from R*a*a = a,
      R*a*a = a
        --> ???
        --> Q*(R*a*b)*(R*a*c)/c/b/a = K*Q
  (N3) combining axioms
    (N1) 1 and  2
      G12 = G1*G2
        = Q*a*b*a*(g*b*a)/b/a/g
      R12 = Y*G
    (N2) 1 and 2 and 4
      G124 =?= G1*G2*G4
      R124 =?= Y*G12
  (A1) the difficulty of implementing the deep-symmetry axiom in lambda (and hence
    <S,K> suggests R is not in <S,k>.
  (A2) perhaps Wos's kernel strategy could help find R in <S,K> or in <S,K,Q>.
  (N2) what about expandable R's:
"""
R*a*b*c*d = R*b*a*c*d
          = R*a*c*b*d
          = R*a*b*d*c #generates symmetric group S_4
R*a*a*a*a = a
R*a*b*c*d = R*(R*a*a*b*b)*(R*a*a*b*b)*(R*c*c*d*d)*(R*c*c*d*d)
...
# or maybe ternary, like vNBG set theory:
R*a*b*c = R*b*a*c = R*a*c*b
R*a*a*a = a
R*a*b*c = R*(R*a*a*b)*(R*b*b*c)*(R*c*c*a)
# now need to be able to show that
R*(R*a*x*x)*(R*b*x*x)*x =?= R*(R*a*b*x)*x*x
"""
    No, this doesn't solve the problem.

(Q2) how to finitely approximate recursive R-PMFs?
  (N1) the current exprDB randomly adds finitely many obs and ob apps.  thus,
    it could efficiently make many different expr representations of a given ob
    (e.g., an R-PMF).
  (N2) on the set of expr representations of a given R-PMF ob, define the functions
    (N1) graph complexity. i.e., how much space is needed to store the expr graph
    (N2) immediate entropy, defined as the R-PMF entropy of the particular ob,
      where branches not explicitly begining with R are not expanded.  For example
        H0(R*S*K) = 1 bit
        H0(R*(R*K*(K*K))*(R*(K*(K*K))*(K*(K*(K*K))))) = 2 bit
        H0(K*(R*K*(K*K))*K) = 0 bit
      thus, H0 cannot be an ob, for it depends on the expr representation.
  (A1) one never really needs finite approximations; only approximate answers.  Thus,
    one needs to:
    (E1) choose random obs
    (E2) approximate the entropy
    (E3) approximate individual ob probability

(Q3) how to interface the resulting system?
  (N1) all interface will be done through exprs, (not obs), which are the
    lingustic projection of the equivalence structure.
  (N2) output might be in the form of a response partial PMF, from which a random
    chooser proxy can select a single output.  To prevent choosing an entire sub-tree,
    mistaken by the finite db as a leaf,  one can limit choices to be known to be of
    certain types, e.g., propositions = {T,F}.
  (N3) even the current simplify() function might better be implemented as finding
    a random expr to match a given ob.  This might be done, e.g., by using string
    representations to encode exprs (as opposed to obs) as obs, and then defining for
    each ob, a pmf of simplified versions.  In python,
"""
      def express(ob):
        "randomly express an ob WRT combineProb and initPMF"
        if (ob in map(expr2ob,initPMF)) and not bernoulli(combineProb):
          for atom in initPMF: # an expr PMF
            if ob is expr2ob(atom):
              if bernoulli(initPMF[atom]):
                result = atom
        else:
          for lhs in naivePMF: # an ob PMF
            for rhs in naivePMF:
              if lhs*rhs is ob:
                result = express(lhs)*express(rhs)
        return result
"""
  (N4) turing's universal combinator U (or tromp's shorter version) is an <S,K>
    implementation of the current expr2ob function.  its stochastic inverse is the
    aforementioned express() function.
  (N5) output can be restricted to ob strings, i.e., ob exprs, or better, to R-pmfs
    of ob strings.  an ob-external function can then implement the single function
    .choose(), which randomly selects elements of the restricted pmf subject to a
    known-to-be-strings selection constraint.  This seems reasonable, as neither
    random choice nor selection based on current db state can be proper obs.  It is
    also important to note that string representations are unique, if they exist.
  (A1) the db (johann) will have the following interface:
    (V1) obs <--> exprs, and directed thinking:
      (N1) .expr2ob(expr)
      (N2) .thinkAbout(ob)
      (N3) .ob2expr(ob)
    (V2) directed thinking and simplification
      (N1) setPriorBasis: PMF --|
      (N2) setParseBasis: PMF --|
      (N3) setApplyProb: float --|
      (N4) simplify: Expr --> Expr #does random choice and knowledge-based selection
    (V2) directed thinking and non-ob functions
      (N1) setPriorBasis: PMF(expr) --|
      (N2) setParseBasis: PMF(expr) --|
      (N3) setApplyProb: float --|
      (N4) expr2ob: Expr --> Ob
      (N4') expr2obString: expr --> ob
      (N5) express: Ob --> Ob
      (N6) choose: Ob --> Ob
      (N6) isKnown: Ob --> bool #i.e., is known to be true
  (N6) it may be best to interface via pseudo-obs, for which the usual axioms do not
    apply.  For example, a random bit-stream ob, a random-chooser ob, an input ob,
    a known-true ob, etc.  The output ob might be a real ob, but have the special
    property of being the source of all parse-weight.
  (N7) finding obs which satisfy certain equations might best be done by an estimate()
    pseudo-ob.

(Q3) (less immediate) How are memories accumulated over time?
  (N1) the most notable difference between any living thinking creature and a slicon
    computer is the biological creature's ability to accumulate habits, for behavior to
    repeat with only slight modifications.  computers, on the contrary, accumulate
    information not by practice, but by listening exactly to what they hear.  thus, to
    exhibit expert ability, accumulated progressively, interactively, and organically,
    a computer must support an individual living organism.
    
    This living organism must be trained by a single good/bad control stream (for
    learning) and an input/output pair of information streams (the control is assumed
    to be much less verbose).  The input/output might be, e.g., a pair of sentence
    streams, controlled by a PMF ranking of multiple outputs.  alternatively, the
    input/output could be character streams, ?but what can the control be?.

    Humans notice what circumstances have led to pain, and try to control future
    soas not to repeat the past painful circumstances.  Thus, some idea of state should
    be understood, and a pain-labelled map of all states should be maintained.  As a
    controller, the robot should also abe able to navigate this state map soas to
    dynamically maximize some time-integrated pain-of-state, and predict far enough
    ahead to understand "no pain, no gain".
    
(N1) it might be best to find the most natural relationship between a finite structure
  (of ?some? sort) and the infinite extionsional ob equivalence structure, and then
  fit [directed thinking / PMF expansion / estimation / control] into that framework.
  (N1) the only known relationship is this stochastic Todd-Coxeter-like algorithm,
    whereby obs are enumerated based on their estimated solomonoff complexity.
    (N1) the estimated Solomonoff complexity can be defined as an <S,K,Q,R> function,
      i.e., as the probability in a particular type (app-symmetric) of R-PMF.
    (N2) one could generalize this to enumerating obs (Todd-Coxeter-style) in an
      arbitrary R-PMF.  consider some example R-PMFs:
      (E1) singleton
        G = l*(f*x)/f/x/l
        Lower = Y*G
      (E2) sampling
        G = ???
        Raise = Y*G
      (E3) disjunction
        Disj = R*x*y
      (E4) conjunction
        Conj = 
      (E5) PMF mapping
        r*f
      (E6) stochastic function
        r*(f*x/f)
      (E7) stochastic PMF mapping
        ???
      (E8) selection
        G = g*(p*...)
        Sel = Y*G/r/p
        Sel*p*r
      (E9) estimation
        x_{i+1} = ...
      (E10) control
      Is there a computational advantagage in using floating-point probs to
      approximate the Solomonoff measure for randomly selecting untabulated obs?
      It does make it possible, however, to take statistics, such as total tabulated
      prob, entropy, and number.
      If one actually selects obs to create/delete not from the pure naive measure,
      but from a posterior measure accounting for computational resource consumption,
      then floating-point calculation might be faster.
      Alternatively, a clever db might be able to figure out how to do that
      automatically/autonomously.
    (N3) in the Solomonoff-style measure, probability correlates quite well with
      computational resource consumption (space).  In more general settings, e.g.,
      when selection or mapping pmfs are used, or when pmf-pmfs are used, the
      correlation will not hold as well.  In this case, one might use two PMFs: one
      for the problem of interest, and another for managing simplification.  The
      total measure would thus be defined by:
        P1(x) = p  #interest measure, p is an R-PMF
        P2(x) = Parse*tau*p  #parse measure, tau is an R-number in (0,1)
        P3(x) = Conj*(Naive*tau)*(Parse*tau*p)  #posterior measure
      (N1) it might be better to include the naive measure in parsing.

#==========[ design plans ]==========
(V1)
  (P1) build an ob db structurally following the extended Todd-Coxeter algorithm
  (P2) ob prob is determined by a postPMF: determined by interestPMF, naivePMF, appProb
  (P3) obs are input/named by expr [character] strings (expr --> ob)
  (P4) obs are output by a single chooseString operation (ob --> expr) 

(V2)
  (P1) a db is built up by extended Todd-Coxeter
  (P2) a single ob is input.  assume this takes the form of an ob string pmf.
  (P3) the db changes shape according to the input ob
  (P4)
=
(V3)
  (P1) a world --> input map is randomly learned, to estimate future inputs
    (N1) this is essentialy the conceptual world-state
    (N2) this is pure estimation theory
  (P2) the output --> world map is learned through time
    (N1) this is objective control theory
  (P3) the world --> contol map is randomly learned, to estimate future control
    (N1) this is subjective control
    (N2) thus, the world --> input map is augmented with control information, "another dim"
  (P4) output is constrained to optimize the control
  (N1) all maps are learned by pulling empirical measures back to the finitely approximated
    world-state.  the world-state is adaptively pruned based on the resulting correlation
    between input, output, and control.  this is sort of a triple version of relentropy,
    sort of a control entropy.
  (N2) looking through memories, i find a variety of records linked to each.  for example,
    cognition makes me think of asking karl about cognition, and the exact place and time
    of day, textures, sunlighting, our orientation, the sounds, emotional reactions, etc.,
    all linked together, and also to the relative future and past, if through induced
    metrics like "later... (means we moved up the trail, reference that) ..." and i
    accidentally reference brian brew dropping a bottle at the base of greyrock (though
    this time in the dark).  so very truly, there is a map in my brain of what i was doing,
    thinking, etc at each place i've been, and things are locationally connected.  even
    the positional map is really just a bunch of visual images linked together, the most
    vivid of which are the aerials which relate many concepts in a single image.
    (N1) it might be, therefore, a good teaching technique to give an overall map, present
      some local images associated with their little memories, and then in summary, review
      each little memory in mentally traversing time through a re-presentation of the
      overall map.
  (N3) pruning must eventually be able to randomly select old obs to prune and new obs to
    make, all WRT an importance measure.  Thus, there must be, at least implicitly, a
    probability measure defined on the db's obs.
  (Q1) what is the world state?
    (A1) a partially observable stochastic function
  (Q2) in estimation, what is the importance semimetric?
    (A1) relative entropy
  (Q2) in objective control, " " " " "?
    (A1) one can consider the control as another observable of the system, then
      stochastically model the "subjective" controller, and unify the resulting system in
      an estimation setting.
  (Q3) in subjective control, " " " " "?
    (A1) one now considers the input, output, and control all as observables, with a sort of
      constraint that the control is sort of known already.
  (N4) the point is that all approximation processes can be unified under a single db
    pruning operation.  the current model modifies the db at each step in an operation
    that ought to be moved one level down.  one then needs to evaluate the utility of adding
    or deleting individual obs.

(N1) the total number of obs is eventually bounded by the number of applications.
  (N1) each application has overhead
      |{l,r,u}x{MIO,MOI,IMO,IOM,OMI,OIM} + {a,l,r} + {prev,next}| = 3*6 + 3 = 21 pointers
    or 24 to be safe.  now each pointer is 4 bytes, so sizeof(App) = 4*24 = 96, almost 100B.
    that means a 10GB space can hold ~ 10^(10-2) = 10^8 = 100MApps.  That's a fucking lot.
    That's a worst-case 10^(8/2) = 10^4 = 10KObs, probably more like 2^14 =1.6e4.
    now each ob has
      |{MIO,MOI,IMO,IOM,OMI,OIM} + {prev,next} + {rep}| = 9 pointers
    or 10 to be safe.  these are so few, however, that extra overhead is no problem.  throw
    some floating points in there.
  (N2) such numerous obs/apps requires sparse probability update.  thus there is an update
    queue.
  (N3) all the processing queues are implemented as heaps

(N2) the db's interface is:
  resize(numApps) #the number of apps is always bounding
  think(time) #reshapes the db
  focusOn(ob) #sets the interest measure
  getChoice(ob) #randomly draws from the interest measure
  saveToTable(table) #also reorders WRT importance
  loadFromTable(table)
  enforceTheorems() #run a global theorem enforcement step
  update() #proccesses all task queues

(Q1) is there an ob-free app-only way to set up the data structures?  Perhaps each ob x
  can be represented by the app I*x=x.  thus,
    x.a = x.r = a
    x.l = I
    x.u_{M,I,O} = x (x is its own root)
    x.rep = ??? (all apps now need reps)
    I.MIO lists all obs

(Q2) can it be shown that Y*I=I ?
"""
Y = (f*(x*x)/x)*(f*(x*x)/x)/f
I = x/x
Y*I = x*x/x*(x*x/x)
    = x*x/x*(x*x/x)*y/y
    = x*x*y/x*(x*x/x)/y
    = ...



