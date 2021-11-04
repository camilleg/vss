#pragma once
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

//	Simple filtered-noise generation, using sample-and-hold with optional
//	linear interpolation.
//	Camille Goudeseune, Carlos Ricci
class noiseAlg : public VAlgorithm
{
//	synthesis parameters
	float fCutoff;	// -3dB bandwidth (half-power bandwidth) in Hz
	int order;		// effective filter order, slope = (order+1)*6dB/oct

//	internal synthesis stateholders
	float decimationPeriod;			// related directly to bandwidth
	float noizTimer;				// for timing the decimation process
	float sampValue, sampValuePrev; // noise value states

	float Lerp(float a, float b, float t) { return a * (1.0-t) + b * t; }

public:
	float getCutoff() const { return fCutoff; }
	float getOrder() const { return order; }
	void	setCutoff(float);
	void	setOrder(int);
	void	generateSamples(int);
	noiseAlg();
	~noiseAlg() {}
};

class noiseHand : public VHandler
{
	float fCutoff;
	enum { isetCutoff };
	noiseAlg* getAlg() { return (noiseAlg*)VHandler::getAlg(); }
public:
	float	getCutoff() { return getAlg()->getCutoff(); }
	float	getOrder() { return getAlg()->getOrder(); }

//	parameter modulation
	void SetAttribute(IParam iParam, float z);
	void setCutoff(float z, float t = timeDefault)
		{ modulate(isetCutoff, fCutoff, z, AdjustTime(t)); }

//	parameter setting
	void setOrder(float ord) { getAlg()->setOrder((int) ord); }
	
	float dampingTime() { return 0.03; }

	noiseHand(noiseAlg* alg = new noiseAlg);
	~noiseHand() {}
	int receiveMessage(const char*);
};

class noiseActor : public VGeneratorActor
{
	float defaultCutoff, defaultOrder;
	ostream& dump(ostream&, int tabs);
public:
	VHandler* newHandler() { return new noiseHand(); }
	noiseActor();
	~noiseActor() {}

	void sendDefaults(VHandler*);
	int receiveMessage(const char*);

	void	setCutoff(float f);
	void	setAllCutoff(float f, float t = 0.);
	void	setOrder(float f);
	void	setAllOrder(float f);
};

static inline int CheckCutoff(float f) { return 0.01 <= f && f <= globs.SampleRate; }
