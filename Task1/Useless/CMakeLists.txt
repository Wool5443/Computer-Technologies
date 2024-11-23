set(EXE useless)

set(SOURCES
    "src/InputFiles.c"
    "src/main.c"
    "src/Scheduler.c"
)

add_executable(${EXE} ${SOURCES})
target_include_directories(${EXE} PRIVATE include)
target_link_libraries(${EXE} PRIVATE pthread m cmlibError)
