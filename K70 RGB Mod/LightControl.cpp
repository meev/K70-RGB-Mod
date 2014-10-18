#include "LightControl.h"

extern unsigned char		g_LEDState[144][3], g_PrevLEDState[144][3];
extern unsigned char		g_XY[7][92];
extern unsigned char		g_XYvk[7][92];
extern unsigned char		g_XYa[];
extern unsigned char		g_XYv[];
extern float				g_XYb[];

void LightControl::BuildMatrix()
{
	// Construct XY lookup table
	auto keys = g_XYa;
	auto sizes = g_XYb;

	//for (int y = 0; y < 7; y++) // Y 0 = Brightness key, winlock & mute
	for (int y = 7; y--> 0;) // Y 0 = CTRL, Windowskey, Alt, space...
	{
		unsigned char key;
		int size = 0;

		for (int x = 0; x < 92; x++)
		{
			if (size == 0)
			{
				auto sizef = *sizes++;
				if (sizef < 0)
				{
					size = -sizef * 4;
					key = 255;
				}
				else
				{
					key = *keys++;
					size = sizef * 4;
				}
			}

			g_XY[y][x] = key;
			size--;
		}

		if (*keys++ != 255 || *sizes++ != 0)
			return;
	}
}

void LightControl::BuildMatrixVK()
{
	// Construct XY lookup table
	auto keys = g_XYv;
	auto sizes = g_XYb;

	//for (int y = 0; y < 7; y++) // Y 1 = F1, F2... 
	for (int y = 7; y--> 0;) // Y 1 = CTRL, Windowskey, Alt, space...
	{
		unsigned char key;
		int size = 0;

		for (int x = 0; x < 92; x++)
		{
			if (size == 0)
			{
				auto sizef = *sizes++;
				if (sizef < 0)
				{
					size = -sizef * 4;
					key = 255;
				}
				else
				{
					key = *keys++;
					size = sizef * 4;
				}
			}

			g_XYvk[y][x] = key;
			size--;
		}

		if (*keys++ != 255 || *sizes++ != 0)
			return;
	}
}

int LightControl::SetLedRGB(int led, int r, int g, int b)
{
	/*if (x < 0)
	x = 0;
	else if (x > 91)
	x = 91;

	if (y < 0)
	y = 0;
	else if (y > 6)
	y = 6;

	// Converts Y & X to led number
	int led = g_XY[y][x];
	*/
	if (r < 0)
		r = 0;
	else if (r > 7)
		r = 7;

	if (g < 0)
		g = 0;
	else if (g > 7)
		g = 7;

	if (b < 0)
		b = 0;
	else if (b > 7)
		b = 7;

	if (led < 0 || led >= 144)
		return 0;

	// Minus 7 reverts so r = 6 will be r = 1
	g_LEDState[led][0] = 7 - r;
	g_LEDState[led][1] = 7 - g;
	g_LEDState[led][2] = 7 - b;

	return 0;
}

int LightControl::SetXYRGB(int x, int y, int r, int g, int b)
{
	if (x < 0)
	x = 0;
	else if (x > 91)
	x = 91;

	if (y < 0)
	y = 0;
	else if (y > 6)
	y = 6;

	// Converts Y & X to led number
	int led = g_XY[y][x];

	return SetLedRGB(led, r, g, b);
}
