set(main1 sem7.1)
set(main2 sem7.2)
set(main3 writer)
set(main4 reader)

add_executable(${main1} src/09-1a.c)
add_executable(${main2} src/09-1b.c)
add_executable(${main3} src/writer.c)
add_executable(${main4} src/reader.c)

target_link_libraries(${main1} pthread m)
target_link_libraries(${main2} pthread m)
target_link_libraries(${main3} pthread m)
target_link_libraries(${main4} pthread m)
