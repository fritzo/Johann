#makefile for johann

#compile type
#COMPILE_TYPE = debug
COMPILE_TYPE = fastdebug
#COMPILE_TYPE = fast
#COMPILE_TYPE = profile

#platform
PLATFORM = linux
#PLATFORM = cygwin
#PLATFORM = mac

#these only apply when debugging or optimizing, resp.
DEBUG_LEVEL = 3
OPTIM_LEVEL = 1

#=======[ parameter processing ]===============================================

#processing utilities
#XXX compilation fails with g++-4.3; templates are not instantiated correctly
CXX  = g++
CC   = g++
LEX  = flex
YACC = bison

#warnings
WARNINGS = -Wall -Woverloaded-virtual -Wsign-promo -Wundef -Wpointer-arith -Wcast-qual -Wcast-align -Wno-deprecated

#various levels of optimization (for 'fast' and 'profile' only)
WEAK_OPTIM = -O2 -pipe
ifeq ($(OPTIM_LEVEL), 0)
	OPTIM = -O1 -pipe
endif
ifeq ($(OPTIM_LEVEL), 1)
	OPTIM = -O2 -funswitch-loops -fomit-frame-pointer -pipe
endif
ifeq ($(OPTIM_LEVEL), 2) #this seems to brake in gcc4
	OPTIM = -O3 -repo -fomit-frame-pointer -pipe
endif
ifeq ($(OPTIM_LEVEL), 3)
	OPTIM = -O3 -m32 -march=prescott -malign-double -ffast-math -mfpmath=sse -fomit-frame-pointer -pipe
endif
ifeq ($(OPTIM_LEVEL), 4)
	OPTIM = -Winline -Wdisable-optimization -O3 -m32 -march=prescott -mmmx -msse -msse2 -msse3 -mfpmath=sse,387 -malign-double -fomit-frame-pointer -ffast-math -funsafe-math-optimizations -fsingle-precision-constant -pipe
endif
#LATER: try using the two-pass  -fprofile-arcs and -fbranch-probabilities
#LATER: try -ftracer -funroll-loops, -fprefetch-loop-arrays

#opengl stuff
GL_LINUX = -lglut -lGLU -lGL
GL_MAC = -framework OpenGL -framework Glut
GL_CYGWIN = -lglut32 -lglu32 -lopengl32

#compiler flags
#LFLAGS   = -d #flex is called manually below
YFLAGS   = -d
CPPFLAGS = -Wstrict-aliasing=0 --std=c++0x
CXXFLAGS = -fno-strict-aliasing
LDFLAGS =
ifeq ($(COMPILE_TYPE), debug)
	CPPFLAGS += -DDEBUG_LEVEL=$(DEBUG_LEVEL)
	CXXFLAGS = $(WARNINGS) -ggdb
	LDFLAGS  += -rdynamic
	ifeq ($(PLATFORM), linux)
		LDFLAGS  += -rdynamic
	endif
endif
ifeq ($(COMPILE_TYPE), fastdebug)
	CPPFLAGS += -DDEBUG_LEVEL=$(DEBUG_LEVEL)
	CXXFLAGS = $(WEAK_OPTIM) $(WARNINGS) -ggdb
	LDFLAGS  += $(WEAK_OPTIM)
	ifeq ($(PLATFORM), linux)
		LDFLAGS  += -rdynamic
	endif
endif
ifeq ($(COMPILE_TYPE), fast)
	CPPFLAGS += -DDEBUG_LEVEL=0
	CXXFLAGS = $(OPTIM)
	LDFLAGS  += $(OPTIM)
endif
ifeq ($(COMPILE_TYPE), profile)
	CPPFLAGS += -DDEBUG_LEVEL=0 -DPROFILE
	CXXFLAGS = $(WEAK_OPTIM) -pg -ftest-coverage -fprofile-arcs
	LDFLAGS  += $(WEAK_OPTIM) -pg -ftest-coverage -fprofile-arcs
endif

#platform-dependent flags
ifeq ($(PLATFORM), linux)
	CPPFLAGS += -pthread
	LDFLAGS += -pthread
	GL = $(GL_LINUX)
endif
ifeq ($(PLATFORM), mac)
	CPPFLAGS += -DMAC_HACKS
	GL = $(GL_MAC)
endif

#=======[ modules ]============================================================

#main targets
current_target: all
all: kernel server
#	$(MAKE) -C cgi-src all

#tests
priority_queue_test: priority_queue_test.o definitions.o
fifo_queue_test: fifo_queue_test.o definitions.o
visual_test: visual_test.o visual.o definitions.o
	$(CC) visual_test.o visual.o definitions.o -lpng -o visual_test
linalg_test: linalg_test.o linalg.o definitions.o
	$(CC) linalg_test.o linalg.o definitions.o -llapack -lblas -o linalg_test
moments_test: moments_test.o definitions.o
meas_lite_test: meas_lite.o files.o definitions.o
optimization_test: optimization_test.o optimization.o linalg.o definitions.o
	$(CC) optimization_test.o optimization.o linalg.o definitions.o -llapack -lblas -o optimization_test
binary_relation_test: binary_relation_test.o dense_bin_rel.o dense_set.o aligned_alloc.o definitions.o
dense_sym_fun_test: dense_sym_fun_test.o dense_sym_fun.o dense_set.o aligned_alloc.o definitions.o

EXPR_H = expressions.h substitute.h handling.h symbols.h large_set.h definitions.h
EXPR_O = expressions.o substitute.o handling.o large_set.o

handling_test: handling_test.o handling.o definitions.o
node_heap_test: node_heap_test.o node_heap.o aligned_alloc.o definitions.o
random_choice_test: random_choice_test.o node_heap.o aligned_alloc.o definitions.o

#the tangled mess begins
NODES_O = nodes.o reorder.o obs.o order.o apply.o compose.o join.o node_heap.o aligned_alloc.o
LT_O = lambda_theories.o axiom_tools.o axiom_enforcement.o axiom_validation.o $(EXPR_O) syntax_semantics.o
BRAIN_O = $(NODES_O) brain.o $(LT_O) dense_sym_fun.o dense_bin_fun.o sparse_bin_fun.o dense_bin_rel.o dense_set.o combinatory_structure.o measures.o languages.o fuzzy.o complexity.o linalg.o random_choice.o thought.o context.o statements.o optimization.o files.o visual.o definitions.o
SOCK_O = socket_tools.o thread_tools.o

#console
CONSOLE_H = console.h kernel.h context.h $(EXPR_H) statements.h symbols.h version.h definitions.h
console.tab.c: console.y
	bison -d -v console.y
console.y.c: console.l
	flex -oconsole.y.c console.l
#	flex -d -v -oconsole.y.c console.l
#note: the dependence is really on console.tab.h
console.y.o: console.y.c console.tab.c $(CONSOLE_H)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -Wno-old-style-cast console.y.c -c
console.tab.o: console.tab.c $(CONSOLE_H)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -Wno-old-style-cast console.tab.c -c
console.o: console.C $(CONSOLE_H)
CONSOLE_O = console.tab.o console.y.o console.o

#parser
#parser.tab.o: parser.l parser.y
#	bison -d -v parser.y
#	flex -oparser.y.c parser.l
#	$(CXX) $(CXXFLAGS) -Wno-old-style-cast parser.y.c -c
#	$(CXX) $(CXXFLAGS) -Wno-old-style-cast parser.tab.c -c
PARSER_H = parser.h $(EXPR_H) symbols.h definitions.h
parser.tab.c: parser.y
	bison -d -v parser.y
	#bison -d -v -t parser.y
parser.y.c: parser.l
	flex -oparser.y.c parser.l
	#flex -oparser.y.c -d -v -T parser.l
parser.y.o: parser.y.c parser.tab.c $(PARSER_H)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -Wno-old-style-cast parser.y.c -c
parser.tab.o: parser.tab.c $(PARSER_H)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -Wno-old-style-cast parser.tab.c -c
parser.o: parser.C $(PARSER_H)
PARSER_O = parser.tab.o parser.y.o parser.o

#kernel
KER_O = $(BRAIN_O) kernel.o
kernel: $(KER_O) $(CONSOLE_O) $(PARSER_O) $(SOCK_O)
	$(CXX) $(CXXFLAGS) $(KER_O) $(CONSOLE_O) $(PARSER_O) $(SOCK_O) -lfl -lreadline -lhistory -lcurses -llapack -lblas -lpng -lbz2 -o kernel -g $(LDFLAGS)

#server
SERVER_O = market.o simple.o $(PARSER_O) $(SOCK_O) $(EXPR_O) definitions.o server.o
server: $(SERVER_O)
	$(CXX) $(CPPFLAGS) $(SERVER_O) -ggdb -lpthread -lbz2 -o server -g $(LDFLAGS)

#toy
TOY_O = toy.o dynamics.o animator.o meas_lite.o files.o definitions.o
toy: $(TOY_O)
	$(CXX) $(CXXFLAGS) $(TOY_O) $(GL) -o toy -g $(LDFLAGS)

#=======[ unit tests ]=========================================================

#tests resquiring parser
expressions_test: expressions_test.o $(EXPR_O) $(PARSER_O) definitions.o
unification_test: unification_test.o $(EXPR_O) $(PARSER_O) unification.o definitions.o

TESTS = priority_queue_test visual_test linalg_test handling_test moments_test optimization_test expressions_test binary_relation_test dense_sym_fun_test meas_lite_test
#node_heap_test random_choice_test

#all tests
test: $(TESTS)
	echo "" > log/test.log
	./run_tests

#individual tests
v_test: visual_test
	./visual_test
la_test: linalg_test
	./linalg_test
op_test: optimization_test
	./optimization_test
e_test: expressions_test
	./expressions_test
br_test: binary_relation_test
	./binary_relation_test
ds_test: dense_sym_fun_test
	./dense_sym_fun_test
rc_test: random_choice_test
	./random_choice_test
u_test: unification_test
	./unification_test
ml_test: meas_lite_test
	./meas_lite_test


#=======[ utilities ]==========================================================

#exuberant ctags
tags: FORCE
	ctags-exuberant *.[lyCh]	

#cleaning
EXEC = a.out $(TESTS) kernel server toy
PARSE_TEMPS = *.yy.c *.y.c *.tab.h *.tab.c *.tab.cc *.cc *.hh *.output
PROF_TEMPS = *.gcov gmon.out *.da *.bb *.bbg *.prof cachegrind.out.* *.gcda *.gcno
MISC_TEMPS =  core core.* *.stackdump *.log comb_struct.table temp.diff test.*.png tags TAGS
oclean:
	rm -f *.o *.rpo $(EXEC) $(PARSE_TEMPS)
depclean:
	makedepend
clean: oclean
	rm -f $(MISC_TEMPS) $(PROF_TEMPS)
	$(MAKE) -C cgi-src clean
cleaner: clean depclean

#DEP_INC = -I /usr/lib/gcc/i686-pc-linux-gnu/4.1.1/include/ -I /usr/lib/gcc/i686-pc-linux-gnu/4.1.1/include/g++-v4/ -I /usr/lib/gcc/i686-pc-linux-gnu/4.1.1/include/g++-v4/i686-pc-linux-gnu/
DEP_INC = -Y
#depend: console.tab.c console.y.c parser.tab.cc parser.yy.c
depend: console.tab.c console.y.c
	makedepend $(DEP_INC) -D__GNUG__ -- $(CPPFLAGS) $(CXXFLAGS) -- *.[Ch] &> /dev/null

# DO NOT DELETE THIS LINE -- make depend depends on it.

aligned_alloc.o: definitions.h aligned_alloc.h
animator.o: animator.h definitions.h
animator.o: definitions.h
apply.o: apply.h definitions.h nodes.h node_heap.h aligned_alloc.h
apply.o: dense_bin_fun.h dense_set.h splay_forest.h reorder.h obs.h symbols.h
apply.o: combinatory_structure.h order.h dense_bin_rel.h
apply.o: definitions.h nodes.h node_heap.h aligned_alloc.h dense_bin_fun.h
apply.o: dense_set.h splay_forest.h reorder.h
axiom_enforcement.o: axiom_tools.h lambda_theories.h definitions.h symbols.h
axiom_enforcement.o: expressions.h large_set.h handling.h hash_map.h
axiom_enforcement.o: statements.h nodes.h node_heap.h aligned_alloc.h order.h
axiom_enforcement.o: dense_bin_rel.h dense_set.h obs.h reorder.h apply.h
axiom_enforcement.o: dense_bin_fun.h splay_forest.h compose.h join.h
axiom_enforcement.o: dense_sym_fun.h combinatory_structure.h
axiom_tools.o: axiom_tools.h lambda_theories.h definitions.h symbols.h
axiom_tools.o: expressions.h large_set.h handling.h hash_map.h statements.h
axiom_tools.o: nodes.h node_heap.h aligned_alloc.h order.h dense_bin_rel.h
axiom_tools.o: dense_set.h obs.h reorder.h apply.h dense_bin_fun.h
axiom_tools.o: splay_forest.h compose.h join.h dense_sym_fun.h
axiom_tools.o: combinatory_structure.h
axiom_tools.o: lambda_theories.h definitions.h symbols.h expressions.h
axiom_tools.o: large_set.h handling.h hash_map.h statements.h nodes.h
axiom_tools.o: node_heap.h aligned_alloc.h order.h dense_bin_rel.h
axiom_tools.o: dense_set.h obs.h reorder.h apply.h dense_bin_fun.h
axiom_tools.o: splay_forest.h compose.h join.h dense_sym_fun.h
axiom_tools.o: combinatory_structure.h
axiom_validation.o: axiom_tools.h lambda_theories.h definitions.h symbols.h
axiom_validation.o: expressions.h large_set.h handling.h hash_map.h
axiom_validation.o: statements.h nodes.h node_heap.h aligned_alloc.h order.h
axiom_validation.o: dense_bin_rel.h dense_set.h obs.h reorder.h apply.h
axiom_validation.o: dense_bin_fun.h splay_forest.h compose.h join.h
axiom_validation.o: dense_sym_fun.h combinatory_structure.h
axiom_validation.o: syntax_semantics.h measures.h linalg.h
binary_relation_test.o: definitions.h dense_bin_rel.h dense_set.h
brain.o: brain.h definitions.h nodes.h node_heap.h aligned_alloc.h measures.h
brain.o: linalg.h languages.h symbols.h fuzzy.h context.h expressions.h
brain.o: large_set.h handling.h hash_map.h statements.h lambda_theories.h
brain.o: order.h dense_bin_rel.h dense_set.h thought.h version.h
brain.o: random_choice.h complexity.h moments.h obs.h reorder.h apply.h
brain.o: dense_bin_fun.h splay_forest.h compose.h join.h dense_sym_fun.h
brain.o: combinatory_structure.h syntax_semantics.h sorting.h files.h
brain.o: visual.h socket_tools.h thread_tools.h
brain.o: definitions.h nodes.h node_heap.h aligned_alloc.h measures.h
brain.o: linalg.h languages.h symbols.h fuzzy.h context.h expressions.h
brain.o: large_set.h handling.h hash_map.h statements.h lambda_theories.h
brain.o: order.h dense_bin_rel.h dense_set.h thought.h
collective.o: collective.h definitions.h
collective.o: definitions.h
combinatory_structure.o: combinatory_structure.h definitions.h symbols.h
combinatory_structure.o: nodes.h node_heap.h aligned_alloc.h order.h
combinatory_structure.o: dense_bin_rel.h dense_set.h reorder.h obs.h apply.h
combinatory_structure.o: dense_bin_fun.h splay_forest.h compose.h join.h
combinatory_structure.o: dense_sym_fun.h lambda_theories.h expressions.h
combinatory_structure.o: large_set.h handling.h hash_map.h statements.h
combinatory_structure.o: priority_queue.h syntax_semantics.h measures.h
combinatory_structure.o: linalg.h files.h version.h visual.h
combinatory_structure.o: definitions.h symbols.h nodes.h node_heap.h
combinatory_structure.o: aligned_alloc.h order.h dense_bin_rel.h dense_set.h
combinatory_structure.o: reorder.h
complexity.o: complexity.h definitions.h symbols.h measures.h nodes.h
complexity.o: node_heap.h aligned_alloc.h linalg.h languages.h moments.h
complexity.o: fuzzy.h order.h dense_bin_rel.h dense_set.h thought.h
complexity.o: expressions.h large_set.h handling.h hash_map.h context.h
complexity.o: statements.h optimization.h obs.h reorder.h apply.h
complexity.o: dense_bin_fun.h splay_forest.h compose.h join.h dense_sym_fun.h
complexity.o: combinatory_structure.h lambda_theories.h
complexity.o: definitions.h symbols.h measures.h nodes.h node_heap.h
complexity.o: aligned_alloc.h linalg.h languages.h moments.h fuzzy.h order.h
complexity.o: dense_bin_rel.h dense_set.h thought.h expressions.h large_set.h
complexity.o: handling.h hash_map.h context.h statements.h
compose.o: compose.h definitions.h nodes.h node_heap.h aligned_alloc.h
compose.o: dense_bin_fun.h dense_set.h splay_forest.h reorder.h obs.h
compose.o: symbols.h combinatory_structure.h order.h dense_bin_rel.h
compose.o: definitions.h nodes.h node_heap.h aligned_alloc.h dense_bin_fun.h
compose.o: dense_set.h splay_forest.h reorder.h
console.o: console.h definitions.h version.h symbols.h expressions.h
console.o: large_set.h handling.h hash_map.h statements.h substitute.h
console.o: kernel.h context.h
console.o: definitions.h version.h symbols.h expressions.h large_set.h
console.o: handling.h hash_map.h statements.h substitute.h kernel.h context.h
context.o: context.h definitions.h symbols.h expressions.h large_set.h
context.o: handling.h hash_map.h statements.h syntax_semantics.h nodes.h
context.o: node_heap.h aligned_alloc.h order.h dense_bin_rel.h dense_set.h
context.o: measures.h linalg.h
context.o: definitions.h symbols.h expressions.h large_set.h handling.h
context.o: hash_map.h statements.h
definitions.o: definitions.h
dense_bin_fun.o: dense_bin_fun.h definitions.h dense_set.h aligned_alloc.h
dense_bin_fun.o: definitions.h dense_set.h
dense_bin_rel.o: dense_bin_rel.h definitions.h dense_set.h
dense_bin_rel.o: definitions.h dense_set.h
dense_set.o: dense_set.h definitions.h
dense_set.o: definitions.h
dense_sym_fun.o: dense_sym_fun.h definitions.h dense_set.h aligned_alloc.h
dense_sym_fun.o: definitions.h dense_set.h
dense_sym_fun_test.o: definitions.h dense_sym_fun.h dense_set.h
dynamics.o: dynamics.h definitions.h meas_lite.h
dynamics.o: definitions.h meas_lite.h
expressions.o: expressions.h definitions.h symbols.h large_set.h handling.h
expressions.o: hash_map.h parser.h substitute.h buffer.h output.h
expressions.o: definitions.h symbols.h large_set.h handling.h hash_map.h
expressions_test.o: definitions.h expressions.h symbols.h large_set.h
expressions_test.o: handling.h hash_map.h
fifo_queue.o: definitions.h hash_map.h
fifo_queue_test.o: definitions.h fifo_queue.h hash_map.h
files.o: files.h definitions.h version.h
files.o: definitions.h version.h
fuzzy.o: fuzzy.h definitions.h symbols.h nodes.h node_heap.h aligned_alloc.h
fuzzy.o: version.h
fuzzy.o: definitions.h symbols.h nodes.h node_heap.h aligned_alloc.h
handling.o: handling.h definitions.h
handling.o: definitions.h
handling_test.o: handling.h definitions.h
join.o: join.h definitions.h nodes.h node_heap.h aligned_alloc.h
join.o: dense_sym_fun.h dense_set.h splay_forest.h reorder.h obs.h symbols.h
join.o: combinatory_structure.h order.h dense_bin_rel.h
join.o: definitions.h nodes.h node_heap.h aligned_alloc.h dense_sym_fun.h
join.o: dense_set.h splay_forest.h reorder.h
kernel.o: kernel.h definitions.h symbols.h expressions.h large_set.h
kernel.o: handling.h hash_map.h statements.h context.h version.h nodes.h
kernel.o: node_heap.h aligned_alloc.h obs.h reorder.h apply.h dense_bin_fun.h
kernel.o: dense_set.h splay_forest.h compose.h join.h dense_sym_fun.h
kernel.o: substitute.h order.h dense_bin_rel.h brain.h measures.h linalg.h
kernel.o: languages.h fuzzy.h lambda_theories.h thought.h syntax_semantics.h
kernel.o: combinatory_structure.h socket_tools.h thread_tools.h
kernel.o: definitions.h symbols.h expressions.h large_set.h handling.h
kernel.o: hash_map.h statements.h context.h
lambda_theories.o: lambda_theories.h definitions.h symbols.h expressions.h
lambda_theories.o: large_set.h handling.h hash_map.h statements.h nodes.h
lambda_theories.o: node_heap.h aligned_alloc.h order.h dense_bin_rel.h
lambda_theories.o: dense_set.h obs.h reorder.h apply.h dense_bin_fun.h
lambda_theories.o: splay_forest.h compose.h join.h dense_sym_fun.h
lambda_theories.o: combinatory_structure.h syntax_semantics.h measures.h
lambda_theories.o: linalg.h priority_queue.h
lambda_theories.o: definitions.h symbols.h expressions.h large_set.h
lambda_theories.o: handling.h hash_map.h statements.h nodes.h node_heap.h
lambda_theories.o: aligned_alloc.h order.h dense_bin_rel.h dense_set.h
languages.o: languages.h definitions.h symbols.h nodes.h node_heap.h
languages.o: aligned_alloc.h linalg.h files.h version.h
languages.o: definitions.h symbols.h nodes.h node_heap.h aligned_alloc.h
languages.o: linalg.h
large_set.o: large_set.h definitions.h handling.h
large_set.o: definitions.h handling.h
linalg.o: linalg.h definitions.h
linalg.o: definitions.h
linalg_test.o: definitions.h linalg.h
market.o: market.h definitions.h expressions.h symbols.h large_set.h
market.o: handling.h hash_map.h thread_tools.h socket_tools.h
market.o: definitions.h expressions.h symbols.h large_set.h handling.h
market.o: hash_map.h thread_tools.h
meas_lite.o: meas_lite.h definitions.h files.h version.h
meas_lite.o: definitions.h
meas_lite_test.o: definitions.h meas_lite.h
measures.o: measures.h definitions.h nodes.h node_heap.h aligned_alloc.h
measures.o: linalg.h
measures.o: definitions.h nodes.h node_heap.h aligned_alloc.h linalg.h
moments_test.o: definitions.h moments.h
node_heap.o: node_heap.h definitions.h aligned_alloc.h nodes.h
node_heap.o: definitions.h aligned_alloc.h
node_heap_test.o: definitions.h node_heap.h aligned_alloc.h
nodes.o: nodes.h node_heap.h definitions.h aligned_alloc.h
nodes.o: node_heap.h definitions.h aligned_alloc.h
obs.o: obs.h definitions.h symbols.h nodes.h node_heap.h aligned_alloc.h
obs.o: reorder.h apply.h dense_bin_fun.h dense_set.h splay_forest.h compose.h
obs.o: join.h dense_sym_fun.h combinatory_structure.h order.h dense_bin_rel.h
obs.o: definitions.h symbols.h nodes.h node_heap.h aligned_alloc.h reorder.h
optimization.o: optimization.h definitions.h moments.h linalg.h
optimization.o: definitions.h moments.h linalg.h
optimization_test.o: definitions.h linalg.h optimization.h moments.h
order.o: order.h definitions.h nodes.h node_heap.h aligned_alloc.h
order.o: dense_bin_rel.h dense_set.h obs.h symbols.h reorder.h
order.o: definitions.h nodes.h node_heap.h aligned_alloc.h dense_bin_rel.h
order.o: dense_set.h
param_tools.o: definitions.h
parser.o: parser.h definitions.h expressions.h symbols.h large_set.h
parser.o: handling.h hash_map.h substitute.h buffer.h output.h
parser.o: definitions.h expressions.h symbols.h large_set.h handling.h
parser.o: hash_map.h substitute.h buffer.h output.h
parser.tab.o: stack.hh location.hh position.hh
priority_queue.o: definitions.h
priority_queue_test.o: definitions.h priority_queue.h
random_choice.o: random_choice.h definitions.h measures.h nodes.h node_heap.h
random_choice.o: aligned_alloc.h linalg.h
random_choice.o: definitions.h measures.h nodes.h node_heap.h aligned_alloc.h
random_choice.o: linalg.h
random_choice_test.o: measures.h definitions.h nodes.h node_heap.h
random_choice_test.o: aligned_alloc.h linalg.h random_choice.h
reorder.o: reorder.h definitions.h node_heap.h aligned_alloc.h nodes.h
reorder.o: definitions.h node_heap.h aligned_alloc.h
search.o: search.h definitions.h priority_queue.h
search.o: definitions.h priority_queue.h
server.o: definitions.h expressions.h symbols.h large_set.h handling.h
server.o: hash_map.h substitute.h socket_tools.h thread_tools.h simple.h
server.o: market.h version.h
simple.o: simple.h definitions.h expressions.h symbols.h large_set.h
simple.o: handling.h hash_map.h thread_tools.h
simple.o: definitions.h expressions.h symbols.h large_set.h handling.h
simple.o: hash_map.h thread_tools.h
small_set.o: small_set.h definitions.h
small_set.o: definitions.h
socket_tools.o: socket_tools.h definitions.h thread_tools.h
socket_tools.o: definitions.h thread_tools.h
solver.o: solver.h definitions.h expressions.h symbols.h large_set.h
solver.o: handling.h hash_map.h simple.h thread_tools.h search.h
solver.o: priority_queue.h
solver.o: definitions.h expressions.h symbols.h large_set.h handling.h
solver.o: hash_map.h simple.h thread_tools.h search.h priority_queue.h
sorting.o: definitions.h
sparse_bin_fun.o: sparse_bin_fun.h definitions.h dense_set.h hash_map.h
sparse_bin_fun.o: aligned_alloc.h
sparse_bin_fun.o: definitions.h dense_set.h hash_map.h
splay_forest.o: definitions.h
statements.o: statements.h definitions.h handling.h expressions.h symbols.h
statements.o: large_set.h hash_map.h
statements.o: definitions.h handling.h expressions.h symbols.h large_set.h
statements.o: hash_map.h
substitute.o: substitute.h definitions.h symbols.h expressions.h large_set.h
substitute.o: handling.h hash_map.h parser.h buffer.h output.h
substitute.o: definitions.h symbols.h expressions.h large_set.h handling.h
substitute.o: hash_map.h
syntax_semantics.o: syntax_semantics.h definitions.h nodes.h node_heap.h
syntax_semantics.o: aligned_alloc.h order.h dense_bin_rel.h dense_set.h
syntax_semantics.o: measures.h linalg.h expressions.h symbols.h large_set.h
syntax_semantics.o: handling.h hash_map.h statements.h obs.h reorder.h
syntax_semantics.o: apply.h dense_bin_fun.h splay_forest.h compose.h join.h
syntax_semantics.o: dense_sym_fun.h combinatory_structure.h brain.h
syntax_semantics.o: languages.h fuzzy.h context.h lambda_theories.h thought.h
syntax_semantics.o: definitions.h nodes.h node_heap.h aligned_alloc.h order.h
syntax_semantics.o: dense_bin_rel.h dense_set.h measures.h linalg.h
syntax_semantics.o: expressions.h symbols.h large_set.h handling.h hash_map.h
syntax_semantics.o: statements.h
thought.o: thought.h definitions.h nodes.h node_heap.h aligned_alloc.h
thought.o: order.h dense_bin_rel.h dense_set.h expressions.h symbols.h
thought.o: large_set.h handling.h hash_map.h context.h statements.h
thought.o: measures.h linalg.h fuzzy.h obs.h reorder.h apply.h
thought.o: dense_bin_fun.h splay_forest.h compose.h join.h dense_sym_fun.h
thought.o: combinatory_structure.h lambda_theories.h syntax_semantics.h
thought.o: brain.h languages.h complexity.h moments.h random_choice.h
thought.o: definitions.h nodes.h node_heap.h aligned_alloc.h order.h
thought.o: dense_bin_rel.h dense_set.h expressions.h symbols.h large_set.h
thought.o: handling.h hash_map.h context.h statements.h measures.h linalg.h
thought.o: fuzzy.h
thread_tools.o: thread_tools.h definitions.h
thread_tools.o: definitions.h
toy.o: definitions.h meas_lite.h animator.h dynamics.h
unification.o: definitions.h expressions.h symbols.h large_set.h handling.h
unification.o: hash_map.h
unification_test.o: definitions.h expressions.h symbols.h large_set.h
unification_test.o: handling.h hash_map.h unification.h
visual.o: visual.h definitions.h
visual.o: definitions.h
visual_test.o: definitions.h visual.h
