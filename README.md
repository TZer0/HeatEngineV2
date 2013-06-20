HeatEngineV2
=========

This is an engine for simulating diffusion in a system.


Instructions:

In the root folder (this folder), you should do this (or corresponding for your system):

ln -S /usr/share/YOUROGREFOLDER OGRE

ln -S /usr/lib/x86(_64)-linux-gnu/OGRE-YOUROGREVERSION OGRElib

Building and running:

mkdir build

cd build

cmake ..

make -j4

cd ../bin

./HeatEngineV2

OR:

./HeatEngineV2 test.map

