#include "area.h"

Area::Area()
{
	dH[0] = dH[1] = 10;
	mMat = 0;
	mState = UNDEFINED;
	mHover = mSelected = false;
}

Area::Area(int mat, double H)
{
	mMat = mat;
	dH[0] = dH[1] = H;
	mState = UNDEFINED;
	mHover = mSelected = false;
}

Area::~Area()
{

}

