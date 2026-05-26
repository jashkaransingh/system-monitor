# system-monitor

C++ background daemon that watches Linux systems in real time. polls CPU, memory, and disk every 10 seconds. uses inotify for filesystem events. runs as a systemd service on a fleet of Raspberry Pis. caught 3 SD card failures before data loss.

## what it does

- polls CPU, memory, and disk every 10 seconds with under 0.1% CPU overhead
- watches configured directories via inotify, logs creates, deletes, and modifications
- rotates log files automatically when they hit 10MB
- runs on boot via systemd, restarts on crash
- alert thresholds configurable in `config.ini`

## the hard part

the first version ate 3% CPU just polling. profiled it and turned out I was reading `/proc/stat` and `/proc/meminfo` way too aggressively and parsing strings in a hot loop. switched to a longer polling interval with delta-based sampling, replaced repeated string parsing with cached buffers, and moved file watches to inotify instead of polling directory listings. now it sits at under 0.1% CPU. running on 5 Pis for months. caught 3 real SD card failures via disk health alerts before data was lost.

## build

```bash
git clone https://github.com/jashkaransingh/system-monitor
cd system-monitor
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## install as a systemd service

```bash
sudo ./scripts/install.sh
sudo systemctl enable system-monitor
sudo systemctl start system-monitor
journalctl -u system-monitor -f
```

## sample output

```
[2025-10-14 09:23:10] CPU 12.3% | MEM 58.1% (4.6GB/8GB) | DISK 71.2% (/dev/sda1)
[2025-10-14 09:23:10] FS WATCH CREATED /home/user/documents/report.pdf
[2025-10-14 09:23:20] CPU 8.7%  | MEM 57.9% (4.6GB/8GB) | DISK 71.2% (/dev/sda1)
[2025-10-14 09:23:30] ALERT MEM usage 89.2% exceeds threshold 85%
```

## stack

C++17, POSIX, Linux inotify, CMake, systemd. deployed on 5 Raspberry Pis.
