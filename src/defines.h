#ifndef DEFINES_H
#define DEFINES_H
#include<OgrePrerequisites.h>

#define TILESIZE 10
enum State { SOLID = 0, LIQUID, GAS, UNDEFINED};
static const Ogre::String StateStrings[] = {"Solid", "Liquid", "Gas", "Undefined"};
enum SimTool { INSERTMATERIAL = 0, HEAT, COOL, MOVE, TOGGLESOURCE };
static const Ogre::String SimToolStrings[] = {"Insert Material", "Heat", "Cool", "Move", "Toggle source"};
static const Ogre::String SizeStrings[] = {"xSize", "ySize", "zSize"};
#define FARCLIP 1000
#define CLOSECLIP 5
#define DEFAULTTEMP 280

#endif