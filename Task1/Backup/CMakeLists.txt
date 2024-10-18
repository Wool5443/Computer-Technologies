set(EXE backup)

set(SOURCES
    "src/main.c"
    "src/Backup.c"
    "src/Error.c"
)

add_executable(${EXE} ${SOURCES})
target_include_directories(${EXE} PRIVATE include)
target_link_libraries(${EXE} pthread m)
