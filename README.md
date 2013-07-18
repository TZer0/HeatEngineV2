HeatEngineV2
=========

This is an engine for simulating heat diffusion in a system.

Controls
====
ESC - quit
A/Q - cycle materials (only during insert material mode)
1/2/3 - Hide/show solid materials/liquid/gas
P - Pause
Right mouse click/Middle mouse click - change camera or look at-position.
Left mouse click - Execute mode action on highlighted block
Mouse wheel - zoom
shift + mouse wheel - change selection depth (used to manipulate the insides of the block)
Mode selection:
W - Insert material mode
S - Move mode
E/D - Heat/cool mode
C - Toggle changable mode (will prevent changes in temperature)


Instructions
====

In the root folder (this folder), you should do this (or corresponding for your system):

ln -S /usr/share/YOUROGREFOLDER OGRE

ln -S /usr/lib/x86(_64)-linux-gnu/OGRE-YOUROGREVERSION OGRElib

Building and running
====

mkdir build

cd build

cmake ..

make -j4

cd ../bin

./HeatEngineV2

OR:

./HeatEngineV2 test.map

(it will only use files located in ../testScenes relative to the bin folder.)
