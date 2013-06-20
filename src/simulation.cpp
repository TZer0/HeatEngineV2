#include "simulation.h"


std::array<std::tuple<int, int, int>, 6> relativePositions = {{
	std::tuple<int, int, int>(1,0,0),
	std::tuple<int, int, int>(0,1,0),
	std::tuple<int, int, int>(0,0,1),
	std::tuple<int, int, int>(-1,0,0),
	std::tuple<int, int, int>(0,-1,0),
	std::tuple<int, int, int>(0,0,-1),
}};

Simulation::Simulation()
{
	mData.xSize = mData.ySize = mData.zSize = 10;
	mData.latest = 0;
	mData.materials.push_back(Material());
	mData.materials.push_back(Material(273.15, 373.15, 10, "Water"));
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
	mData.time = 0;
	mData.lastX = mData.lastY = mData.lastZ = -1;
	mData.latest = mData.click = false;
	mData.curMat = 0;
	mData.tool = INSERTMATERIAL;
}

void Simulation::tick(Ogre::Real dt)
{
	mData.time += dt;
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
				State s = ar->mState;
				ar->mState = m.getState(ar->dH[mData.latest]);
				if (s != ar->mState && ar->mState == SOLID) {
					updateLinks(x, y, z);
				} else if (s != ar->mState) {
					updateLinks(x, y, z, false);
				}
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
	Area *ar = mData.area[fx][fy][fz];
	if (ar->mState == SOLID) {
		fill(selection, fx, fy, fz, mData.area[fx][fy][fz]->mMat);
	} else if (ar->mState == LIQUID) {
		selection.push_back(std::tuple<int, int, int>(fx, fy, fz));
	} else {
		return;
	}
	int dx, dy, dz, mat;
	dx = tx-fx; dy = ty-fy; dz = tz-fz;
	mat = ar->mMat;
	// Check if movement is possible
	for (auto itr = selection.begin(); itr != selection.end(); itr++) {
		int x = std::get<0>(*itr) + dx;
		int y = std::get<1>(*itr) + dy;
		int z = std::get<2>(*itr) + dz;
		if (!mData.withinArea(x, y, z)) {
			return;
		}
	}
	
	// Remove material where it was, store temperature and links in the temporary variables.
	for (auto itr = selection.begin(); itr != selection.end(); itr++) {
		int x = std::get<0>(*itr);
		int y = std::get<1>(*itr);
		int z = std::get<2>(*itr);
		Area *from = mData.area[x][y][z];
		from->mMat = 0;
		from->dH[mData.latest] = from->dH[!mData.latest];
		from->dH[!mData.latest] = DEFAULTTEMP;
		for (int i = 0; i < 3; i++) {
			from->mLinksTmp[i] = from->mLinks[i];
			from->mLinks[i] = false;
		}
	}
	
	// Execute movement
	for (auto itr = selection.begin(); itr != selection.end(); itr++) {
		int x = std::get<0>(*itr);
		int y = std::get<1>(*itr);
		int z = std::get<2>(*itr);
		Area *from = mData.area[x][y][z];
		Area *to = mData.area[x+dx][y+dy][z+dz];
		to->mMat = mat;
		to->dH[!mData.latest] = from->dH[mData.latest];
		to->mState = from->mState;
		from->mState = mData.materials.at(0).getState(DEFAULTTEMP);
		for (int i = 0; i < 3; i++) {
			to->mLinks[i] = from->mLinksTmp[i];
		}
	}
}

void Simulation::fill(std::vector< std::tuple< int, int, int > > &selection, int x, int y, int z, int mat)
{
	if (mData.area[x][y][z]->mState == GAS || mData.area[x][y][z]->mState == LIQUID) {
		return;
	}
	Area *ar = mData.area[x][y][z];
	auto pos = std::tuple<int, int, int>(x,y,z);
	for (auto itr = selection.begin(); itr != selection.end(); itr++) {
		if (*itr == pos) {
			return;
		}
	}
	selection.push_back(pos);
	for (int i = 0; i < 3; i++) {
		std::tuple<int, int, int> rel = relativePositions[i];
		int tx = x + std::get<0>(rel);
		int ty = y + std::get<1>(rel);
		int tz = z + std::get<2>(rel);
		if (!mData.withinArea(tx, ty, tz)) {
			continue;
		}
		if (mData.withinArea(tx, ty, tz) && ar->mLinks[i]) {
			fill(selection, tx, ty, tz, mat);
		}
	}
	
	for (int i = 0; i < 3; i++) {
		std::tuple<int, int, int> rel = relativePositions[i+3];
		int tx = x + std::get<0>(rel);
		int ty = y + std::get<1>(rel);
		int tz = z + std::get<2>(rel);
		if (!mData.withinArea(tx, ty, tz)) {
			continue;
		}
		Area *otherAr = mData.area[tx][ty][tz];
		if (otherAr->mLinks[i]) {
			fill(selection, tx, ty, tz, mat);
		}
	}
}

void Simulation::updateLinks(int x, int y, int z, bool freeze)
{
	Area *ar = mData.area[x][y][z];
	for (int i = 0; i < 3; i++) {
		std::tuple<int, int, int> rel = relativePositions[i];
		int tx = x + std::get<0>(rel);
		int ty = y + std::get<1>(rel);
		int tz = z + std::get<2>(rel);
		if (!mData.withinArea(tx, ty, tz)) {
			continue;
		}
		Area *otherAr = mData.area[tx][ty][tz];
		if (freeze && (ar->mState == otherAr->mState && ar->mMat == otherAr->mMat)) {
			ar->mLinks[i] = true;
		} else {
			ar->mLinks[i] = false;
		}
	}
	
	for (int i = 0; i < 3; i++) {
		std::tuple<int, int, int> rel = relativePositions[i+3];
		int tx = x + std::get<0>(rel);
		int ty = y + std::get<1>(rel);
		int tz = z + std::get<2>(rel);
		if (!mData.withinArea(tx, ty, tz)) {
			continue;
		}
		Area *otherAr = mData.area[tx][ty][tz];
		if (freeze && (ar->mState == otherAr->mState && ar->mMat == otherAr->mMat)) {
			otherAr->mLinks[i] = true;
		} else {
			otherAr->mLinks[i] = false;
		}
	}
}

void Simulation::click(bool state)
{
	mData.click = state;
}

void Simulation::insertMaterialBlock(int fx, int fy, int fz, int tx, int ty, int tz, int mat, double temp)
{
	for (int x = fx; x <= tx; x++) {
		for (int y = fy; y <= ty; y++) {
			for (int z = fz; z <= tz; z++) {
				if (!mData.withinArea(x, y, z)) {
					continue;
				}
				
				Area *ar = mData.area[x][y][z];
				ar->dH[0] = ar->dH[1] = temp;
				ar->mMat = mat;
			}
		}
	}
}


Simulation::~Simulation()
{

}

