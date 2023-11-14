#pragma once
#include <iostream>
#include <string>
#include "ConfigFileUtils.h"
class MyRpcApplication {
public:
    //初始化
    MyRpcApplication& init(int argc, char** argv);
    //获取单例对象
    static MyRpcApplication& getInstance();
    //获取文件配置信息
    std::unordered_map<std::string, std::string> getFileInfo();

    MyRpcApplication() = default;

    MyRpcApplication(const MyRpcApplication&) = delete;

    MyRpcApplication& operator=(const MyRpcApplication&) = delete;
private:
    ConfigFileUtils fileUtils;
    //存储文件配置信息
    std::unordered_map<std::string, std::string> fileInfo;
};