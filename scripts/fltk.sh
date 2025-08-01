#!/bin/bash

sudo apt update
sudo apt install -y git build-essential cmake libpango1.0-dev libwayland-dev wayland-protocols libxkbcommon-dev libxinerama-dev libdbus-1-dev libglew-dev cmake libdecor-0-dev libgtk-3-dev

cd "${HOME}"
mkdir -p "SI_COMM"
cd "SI_COMM"
git clone https://github.com/fltk/fltk.git
cd fltk
git checkout release-1.4.1
mkdir -p build
cd build
cmake ..
make -j 4
sudo make install
