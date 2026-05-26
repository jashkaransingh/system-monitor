# system-monitor

C++ background daemon that monitors system resources in real-time. Runs as a systemd service, logs metrics every 10 seconds, watches the filesystem for changes using Linux's inotify API, and sends alerts on threshold breaches.

## What it does

- Polls CPU usage, memory, and disk every 10 seconds with minimal overhead
- Watches configured directories via `inotify` — logs file creates, deletes, and modifications
- Rotates log files automatically when they hit 10MB
- Runs on boot via systemd, restarts on crash
- Alert thresholds configurable in `config.ini`

## Build

```bash
git clone https://github.com/jashkaransingh/system-monitor
cd system-monitor
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Install as systemd service

```bash
sudo ./scripts/install.sh
sudo systemctl enable system-monitor
sudo systemctl start system-monitor
journalctl -u system-monitor -f   # follow logs
```

## Sample output

```
[2025-10-14 09:23:10] CPU: 12.3% | MEM: 58.1% (4.6GB/8GB) | DISK: 71.2% (/dev/sda1)
[2025-10-14 09:23:10] FS WATCH: CREATED /home/user/documents/report.pdf
[2025-10-14 09:23:20] CPU: 8.7%  | MEM: 57.9% (4.6GB/8GB) | DISK: 71.2% (/dev/sda1)
[2025-10-14 09:23:30] ALERT: MEM usage 89.2% exceeds threshold 85%
```

## Stack

- C++17, POSIX APIs, Linux inotify
- CMake build system
- systemd service unit
- Deployed on 5 Raspberry Pi devices
