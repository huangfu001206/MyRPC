file(GLOB source_file *.cpp *.cc)
# 查找Boost库
find_package(Boost REQUIRED COMPONENTS program_options)

# 编译为动态库
add_library(myrpc SHARED ${source_file})

# 链接Boost库
target_link_libraries(myrpc PRIVATE Boost::program_options)