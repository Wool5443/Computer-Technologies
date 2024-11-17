set(EXE backup)

set(SOURCES
    "src/main.c"
    "src/Backup.c"
    "src/FileList.c"
)

add_executable(${EXE} ${SOURCES})
target_include_directories(${EXE}
    PRIVATE include)
target_link_libraries(${EXE} PRIVATE pthread m ScratchBuf Vector)
