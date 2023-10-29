#include "ConfigFileUtils.h"
#include <fstream>
ConfigFileUtils& ConfigFileUtils::load(const std::string& filePath, std::unordered_map<std::string, std::string>& fileConfig) {
    std::ifstream file(filePath);
    if(file.is_open()) {
        std::string line;
        while(std::getline(file, line)) {
            deleteAllEmptyChar(line);
            auto parseRes = parseLine(line);
            fileConfig.insert(parseRes);
        }
        file.close();
    } else {
        std::cerr << "Failed to open file." << std::endl;
        return *this;
    }
}

void ConfigFileUtils::deleteAllEmptyChar(std::string& line) {
    auto it = line.begin();
    while(it != line.end()) {
        if(*it == ' ') {
            it = line.erase(it);
        } else {
            it++;
        }
    }
}

std::pair<std::string, std::string> ConfigFileUtils::parseLine(const std::string &line) {
    size_t index = line.find_first_of(':');
    if(index == std::string::npos) {
        std::cout<<"Error: Invalid line format --- <key>:<value>"<<std::endl;
        exit(EXIT_FAILURE);
    }
    std::string key = line.substr(0,index);
    std::string value = line.substr(index + 1);
    return {key, value};
}