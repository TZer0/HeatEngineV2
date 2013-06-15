#ifndef AREA_H
#define AREA_H
#include "defines.h"

class Area {
	
public:
	Area();
	Area(int mat, double H);
	int mMat;
	State mState;
	bool mSelected, mHover;
	double dH[2];
	virtual ~Area();
	void swap(Area *area);
};

#endif // AREA_H
