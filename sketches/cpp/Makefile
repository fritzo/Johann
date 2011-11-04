
current_target: initializer_list

all:

CXX = g++

TEMPS = 

#----( template users )--------------------------------------------------------

TEMPS += template_static_data_main
template_static_data_main: template_static_data_main.o template_static_data.o
	$(CXX) -Wall template_static_data.o template_static_data_main.o -o template_static_data_main

#----( template users )--------------------------------------------------------

TEMPS += template_usr
template_usr: template_usr.o template_lib.o
	g++ -Wall template_usr.o template_lib.o -o template_usr

#----( including template methods )--------------------------------------------

TEMPS += template_method_include_main
template_method_include_main: template_method_include_main.o template_method_include_lib.o
	g++ template_method_include_main.o template_method_include_lib.o -o template_method_include_main

#----( readline stuff )--------------------------------------------------------

#note the linking req's of gnu readline
TEMPS += rl_console
rl_console: rl_console.C
	g++ rl_console.C -o rl_console -lreadline -lhistory -lcurses

#----( posix threads )---------------------------------------------------------

TEMPS += posix_threads
posix_threads: posix_threads.C
	g++ -pthread posix_threads.C -o posix_threads
	#-fstack-check seems to seg fault

#----( random graph )----------------------------------------------------------

#various levels of optimization
OPTS1 =
 #45.23 sec on old box
OPTS2 = -O3
 #37.88 sec on old box
OPTS3 = -O3 -floop-optimize -fexpensive-optimizations -funroll-loops
 #35.35 sec on old box
OPTS4 = -O3 -floop-optimize -fexpensive-optimizations -funroll-loops -ffast-math -funsafe-math-optimizations -ffinite-math-only
 #35.49 sec on old box
OPTS5 = -O3 -floop-optimize -fexpensive-optimizations -funroll-loops -march=pentium3 -mfpmath=sse,387 -msse -mmmx
 #36.36 sec on old box
OPTS6 = -O3 -floop-optimize -fexpensive-optimizations -funroll-loops -march=pentium3
 #35.29 sec on old box
OPTS7 = -O3 -floop-optimize -fexpensive-optimizations -funroll-loops -march=pentium4
test: random_graph
	./random_graph
	rm random_graph

TEMPS += random_graph
random_graph : random_graph.C
	g++ $(OPTS7) -o random_graph random_graph.C

#----( debugging features )----------------------------------------------------

TEMPS += dbf_debug
debug:  debug_features.C
	g++ -DDEBUG_LEVEL=2 -rdynamic debug_features.C -o dbf_debug
	./dbf_debug
	rm ./dbf_debug

TEMPS += dbf_fast
fast:  debug_features.C
	g++ -fomit-frame-pointer debug_features.C -o dbf_fast
	./dbf_fast
	rm ./dbf_fast

#----( ctors & dtors )---------------------------------------------------------

TEMPS += copying
copying: copying.C
	g++ copying.C -o copying
	./copying

TEMPS += virtual_destructors
virtual_destructors: virtual_destructors.C
	g++ virtual_destructors.C -o virtual_destructors
	./virtual_destructors

#----( misc )------------------------------------------------------------------

TEMPS += zip_file
zip_file: zip_file.C
	g++ zip_file.C -lbz2 -ggdb -o zip_file
	./zip_file

TEMPS += splay_trees
splay_trees: splay_trees.C
	g++ splay_trees.C -o splay_trees

TEMPS += lapack_test
lapack_test: lapack_test.C
	g++ lapack_test.C -L/usr/local/lib -llapack -lblas -o lapack_test

TEMPS += date
date: date.C
	g++ date.C -o date
	./date

TEMPS += log_floats
log_floats: log_floats.C FORCE
	g++ -O3 -ffast-math log_floats.C -o log_floats
	time ./log_floats 100000000

TEMPS += virtual_overhead
virtual_overhead: virtual_overhead.C FORCE
	g++ virtual_overhead.C -o virtual_overhead
	time ./virtual_overhead 100000000
	time ./virtual_overhead -100000000

TEMPS += method_callback
method_callback: method_callback.C FORCE
	g++ method_callback.C -o method_callback
	./method_callback

TEMPS += argument_scope
argument_scope: argument_scope.C FORCE
	g++ argument_scope.C -o argument_scope
	./argument_scope

TEMPS += initializer_list
initializer_list: initializer_list.C
	g++ -std=c++0x initializer_list.C -o initializer_list

#----( general )---------------------------------------------------------------

FORCE:

clean:
	rm -f a.out *.o $(TEMPS) *.exe core
	$(MAKE) -C sockets clean
	$(MAKE) -C cgi clean

depend:
	makedepend -Y *.[Ch] 2> /dev/null

# DO NOT DELETE

template_lib.o: template_lib.h
template_method_include_lib.o: template_method_include_lib.h
template_method_include_main.o: template_method_include_lib.h
template_static_data.o: template_static_data.h
template_static_data_main.o: template_static_data.h
template_usr.o: template_lib.h
