#include "area.h"

Area::Area()
{
	dH[0] = dH[1] = DEFAULTTEMP;
	mMat = 0;
	mState = UNDEFINED;
	mHover = mSelected = false;
}

Area::Area(int mat, double H)
{
	dH[0] = dH[1] = H;
	mMat = mat;
	mState = UNDEFINED;
	mHover = mSelected = false;
}

Area::~Area()
{

}