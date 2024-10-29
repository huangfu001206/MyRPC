#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "LRUCache.h"

class MyRpcChan : public google::protobuf::RpcChannel {
public:
    MyRpcChan() : _cache(100) {}

    void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, 
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done) override;
    
    std::shared_ptr<std::string> GetCache(const std::string& key);

    void AddCache(const std::string& key, const std::string& value);

private:
    LRUCache<std::string, std::string> _cache;
};