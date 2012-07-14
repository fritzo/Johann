
current_target: all

all: kernel server toy

#----( params )----------------------------------------------------------------

#COMPILE_TYPE = debug
COMPILE_TYPE = fastdebug
#COMPILE_TYPE = fast
#COMPILE_TYPE = profile

PLATFORM := $(shell uname)
#PLATFORM = Linux
#PLATFORM = cygwin
#PLATFORM = Darwin

# these only apply when debugging or optimizing, resp.
DEBUG_LEVEL = 3
OPTIM_LEVEL = 1

#----( compile flags )---------------------------------------------------------

CXX = g++
CC = g++
LEX = flex
YACC = bison

WARNINGS = -Wall -Woverloaded-virtual -Wsign-promo -Wundef -Wpointer-arith -Wcast-qual -Wcast-align -Wno-deprecated

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
#TODO try using the two-pass  -fprofile-arcs and -fbranch-probabilities
#TODO try -ftracer -funroll-loops, -fprefetch-loop-arrays

# opengl stuff
GL_LINUX = -lglut -lGLU -lGL
GL_MAC = -framework OpenGL -framework Glut
GL_CYGWIN = -lglut32 -lglu32 -lopengl32

#LFLAGS = -d #flex is called manually below
YFLAGS = -d
CPPFLAGS = -I. -I../include -Wstrict-aliasing=0 --std=c++0x
CXXFLAGS = -fno-strict-aliasing
LDFLAGS = -L../lib -lpomagma
ifeq ($(COMPILE_TYPE), debug)
	CPPFLAGS += -DDEBUG_LEVEL=$(DEBUG_LEVEL)
	CXXFLAGS = $(WARNINGS) -ggdb
	LDFLAGS += -rdynamic
endif
ifeq ($(COMPILE_TYPE), fastdebug)
	CPPFLAGS += -DDEBUG_LEVEL=$(DEBUG_LEVEL)
	CXXFLAGS = $(WEAK_OPTIM) $(WARNINGS) -ggdb
	LDFLAGS += $(WEAK_OPTIM)
	ifeq ($(PLATFORM), Linux)
		LDFLAGS  += -rdynamic
	endif
endif
ifeq ($(COMPILE_TYPE), fast)
	CPPFLAGS += -DDEBUG_LEVEL=0
	CXXFLAGS = $(OPTIM)
	LDFLAGS += $(OPTIM)
endif
ifeq ($(COMPILE_TYPE), profile)
	CPPFLAGS += -DDEBUG_LEVEL=0 -DPROFILE
	CXXFLAGS = $(WEAK_OPTIM) -pg -ftest-coverage -fprofile-arcs
	LDFLAGS += $(WEAK_OPTIM) -pg -ftest-coverage -fprofile-arcs
endif

#platform-dependent flags
ifeq ($(PLATFORM), Linux)
	CPPFLAGS += -pthread
	LDFLAGS += -pthread
	GL = $(GL_LINUX)
endif
ifeq ($(PLATFORM), Darwin)
	CPPFLAGS += -DMAC_HACKS
	GL = $(GL_MAC)
endif

#----( build rules )----------------------------------------------------------

.SUFFIXES:

%.o: %.C
	@echo compiling $@
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
%_test:
	@echo linking $@
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

#requires_rtti_and_exceptions.o: requires_rtti_and_exceptions.C
#	@echo compiling $<
#	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -frtti -fexceptions -c $< -o $@

#----( modules )--------------------------------------------------------------

EXPR_H = expressions.h substitute.h handling.h symbols.h large_set.h definitions.h
EXPR_O = expressions.o substitute.o handling.o large_set.o

handling_test: handling_test.o handling.o definitions.o
node_heap_test: node_heap_test.o node_heap.o aligned_alloc.o definitions.o
random_choice_test: random_choice_test.o node_heap.o aligned_alloc.o definitions.o

# the tangled mess begins

NODES_O = nodes.o reorder.o obs.o order.o apply.o compose.o join.o node_heap.o aligned_alloc.o

BRAIN_O = $(NODES_O) brain.o lambda_theories.o axiom_tools.o axiom_enforcement.o axiom_validation.o $(JDB_O) $(EXPR_O) syntax_semantics.o combinatory_structure.o measures.o languages.o fuzzy.o complexity.o linalg.o random_choice.o thought.o context.o statements.o optimization.o files.o visual.o definitions.o
SOCK_O = socket_tools.o thread_tools.o

#console
CONSOLE_H = console.h kernel.h context.h $(EXPR_H) statements.h symbols.h version.h definitions.h
console.tab.c: console.y
	@echo building parser $@
	@bison -d -v console.y
console.y.c: console.l
	@echo building lexer $@
	@flex -oconsole.y.c console.l
	@#flex -d -v -oconsole.y.c console.l
#note: the dependence is really on console.tab.h
console.y.o: console.y.c console.tab.c $(CONSOLE_H)
	@echo compiling $@
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -Wno-old-style-cast console.y.c -c
console.tab.o: console.tab.c $(CONSOLE_H)
	@echo compiling $@
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -Wno-old-style-cast console.tab.c -c
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
	@echo building parser $@
	@bison -d -v parser.y
	@#bison -d -v -t parser.y
parser.y.c: parser.l
	@echo building lexer $@
	@flex -oparser.y.c parser.l
	@#flex -oparser.y.c -d -v -T parser.l
parser.y.o: parser.y.c parser.tab.c $(PARSER_H)
	@echo compiling $@
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -Wno-old-style-cast parser.y.c -c
parser.tab.o: parser.tab.c $(PARSER_H)
	@echo compiling $@
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -Wno-old-style-cast parser.tab.c -c
parser.o: parser.C $(PARSER_H)
PARSER_O = parser.tab.o parser.y.o parser.o

#----( main targets )---------------------------------------------------------

kernel: kernel.o $(BRAIN_O) $(CONSOLE_O) $(PARSER_O) $(SOCK_O)
	@echo linking $@
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -lfl -lreadline -lhistory -lcurses -llapack -lblas -lpng -lbz2 -o $@

server: server.o market.o simple.o $(PARSER_O) $(SOCK_O) $(EXPR_O) definitions.o
	@echo linking $@
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -lpthread -lbz2 -o $@

toy: toy.o dynamics.o animator.o meas_lite.o files.o definitions.o
	@echo linking $@
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(GL) -o $@

#----( testing )--------------------------------------------------------------

priority_queue_test: priority_queue_test.o definitions.o

fifo_queue_test: fifo_queue_test.o definitions.o

visual_test: visual_test.o visual.o definitions.o
	@echo linking $@
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -lpng -o visual_test

linalg_test: linalg_test.o linalg.o definitions.o
	@echo linking $@
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -llapack -lblas -o linalg_test

moments_test: moments_test.o definitions.o

meas_lite_test: meas_lite_test.o meas_lite.o files.o definitions.o

optimization_test: optimization_test.o optimization.o linalg.o definitions.o
	@echo linking $@
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -llapack -lblas -o optimization_test

expressions_test: expressions_test.o $(EXPR_O) $(PARSER_O) definitions.o

unification_test: unification_test.o $(EXPR_O) $(PARSER_O) unification.o definitions.o

TESTS = priority_queue_test visual_test linalg_test handling_test moments_test optimization_test expressions_test meas_lite_test unification_test
#node_heap_test random_choice_test

# TODO automate this
test: $(TESTS)
	echo "" > log/test.log
	./run_tests

# Individual tests
v_test: visual_test
	./visual_test
la_test: linalg_test
	./linalg_test
op_test: optimization_test
	./optimization_test
e_test: expressions_test
	./expressions_test
rc_test: random_choice_test
	./random_choice_test
u_test: unification_test
	./unification_test
ml_test: meas_lite_test
	./meas_lite_test

#----( utilities )------------------------------------------------------------

todo:
	@echo '\033[1m-------- Top 40 of' \
		"$$(cat *.h *.C *.py | grep -c TODO)" \
		'tasks --------\033[0m'
	@ls -t \
	| grep '.\(h\|C\|py\)$$' \
	| xargs grep 'TODO' --color=yes 2>/dev/null\
	| head -n 40

arch: FORCE arch.C
	@$(CXX) $(CXXFLAGS) arch.C -o $@
	@echo 'The following are equivalent:'
	@echo ' gcc -march=native'
	@gcc -march=native -E -v - </dev/null 2>&1 | sed -n 's/.* -quiet -v / gcc /p'

tags: FORCE
	ctags-exuberant *.[lyCh]	

lclean: FORCE
	rm -f a.out kernel server toy *_test arch

oclean: FORCE
	rm -f *.yy.c *.y.c *.tab.h *.tab.c *.tab.cc *.cc *.hh *.output
	rm -f *.o *.rpo

clean: lclean oclean FORCE
	rm -f core core.* *.stackdump *.log temp.* test.*
	rm -f *.gcov gmon.out *.da *.bb *.bbg *.prof cachegrind.out.* *.gcda *.gcno
	rm -f comb_struct.table
	rm -f tags TAGS

depend:
	@makedepend -D__GNUG__ -Y../include *.C -fC.depend 2> makedepend.log
	@! grep -i error makedepend.log

include C.depend

FORCE:

