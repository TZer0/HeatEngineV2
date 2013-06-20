#include "area.h"

Area::Area()
{
	dH[0] = dH[1] = DEFAULTTEMP;
	mMat = 0;
	mState = UNDEFINED;
	mHover = mSelected = false;
	for (int i = 0; i < 3; i++) {
		mLinks[i] = false;
	}
}

Area::Area(int mat, double H)
{
	dH[0] = dH[1] = H;
	mMat = mat;
	mState = UNDEFINED;
	mHover = mSelected = false;
	for (int i = 0; i < 3; i++) {
		mLinks[i] = false;
	}
}

Area::~Area()
{

}