#include "simulation.h"

Simulation::Simulation()
{
	mData.xSize = mData.ySize = mData.zSize = 3;
	initSimArea();
	materials.push_back(Material());
	materials.push_back(Material(0, 100, 10));
}

Simulation::Simulation(int x, int y, int z)
{
	mData.xSize = x;
	mData.ySize = y;
	mData.zSize = z;
	initSimArea();
}

void Simulation::tick(Ogre::Real dt)
{

}


void Simulation::initSimArea()
{
	mData.area = new Area***[mData.xSize];
	for (int x = 0; x < mData.xSize; x++) {
		mData.area[x] = new Area**[mData.ySize];
		for (int y = 0; y < mData.ySize; y++) {
			mData.area[x][y] = new Area*[mData.zSize];
			for (int z = 0; z < mData.zSize; z++) {
				mData.area[x][y][z] = new Area();
			}
		}
	}
}

void Simulation::freeSimArea()
{
	for (int x = 0; x < mData.xSize; x++) {
		for (int y = 0; y < mData.ySize; y++) {
			for (int z = 0; z < mData.zSize; z++) {
				delete mData.area[x][y][z];
			}
			delete mData.area[x][y];
		}
		delete mData.area[x];
	}
	delete mData.area;
}


Simulation::~Simulation()
{

}

