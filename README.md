[![Build Status](https://travis-ci.org/louis-mclaughlin/libresoccer.svg?branch=master)](https://travis-ci.org/louis-mclaughlin/libresoccer)

A fork of [GameplayFootball](https://github.com/BazkieBumpercar/GameplayFootball)

# Setup

## Linux

### Ubuntu
```
sudo apt install libglu1-mesa-dev libsdl1.2-dev libsdl-image1.2-dev libsdl-net1.2-dev gfx1.2-dev libsdl-ttf2.0-dev libopenal-dev libboost-dev libboost-thread-dev libboost-signals-dev libboost-filesystem-dev

git clone https://github.com/flibitijibibo/libSGE.git
cd libSGE
git checkout 1085c4958397b91a42c399bc389e8fddde8f207b
sudo make install
sudo ln -s /usr/lib64/libSGE.so.0 /usr/lib/libSGE.so.0

cd ..
git clone https://github.com/BazkieBumpercar/Blunted2.git
cd Blunted2
sed -i.bak 's/#define v4sil(x) v2dil((((unsigned long long) (x)) << 32) | (x))/#define v4sil(x) v2dil((((long long) (x)) << 32) | (x))/g' src/libs/fastapprox.h
cmake .
sudo make install

cd ..
git clone https://github.com/louis-mclaughlin/libresoccer.git
cd libresoccer
mkdir build
cd build
wget https://github.com/louis-mclaughlin/libresoccer/releases/download/original/game.zip && unzip game.zip && rm game.zip
cmake ..
make
./gameplayfootball
```
