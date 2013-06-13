#include "material.h"

Material::Material()
{
	mTransPoints.push_back(-100);
	mTransPoints.push_back(-90);
	mCap = 1;
}

Material::Material(double solid, double liquid, double cap)
{
	mTransPoints.push_back(solid);
	mTransPoints.push_back(liquid);
	mCap = cap;
}

State Material::getState(double h)
{
	for (uint i = 0; i < mTransPoints.size(); i++) {
		if (h < mTransPoints[i]) {
			return (State) i;
		}
	}
	return GAS;
}

Material::~Material()
{

}

