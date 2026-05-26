#!/bin/bash
set -e

echo "Installing system-monitor..."
mkdir -p /var/log/system-monitor
cp build/system-monitor /usr/local/bin/
cp scripts/system-monitor.service /etc/systemd/system/
systemctl daemon-reload
echo "Done. Run: sudo systemctl enable --now system-monitor"
