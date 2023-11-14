#include "google/protobuf/service.h"
#include <string>

class MyRpcController : public google::protobuf::RpcController {
private:
    std::string errorText;
    bool isFailed;
public:

    inline void Reset() {
        isFailed = false;
        errorText.clear();
    }
    
    inline void SetFailed(const std::string& reason) {
        isFailed = true;
        this->errorText = reason;
    }

    inline std::string ErrorText() const {
        return errorText;
    }

    inline bool Failed() const {
        return isFailed;
    }

    inline void StartCancel() {}
    inline bool IsCanceled() const{ return false; };
    inline void NotifyOnCancel(google::protobuf::Closure* callback) {};
};