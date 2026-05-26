#include <iostream>
#include <csignal>
#include <thread>
#include <atomic>
#include "monitor.h"
#include "filesystem_watcher.h"
#include "logger.h"

static std::atomic<bool> running(true);

void signal_handler(int sig) {
    if (sig == SIGTERM || sig == SIGINT) {
        running = false;
    }
}

int main() {
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGINT, signal_handler);

    Logger logger("/var/log/system-monitor/metrics.log", 10 * 1024 * 1024); // 10MB rotation
    Monitor monitor(logger);
    FilesystemWatcher watcher({"/home", "/etc", "/var/log"}, logger);

    logger.log("system-monitor started (PID: " + std::to_string(getpid()) + ")");

    // Run filesystem watcher in separate thread
    std::thread fs_thread([&]() {
        watcher.watch(running);
    });

    // Main loop: poll system metrics every 10 seconds
    while (running) {
        monitor.poll();
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    fs_thread.join();
    logger.log("system-monitor stopped");
    return 0;
}
