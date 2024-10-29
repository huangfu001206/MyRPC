file(GLOB source_file *.cpp *.cc)
message("source_file : " ${source_file})

add_executable(Server ${source_file})
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIR})
target_link_libraries(Server ${PROTOBUF_LIBRARY} myrpc)