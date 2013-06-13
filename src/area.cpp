#include "area.h"

Area::Area()
{
	dH[0] = dH[1] = 10;
	mMat = 0;
	mState = UNDEFINED;
}

Area::Area(int mat, double H)
{
	mMat = mat;
	dH[0] = dH[1] = H;
	mState = UNDEFINED;
}

Area::~Area()
{

}

