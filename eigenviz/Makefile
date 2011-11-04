# makefile for eigenvalue mapper

####### switch this to whatever is appropriate for your platform ##############

#COMPILE_TYPE = mac
#COMPILE_TYPE = mac_debug
#COMPILE_TYPE = cygwin #XXX this does not seem to work
#COMPILE_TYPE = linux
#COMPILE_TYPE = debug
#COMPILE_TYPE = profile
COMPILE_TYPE = devel

######## leave everything else the same #######################################

CC = g++

OPT = -O3 -ffast-math -funroll-loops -pipe
WARNINGS = -Wall

# opengl stuff
GL_LINUX = -lglut -lGLU -lGL
GL_MAC = -framework OpenGL -framework Glut
GL_CYGWIN = -lglut32 -lglu32 -lopengl32

# arpack
MLIBS = -mt -lblas -llapack -larpack

# compiler flags
ifeq ($(COMPILE_TYPE), mac)
	CPPFLAGS = -I/sw/include -DMAC_HACKS
	CXXFLAGS = $(OPT) -I/sw/include
	LDFLAGS  = $(OPT) -L/sw/libs
	VLIBS = $(GL_MAC)
endif
ifeq ($(COMPILE_TYPE), mac_debug)
	CPPFLAGS = -I/sw/include -DMAC_HACKS
	CXXFLAGS = -I/sw/include -ggdb
	LDFLAGS  = -L/sw/lib -rdynamic -ggdb
	VLIBS = $(GL_MAC)
endif
ifeq ($(COMPILE_TYPE), cygwin)
	CPPFLAGS = -I/usr/local/include -DCYGWIN_HACKS
	CXXFLAGS = -I/usr/local/include -mno-cygwin -mwin32
	LDFLAGS  = -L/usr/local/lib -mno-cygwin -mwin32
	VLIBS = $(GL_CYGWIN)
endif
ifeq ($(COMPILE_TYPE), linux)
	CPPFLAGS =
	CXXFLAGS = $(OPT)
	LDFLAGS  = $(OPT)
	VLIBS = $(GL_LINUX)
endif
ifeq ($(COMPILE_TYPE), debug)
	CPPFLAGS =
	CXXFLAGS = $(WARNINGS) -ggdb
	LDFLAGS  = -rdynamic -ggdb
	VLIBS = $(GL_LINUX)
endif
ifeq ($(COMPILE_TYPE), profile)
	CPPFLAGS =
	CXXFLAGS = -O2 -pg -ftest-coverage -fprofile-arcs
	LDFLAGS  = -O2 -pg -ftest-coverage -fprofile-arcs
	VLIBS = $(GL_LINUX)
endif
ifeq ($(COMPILE_TYPE), devel)
	CPPFLAGS =
	CXXFLAGS = $(OPT)
	LDFLAGS  = $(OPT)
	VLIBS = $(GL_LINUX)
endif

#----( scripting targets )-----------------------------------------------------

current_target: view

all: view_map make_map test.graph test.map

test.graph: test_map.py
	python test_map.py

test.map: test.graph make_map
	./make_map test.graph test.map

view: test.map view_map
	./view_map test.map

#----( c++ targets )-----------------------------------------------------------

MAKE_MAP_O = make_map.o graph.o eigs.o
make_map: $(MAKE_MAP_O)
	$(CC) $(CXXFLAGS) -o make_map $(MAKE_MAP_O) $(MLIBS)

VIEW_MAP_O = view_map.o graph.o
view_map: $(VIEW_MAP_O)
	$(CC) $(CXXFLAGS) -o view_map $(VIEW_MAP_O) $(VLIBS)

#----( utilities )-------------------------------------------------------------

todo:
	grep '(T[0-9]\+)' *.h *.C *.py --color=auto || echo "Nothing to do!"
clean:
	rm -f core core.* *.stackdump
	rm -rf	*.o make_map view_map *.exe
	rm -rf temp.* *.map *.graph
	
depend:
	makedepend -D__GNUG__ -Y -- $(CPPFLAGS) $(CXXFLAGS) -- *.[Ch] &> /dev/null

# DO NOT DELETE THIS LINE -- make depend depends on it.

eigs.o: eigs.h graph.h
eigs.o: graph.h
graph.o: graph.h
make_map.o: graph.h eigs.h
view_map.o: graph.h
