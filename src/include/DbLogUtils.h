#ifndef LOGGER_UTILS_H
#define LOGGER_UTILS_H

#include <iostream>
#include <vector>
#include <deque>
#include <string>
#include <chrono>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <thread>
#include <mutex>
#include <fstream>
#include <condition_variable>

#define LOG_DEBUG(logFormat, ...) \
    do\
    {\
        DbLoggerUtils &logutils = DbLoggerUtils::getInstance();\
        char c[2048] = {0};\
        snprintf(c, 2048, logFormat, ##__VA_ARGS__);\
        std::string content(c);\
        logutils.log(DbLogLevel::DEBUG, c);\
    } while(0)

#define LOG_INFO(logFormat, ...) \
    do\
    {\
        DbLoggerUtils &logutils = DbLoggerUtils::getInstance();\
        char c[2048] = {0};\
        snprintf(c, 2048, logFormat, ##__VA_ARGS__);\
        std::string content(c);\
        logutils.log(DbLogLevel::INFO, c);\
    } while(0)

#define LOG_WARN(logFormat, ...) \
    do\
    {\
        DbLoggerUtils &logutils = DbLoggerUtils::getInstance();\
        char c[2048] = {0};\
        snprintf(c, 2048, logFormat, ##__VA_ARGS__);\
        std::string content(c);\
        logutils.log(DbLogLevel::WARN, c);\
    } while(0)

#define LOG_ERROR(logFormat, ...) \
    do\
    {\
        DbLoggerUtils &logutils = DbLoggerUtils::getInstance();\
        char c[2048] = {0};\
        snprintf(c, 2048, logFormat, ##__VA_ARGS__);\
        std::string content(c);\
        logutils.log(DbLogLevel::ERROR, c);\
    } while(0)

enum class DbLogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

class FileWriterUtils {
public:
    static FileWriterUtils& getInstance();
    void setFilePath(const std::string& path);
    void FlushDisk(std::deque<std::string>& msgs);

    FileWriterUtils(const FileWriterUtils&) = delete;
    FileWriterUtils(FileWriterUtils&&) = delete;

private:
    FileWriterUtils() = default;
    std::ofstream _outputStream;
};

class DbLoggerUtils {
public:
    static DbLoggerUtils& getInstance();
    void log(DbLogLevel level, const std::string& message);
    void setFilePath(const std::string& path);
    void setInterval(size_t interval);
    void setFlushMaxSize(size_t maxSize);
    void stop();

    ~DbLoggerUtils();
    DbLoggerUtils(const DbLoggerUtils&) = delete;
    DbLoggerUtils(DbLoggerUtils&&) = delete;

private:
    std::string _filepath = "../log/log.txt";
    std::deque<std::string> _logDiskQueue[2];
    size_t _curAddLogQueueIndex;
    size_t _interval = 3;
    size_t _needFlushMaxSize = 1000;
    bool _running = false;

    std::thread _flushDiskThread;
    std::mutex _exchangeMutex;
    std::condition_variable _exchangeCv;

    DbLoggerUtils();
    void flushDiskLoop();
    std::string getCurrentTime();
    std::string getLogLevelName(DbLogLevel level);
};
#endif // LOGGER_UTILS_H