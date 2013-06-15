#ifndef SIMULATION_H
#define SIMULATION_H
#include<vector>
#include<OgrePrerequisites.h>
#include<OgreVector3.h>
#include "area.h"
#include "material.h"

class RenderData {
public:
	int xSize, ySize, zSize, curMat;
	bool latest, click;
	std::vector<Material> materials;
	SimTool tool;
	Area ****area;
	bool withinArea(Ogre::Vector3 pos) {
		return withinArea(pos.x, pos.y, pos.z);
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
	RenderData *getData() { return &mData; }
	void injectDepthAndMouse(int depth, Ogre::Vector3 camPos, Ogre::Vector3 dir);
	void click(bool state);
	Area *mPreviousHover;
	void changeMaterial(int dM) { mData.curMat = std::max(std::min((int)mData.materials.size()-1, mData.curMat+dM), 0); }
	void setTool(SimTool tool) { mData.tool = tool; }
private:
	void handleMouseState();
	void commonInit();
	void freeSimArea();
	void initSimArea();
	RenderData mData;
};

#endif // SIMULATION_H
