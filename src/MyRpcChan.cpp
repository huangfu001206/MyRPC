#include "MyRpcChan.h"
#include "MyRpcHeader.pb.h"
#include "MyRpcApplication.h"
#include "ZookeeperUtils.h"
#include "LRUCache.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

std::shared_ptr<std::string> MyRpcChan::GetCache(const std::string& key) {
    auto& _cache = LRUCache<std::string, std::string>::getInstance();
    return _cache.get(key);
}

void MyRpcChan::AddCache(const std::string& key, const std::string& value) {
    auto& _cache = LRUCache<std::string, std::string>::getInstance();
    _cache.put(key, value);
}

void MyRpcChan::CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, 
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done) 
{
    //获取服务名称以及方法索引
    const google::protobuf::ServiceDescriptor* service = method->service();
    std::string service_name = service->name();
    std::string method_name = method->name();

    controller->Reset();

    //封装请求
    uint32_t args_size = 0;
    std::string args_str;
    if(request->SerializeToString(&args_str)) {
        args_size = args_str.size();
    } else {
        controller->SetFailed("请求参数序列化失败，请确认参数是否正确！");
        LOG_ERROR("请求参数序列化失败，请确认参数是否正确！");
        return;
    }

    //封装请求头
    MyRpc::RpcHeader header;
    header.set_service_name(service_name);
    header.set_method_name(method_name);
    header.set_args_size(args_size);

    std::string rpc_header_str;
    uint32_t header_size = 0;
    if(header.SerializeToString(&rpc_header_str)) {
        header_size = rpc_header_str.size();
    } else {
        controller->SetFailed("rpcheader 序列化失败");
        LOG_ERROR("rpcheader 序列化失败");
        return;
    } 

    //将所发送包信息拼接完整
    std::string rpc_request_str;
    //将四个字节的数字，强转为四个字节的字符串（将int类型的地址强转化为char）
    rpc_request_str.insert(0, std::string((char*)&header_size, 4));
    rpc_request_str += rpc_header_str;
    rpc_request_str += args_str;

    //查询缓存/zk 获取服务端地址
    std::string zNodePath = "/"+service_name+"/"+method_name;
    std::shared_ptr<std::string> ip_port_ptr = GetCache(zNodePath);
    std::string server_ip_port;
    if(ip_port_ptr != nullptr) {
        server_ip_port = *(ip_port_ptr);
    } else {
        //连接zookeeper服务器，找到服务以及方法对应的服务ip和端口号
        auto& client = ZookeeperClient::getInstance();
        client.Run();
        server_ip_port = std::string(client.GetZNodeData(zNodePath.c_str()));

        if(!server_ip_port.empty()) {
            AddCache(zNodePath, server_ip_port);
            //监听节点
            client.WatchNode(zNodePath);
        }
        else {
            controller->SetFailed(zNodePath + " 服务不存在");
            return;
        }
    }

    //解析IP & Port
    size_t index = server_ip_port.find_first_of(':');
    std::string server_ip;
    int port;
    try {
        server_ip = server_ip_port.substr(0, index);
        port = std::atoi(server_ip_port.substr(index + 1).c_str());
        // std::cout<<"服务： "<<service_name<<"."<<method_name<<" is at "<<server_ip<<":"<<port<<std::endl;
    } catch (const std::exception& e) {
        LOG_ERROR("%s", e.what());
        controller->SetFailed(e.what());
        return;
    }

    //使用socket进行请求发送
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd == -1) {
        controller->SetFailed("socket 创建失败");
        LOG_ERROR("socket 创建失败");
        return;
    }

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    
    //向服务器发起连接请求
    if(connect(client_fd, (const sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        close(client_fd);
        controller->SetFailed("无法与服务器建立连接");
        LOG_ERROR("无法与服务器建立连接");
        return;
    }
    //发送rpc请求信息
    if(send(client_fd, rpc_request_str.c_str(), rpc_request_str.size(), 0) == -1) {
        close(client_fd);
        controller->SetFailed("rpc 请求发送失败");
        LOG_ERROR("rpc 请求发送失败");
        return;
    }

    //等待请求响应
    char recv_buf[1024];
    int recv_buf_size = 0;
    if((recv_buf_size = recv(client_fd, recv_buf, 1024, 0)) == -1) {
        close(client_fd);
        controller->SetFailed(server_ip+":"+std::to_string(port)+" ,服务器未响应请求");
        LOG_ERROR("%s:%d,服务器未响应请求", server_ip.c_str(), port);
        return;
    }

    //响应结果进行反序列化
    if(!response->ParseFromArray(recv_buf, recv_buf_size)) {
        close(client_fd);
        controller->SetFailed("响应结果反序列化失败");
        LOG_ERROR("响应结果反序列化失败");
        return;
    }
    close(client_fd);
    LOG_INFO("发送RPC并接收回复成功");
}