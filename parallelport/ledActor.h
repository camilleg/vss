#pragma once
#include "VActor.h"

//	An LedActor sends a data byte out the parallel port of the PC on which
//	VSS is running.
//	With the appropriate circuitry (an 8-bit latch, 3 transistors,
//	and a pile of resistors) this can adjust the hue and brightness
//	of a red-green-blue LED.

class LedActor : public VActor	
{
public:
	LedActor();
	~LedActor();
	void act();
	int receiveMessage(const char*);
	
	void setBrightness(float);
	void setHue(float);
	void setBlink(float period /*seconds*/, float dutyCycle /*[0,1]*/);

private:
	void recalc();

	float zBright; // 0..1 black..full
	float zHue;    // 0=r  1/6=rg  1/3=g  1/2=gb  2/3=b  5/6=br  1=r

	float zR, zG, zB; // red, green, blue components: 0..1 black..full

	float zBlinkPeriod;
	float zBlinkDutyCycle;

	int fStrobe; // toggle

	unsigned char b; // data byte
	unsigned char bPrev; // local cache

	int fDisabled;
};
