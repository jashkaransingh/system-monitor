#pragma once
#include <string>
#include <vector>
#include <atomic>
#include "logger.h"

class FilesystemWatcher {
public:
    FilesystemWatcher(std::vector<std::string> watch_paths, Logger& logger);
    ~FilesystemWatcher();
    void watch(const std::atomic<bool>& running);

private:
    std::vector<std::string> paths_;
    Logger& logger_;
    int inotify_fd_;

    std::string event_type_string(uint32_t mask);
};
