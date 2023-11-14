#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include "client.pb.h"

class clientService : public ClientPro::ClientServiceRPC {
    void Login(google::protobuf::RpcController* controller,
                       const ::ClientPro::LoginRequest* request,
                       ::ClientPro::LoginResponse* response,
                       ::google::protobuf::Closure* done) {
                        std::cout<<"Login Service execute"<<std::endl;
                        response->set_msg("SUCCESS");
                        done->Run();
                       }
};