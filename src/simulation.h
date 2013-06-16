#ifndef SIMULATION_H
#define SIMULATION_H
#include<vector>
#include<OgrePrerequisites.h>
#include<OgreVector3.h>
#include<boost/tuple/tuple.hpp>
#include "area.h"
#include "material.h"

class CommonData {
public:
	int xSize, ySize, zSize, curMat, lastX, lastY, lastZ;
	bool latest, click;
	std::vector<Material> materials;
	SimTool tool;
	Ogre::Real time;
	Area ****area;
	bool withinArea(Ogre::Vector3 pos) {
		return withinArea(std::floor(pos.x), std::floor(pos.y), std::floor(pos.z));
	}
	bool withinArea(int x, int y, int z) {
		return 0 <= x && x < xSize && 0 <= y && y < ySize && 0 <= z && z < zSize;
	}
};


class Simulation
{
	
public:
	Simulation();
	Simulation(int x, int y, int z);
	virtual ~Simulation();
	void tick(Ogre::Real dt);
	CommonData *getData() { return &mData; }
	void injectDepthAndMouse(int depth, Ogre::Vector3 camPos, Ogre::Vector3 dir);
	void click(bool state);
	void changeMaterial(int dM) { mData.curMat = std::max(std::min((int)mData.materials.size()-1, mData.curMat+dM), 0); }
	void setTool(SimTool tool) { mData.tool = tool; }
private:
	void moveObjectIter(int fx, int fy, int fz, int tx, int ty, int tz);
	void moveObject(int fx, int fy, int fz, int tx, int ty, int tz);
	void fill(std::vector<std::tuple<int, int, int>> &selection, int x, int y, int z, int mat);
	void handleMouseState(Ogre::Real dt);
	void commonInit();
	void freeSimArea();
	void initSimArea();
	CommonData mData;
};

#endif // SIMULATION_H
