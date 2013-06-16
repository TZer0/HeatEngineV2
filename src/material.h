#ifndef MATERIAL_H
#define MATERIAL_H
#include<vector>
#include<OgrePrerequisites.h>
#include "defines.h"

class Material
{
	
public:
	Material();
	Material(double solid, double liquid, double cap, Ogre::String name);
	virtual ~Material();
	double mTransPoints[3];
	State getState(double h);
	double mCap;
	Ogre::String mName;
};

#endif // MATERIAL_H
