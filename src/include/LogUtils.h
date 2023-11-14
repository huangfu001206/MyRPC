#include "ThreadSafeQueue.h"
#include <string>
#include <fstream>


#define LOG_DEBUG(logFormat, ...) \
    do\
    {\
        LogUtils &logutils = LogUtils::getInstance();\
        char c[1024] = {0};\
        snprintf(c, 1024, logFormat, ##__VA_ARGS__);\
        std::string content(c);\
        logutils.WriteLog2Queue(c, DEBUG);\
    } while(0)

#define LOG_INFO(logFormat, ...) \
    do\
    {\
        LogUtils &logutils = LogUtils::getInstance();\
        char c[1024] = {0};\
        snprintf(c, 1024, logFormat, ##__VA_ARGS__);\
        std::string content(c);\
        logutils.WriteLog2Queue(c, INFO);\
    } while(0)
    
#define LOG_ERROR(logFormat, ...) \
    do\
    {\
        LogUtils &logutils = LogUtils::getInstance();\
        char c[1024] = {0};\
        snprintf(c, 1024, logFormat, ##__VA_ARGS__);\
        std::string content(c);\
        logutils.WriteLog2Queue(c, ERROR);\
    } while(0)

enum LogLevel : int{
    DEBUG,
    INFO,
    ERROR
};

//日志内容结构体
struct LogInfo
{
    LogLevel level;
    std::string content;
};

class LogUtils {
public:
    LogUtils(const LogUtils&) = delete;
    void operator=(const LogUtils&) = delete;

    LogUtils();
    ~LogUtils();

    //获取日志时间
    std::string getTime();

    //获取单例日志对象
    static LogUtils& getInstance();

    //异步写日志到文件中
    void WriteLog2File();

    //同步写日志到队列中
    void WriteLog2Queue(const std::string& msg, LogLevel level = DEBUG);
private:
    std::ofstream debug_fstream;
    std::ofstream info_fstream;
    std::ofstream error_fstream;
    ThreadSafeQueue<LogInfo> logQueue;
};
