
#include_directories(${CMAKE_SOURCE_DIR}/)

add_library(pomagma SHARED
	util.cpp
	aligned_alloc.cpp
	dense_set.cpp
	carrier.cpp
	base_bin_rel.cpp
	dense_bin_rel.cpp
	dense_bin_fun.cpp
	dense_sym_fun.cpp
	inverse_bin_fun.cpp
	)
install(TARGETS pomagma LIBRARY DESTINATION ${PROJECT_SOURCE_DIR}/../lib)

add_executable(dense_set_test dense_set_test.cpp)
target_link_libraries(dense_set_test pomagma tbb)
add_test(NAME dense_set COMMAND dense_set_test)

add_executable(carrier_test carrier_test.cpp)
target_link_libraries(carrier_test pomagma tbb)
add_test(NAME carrier COMMAND carrier_test)

add_executable(dense_bin_rel_test dense_bin_rel_test.cpp)
target_link_libraries(dense_bin_rel_test pomagma tbb)
add_test(NAME dense_bin_rel COMMAND dense_bin_rel_test)

add_executable(dense_bin_fun_test dense_bin_fun_test.cpp)
target_link_libraries(dense_bin_fun_test pomagma tbb)
add_test(NAME dense_bin_fun COMMAND dense_bin_fun_test)

add_executable(dense_sym_fun_test dense_sym_fun_test.cpp)
target_link_libraries(dense_sym_fun_test pomagma tbb)
add_test(NAME dense_sym_fun COMMAND dense_sym_fun_test)

add_executable(inverse_bin_fun_test inverse_bin_fun_test.cpp)
target_link_libraries(inverse_bin_fun_test pomagma tbb)
add_test(NAME inverse_bin_fun COMMAND inverse_bin_fun_test)

#install(TARGETS pomagma LIBRARY DESTINATION ${LIBRARY_OUTPUT_PATH})

