#pragma once
#include <iostream>
#include <string>
#include "client.pb.h"
#include "MYRPCapplication.h"

int main(int argc, char** argv) {
    auto fileInfo = MYRPCapplication::getInstance().init(argc, argv).getFileInfo();
    for(auto p : fileInfo) {
        std::cout<<p.first << " : " << p.second<<std::endl;
    }
    // ClientPro::LoginRequest request;
    // request.set_username("huangfu");
    // request.set_password("xxxxxxxx");
    // std::string serial_result = request.SerializeAsString();
    // std::cout<<"序列化结果为： "<<serial_result<<std::endl; 
    return 0;
}