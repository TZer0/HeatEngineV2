#ifndef MATERIAL_H
#define MATERIAL_H
#include<vector>
#include<OgrePrerequisites.h>
#include "defines.h"

class Material
{
	
public:
	Material();
	Material(double solid, double liquid, std::vector<std::pair<double, double>> cap, 
		std::vector<std::pair<double, double>> dens, std::vector<std::pair<double, double>> cond, 
		Ogre::String name);
	virtual ~Material();
	double mTransPoints[3];
	State getState(double h);
	std::vector<std::vector<std::pair<double, double>>> mProperties; // in order: capacity, density, conductivity
	std::tuple<double, double, double> getProperties(double h); // in order: capacity, density, conductivity
	Ogre::String mName;
	bool mTempSensitive;
	bool getTempSensitive() { return mTempSensitive; }
	void updateTempSensitive();
};

#endif // MATERIAL_H
