#include "simulation.h"

Simulation::Simulation()
{
	mData.xSize = mData.ySize = mData.zSize = 10;
	mData.latest = 0;
	mData.materials.push_back(Material());
	mData.materials.push_back(Material(273.15, 373.15, 10));
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
	mData.lastX = mData.lastY = mData.lastZ = -1;
	mData.latest = mData.click = false;
	mData.curMat = 0;
	mData.tool = INSERTMATERIAL;
}

void Simulation::tick(Ogre::Real dt)
{
	mData.latest = !mData.latest;
	handleMouseState(dt);
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

void Simulation::handleMouseState(Ogre::Real dt)
{
	if (!mData.withinArea(mData.lastX, mData.lastY, mData.lastZ) || !mData.click) {
		return;
	}
	Area *area = mData.area[mData.lastX][mData.lastY][mData.lastZ];
	if (mData.tool == HEAT) {
		area->dH[mData.latest] += 100*dt;
	} else if (mData.tool == COOL) {
		area->dH[mData.latest] = std::max(0., area->dH[mData.latest] - 100*dt);
		std::cout << area->dH[mData.latest] << std::endl;
	} else if (mData.tool == INSERTMATERIAL) {
		if (area->mMat != mData.curMat) {
			area->mMat = mData.curMat;
			area->dH[mData.latest] = DEFAULTTEMP;
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

void Simulation::injectDepthAndMouse(int depth, Ogre::Vector3 camPos, Ogre::Vector3 dir)
{
	bool lastWithinArea = false;
	if (mData.withinArea(mData.lastX, mData.lastY, mData.lastZ)) {
		mData.area[mData.lastX][mData.lastY][mData.lastZ]->mHover = false;
		lastWithinArea = true;
	}
	int startDepth = depth;
	int steps = FARCLIP-CLOSECLIP+200;
	int newX, newY, newZ;
	newX = newY = newZ = -1;
	while (steps > 0) {
		Ogre::Vector3 simPos = camPos/TILESIZE;
		int curX = std::floor(simPos.x), curY = std::floor(simPos.y), curZ = std::floor(simPos.z);
		bool withinArea = mData.withinArea(simPos);
		if (withinArea && (curX != newX || curY != newY || curZ != newZ)) {
			depth--;
		}
		camPos += dir;
		steps--;
		if (!withinArea && depth != startDepth) 
		{
			break;
		}
		newX = curX; newY = curY; newZ = curZ;
		if (depth == 0) {
			break;
		}
	}
	if (!mData.withinArea(newX, newY, newZ)) {
		mData.lastX = mData.lastY = mData.lastZ = -1;
		return;
	}
	mData.area[newX][newY][newZ]->mHover = true;
	if (mData.click && mData.tool == MOVE && lastWithinArea) {
		moveObject(mData.lastX, mData.lastY, mData.lastZ, newX, newY, newZ);
	}
	mData.lastX = newX; mData.lastY = newY; mData.lastZ = newZ;
}

void Simulation::moveObject(int fx, int fy, int fz, int tx, int ty, int tz)
{
	int diff[3];
	diff[0] = tx-fx; diff[1] = ty-fy; diff[2] = tz-fz;
	int curPos[3];
	curPos[0] = fx; curPos[1] = fy; curPos[2] = fz;
	for (int i = 0; i < 3; i++) {
		while (diff[i] != 0) {
			int step = 1-2*(diff[i] > 0);
			moveObjectIter(curPos[0], curPos[1], curPos[2], curPos[0]-step*(i==0), curPos[1]-step*(i==1), curPos[2]-step*(i==2));
			curPos[i]-=step;
			diff[i]+=step;
		}
	}
}
void Simulation::moveObjectIter(int fx, int fy, int fz, int tx, int ty, int tz)
{
	// Fetch selected blocks
	std::vector<std::tuple<int, int, int>> selection;
	fill(selection, fx, fy, fz, mData.area[fx][fy][fz]->mMat);
	int dx, dy, dz;
	dx = tx-fx; dy = ty-fy; dz = tz-fz;
	
	// Check if movement is possible
	for (auto itr = selection.begin(); itr != selection.end(); itr++) {
		int x = std::get<0>(*itr) + dx;
		int y = std::get<1>(*itr) + dy;
		int z = std::get<2>(*itr) + dz;
		if (!mData.withinArea(x, y, z)) {
			return;
		}
	}
	
	// Execute movement
	for (auto itr = selection.begin(); itr != selection.end(); itr++) {
		int x = std::get<0>(*itr);
		int y = std::get<1>(*itr);
		int z = std::get<2>(*itr);
		Area *tmp = mData.area[x][y][z];
		mData.area[x][y][z] = mData.area[x+dx][y+dy][z+dz];
		mData.area[x+dx][y+dy][z+dz] = tmp;
	}
}

void Simulation::fill(std::vector< std::tuple< int, int, int > > &selection, int x, int y, int z, int mat)
{
	if (mData.area[x][y][z]->mState == GAS || mData.area[x][y][z]->mState == LIQUID) {
		return;
	}
	auto pos = std::tuple<int, int, int>(x,y,z);
	for (auto itr = selection.begin(); itr != selection.end(); itr++) {
		if (*itr == pos) {
			return;
		}
	}
	selection.push_back(pos);
	for (int dx = -1; dx < 2; dx++) {
		for (int dy = -1; dy < 2; dy++) {
			for (int dz = -1; dz < 2; dz++) {
				if ((dx == 0) + (dy == 0) + (dz == 0) != 2) {
					continue;
				}
				if (mData.withinArea(x+dx, y+dy, z+dz)) {
					fill(selection, x+dx, y+dy, z+dz, mat);
				}
			}
		}
	}
}

void Simulation::click(bool state)
{
	mData.click = state;
}


Simulation::~Simulation()
{

}

