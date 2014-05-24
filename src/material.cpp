#include "material.h"

Material::Material()
{
	mTransPoints[0] = 172;
	mTransPoints[1] = 182;
	mTransPoints[2] = mTransPoints[1] + 400;
	std::vector<std::pair<double, double>> capacity, density, conductivity;
	capacity.push_back(std::pair<double, double>(0, 1009));
	conductivity.push_back(std::pair<double, double>(0, 0.0204));
	density.push_back(std::pair<double, double>(0, 1.29));
	mProperties.push_back(capacity);
	mProperties.push_back(density);
	mProperties.push_back(conductivity);
	mName = "Air";
	updateTempSensitive();
}

Material::Material(double solid, double liquid, std::vector<std::pair<double, double>> cap, std::vector<std::pair<double, double>> dens, 
		   std::vector<std::pair<double, double>> conduct, Ogre::String name)
{
	mTransPoints[0] = solid;
	mTransPoints[1] = liquid;
	mTransPoints[2] = mTransPoints[1] + 400;
	mProperties.push_back(cap);
	mProperties.push_back(dens);
	mProperties.push_back(conduct);
	mName = name;
	updateTempSensitive();
}

void Material::updateTempSensitive()
{
	mTempSensitive = false;
	for (uint i = 0; i < 3; i++) {
		if (mProperties[i].size() != 1) {
			mTempSensitive = true;
			return;
		}
	}
}

// First element represents heat capacity (J/(kg*K), the second density (kg/m^3) and the last heat conductivity (W/(m*K))
std::tuple< double, double, double > Material::getProperties(double h)
{
	double ret[3];
	for (uint i = 0; i < 3; i++) {
		ret[i] = mProperties[i].back().second;
		for (uint j = 0; j < mProperties[i].size(); j++) {
			if (h < mProperties[i][j].first) {
				if (j == 0) {
					ret[i] = mProperties[i][j].second;
					break;
				} else {
					double factor = (h-mProperties[i][j-1].first)/(mProperties[i][j].first-mProperties[i][j-1].first);
					ret[i] = mProperties[i][j].second*factor + mProperties[i][j-1].second*(1-factor);
					break;
				}
			}
		}
	}
	std::cout << ret[0] << " " << ret[1] << " " << ret[2] << std::endl;
	return std::tuple<double, double, double>(ret[0], ret[1], ret[2]);
}


State Material::getState(double h)
{
	for (uint i = 0; i < 2; i++) {
		if (h < mTransPoints[i]) {
			return (State) i;
		}
	}
	return GAS;
}

Material::~Material()
{

}

