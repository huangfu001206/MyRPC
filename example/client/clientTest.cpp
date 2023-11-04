#pragma once
#include <iostream>
#include <string>
#include "client.pb.h"
#include "MyRpcApplication.h"
#include "MyRpcProvider.h"
#include "clientService.h"

int main(int argc, char** argv) {
    int header_size = 100000;
    std::string result = std::string((char*)&header_size, 4);
    std::cout<<"result : "<<result<<std::endl;

    auto fileInfo = MyRpcApplication::getInstance().init(argc, argv).getFileInfo();
    for(auto p : fileInfo) {
        std::cout<<p.first << " : " << p.second<<std::endl;
    }
    RpcProvider provider;
    provider.NotifyService(new clientService());
    provider.Start();

    
    // ClientPro::LoginRequest request;
    // request.set_username("huangfu");
    // request.set_password("xxxxxxxx");
    // std::string serial_result = request.SerializeAsString();
    // std::cout<<"序列化结果为： "<<serial_result<<std::endl; 
    return 0;
}