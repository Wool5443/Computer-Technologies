set(EXE restaurant)

set(SOURCES
    "src/main.c"
    "src/Restaurant.c"
)

add_executable(${EXE} ${SOURCES})
target_include_directories(${EXE} PRIVATE include)
target_link_libraries(${EXE} PRIVATE pthread m cmlibScratchBuffer cmlibVector cmlibLogger cmlibString)
