#include "MyRpcProvider.h"
#include <functional>
void RpcProvider::NotifyService(google::protobuf::Service* service) {
    //获取服务指针 服务名称
    const google::protobuf::ServiceDescriptor *serviceDesp = service->GetDescriptor();
    std::string service_name = serviceDesp->name();

    //获取方法数量，并记录方法描述符指针
    int methodCnt = serviceDesp->method_count();
    std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> methodMap;
    for(int i = 0; i < methodCnt; i++) {
        std::string method_name = serviceDesp->method(i)->name();
        methodMap.insert({method_name, serviceDesp->method(i)});
    }
    //记录 服务名称 和 方法 的映射关系
    serviceMaps[service_name] = methodMap;
}
//连接回调函数
void RpcProvider::ConnectionHandler(const muduo::net::TcpConnectionPtr &conn) {
    if(!conn->connected()) {
        conn->shutdown();
    }

}

//接收到请求信息的回调函数
void RpcProvider::MessageHandler(const muduo::net::TcpConnectionPtr &conn,
                        muduo::net::Buffer* buffer,
                        muduo::Timestamp) {
    std::string recv_msg = buffer->retrieveAllAsString();
    std::cout<<"received message : "<<recv_msg<<std::endl;
                        }



void RpcProvider::Start() {
    //获取配置文件信息
    auto fileInfo = MyRpcApplication::getInstance().getFileInfo();
    std::string server_ip = fileInfo["server_ip"];
    uint16_t server_port = std::atoi(fileInfo["server_port"].c_str());
    muduo::net::InetAddress address(server_ip, server_port);

    //建立TCPServer
    muduo::net::TcpServer server(&eventLoop, address, "RpcProvider");

    //设置连接回调
    server.setConnectionCallback(std::bind(&RpcProvider::ConnectionHandler, this, std::placeholders::_1));
    //设置接收到信息回调
    server.setMessageCallback(std::bind(&RpcProvider::MessageHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    server.setThreadNum(5);

    std::cout<<"Server [ip = "<<server_ip <<", port = " <<server_port<<"] Running ……"<<std::endl;
    server.start();
    eventLoop.loop();
}
