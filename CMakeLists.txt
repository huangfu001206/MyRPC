cmake_minimum_required(VERSION 3.16)
PROJECT(MYRPC)
set(CMAKE_CXX_STANDARD 17)

#生成debug版本以支持gdb调试
set(CMAKE_BUILD_TYPE "Debug")

#设置静/动态库输出路径 以及 可执行文件输出路径 
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#设置头文件搜索路径 以及 动态库链接路径
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example/include)
link_directories(${PROJECT_SOURCE_DIR}/lib)

add_subdirectory(src)
add_subdirectory(example)

# 添加测试目录
enable_testing()
add_subdirectory(tests)

