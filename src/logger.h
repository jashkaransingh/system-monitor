#pragma once
#include <string>
#include <fstream>
#include <mutex>

class Logger {
public:
    Logger(const std::string& log_path, size_t max_size_bytes);
    void log(const std::string& message);

private:
    std::string log_path_;
    size_t max_size_bytes_;
    std::ofstream file_;
    std::mutex mutex_;

    void rotate();
    std::string timestamp();
};
