#include "monitor.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sys/statvfs.h>

Monitor::Monitor(Logger& logger, double cpu_threshold, double mem_threshold, double disk_threshold)
    : logger_(logger), cpu_threshold_(cpu_threshold),
      mem_threshold_(mem_threshold), disk_threshold_(disk_threshold) {}

void Monitor::poll() {
    SystemMetrics m = collect();

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1)
        << "CPU: " << m.cpu_percent << "% | "
        << "MEM: " << m.mem_percent << "% ("
        << m.mem_used_mb / 1024 << "GB/" << m.mem_total_mb / 1024 << "GB) | "
        << "DISK: " << m.disk_percent << "% (/)";

    logger_.log(oss.str());
    check_thresholds(m);
}

SystemMetrics Monitor::collect() {
    SystemMetrics m;
    m.cpu_percent = get_cpu_usage();
    m.disk_percent = get_disk_usage("/");
    m.mem_percent = get_mem_usage(m.mem_used_mb, m.mem_total_mb);
    return m;
}

double Monitor::get_cpu_usage() {
    // Read /proc/stat to compute CPU delta between polls
    std::ifstream stat("/proc/stat");
    std::string line;
    std::getline(stat, line);

    long long user, nice, system, idle, iowait, irq, softirq;
    std::istringstream ss(line);
    std::string cpu;
    ss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq;

    long long total = user + nice + system + idle + iowait + irq + softirq;
    long long delta_total = total - prev_total_;
    long long delta_idle = idle - prev_idle_;

    prev_total_ = total;
    prev_idle_ = idle;

    if (delta_total == 0) return 0.0;
    return 100.0 * (1.0 - static_cast<double>(delta_idle) / delta_total);
}

double Monitor::get_mem_usage(long& used_mb, long& total_mb) {
    std::ifstream meminfo("/proc/meminfo");
    long total = 0, available = 0;
    std::string line;

    while (std::getline(meminfo, line)) {
        std::istringstream ss(line);
        std::string key;
        long value;
        ss >> key >> value;
        if (key == "MemTotal:") total = value;
        if (key == "MemAvailable:") available = value;
        if (total && available) break;
    }

    total_mb = total / 1024;
    used_mb = (total - available) / 1024;
    return total ? 100.0 * (total - available) / total : 0.0;
}

double Monitor::get_disk_usage(const std::string& path) {
    struct statvfs stat;
    if (statvfs(path.c_str(), &stat) != 0) return 0.0;
    unsigned long total = stat.f_blocks * stat.f_frsize;
    unsigned long free = stat.f_bfree * stat.f_frsize;
    if (total == 0) return 0.0;
    return 100.0 * (total - free) / total;
}

void Monitor::check_thresholds(const SystemMetrics& m) {
    if (m.cpu_percent > cpu_threshold_)
        logger_.log("ALERT: CPU usage " + std::to_string(m.cpu_percent) +
                    "% exceeds threshold " + std::to_string(cpu_threshold_) + "%");
    if (m.mem_percent > mem_threshold_)
        logger_.log("ALERT: MEM usage " + std::to_string(m.mem_percent) +
                    "% exceeds threshold " + std::to_string(mem_threshold_) + "%");
    if (m.disk_percent > disk_threshold_)
        logger_.log("ALERT: DISK usage " + std::to_string(m.disk_percent) +
                    "% exceeds threshold " + std::to_string(disk_threshold_) + "%");
}
