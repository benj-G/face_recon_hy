#!/bin/bash
sudo -v

# install packages
sudo apt-get update
sudo apt-get -y upgrade
sudo apt-get -y install build-essential cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev libboost-all-dev libtbb-dev libopenblas-dev libeigen3-dev default-jdk ant libpq-dev postgresql-server-dev-all

# install opencv
sudo -v
cd ~
mkdir src
cd src
wget https://github.com/Itseez/opencv/archive/3.1.0.zip -O opencv-3.1.0.zip
unzip opencv-3.1.0.zip
rm opencv-3.1.0.zip
cd opencv-3.1.0
mkdir build
cd build
cmake -D WITH_TBB=ON ..
make -j4
sudo make install
# delete build directory because it's huge and no longer needed
cd ..
rm -rf build

# install dlib
sudo -v
cd ~/src
wget http://dlib.net/files/dlib-19.1.tar.bz2
bunzip2 dlib-19.1.tar.bz2
tar xf dlib-19.1.tar
rm dlib-19.1.tar
cd dlib-19.1
mkdir build
cd build
cmake ..
make
sudo make install

# install openface 
sudo -v
cd ~/src
git clone https://github.com/TadasBaltrusaitis/OpenFace.git
cd OpenFace
mkdir build
cd build
cmake ..
make
sudo make install
