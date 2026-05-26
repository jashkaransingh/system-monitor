#include "logger.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <sstream>

Logger::Logger(const std::string& log_path, size_t max_size_bytes)
    : log_path_(log_path), max_size_bytes_(max_size_bytes) {
    std::filesystem::create_directories(std::filesystem::path(log_path).parent_path());
    file_.open(log_path, std::ios::app);
}

void Logger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (file_.tellp() > static_cast<std::streamoff>(max_size_bytes_)) {
        rotate();
    }

    std::string entry = "[" + timestamp() + "] " + message;
    file_ << entry << "\n";
    file_.flush();
    std::cout << entry << "\n";
}

void Logger::rotate() {
    file_.close();
    std::string rotated = log_path_ + ".1";
    std::filesystem::rename(log_path_, rotated);
    file_.open(log_path_, std::ios::app);
}

std::string Logger::timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
