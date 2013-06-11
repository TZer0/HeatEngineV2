#ifndef AREA_H
#define AREA_H

class Area {
	
public:
	Area();
	Area(int mat, double H);
	int mMat;
	double dH[2];
	virtual ~Area();
};

#endif // AREA_H
