set(EXE backup)

set(SOURCES
    "src/main.c"
    "src/Backup.c"
)

add_executable(${EXE} ${SOURCES})
target_include_directories(${EXE} PRIVATE include PRIVATE ${PROJECT_ROOT}/Error PRIVATE ${PROJECT_ROOT}/ScratchBuffer)
target_link_libraries(${EXE} pthread m Error)
