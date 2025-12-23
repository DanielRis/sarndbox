FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="none"

# Optionally install the cmake for vcpkg
COPY ./reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

# Install dependencies required for Vrui, Kinect, and libfreenect2
# Based on official UC Davis SARndbox instructions
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
        build-essential \
        g++ \
        cmake \
        pkg-config \
        libusb-1.0-0-dev \
        libgl1-mesa-dev \
        libglu1-mesa-dev \
        libudev-dev \
        libasound2-dev \
        libpng-dev \
        libjpeg-dev \
        libtiff-dev \
        libxi-dev \
        libxrandr-dev \
        zlib1g-dev \
        libssl-dev \
        libbluetooth-dev \
        libspeex-dev \
        libogg-dev \
        libtheora-dev \
        libv4l-dev \
        libdc1394-dev \
        libfreetype6-dev \
        libturbojpeg0-dev \
        libglfw3-dev \
        libva-dev \
        libopenni2-dev \
        libopenal-dev \
        fonts-freefont-ttf \
        mesa-utils \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Build and install libfreenect2 (for Kinect v2 / Xbox One Kinect)
# Uses CPU/OpenGL for depth processing (no CUDA required)
WORKDIR /tmp
RUN git clone https://github.com/OpenKinect/libfreenect2.git \
    && cd libfreenect2 \
    && mkdir build && cd build \
    && cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DENABLE_VAAPI=OFF \
    && make -j$(nproc) \
    && make install \
    && install -m 755 bin/Protonect /usr/local/bin/ \
    && cd /tmp \
    && rm -rf libfreenect2

# Build and install Vrui (version 8.0 build 002)
# Downloaded from official UC Davis distribution
# Patch OpenAL typedef conflicts with newer OpenAL-soft headers (1.20.0+)
RUN curl -L -o Vrui-8.0-002.tar.gz https://web.cs.ucdavis.edu/~okreylos/ResDev/Vrui/Vrui-8.0-002.tar.gz \
    && tar xzf Vrui-8.0-002.tar.gz \
    && cd Vrui-8.0-002 \
    && sed -i 's/struct ALCdevice_struct/struct ALCdevice/g' Vrui/SoundContext.h \
    && sed -i 's/struct ALCcontext_struct/struct ALCcontext/g' Vrui/SoundContext.h \
    && make -j$(nproc) \
    && make install \
    && cd /tmp \
    && rm -rf Vrui-8.0-002.tar.gz Vrui-8.0-002

# Build and install Kinect 3D Video Capture Project (version 3.10)
# Downloaded from official UC Davis distribution
RUN curl -L -o Kinect-3.10.tar.gz https://web.cs.ucdavis.edu/~okreylos/ResDev/Kinect/Kinect-3.10.tar.gz \
    && tar xzf Kinect-3.10.tar.gz \
    && cd Kinect-3.10 \
    && make -j$(nproc) \
    && make install \
    && cd /tmp \
    && rm -rf Kinect-3.10.tar.gz Kinect-3.10

WORKDIR /workspaces
