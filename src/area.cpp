#include "area.h"

Area::Area()
{
	mH[0] = mH[1] = DEFAULTTEMP;
	mMat = 0;
	mHover = mSelected = false;
	initCommon();
}

Area::Area(int mat, double H)
{
	mH[0] = mH[1] = H;
	mMat = mat;
	mHover = mSelected = false;
	initCommon();
}

void Area::initCommon()
{
	mState = UNDEFINED;
	for (int i = 0; i < 3; i++) {
		mLinksTmp[i] = mLinks[i] = false;
	}
}


Area::~Area()
{

}