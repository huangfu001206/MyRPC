#include "MyRpcProvider.h"
#include "MyRpcHeader.pb.h"
#include <functional>
void MyRpcProvider::NotifyService(google::protobuf::Service* service) {
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
    std::cout<<"------------ Service Register Info --------------"<<std::endl;
    std::cout<<"service_name : "<<service_name<<std::endl;
    int index = 0;
    for(auto mp : methodMap) {
        std::cout<<"method_name"<<index++<<" : "<<mp.first<<std::endl;
    }
    std::cout<<"-------------------------------------------------"<<std::endl;
}
//连接回调函数
void MyRpcProvider::ConnectionHandler(const muduo::net::TcpConnectionPtr &conn) {
    if(!conn->connected()) {
        conn->shutdown();
    }

}

void MyRpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn,
                            google::protobuf::Message *response) {
        std::string response_str;
        if(response->SerializeToString(&response_str)) {
            conn->send(response_str);
            std::cout<<"请求回复成功！"<<std::endl;
        } else {
            std::cout<<"序列化 response 失败!"<<std::endl;
        }
        //每次回复之后，直接主动断开连接
        conn->shutdown();
}

//接收到请求信息的回调函数
void MyRpcProvider::MessageHandler(const muduo::net::TcpConnectionPtr &conn,
                        muduo::net::Buffer* buffer,
                        muduo::Timestamp) {
    //对接收到的请求进行解析
    std::string recv_msg = buffer->retrieveAllAsString();
    std::cout<<"received message : "<<recv_msg<<std::endl;
    int header_size;
    recv_msg.copy((char*)&header_size, 4, 0);
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

    std::cout<<"-------------- MessageHandler Info --------------"<<std::endl;
    std::cout<<"service_name : "<<service_name<<std::endl;
    std::cout<<"method_name : "<<method_name<<std::endl;
    std::cout<<"args_size : "<<args_size<<std::endl;
    std::cout<<"args_str : "<<args_str<<std::endl;
    std::cout<<"-------------------------------------------------"<<std::endl;

    //通过service_name 以及 method_name 找到对应的method 描述符
    if(serviceMaps.find(service_name) == serviceMaps.end()) {
        LOG_ERROR("请求所对应的服务不存在服务注册表中");
        return;
    }
    auto methodList = serviceMaps[service_name];
    if(methodList.find(method_name) == methodList.end()) {
        LOG_ERROR("服务所对应的方法不存在于方法表中");
        return;
    }
    ServiceAndMethodDesc sm_desc = methodList[method_name];
    google::protobuf::Service* service= sm_desc.service;
    const google::protobuf::MethodDescriptor*  method_desc = sm_desc.method_desc;
    google::protobuf::Message* request = service->GetRequestPrototype(method_desc).New();
    if(!request->ParseFromArray(args_str.c_str(), args_str.size())) {
        LOG_ERROR("请求参数解析失败");
        return;
    }
    google::protobuf::Message* response = service->GetResponsePrototype(method_desc).New();

    //设置callmethod的回调函数
    google::protobuf::Closure* done = google::protobuf::NewCallback<MyRpcProvider, 
                                    const muduo::net::TcpConnectionPtr&,
                                    google::protobuf::Message*>(this, &MyRpcProvider::SendRpcResponse,
                                            conn, response);

    //调用相应的method方法，并将结果发回请求者
    service->CallMethod(method_desc, nullptr, request, response, done);
}


void MyRpcProvider::Start() {
    //获取配置文件信息
    auto fileInfo = MyRpcApplication::getInstance().getFileInfo();
    std::string server_ip = fileInfo["server_ip"];
    uint16_t server_port = std::atoi(fileInfo["server_port"].c_str());
    muduo::net::InetAddress address(server_ip, server_port);

    //建立TCPServer
    muduo::net::TcpServer server(&eventLoop, address, "RpcProvider");

    //设置连接回调
    server.setConnectionCallback(std::bind(&MyRpcProvider::ConnectionHandler, this, std::placeholders::_1));
    
    //设置接收到信息回调
    server.setMessageCallback(std::bind(&MyRpcProvider::MessageHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    server.setThreadNum(5);

    std::cout<<"Server [ip = "<<server_ip <<", port = " <<server_port<<"] Running ......"<<std::endl;
    server.start();
    eventLoop.loop();
}
