cmake_minimum_required(VERSION 2.6)
project(pomagma)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Woverloaded-virtual -Wsign-promo -Wundef -Wpointer-arith -Wcast-qual -Wcast-align -Wno-deprecated")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -funswitch-loops -fomit-frame-pointer -pipe")

add_definitions(-DPOMAGMA_DEBUG_LEVEL=0)
#add_definitions(-DNDEBUG)

enable_testing()

install(DIRECTORY src/
	DESTINATION ${PROJECT_SOURCE_DIR}/../include/pomagma
	FILES_MATCHING PATTERN *.hpp)

add_subdirectory(src)
