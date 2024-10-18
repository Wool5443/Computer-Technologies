set(SOURCES
    "ScratchBuf.c")

add_library(ScratchBuf ${SOURCES})

target_include_directories(ScratchBuf PUBLIC ${PROJECT_ROOT}/Error)
