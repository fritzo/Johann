
#include_directories(${CMAKE_SOURCE_DIR}/)

add_library(pomagma SHARED
	util.cpp
	aligned_alloc.cpp
	dense_set.cpp
	dense_bin_rel.cpp
	dense_bin_fun.cpp
	dense_sym_fun.cpp
	sparse_bin_fun.cpp
	)
install(TARGETS pomagma LIBRARY DESTINATION /usr/lib)


# TODO use google test as in
# http://code.google.com/p/googletest/wiki/Primer
# http://stackoverflow.com/questions/5900447
#add_executable(run_test
#	dense_sym_fun_test.cpp
#	binary_relation_test.cpp
#	)
#target_link_libraries(run_test
#	pomagma
#	gtest
#	gtest_main)
#add_test(NAME run_test COMMAND run_test)

add_executable(binary_relation_test binary_relation_test.cpp)
target_link_libraries(binary_relation_test pomagma)
add_test(NAME binary_relation COMMAND binary_relation_test)

add_executable(dense_sym_fun_test dense_sym_fun_test.cpp)
target_link_libraries(dense_sym_fun_test pomagma)
add_test(NAME dense_sym_fun COMMAND dense_sym_fun_test)

#install(TARGETS pomagma LIBRARY DESTINATION ${LIBRARY_OUTPUT_PATH})

