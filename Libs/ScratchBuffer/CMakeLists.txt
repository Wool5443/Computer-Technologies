set(SOURCES
    "ScratchBuf.c")

add_library(ScratchBuf ${SOURCES})

target_include_directories(ScratchBuf PUBLIC ${LIBS_ROOT}/Error)
target_link_libraries(ScratchBuf Error)
