#include "DbLogUtils.h"

FileWriterUtils& FileWriterUtils::getInstance() {
    static FileWriterUtils utils; 
    return utils;
}

void FileWriterUtils::setFilePath(const std::string& path) {
    _outputStream.open(path, std::ios::app);
    if (!_outputStream.is_open()) {
        std::cerr << "open file failed : " << path << std::endl;
    }
}

void FileWriterUtils::FlushDisk(std::deque<std::string>& msgs) {
    if (msgs.empty()) {
        return;
    }
    for (const std::string& s : msgs) {
        _outputStream << s << std::endl;
    }
}

void DbLoggerUtils::setFilePath(const std::string& path) {
    _filepath = path;
}

DbLoggerUtils& DbLoggerUtils::getInstance() {
    static DbLoggerUtils utils;
    return utils;
}

void DbLoggerUtils::setInterval(size_t interval) {
    _interval = interval;
}
void DbLoggerUtils::setFlushMaxSize(size_t maxSize) {
    _needFlushMaxSize = maxSize;
}

DbLoggerUtils::~DbLoggerUtils() {
    stop();
}

DbLoggerUtils::DbLoggerUtils()
    : _curAddLogQueueIndex(0), _running(true) {
    FileWriterUtils::getInstance().setFilePath(_filepath);
    _flushDiskThread = std::thread(&DbLoggerUtils::flushDiskLoop, this);
    // _flushDiskThread.detach();
}

void DbLoggerUtils::log(DbLogLevel level, const std::string& message) {
    std::stringstream ss;
    ss << "[" << getCurrentTime() << "] " << "[" << getLogLevelName(level) << "] " << message;
    {
        std::lock_guard<std::mutex> lock(_exchangeMutex);
        _logDiskQueue[_curAddLogQueueIndex].push_back(ss.str());

        if (_logDiskQueue[_curAddLogQueueIndex].size() >= _needFlushMaxSize) {
            _exchangeCv.notify_one();
        }
    }
}

void DbLoggerUtils::stop() {
    if (_running) {
        _running = false;
        _exchangeCv.notify_all();
        _flushDiskThread.join();
        FileWriterUtils::getInstance().FlushDisk(_logDiskQueue[_curAddLogQueueIndex]);
        _logDiskQueue[_curAddLogQueueIndex].clear();
    }
}

void DbLoggerUtils::flushDiskLoop() {
    while (_running) {
        int diskQueueIndex;
        {
            std::unique_lock<std::mutex> lock(_exchangeMutex);
            auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(_interval);
            _exchangeCv.wait_until(lock, timeout, [&]() {
                return !_running || _logDiskQueue[_curAddLogQueueIndex].size() >= _needFlushMaxSize;
            });

            if (!_running) {
                return;
            }

            if (_logDiskQueue[_curAddLogQueueIndex].empty()) {
                continue;
            }
            diskQueueIndex = _curAddLogQueueIndex;
            _curAddLogQueueIndex = (_curAddLogQueueIndex + 1) % 2;
        }
        FileWriterUtils::getInstance().FlushDisk(_logDiskQueue[diskQueueIndex]);
        _logDiskQueue[diskQueueIndex].clear();
    }
}

std::string DbLoggerUtils::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* local_tm = std::localtime(&now_c);
    
    std::ostringstream oss;
    oss << std::put_time(local_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string DbLoggerUtils::getLogLevelName(DbLogLevel level) {
    switch (level) {
    case DbLogLevel::DEBUG: return "DEBUG";
    case DbLogLevel::INFO: return "INFO";
    case DbLogLevel::WARN: return "WARN";
    case DbLogLevel::ERROR: return "ERROR";
    default: return "UNKNOWN";
    }
}
