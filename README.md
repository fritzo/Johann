
Johann
======

Johann is a theorem-proving system that implements data-driven
automated-conjecturing by building and statistically analyzing
a large database of equtations in various extensions of combinatory algebra
(equivalently, lambda-calculus).
This repository includes

* C++ code to build combinatory databases.
  See the cpp/ directory.

* A collection of code-as-dissertation in a literate programming style ".jtext",
  which develops a lambda-calculus corpus for datamining.
  See the scripts/ directory

* A Python front end to latex for typesetting the literate programs
  ( __jtext2latex___ ).
  See the jtext/ directory.

* A C++ mapper to visualize johann databases.
  See the mapper/ directory.

* Some sketches of web-apps using Johann databases.
  See the html/ and cpp/ directories.

From scripts/abstract.text:

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

EigenViz
--------

EigenViz is a little sub-project for visualizing sparse graphs
in 3D space + 3D color.
It is fast, simple, and based on OpenGL.
See the eigenvis/ directory for further information.

