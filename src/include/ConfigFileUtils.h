#pragma once
#include <string>
#include <iostream>
#include <unordered_map>

class ConfigFileUtils {
public:
    //加载文件
    ConfigFileUtils& load(const std::string& filePath, std::unordered_map<std::string, std::string>& fileConfig);
    //删除空白符
    void deleteAllEmptyChar(std::string&);
    //解析每一行的数据
    std::pair<std::string, std::string> parseLine(const std::string &line);
};