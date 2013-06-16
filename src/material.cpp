#include "material.h"

Material::Material()
{
	mTransPoints[0] = 172;
	mTransPoints[1] = 182;
	mTransPoints[2] = mTransPoints[1] + 400;
	mCap = 1;
	mName = "Air";
}

Material::Material(double solid, double liquid, double cap, Ogre::String name)
{
	mTransPoints[0] = solid;
	mTransPoints[1] = liquid;
	mTransPoints[2] = mTransPoints[1] + 400;
	mCap = cap;
	mName = name;
}

State Material::getState(double h)
{
	for (uint i = 0; i < 2; i++) {
		if (h < mTransPoints[i]) {
			return (State) i;
		}
	}
	return GAS;
}

Material::~Material()
{

}

