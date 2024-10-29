#include "client.pb.h"
#include "MyRpcApplication.h"
#include "MyRpcChan.h"
#include "MyRpcController.h"

int main(int argc, char** argv) {

    MyRpcApplication::getInstance().init(argc, argv);
    ClientPro::ClientServiceRPC_Stub stub(new MyRpcChan());

    //准备请求信息
    ClientPro::LoginRequest request;
    request.set_username("huangfu");
    request.set_password("123456");

    //接收请求结构体
    ClientPro::LoginResponse response;

    //调用远程Rpc方法
    for(int i = 0; i < 100; i++) {
        MyRpcController controller;
        stub.Login(&controller, &request, &response, nullptr);

        if(controller.Failed()) {
            std::cout<<controller.ErrorText()<<std::endl;
        } else {
            // std::cout<<"++++++++ response info ++++++++"<<std::endl;
            // std::cout<<"success : "<<response.status().success()<<std::endl;
            // std::cout<<"errormsg : "<<response.status().errormsg()<<std::endl;
            // std::cout<<"msg : "<<response.msg()<<std::endl;
            // std::cout<<"+++++++++++++++++++++++++++++++"<<std::endl;
            std::cout << "success [" << i << "]" << std::endl;
        }
    }
    
    // DbLoggerUtils::getInstance().stop();
    return 0;
}