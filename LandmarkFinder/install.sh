#!/bin/bash

sudo cp -rp ./OpenFace ~/src/
cd ~/src/OpenFace/build
make FaceLandmarkImg
sudo make install
