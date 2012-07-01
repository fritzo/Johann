
Johann
======
Bayesian induction of programs, à la Solomonoff

Johann is an equational theorem-proving system and Bayesian inference engine for various extensions of combinatory algebra (equivalently, lambda-calculus),
making particular use of an interpretation of typed lambda-calculus in untyped concurrent lambda-calculus.
Details can be found in the Ph.D. thesis http://fritzo.org/thesis.pdf .
Johann implements:

* Equational theorem proving for various programming languages.

* Empirical Bayes learning of nonparametric PCFG models of programming languages.

* Bayesian inference of probabilistic programs (aka Solomonoff induction).

* Data-driven automated-conjecturing of equations in undecidable theories.

This repository includes:

* A C++ kernel for building and reasoning about combinatory databases.
  See the src/ directory.

* A collection of code-as-dissertation in a literate programming style ".jtext",
  which develops a lambda-calculus corpus for datamining.
  See the scripts/ directory

* A Python front end to latex for typesetting the literate programs
  ( __jtext2latex___ ).
  See the jtext/ directory.

* A C++ mapper to visualize Johann databases.
  See the mapper/ directory.

* Some sketches of web-apps using Johann databases.
  See the html/ and src/ directories.

From the thesis abstract (scripts/abstract.text):

> ...
> The final component of this thesis is a system, Johann, for automated
> reasoning about equality and order in the above languages.
> Johann was used to formally verify many of the theorems in this thesis
> (and even conjecture some simple theorems).
> The general design focus is on efficient knowledge representation, rather than
> proof search strategies.
> Johann maintains a database of all facts about a set of (say 10k) objects, or
> terms-modulo-equivalence.
> The database evolves in time by randomly adding or removing objects.
> Each time an object is added, the database is saturated with facts using a
> forward chaining algorithm.
>
> A specific design goal is to be able to run Johann for long periods of time
> (weeks) and accumulate useful knowledge, subject to limited memory.
> This requires statistical analysis of a corpus of interest (e.g.
> the set of problems to be verified in this thesis), statistical search for
> missing equations (from our sigma-01 approximation to a pi-02-complete
> theory), and careful choice of sampling distributions from which to draw
> objects to add-to and remove-from the database.
> The (add,remove) pair of distributions is chosen to achieve a detailed balance
> theorem, so that, at steady state, Johann (provably) remembers simple facts
> relevant to the corpus.

Installation
------------
On an ubuntu 12.04 system:

* to build the johann system run requirements.sh, then make

* to build documentation run pdf-requirements.sh, then make pdf

Roadmap
-------

* Architecture:

    * depricate pervasive use of nodes (esp, for equations)

    * rearchitect to separate symbolic from probabilistic:
      brain, combinatory structure, languages, ...

    * implement finer-grained threading in kernel

    * separate kernel from console

    * separate jdb into generic relational database

* Parallelization:

    * replace splay forests with hash tables of arrays

    * parallelize forward-chaining workers (dropping the saturation guarantee)
      but run axiom validation in background

    * implpement multi-kernel collaboration via equation-trading

* Infrastructure:

    * wrap expressions in python and move syntactic algorithms to python

    * switch to better build system, e.g. cmake, jam

    * switch to zeromq for kernel interface

    * replace custom .jdb format with HDF5

    * switch to TBB for threading library

