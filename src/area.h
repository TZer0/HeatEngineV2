#ifndef AREA_H
#define AREA_H
#include "defines.h"

class Area {
	
public:
	Area();
	Area(int mat, double H);
	int mMat;
	State mState;
	double dH[2];
	virtual ~Area();
};

#endif // AREA_H
