#include "MyRpcProvider.h"
#include "MyRpcHeader.pb.h"
#include <functional>
void RpcProvider::NotifyService(const google::protobuf::Service* service) {
    //获取服务指针 服务名称
    const google::protobuf::ServiceDescriptor *serviceDesp = service->GetDescriptor();
    std::string service_name = serviceDesp->name();

    //获取方法数量，并记录方法描述符指针
    int methodCnt = serviceDesp->method_count();
    ServiceAndMethodDesc desc;
    desc.service = service;
    std::unordered_map<std::string, ServiceAndMethodDesc> methodMap;
    for(int i = 0; i < methodCnt; i++) {
        std::string method_name = serviceDesp->method(i)->name();
        desc.method_desc = serviceDesp->method(i);
        methodMap.insert({method_name, desc});
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
    int header_size = std::stoi(recv_msg.substr(0, 4));
    std::string rpchead_str = recv_msg.substr(4, header_size);
    MyRpc::RpcHeader header;
    if(!header.ParseFromArray(rpchead_str.c_str(), rpchead_str.size())) {
        std::cout<<"rpcheader 解析失败"<<std::endl;
        return;
    }
    std::string service_name = header.service_name();
    std::string method_name = header.method_name();
    uint32_t args_size = header.args_size();
    std::string args_str = recv_msg.substr(4 + header_size, args_size);

    //通过service_name 以及 method_name 找到对应的method 描述符
    if(serviceMaps.find(service_name) == serviceMaps.end()) {
        std::cout<<"请求所对应的服务不存在服务注册表中"<<std::endl;
        return;
    }
    auto methodList = serviceMaps[service_name];
    if(methodList.find(method_name) == methodList.end()) {
        std::cout<<"服务所对应的方法不存在于方法表中"<<std::endl;
        return;
    }
    ServiceAndMethodDesc sm_desc = methodList[method_name];
    const google::protobuf::Service* service= sm_desc.service;
    const google::protobuf::MethodDescriptor*  method_desc = sm_desc.method_desc;
    google::protobuf::Message* request = service->GetRequestPrototype(method_desc).New();
    if(!request->ParseFromArray(args_str.c_str(), args_str.size())) {
        std::cout<<"请求参数解析失败"<<std::endl;
        return;
    }
    google::protobuf::Message* response = service->GetResponsePrototype(method_desc).New();
    // google::protobuf::Closure* done = google::protobuf::NewCallback(this, &)
    
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
