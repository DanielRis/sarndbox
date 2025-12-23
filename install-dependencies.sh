#!/bin/bash
# SARndbox Dependency Installer
# Installs Vrui 8.0, libfreenect2, and Kinect 3.10 on Ubuntu/Debian systems
# Based on official UC Davis instructions
set -e

echo "=== SARndbox Dependency Installer ==="
echo "This script installs Vrui 8.0, libfreenect2, and Kinect 3.10"
echo ""

# Check for root
if [ "$EUID" -ne 0 ]; then
    echo "Please run with sudo: sudo ./install-dependencies.sh"
    exit 1
fi

# Install system packages
echo "[1/4] Installing system dependencies..."
apt-get update
apt-get install -y \
    build-essential g++ cmake pkg-config \
    libusb-1.0-0-dev libgl1-mesa-dev libglu1-mesa-dev \
    libudev-dev libasound2-dev libpng-dev libjpeg-dev libtiff-dev \
    libxi-dev libxrandr-dev zlib1g-dev libssl-dev libbluetooth-dev \
    libspeex-dev libogg-dev libtheora-dev libv4l-dev libdc1394-dev \
    libfreetype6-dev libturbojpeg0-dev libglfw3-dev libva-dev \
    libopenni2-dev libopenal-dev fonts-freefont-ttf \
    mesa-utils \
    git curl

# Build libfreenect2
echo ""
echo "[2/4] Building libfreenect2..."
cd /tmp
rm -rf libfreenect2
git clone https://github.com/OpenKinect/libfreenect2.git
cd libfreenect2
mkdir -p build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DENABLE_VAAPI=OFF
make -j$(nproc)
make install
install -m 755 bin/Protonect /usr/local/bin/

# Install official udev rules for Kinect v2
echo ""
echo "Installing Kinect udev rules..."
cp ../platform/linux/udev/90-kinect2.rules /etc/udev/rules.d/
udevadm control --reload-rules
udevadm trigger

cd /tmp && rm -rf libfreenect2

# Build Vrui 8.0
echo ""
echo "[3/4] Building Vrui 8.0..."
cd /tmp
curl -L -o Vrui-8.0-002.tar.gz \
    https://web.cs.ucdavis.edu/~okreylos/ResDev/Vrui/Vrui-8.0-002.tar.gz
tar xzf Vrui-8.0-002.tar.gz
cd Vrui-8.0-002
# Patch for newer OpenAL headers
sed -i 's/struct ALCdevice_struct/struct ALCdevice/g' Vrui/SoundContext.h
sed -i 's/struct ALCcontext_struct/struct ALCcontext/g' Vrui/SoundContext.h
make -j$(nproc)
make install
cd /tmp && rm -rf Vrui-8.0-002.tar.gz Vrui-8.0-002

# Build Kinect 3.10
echo ""
echo "[4/4] Building Kinect 3.10..."
cd /tmp
curl -L -o Kinect-3.10.tar.gz \
    https://web.cs.ucdavis.edu/~okreylos/ResDev/Kinect/Kinect-3.10.tar.gz
tar xzf Kinect-3.10.tar.gz
cd Kinect-3.10
make -j$(nproc)
make install
cd /tmp && rm -rf Kinect-3.10.tar.gz Kinect-3.10

# Update library cache
ldconfig

# Clean up build dependencies and cached packages
echo ""
echo "Cleaning up..."
apt-get autoremove -y
apt-get clean
rm -rf /var/lib/apt/lists/*

echo ""
echo "========================================"
echo "=== Installation Complete ==="
echo "========================================"
echo ""
echo "Installed:"
echo "  - System packages (OpenGL, dev libraries)"
echo "  - libfreenect2 (Kinect v2 driver)"
echo "  - Kinect udev rules (non-root USB access)"
echo "  - Vrui 8.0 (VR toolkit framework)"
echo "  - Kinect 3.10 (Kinect integration library)"
echo ""
echo "IMPORTANT: Unplug and replug your Kinect for udev rules to take effect!"
echo ""
echo "Test your Kinect with:"
echo "  Protonect"
echo ""
echo "Then build SARndbox:"
echo "  cd sarndbox"
echo "  make"
echo "  sudo make install  # optional"
echo ""
