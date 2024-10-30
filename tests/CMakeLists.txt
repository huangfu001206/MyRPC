# tests/CMakeLists.txt
include(CTest)

# 查找 Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# 添加测试源文件
add_executable(ZkTest ZkTest.cpp)

# 链接 Google Test 和 pthread
target_link_libraries(ZkTest ${GTEST_LIBRARIES} pthread zookeeper_mt)

# 添加测试
add_test(NAME ZkTest COMMAND MyTests)