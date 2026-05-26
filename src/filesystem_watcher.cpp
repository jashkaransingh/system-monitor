#include "filesystem_watcher.h"
#include <sys/inotify.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <map>

#define EVENT_BUF_SIZE (1024 * (sizeof(inotify_event) + NAME_MAX + 1))

FilesystemWatcher::FilesystemWatcher(std::vector<std::string> watch_paths, Logger& logger)
    : paths_(std::move(watch_paths)), logger_(logger) {
    inotify_fd_ = inotify_init1(IN_NONBLOCK);
    if (inotify_fd_ < 0) {
        throw std::runtime_error("inotify_init failed: " + std::string(strerror(errno)));
    }
}

FilesystemWatcher::~FilesystemWatcher() {
    if (inotify_fd_ >= 0) close(inotify_fd_);
}

void FilesystemWatcher::watch(const std::atomic<bool>& running) {
    std::map<int, std::string> wd_to_path;
    uint32_t mask = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO;

    for (const auto& path : paths_) {
        int wd = inotify_add_watch(inotify_fd_, path.c_str(), mask);
        if (wd >= 0) {
            wd_to_path[wd] = path;
        }
    }

    char buf[EVENT_BUF_SIZE];

    while (running) {
        ssize_t len = read(inotify_fd_, buf, EVENT_BUF_SIZE);
        if (len < 0) {
            // EAGAIN = no events ready (non-blocking), sleep and retry
            if (errno == EAGAIN) {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                continue;
            }
            break;
        }

        for (char* ptr = buf; ptr < buf + len;) {
            auto* event = reinterpret_cast<inotify_event*>(ptr);
            if (event->len > 0) {
                std::string path = wd_to_path[event->wd] + "/" + event->name;
                std::string type = event_type_string(event->mask);
                logger_.log("FS WATCH: " + type + " " + path);
            }
            ptr += sizeof(inotify_event) + event->len;
        }
    }
}

std::string FilesystemWatcher::event_type_string(uint32_t mask) {
    if (mask & IN_CREATE) return "CREATED";
    if (mask & IN_DELETE) return "DELETED";
    if (mask & IN_MODIFY) return "MODIFIED";
    if (mask & IN_MOVED_FROM) return "MOVED_FROM";
    if (mask & IN_MOVED_TO) return "MOVED_TO";
    return "UNKNOWN";
}
