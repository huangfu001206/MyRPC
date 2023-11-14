#pragma once
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include "MyRpcApplication.h"
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpServer.h>
#include <muduo/net/TcpConnection.h>
#include <muduo/net/Buffer.h>
#include <unordered_map>

struct ServiceAndMethodDesc
{
    const google::protobuf::MethodDescriptor* method_desc;
    google::protobuf::Service* service;
};


class MyRpcProvider {
public:
    //注册rpc服务
    void NotifyService(google::protobuf::Service* service);

    //开始运行rpc监听服务
    void Start();

    //连接回调函数
    void ConnectionHandler(const muduo::net::TcpConnectionPtr&);

    //接收到请求信息的回调函数
    void MessageHandler(const muduo::net::TcpConnectionPtr &conn,
                            muduo::net::Buffer*,
                            muduo::Timestamp);

    void SendRpcResponse(std::shared_ptr<muduo::net::TcpConnection> const&,
                            google::protobuf::Message *response);
private:
    muduo::net::EventLoop eventLoop;
    //记录服务映射关系，便于查询某个服务的某个方法
    std::unordered_map<std::string,
        std::unordered_map<std::string, ServiceAndMethodDesc>> serviceMaps;
};