file(GLOB source_file *.cpp *.cc)
# 查找Boost库
find_package(Boost REQUIRED COMPONENTS program_options)
# find_package(muduo REQUIRED)
# 编译为动态库
add_library(myrpc STATIC ${source_file})

# 链接Boost库
target_link_libraries(myrpc PRIVATE Boost::program_options muduo_net muduo_base muduo_http pthread zookeeper_mt)