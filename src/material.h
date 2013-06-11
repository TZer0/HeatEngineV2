#ifndef MATERIAL_H
#define MATERIAL_H

class Material
{
	
public:
	Material();
	Material(double solid, double liquid, double cap);
	virtual ~Material();
	double mSolid, mLiquid, mCap;
};

#endif // MATERIAL_H
