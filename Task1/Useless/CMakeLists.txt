set(EXE useless)

set(SOURCES
    "src/Error.c"
    "src/InputFiles.c"
    "src/main.c"
    "src/Scheduler.c"
)

add_executable(${EXE} ${SOURCES})
target_include_directories(useless PRIVATE include)
target_link_libraries(${EXE} pthread m)
