#include <iostream>
#include <string>
#include "client.pb.h"
#include "MyRpcApplication.h"
#include "MyRpcProvider.h"
#include "clientService.h"

int main(int argc, char** argv) {
    //读取配置信息
    auto fileInfo = MyRpcApplication::getInstance().init(argc, argv).getFileInfo();
    for(auto p : fileInfo) {
        std::cout<<p.first << " : " << p.second<<std::endl;
    }

    MyRpcProvider provider;
    //服务注册
    provider.NotifyService(new clientService());
    provider.Start();
    return 0;
}