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
	std::vector<std::pair<double, double>> mCap, mDens, mCond;
	mCap.push_back(std::pair<double, double>(273.15, 420));
	mDens.push_back(std::pair<double, double>(273.15, 1));
	mCond.push_back(std::pair<double, double>(273.15, 0.58));
	mData.materials.push_back(Material(273.15, 373.15, mCap, mDens, mCond, "Water"));
	mData.spacing = 0.1;
	commonInit();
}

Simulation::Simulation(int x, int y, int z, double spacing)
{
	mData.xSize = x;
	mData.ySize = y;
	mData.zSize = z;
	mData.spacing = spacing;
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

void Simulation::tick(Ogre::Real simDt, Ogre::Real actualDt, bool pause)
{
	handleMouseState(actualDt);
	if (!pause) {
		double dS = mData.spacing;
		mData.latest = !mData.latest;
		bool ind = mData.latest;
		mData.time += simDt;
		double timeSpace = simDt/(dS*dS);
		for (int x = 0; x < mData.xSize; x++) {
			for (int y = 0; y < mData.ySize; y++) {
				for (int z = 0; z < mData.zSize; z++) {
					Area *ar = mData.area[x][y][z];
					if (ar->mSource) {
						continue;
					}
					std::tuple<double, double, double> props = ar->mProps;
					double timeSpaceHeat = timeSpace * std::get<2>(props)/(std::get<0>(props) * std::get<1>(props));
					int relUsed = 0;
					ar->mH[ind] = ar->mH[!ind];
					for (auto itr = relativePositions.begin(); itr != relativePositions.end(); itr++) {
						int tx = std::get<0>(*itr) + x;
						int ty = std::get<1>(*itr) + y;
						int tz = std::get<2>(*itr) + z;
						if (!mData.withinArea(tx, ty, tz)) {
							continue;
						}
						ar->mH[ind] += timeSpaceHeat*mData.area[tx][ty][tz]->mH[!ind];
						relUsed++;
					}
					ar->mH[ind] -= timeSpaceHeat*relUsed*ar->mH[!ind];
				}
			}
		}
	}
	updateStatesAndLinks();
}

void Simulation::updateStatesAndLinks(bool forcePropertyUpdate)
{
	for (int x = 0; x < mData.xSize; x++) {
		for (int y = 0; y < mData.ySize; y++) {
			for (int z = 0; z < mData.zSize; z++) {
				Area *ar = mData.area[x][y][z];
				State s = ar->mState;
				Material *m = &mData.materials.at(ar->mMat);
				ar->mState = m->getState(ar->mH[mData.latest]);
				if (s != ar->mState && ar->mState == SOLID) {
					updateLinks(x, y, z);
				} else if (s != ar->mState) {
					updateLinks(x, y, z, false);
				}
				if (m->getTempSensitive() || forcePropertyUpdate) {
					ar->mProps = m->getProperties(ar->mH[mData.latest]);
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
	bool ind = mData.latest;
	Area *area = mData.area[mData.lastX][mData.lastY][mData.lastZ];
	if (mData.tool == HEAT) {
		area->mH[ind] += 100*dt;
	} else if (mData.tool == COOL) {
		area->mH[ind] = std::max(0., area->mH[ind] - 100*dt);
	} else if (mData.tool == INSERTMATERIAL) {
		if (area->mMat != mData.curMat) {
			area->mMat = mData.curMat;
			area->mH[ind] = DEFAULTTEMP;
		}
	}
	if (area->mSource) {
		area->mH[!ind] = area->mH[ind];
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
	// Fetch selected blocks
	std::vector<std::tuple<int, int, int>> selection;
	Area *ar = mData.area[fx][fy][fz];
	if (ar->mState == SOLID) {
		fill(selection, fx, fy, fz, mData.area[fx][fy][fz]->mMat);
	} else if (ar->mState == LIQUID) {
		selection.push_back(std::tuple<int, int, int>(fx, fy, fz));
		ar->mSelected = true;
	} else {
		return;
	}
	int dx, dy, dz, mat;
	dx = tx-fx; dy = ty-fy; dz = tz-fz;
	mat = ar->mMat;
	bool ind = mData.latest;
	
	// Check if movement is possible
	for (auto itr = selection.begin(); itr != selection.end(); itr++) {
		int x = std::get<0>(*itr) + dx;
		int y = std::get<1>(*itr) + dy;
		int z = std::get<2>(*itr) + dz;
		if (!mData.withinArea(x, y, z)) {
			deselect(selection);
			return;
		}
		
		Area *ar = mData.area[x][y][z];
		if (!ar->mSelected && ar->mState != GAS) {
			deselect(selection);
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
		from->mH[!ind] = from->mH[ind];
		from->mH[ind] = DEFAULTTEMP;
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
		to->mH[ind] = from->mH[!ind];
		to->mState = from->mState;
		from->mState = mData.materials.at(0).getState(DEFAULTTEMP);
		for (int i = 0; i < 3; i++) {
			to->mLinks[i] = from->mLinksTmp[i];
		}
	}
	deselect(selection);
	
	for (int x = 0; x < mData.xSize; x++) {
		for (int y = 0; y < mData.ySize; y++) {
			for (int z = 0; z < mData.zSize; z++) {
				mData.area[x][y][z]->mH[!ind] = mData.area[x][y][z]->mH[ind];
			}
		}
	}
}

void Simulation::deselect(std::vector< std::tuple< int, int, int > >& selection)
{
	for (auto itr = selection.begin(); itr != selection.end(); itr++) {
		int x = std::get<0>(*itr);
		int y = std::get<1>(*itr);
		int z = std::get<2>(*itr);
		mData.area[x][y][z]->mSelected = false;
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
	ar->mSelected = true;
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
	if (mData.tool == TOGGLESOURCE && state) {
		mData.click = false;
		int x = mData.lastX; int y = mData.lastY; int z = mData.lastZ;
		if (mData.withinArea(x, y, z)) {
			Area *ar = mData.area[x][y][z];
			ar->mSource = !ar->mSource;
			ar->mH[!mData.latest] = ar->mH[mData.latest];
			if (ar->mSource) {
				updateLinks(x, y, z, false);
			} else {
				updateLinks(x, y, z, true);
			}
		}
	}
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
				ar->mH[0] = ar->mH[1] = temp;
				ar->mMat = mat;
			}
		}
	}
}


Simulation::~Simulation()
{

}

