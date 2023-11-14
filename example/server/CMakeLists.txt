file(GLOB source_file *.cpp *.cc)
message("source_file : " ${source_file})

add_executable(Server ${source_file})

target_link_libraries(Server protobuf myrpc)