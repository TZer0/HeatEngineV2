#ifndef SIMULATION_H
#define SIMULATION_H
#include<vector>
#include<OgrePrerequisites.h>
#include "area.h"
#include "material.h"

class RenderData {
public:
	int xSize, ySize, zSize;
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
	std::vector<Material> materials;
	RenderData *getData() { return &mData; }
private:
	void freeSimArea();
	void initSimArea();
	RenderData mData;
};

#endif // SIMULATION_H
