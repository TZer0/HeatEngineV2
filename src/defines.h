#ifndef DEFINES_H
#define DEFINES_H

#define TILESIZE 10
enum State { SOLID = 0, LIQUID, GAS, UNDEFINED };
static const char StateStrings[][10] = {"Solid", "Liquid", "Gas", "Undefined"};
enum SimTool { INSERTMATERIAL, HEAT, COOL, MOVE };
#define FARCLIP 600
#define CLOSECLIP 5
#define DEFAULTTEMP 280

#endif