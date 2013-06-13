#ifndef SIMULATION_H
#define SIMULATION_H
#include<vector>
#include<OgrePrerequisites.h>
#include<OgreVector3.h>
#include "area.h"
#include "material.h"

class RenderData {
public:
	int xSize, ySize, zSize;
	bool latest;
	std::vector<Material> materials;
	Area ****area;
	bool withinArea(int x, int y, int z) {
		return 0 <= x && x < xSize && 0 <= y && y < ySize && 0 <= z && z <= zSize;
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
	void click(bool shift);
	Area *mPreviousHover;
private:
	void freeSimArea();
	void initSimArea();
	RenderData mData;
};

#endif // SIMULATION_H
