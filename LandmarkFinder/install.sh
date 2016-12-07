#!/bin/bash

sudo cp -rp ./OpenFace ~/src/OpenFace
cd ~/src/OpenFace/build
make FaceLandmarkImg
sudo make install
