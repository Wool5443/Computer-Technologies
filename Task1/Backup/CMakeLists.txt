set(EXE backup)

set(SOURCES
    "src/main.c"
    "src/Backup.c"
    "src/FileList.c"
)

add_executable(${EXE} ${SOURCES})
target_include_directories(${EXE}
    PRIVATE include
    PRIVATE ${LIBS_ROOT}/Error
    PRIVATE ${LIBS_ROOT}/ScratchBuffer
    PRIVATE ${LIBS_ROOT}/Vector)
target_link_libraries(${EXE} pthread m Error ScratchBuf)
