#include "simulation.h"

Simulation::Simulation()
{
	mData.xSize = mData.ySize = mData.zSize = 3;
	mData.latest = 0;
	initSimArea();
	mData.materials.push_back(Material());
	mData.materials.push_back(Material(0, 100, 10));
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
	mData.latest = !mData.latest;
	bool ind = mData.latest;
	for (int x = 0; x < mData.xSize; x++) {
		for (int y = 0; y < mData.ySize; y++) {
			for (int z = 0; z < mData.zSize; z++) {
				mData.area[x][y][z]->dH[!ind] = mData.area[x][y][z]->dH[ind]+(x+y+z-9)*0.1;
			}
		}
	}
	for (int x = 0; x < mData.xSize; x++) {
		for (int y = 0; y < mData.ySize; y++) {
			for (int z = 0; z < mData.zSize; z++) {
				Material m = mData.materials.at(mData.area[x][y][z]->mMat);
				mData.area[x][y][z]->mState = m.getState(mData.area[x][y][z]->dH[mData.latest]);
			}
		}
	}
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

