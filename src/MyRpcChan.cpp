#include "MyRpcChan.h"
#include "MyRpcHeader.pb.h"
#include "MyRpcApplication.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

void MyRpcChan::CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, 
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done) 
{
    //获取服务名称以及方法索引
    const google::protobuf::ServiceDescriptor* service = method->service();
    std::string service_name = service->full_name();
    std::string method_name = method->name();

    //封装请求
    uint32_t args_size = 0;
    std::string args_str;
    if(request->SerializeToString(&args_str)) {
        args_size = args_str.size();
    } else {
        std::cout<<"参数序列化失败，请确认参数是否正确！"<<std::endl;
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
        std::cout<<"rpcheader 序列化失败"<<std::endl;
        return;
    } 

    //将所发送包信息拼接完整
    std::string rpc_request_str;
    //将四个字节的数字，强转为四个字节的字符串（将int类型的地址强转化为char）
    rpc_request_str.insert(0, std::string((char*)&header_size, 4));
    rpc_request_str += rpc_header_str;
    rpc_request_str += args_str;

    std::cout<<"header size : "<<header_size<<std::endl;
    std::cout<<"rpc header : "<<rpc_header_str<<std::endl;
    std::cout<<"service_name : "<<service_name<<std::endl;
    std::cout<<"method_name : "<<method_name<<std::endl;
    std::cout<<"args : "<<args_str<<std::endl;

    //使用socket进行请求发送
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(client_fd == -1) {
        std::cout<<"socket 创建失败"<<std::endl;
        return;
    }
    auto config_file_info = MyRpcApplication::getInstance().getFileInfo();
    std::string server_ip = config_file_info["server_ip"];
    int port = atoi(config_file_info["server_port"].c_str());
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    
    //向服务器发起连接请求
    if(connect(client_fd, (const sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        close(client_fd);
        std::cout<<"无法与服务器建立连接"<<std::endl;
        return;
    }
    //发送rpc请求信息
    if(send(client_fd, rpc_request_str.c_str(), rpc_request_str.size(), 0) == -1) {
        close(client_fd);
        std::cout<<"rpc 请求发送失败"<<std::endl;
        return;
    }
    //等待请求响应
    char recv_buf[1024];
    int recv_buf_size = 0;
    if((recv_buf_size = recv(client_fd, recv_buf, 1024, 0)) == -1) {
        close(client_fd);
        std::cout<<"服务器未响应请求"<<std::endl;
        return;
    }

    //响应结果进行反序列化
    if(!response->ParseFromArray(recv_buf, recv_buf_size)) {
        close(client_fd);
        std::cout<<"响应结果反序列化失败"<<std::endl;
        return;
    }
    close(client_fd);
}