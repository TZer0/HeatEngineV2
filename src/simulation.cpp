#include "simulation.h"

Simulation::Simulation()
{
	mData.xSize = mData.ySize = mData.zSize = 10;
	mData.latest = 0;
	mData.materials.push_back(Material());
	mData.materials.push_back(Material(0, 100, 10));
	commonInit();
}

Simulation::Simulation(int x, int y, int z)
{
	mData.xSize = x;
	mData.ySize = y;
	mData.zSize = z;
	commonInit();
}

void Simulation::commonInit()
{
	initSimArea();
	mData.latest = mData.click = false;
	mData.curMat = 0;
	mData.tool = INSERTMATERIAL;
}

void Simulation::tick(Ogre::Real dt)
{
	mData.latest = !mData.latest;
	handleMouseState();
	bool ind = mData.latest;
	for (int x = 0; x < mData.xSize; x++) {
		for (int y = 0; y < mData.ySize; y++) {
			for (int z = 0; z < mData.zSize; z++) {
				mData.area[x][y][z]->dH[!ind] = mData.area[x][y][z]->dH[ind];//+(x+y+z-mData.xSize*3)*0.1;
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

void Simulation::handleMouseState()
{
	if (mPreviousHover == nullptr || !mData.click) {
		return;
	}
	if (mData.tool == HEAT) {
		mPreviousHover->dH[mData.latest] += 5;
		std::cout << mPreviousHover->dH[mData.latest] << std::endl;
	} else if (mData.tool == COOL) {
		mPreviousHover->dH[mData.latest] -= 5;
		std::cout << mPreviousHover->dH[mData.latest] << std::endl;
	} else if (mData.tool == INSERTMATERIAL) {
		if (mPreviousHover->mMat != mData.curMat) {
			mPreviousHover->mMat = mData.curMat;
			mPreviousHover->dH[mData.latest] = DEFAULTTEMP;
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
	if (mPreviousHover != nullptr) {
		mPreviousHover->mHover = false;
	}
	int startDepth = depth;
	int steps = FARCLIP-CLOSECLIP+200;
	int prevX, prevY, prevZ;
	prevX = prevY = prevZ = -1;
	while (steps > 0) {
		Ogre::Vector3 simPos = camPos/TILESIZE + 1;
		int curX = simPos.x, curY = simPos.y, curZ = simPos.z;
		bool withinArea = mData.withinArea(simPos);
		if (withinArea && (curX != prevX || curY != prevY || curZ != prevZ)) {
			depth--;
		}
		camPos += dir;
		steps--;
		if (!withinArea && depth != startDepth) 
		{
			break;
		}
		prevX = curX; prevY = curY; prevZ = curZ;
		if (depth == 0) {
			break;
		}
	}
	if (!mData.withinArea(prevX, prevY, prevZ)) {
		mPreviousHover = nullptr;
		return;
	}
	std::cout << prevX << " " << prevY << " " << prevZ << std::endl;
	mPreviousHover = mData.area[prevX][prevY][prevZ];
	mPreviousHover->mHover = true;
}

void Simulation::click(bool state)
{
	mData.click = state;
}


Simulation::~Simulation()
{

}

