#pragma once
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

//	Simple filtered-noise generation (using sample-and-hold with
//	linear interpolation).
//	Camille Goudeseune, Carlos Ricci.

class pnoiseAlg : public VAlgorithm
{
//	synthesis parameters
	float durMax;		// controls spectral width of pseudonoise
	float moddurMax;	// controls spectral width of random modulation noise
	float modIndex;		// controls amount of random modulation (output non-periodicity)

//	internal synthesis stateholders
	float durMaxInv;	// 1.0 / durMax
	float moddurMaxInv;	// 1.0 / moddurMax
	float dur;			// timer for pseudonoise sample/hold
	float moddur;		// timer for random modulation sample/hold
	int   iRand;		// pseudonoise table index, private to SRandom()
	float sampValue, sampValuePrev;
	float mod, modPrev;

	inline float SRandom();
	float Lerp(float a, float b, float t) { return a * (1.0-t) + b * t; }

public:
	float	getCutoff()		{ return 1.0f / durMax; }
	float	getModCutoff()	{ return 1.0f / moddurMax; }
	float	getModIndex()	{ return modIndex; }
	void	setCutoff(float);
	void	setModCutoff(float);
	void	setModIndex(float);
	void	generateSamples(int);
	pnoiseAlg();
	~pnoiseAlg() {}
};

class pnoiseHand : public VHandler
{
	float fCutoff;
	float modCutoff;
	float modIndex;
	enum { isetCutoff, isetModCutoff, isetModIndex };
protected:
	pnoiseAlg* getAlg() { return (pnoiseAlg*)VHandler::getAlg(); }
public:
	float	getCutoff()		{ return getAlg()->getCutoff(); }
	float	getModCutoff()	{ return getAlg()->getModCutoff(); }
	float	getModIndex()	{ return getAlg()->getModIndex(); }

	void SetAttribute(IParam iParam, float z);
	void setCutoff(float z, float t = timeDefault)
		{ modulate(isetCutoff, fCutoff, z, AdjustTime(t)); }
	void setModCutoff(float z, float t = timeDefault)
		{ modulate(isetModCutoff, modCutoff, z, AdjustTime(t)); }
	void setModIndex(float z, float t = timeDefault)
		{ modulate(isetModIndex, modIndex, z, AdjustTime(t)); }

	float dampingTime() { return 0.03; }

	pnoiseHand(pnoiseAlg* alg = new pnoiseAlg);
	~pnoiseHand() {}
	int receiveMessage(const char*);
};

class pnoiseActor : public VGeneratorActor
{
public:
	VHandler* newHandler() { return new pnoiseHand(); }
	pnoiseActor();
	~pnoiseActor() {}

	void sendDefaults(VHandler*);
	int receiveMessage(const char*);

	void	setCutoff(float f);
	void	setAllCutoff(float f, float t = 0.);
	void	setModCutoff(float f);
	void	setAllModCutoff(float f, float t = 0.);
	void	setModIndex(float f);
	void	setAllModIndex(float f, float t = 0.);

protected:
	float defaultCutoff, defaultModCutoff, defaultModIndex;
	ostream& dump(ostream&, int);
};

static inline int CheckCutoff(float f) { return 0.01 <= f && f <= globs.SampleRate; }
static inline int CheckMod(float f) { return 0.0 <= f && f <= 10.0; }
