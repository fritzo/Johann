#makefile for Johann's mapper

####### switch this to whatever is appropriate for your platform ##############

#COMPILE_TYPE = mac
#COMPILE_TYPE = mac_debug
#COMPILE_TYPE = cygwin
#COMPILE_TYPE = linux
#COMPILE_TYPE = debug
#COMPILE_TYPE = profile
COMPILE_TYPE = devel

#### if you have libpng installed, uncomment this:

HAVE_PNG = true

######## leave everything else the same #######################################

CC = g++

#OPT = -O3 -funroll-loops -pipe
OPT = -O3 -ffast-math -fomit-frame-pointer -funroll-loops -pipe
#OPT = -O3 -m32 -march=prescott -malign-double -mfpmath=sse -ffast-math -fomit-frame-pointer -funroll-loops -fprefetch-loop-arrays -ftree-vectorize -fno-exceptions -fno-check-new -pipe
WARNINGS = -Wall

#opengl stuff
GL_LINUX = -lglut -lGLU -lGL
GL_MAC = -framework OpenGL -framework Glut
GL_CYGWIN = -lglut32 -lglu32 -lopengl32

#png stuff
ifdef HAVE_PNG
	PNG_LINUX = -lpng
	PNG_MAC = -L/sw/lib -lpng -lz
	PNG_CYGWIN = -L/usr/local/lib -lpng -lz
else
	PNG_LINUX =
	PNG_MAC =
	PNG_CYGWIN =
endif

#compiler flags
ifeq ($(COMPILE_TYPE), mac)
	CPPFLAGS = -I/sw/include -DDEBUG_LEVEL=0 -DMAC_HACKS
	CXXFLAGS = $(OPT) -I/sw/include
	LDFLAGS  = $(OPT) -L/sw/libs
	LIBS = $(GL_MAC) $(PNG_MAC)
endif
ifeq ($(COMPILE_TYPE), mac_debug)
	CPPFLAGS = -I/sw/include -DDEBUG_LEVEL=2 -DMAC_HACKS
	CXXFLAGS = -I/sw/include -ggdb
	LDFLAGS  = -L/sw/lib -rdynamic -ggdb
	LIBS = $(GL_MAC) $(PNG_MAC)
endif
ifeq ($(COMPILE_TYPE), cygwin)
	CPPFLAGS = -I/usr/local/include -DDEBUG_LEVEL=0 -DCYGWIN_HACKS
	CXXFLAGS = -I/usr/local/include -mno-cygwin -mwin32
	LDFLAGS  = -L/usr/local/lib -mno-cygwin -mwin32
	LIBS = $(GL_CYGWIN) $(PNG_CYGWIN)
endif
ifeq ($(COMPILE_TYPE), linux)
	CPPFLAGS = -DDEBUG_LEVEL=0
	CXXFLAGS = $(OPT)
	LDFLAGS  = $(OPT)
	LIBS = $(GL_LINUX) $(PNG_LINUX)
endif
ifeq ($(COMPILE_TYPE), debug)
	CPPFLAGS = -DDEBUG_LEVEL=2
	CXXFLAGS = $(WARNINGS) -ggdb
	LDFLAGS  = -rdynamic -ggdb
	LIBS = $(GL_LINUX) $(PNG_LINUX)
endif
ifeq ($(COMPILE_TYPE), profile)
	CPPFLAGS = -DDEBUG_LEVEL=0
	CXXFLAGS = -O2 -pg -ftest-coverage -fprofile-arcs
	LDFLAGS  = -O2 -pg -ftest-coverage -fprofile-arcs
	LIBS = $(GL_LINUX) $(PNG_LINUX)
endif
ifeq ($(COMPILE_TYPE), devel)
	CPPFLAGS = -DDEBUG_LEVEL=0
	CXXFLAGS = $(OPT)
	LDFLAGS  = $(OPT)
	LIBS = $(GL_LINUX) $(PNG_LINUX)
endif

#default target
current_target: view
all: jmapper
view: jmapper
	./jmapper

#final product
MAPPER_O = mapper.o
jmapper: $(MAPPER_O)
	$(CC) $(CXXFLAGS) -o jmapper $(MAPPER_O) $(LIBS)

clean:
	rm -f core core.* *.o jmapper temp.* *.jmap
