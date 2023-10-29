#pragma once
#include <iostream>
#include <string>
#include "ConfigFileUtils.h"
class MYRPCapplication {
public:
    //初始化
    MYRPCapplication& init(int argc, char** argv);
    //获取单例对象
    static MYRPCapplication& getInstance();
    //获取文件配置信息
    std::unordered_map<std::string, std::string> getFileInfo();
private:
    ConfigFileUtils fileUtils;
    //存储文件配置信息
    std::unordered_map<std::string, std::string> fileInfo;
};