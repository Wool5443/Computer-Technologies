cmake_minimum_required(VERSION 3.5)
set(projectName "CT")

project(${projectName} C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED true)

set(CMAKE_C_FLAGS_DEBUG "\
-pthread -g -D _DEBUG -ggdb3 -std=gnu17 -O0 -Wall -Wextra \
-fsanitize=address,undefined,leak,shift,float-divide-by-zero,signed-integer-overflow\
")
set(CMAKE_CXX_FLAGS_RELASE "-pthread -O3 -g -DNDEBUG -march=native -mavx512f")

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Task1)
# add_subdirectory(Sem5)
# add_subdirectory(Sem7)
