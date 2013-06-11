#include "material.h"

Material::Material()
{
	mSolid = -100;
	mLiquid = -90;
	mCap = 1;
}

Material::Material(double solid, double liquid, double cap)
{
	mSolid = solid;
	mLiquid = liquid;
	mCap = cap;
}

Material::~Material()
{

}

