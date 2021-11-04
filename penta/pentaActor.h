#pragma once
#include "VActor.h"

//	Camille's mysterious pentatonic trick, first used in the
//	quartet "The Bottom Line, Drawn Freehand" circa 1993.
//
//	Poisson/Xenakis temporal distribution from Camille's improvement
//	of Myhill's article in CMJ 3(3):12-14.
//
//	Camille Goudeseune, 4/24/98.
//
class PentaActor : public VActor	
{
	int fRecomputeDuration;
	void RecomputeDuration();
	float zDuration;

public:
	PentaActor();
	~PentaActor() {}

	void act();
	int receiveMessage(const char*);
	
	void setMG(char*);		// Message group to send [ampl, pitch, value] to.
	void setDensity(float);	// Mean events per second.
	void setIrreg(float);  	// Irregularity (0 = steady pulse).
	void setHue(float);    	// Pitch set, 0 to 1.  Twelve pentachords.
	void setHueExact(int); 	// Pitch set, 0 to 11 exactly.
	void setValue(float);		// User-defined parameter passed to MG.
	void setAmp(float);			// Amplitude scaling factor.
	void setWidth(float);		// Average width of pitch set, 0 to 1.
	void setLowestFreq(float);	// Base of our tuning system, in Hz.

private:
	float zAmplScale;	// Amplitude scaling factor
	float zIrreg;		// irregularity (0 to infinity)
	float zAlpha;		// density
	int wWidth;			// average width of pitch set
	int iPS;			// PS = pitch set.
	float zValue;		// reserved for use by the message group
	float zFreqLowest;	// base of our tuning system
	int fSkipFirstTime;	// hack
	float tPrev;		// ...compared to tNow
	int iPitchPrev;
	char szMG[80];		// message-group to send to
	int rgPS[12][5];	// twelve pentatonic chords
};
