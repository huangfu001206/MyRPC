file(GLOB source_file *.cpp *.cc)
message("source_file : " ${source_file})

add_executable(Client ${source_file})

target_link_libraries(Client protobuf myrpc)