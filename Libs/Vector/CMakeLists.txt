set(SOURCES
    "Vector.c")

add_library(Vector ${SOURCES})

target_include_directories(Vector PUBLIC ${LIBS_ROOT}/Error)
target_link_libraries(Vector Error)
