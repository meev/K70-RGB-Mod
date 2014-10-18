#ifndef LIGHTCONTROL_H
#define LIGHTCONTROL_H

#include "Main.h"

class LightControl
{
public:
	void BuildMatrix();
	void BuildMatrixVK();
	int SetLedRGB(int led, int r, int g, int b);
	int SetXYRGB(int x, int y, int r, int g, int b);
};

#endif