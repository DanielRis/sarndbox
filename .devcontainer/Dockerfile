FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="none"

# Optionally install the cmake for vcpkg
COPY ./reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

# Install dependencies required for Vrui, Kinect, and libfreenect2 (Kinect v2)
# Includes CUDA toolkit + GCC 12 (CUDA requires GCC <= 12) for GPU depth processing
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
        build-essential \
        g++ \
        gcc-12 \
        g++-12 \
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
        nvidia-cuda-toolkit \
        nvidia-cuda-toolkit-gcc \
        fakeroot \
        debhelper \
        dpkg-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Build and install libfreenect2 (for Kinect v2 / Xbox One Kinect)
# - Use CUDA for GPU depth processing (target: GTX 1060, Pascal SM 6.1)
# - Use GCC 12 for CUDA compatibility
# - Disable OpenCL due to header conflicts with OpenCL 3.0
# - Suppress packed-member warning that newer GCC treats as error (no upstream fix)
# See: https://aur.archlinux.org/packages/libfreenect2
WORKDIR /tmp
RUN git clone https://github.com/OpenKinect/libfreenect2.git \
    && cd libfreenect2 \
    # Download missing helper_math.h from CUDA samples
    && curl -L -o include/internal/helper_math.h \
        https://raw.githubusercontent.com/NVIDIA/cuda-samples/master/Common/helper_math.h \
    && mkdir build && cd build \
    && cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local \
        -DCMAKE_C_COMPILER=gcc-12 \
        -DCMAKE_CXX_COMPILER=g++-12 \
        -DCMAKE_CXX_FLAGS="-Wno-error=address-of-packed-member" \
        -DCUDA_HOST_COMPILER=/usr/bin/gcc-12 \
        -DENABLE_OPENCL=OFF \
        -DENABLE_CUDA=ON \
        -DCMAKE_CUDA_ARCHITECTURES=61 \
    && make -j$(nproc) \
    && make install \
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
