#pragma once
#include <string>
#include "logger.h"

struct SystemMetrics {
    double cpu_percent;
    double mem_percent;
    double disk_percent;
    long mem_used_mb;
    long mem_total_mb;
};

class Monitor {
public:
    explicit Monitor(Logger& logger, double cpu_threshold = 90.0,
                     double mem_threshold = 85.0, double disk_threshold = 90.0);
    void poll();

private:
    Logger& logger_;
    double cpu_threshold_;
    double mem_threshold_;
    double disk_threshold_;

    // State for CPU delta calculation
    long long prev_idle_ = 0;
    long long prev_total_ = 0;

    SystemMetrics collect();
    double get_cpu_usage();
    double get_mem_usage(long& used_mb, long& total_mb);
    double get_disk_usage(const std::string& path = "/");
    void check_thresholds(const SystemMetrics& m);
};
