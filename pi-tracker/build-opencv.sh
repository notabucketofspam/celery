#!/usr/bin/sh

# note: this is on a raspberry pi 4 model b
# the os is raspberry pi os lite 64-bit (debian 13)

# add the swapfile
sudo fallocate -l 4G /swapfile
sudo chmod 600 /swapfile
sudo mkswap /swapfile
sudo swapon /swapfile

# apt
sudo apt update
sudo apt upgrade -y

sudo apt install -y build-essential cmake pkg-config git \
libjpeg-dev libpng-dev libtiff-dev \
libavcodec-dev libavformat-dev libswscale-dev \
libv4l-dev v4l-utils \
libxvidcore-dev libx264-dev \
libopenblas-dev liblapack-dev libeigen3-dev \
libtbb-dev gfortran liblapacke-dev ninja

sudo ln -sf /usr/include/lapacke*.h /usr/include/aarch64-linux-gnu/

# clone the repo
cd ~
git clone --depth 1 https://github.com/opencv/opencv.git
git clone --depth 1 https://github.com/opencv/opencv_contrib.git

# build it
mkdir -p ~/opencv/build
cd ~/opencv/build
cmake -D CMAKE_BUILD_TYPE=RELEASE \
      -D CMAKE_INSTALL_PREFIX=/usr/local \
      -D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib/modules \
      -D OpenBLAS_INCLUDE_DIR=/usr/include/aarch64-linux-gnu \
      -D OpenBLAS_LIB=/usr/lib/aarch64-linux-gnu/libopenblas.so \
      -D WITH_TBB=ON \
      -D WITH_V4L=ON \
      -D WITH_EIGEN=ON \
      -D WITH_OPENBLAS=ON \
      -D WITH_LAPACK=ON \
      -D WITH_GTK=OFF \
      -D BUILD_TESTS=OFF \
      -D BUILD_PERF_TESTS=OFF \
      -D BUILD_EXAMPLES=OFF \
      -D BUILD_opencv_python3=OFF \
      -D BUILD_opencv_python2=OFF \
      -D BUILD_opencv_java=OFF \
      -D OPENCV_ENABLE_NONFREE=ON ..

make -j$(nproc)
sudo make install
sudo ldconfig

# disable the swapfile
sudo swapoff /swapfile
sudo rm /swapfile
