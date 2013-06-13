#include "simulation.h"

Simulation::Simulation()
{
	mData.xSize = mData.ySize = mData.zSize = 10;
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
				mData.area[x][y][z]->dH[!ind] = mData.area[x][y][z]->dH[ind]+(x+y+z-mData.xSize*3)*0.1;
			}
		}
	}
	for (int x = 0; x < mData.xSize; x++) {
		for (int y = 0; y < mData.ySize; y++) {
			for (int z = 0; z < mData.zSize; z++) {
				Area *ar = mData.area[x][y][z];
				Material m = mData.materials.at(ar->mMat);
				ar->mState = m.getState(ar->dH[mData.latest]);
			}
		}
	}
}


void Simulation::initSimArea()
{
	mPreviousHover = nullptr;
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

void Simulation::injectDepthAndMouse(int depth, Ogre::Vector3 camPos, Ogre::Vector3 dir)
{
	std::cout << depth << " " << camPos << " " << dir << std::endl;
	if (mPreviousHover != nullptr) {
		mPreviousHover->mHover = false;
	}
}

void Simulation::click(bool shift)
{
	
}


Simulation::~Simulation()
{

}

