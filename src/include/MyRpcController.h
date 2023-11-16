#include "google/protobuf/service.h"
#include <string>

class MyRpcController : public google::protobuf::RpcController {
private:
    std::string errorText;
    bool isFailed;
public:

    inline void Reset() override {
        isFailed = false;
        errorText.clear();
    }
    
    inline void SetFailed(const std::string& reason) override {
        isFailed = true;
        this->errorText = reason;
    }

    inline std::string ErrorText() const override {
        return errorText;
    }

    inline bool Failed() const override {
        return isFailed;
    }

    inline void StartCancel() override {}
    inline bool IsCanceled() const override{ return false; };
    inline void NotifyOnCancel(google::protobuf::Closure* callback) override {};
};