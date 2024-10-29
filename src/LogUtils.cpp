#include "LogUtils.h"
#include <thread>
#include <ctime>
#include <chrono>
#include <sstream>

LogUtils::LogUtils() {
    debug_fstream.open("../log/debug.log", std::ios::app);
    info_fstream.open("../log/info.log", std::ios::app);
    error_fstream.open("../log/error.log", std::ios::app);
    if(!debug_fstream.is_open() || !info_fstream.is_open() || !info_fstream.is_open()) {
        std::cout<<"日志文件打开失败"<<std::endl;
    }
    //开启写日志循环
    std::thread logThread(&LogUtils::WriteLog2File, this);
    //该线程与主线程分离
    logThread.detach();
}

std::string LogUtils::getTime() {
    // 获取当前系统时间的时间点
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    
    // 将时间点转换为时间结构
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    
    // 使用本地时间转换函数将时间结构转换为本地时间
    std::tm* local_time = std::localtime(&now_time);
    
    // 从时间结构中获取年、月、日和时间
    int year = local_time->tm_year + 1900;  // 年份需要加上1900
    int month = local_time->tm_mon + 1;     // 月份从0开始，需要加上1
    int day = local_time->tm_mday;          // 日期
    int hour = local_time->tm_hour;         // 小时
    int minute = local_time->tm_min;        // 分钟
    int second = local_time->tm_sec;        // 秒钟

    std::stringstream ss;
    ss<<year<<"/"<<month<<"/"<<day<<" "<<hour<<":"<<minute<<":"<<second<<" ";
    return ss.str();
}

LogUtils& LogUtils::getInstance() {
    static LogUtils logUtils;
    return logUtils;
}

void LogUtils::WriteLog2File() {
    while (true)
    {
        std::string time_str = getTime();
        LogInfo loginfo = logQueue.pop();
        loginfo.content = time_str + loginfo.content;
        switch (loginfo.level)
        {
            case DEBUG:
                debug_fstream<<loginfo.content<<std::endl;
                break;
            case INFO:
                info_fstream<<loginfo.content<<std::endl;
                break;
            case ERROR:
                error_fstream<<loginfo.content<<std::endl;
                break;
            default:
                std::cout<<"日志信息错误"<<std::endl;
                exit(EXIT_FAILURE);
        }
    }
}

void LogUtils::WriteLog2Queue(const std::string& msg, LogLevel level) {
    logQueue.push(LogInfo{
        .level = level,
        .content = msg,
    });
}

LogUtils::~LogUtils() {
    if(debug_fstream.is_open()) {
        debug_fstream.close();
    }
    if(info_fstream.is_open()) {
        info_fstream.close();
    }
    if(error_fstream.is_open()) {
        error_fstream.close();
    }
}