#!/bin/bash
# SARndbox Dependency Uninstaller
# Removes Vrui 8.0, libfreenect2, and Kinect 3.10
# Use this to clean up before reinstalling
set -e

echo "=== SARndbox Dependency Uninstaller ==="
echo "This script removes Vrui 8.0, libfreenect2, and Kinect 3.10"
echo ""

# Check for root
if [ "$EUID" -ne 0 ]; then
    echo "Please run with sudo: sudo ./uninstall-dependencies.sh"
    exit 1
fi

echo "This will remove:"
echo "  - libfreenect2 from /usr/local"
echo "  - Vrui 8.0 from /usr/local"
echo "  - Kinect 3.10 from /usr/local"
echo "  - Kinect udev rules"
echo "  - Protonect binary"
echo ""
read -p "Continue? [y/N] " -n 1 -r
echo ""

if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Aborted."
    exit 1
fi

echo ""
echo "[1/5] Removing libfreenect2..."
rm -rf /usr/local/lib/libfreenect2* 2>/dev/null || true
rm -rf /usr/local/include/libfreenect2 2>/dev/null || true
rm -rf /usr/local/lib/cmake/freenect2 2>/dev/null || true
rm -f /usr/local/bin/Protonect 2>/dev/null || true

echo "[2/5] Removing Vrui 8.0..."
rm -rf /usr/local/lib/Vrui-8.0 2>/dev/null || true
rm -rf /usr/local/lib/x86_64-linux-gnu/Vrui-8.0 2>/dev/null || true
rm -rf /usr/local/include/Vrui-8.0 2>/dev/null || true
rm -rf /usr/local/share/Vrui-8.0 2>/dev/null || true
rm -rf /usr/local/etc/Vrui-8.0 2>/dev/null || true
rm -f /usr/local/bin/Vrui* 2>/dev/null || true
rm -f /usr/local/bin/DeviceDaemon* 2>/dev/null || true
rm -f /usr/local/bin/AlignTrackingMarkers 2>/dev/null || true
rm -f /usr/local/bin/MeasureEnvironment 2>/dev/null || true

echo "[3/5] Removing Kinect 3.10..."
rm -rf /usr/local/lib/Kinect-3.10 2>/dev/null || true
rm -rf /usr/local/lib/x86_64-linux-gnu/Kinect-3.10 2>/dev/null || true
rm -rf /usr/local/include/Kinect-3.10 2>/dev/null || true
rm -rf /usr/local/share/Kinect-3.10 2>/dev/null || true
rm -rf /usr/local/etc/Kinect-3.10 2>/dev/null || true
rm -f /usr/local/bin/Kinect* 2>/dev/null || true
rm -f /usr/local/bin/RawKinectViewer 2>/dev/null || true
rm -f /usr/local/bin/KinectViewer 2>/dev/null || true

echo "[4/5] Removing udev rules..."
rm -f /etc/udev/rules.d/90-kinect2.rules 2>/dev/null || true
rm -f /etc/udev/rules.d/90-kinect.rules 2>/dev/null || true
udevadm control --reload-rules 2>/dev/null || true
udevadm trigger 2>/dev/null || true

echo "[5/5] Updating library cache..."
ldconfig

echo ""
echo "========================================"
echo "=== Cleanup Complete ==="
echo "========================================"
echo ""
echo "You can now re-run install-dependencies.sh for a fresh install."
echo ""
