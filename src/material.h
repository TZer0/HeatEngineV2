#ifndef MATERIAL_H
#define MATERIAL_H
#include<vector>
#include<OgrePrerequisites.h>
#include "defines.h"

class Material
{
	
public:
	Material();
	Material(double solid, double liquid, double cap);
	virtual ~Material();
	std::vector<double> mTransPoints;
	State getState(double h);
	double mCap;
};

#endif // MATERIAL_H
