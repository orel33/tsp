cmake_minimum_required(VERSION 3.0)

project("tsp" C)

include(CTest)
enable_testing()

set(CMAKE_C_FLAGS "-std=c99 -Wall")

### library tsp
add_library(tsp tsp.c)

### solver
add_executable(solve solve.c)
target_link_libraries(solve tsp)

### checksol
add_executable(checksol checksol.c)
target_link_libraries(checksol tsp)

### random
add_executable(random random.c)
target_link_libraries(random tsp)


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/data/)

add_test(test1 ./checksol data/test1.txt 22)
add_test(test2 ./checksol data/test2.txt 10)
