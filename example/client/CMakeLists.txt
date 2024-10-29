file(GLOB source_file *.cpp *.cc)
message("source_file : " ${source_file})

add_executable(Client ${source_file})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})
target_link_libraries(Client ${PROTOBUF_LIBRARY} myrpc)