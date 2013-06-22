#ifndef AREA_H
#define AREA_H
#include "defines.h"

class Area {
	
public:
	Area();
	Area(int mat, double H);
	int mMat;
	State mState;
	bool mSelected, mHover, mLinks[3], mLinksTmp[3], mSource;
	double dH[2];
	virtual ~Area();
	void swap(Area *area);
	void initCommon();
};

#endif // AREA_H
