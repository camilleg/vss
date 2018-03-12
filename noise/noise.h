#ifndef _NOISE_H_
#define _NOISE_H_

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

//===========================================================================
//		noiseAlg 
//
//	Simple filtered-noise generation, using sample-and-hold with optional
//	linear interpolation.
//
//	Camille Goudeseune, Carlos Ricci
//


class noiseAlg : public VAlgorithm
{
private:
//	synthesis parameters
	float fCutoff;			// -3dB bandwidth (half-power bandwidth) in Hz
	int order;			// effective filter order, slope = (order+1)*6dB/oct

//	internal synthesis stateholders
	float decimationPeriod;		// related directly to bandwidth
	float noizTimer;		// for timing the decimation process
	float sampValue, sampValuePrev; // noise value states

//	utility inlines
	float Lerp(float a, float b, float t) { return a * (1-t) + b * t; }

//	access members
public:
	float	getCutoff(void) { return fCutoff; }
	float	getOrder(void) { return (float) order; }

//	parameter update members
	void	setCutoff(float);
	void	setOrder(int);

//	sample generation
	void	generateSamples(int);

//	construction/destruction
		noiseAlg(void);
		~noiseAlg();
};	// end of class noiseAlg

//===========================================================================
//		noiseHand 
//
//	class noiseHand is a handler class for noiseAlg.
//
class noiseHand : public VHandler
{
//	modulating parameters of noiseAlg
private:
	float fCutoff;

	enum { isetCutoff };
	
//	Algorithm access:
// 	Define a version of getAlg() that returns a pointer to noiseAlg.
protected:
	noiseAlg * getAlg(void)	{ return (noiseAlg *) VHandler::getAlg(); }

//	parameter access
public:
	float	getCutoff(void)	{ return getAlg()->getCutoff(); }
	float	getOrder(void)	{ return getAlg()->getOrder(); }

//	parameter modulation
	void SetAttribute(IParam iParam, float z);
	void setCutoff(float z, float  t = timeDefault)
		{ modulate(isetCutoff, fCutoff, z, AdjustTime(t)); }

//	parameter setting
	void	setOrder(float ord) { getAlg()->setOrder((int) ord); }
	
//	damp amplitude changes
	float	dampingTime(void)	{ return 0.03; }

//	construction
		noiseHand(noiseAlg * alg = new noiseAlg);
		
//	destruction
virtual		~noiseHand() {}

	int receiveMessage(const char * Message);

};	// end of class noiseHand

//===========================================================================
//		noiseActor
//
//	class noiseActor is a generator actor class for noiseAlg
//
class noiseActor : public VGeneratorActor
{
public:
virtual	VHandler * newHandler(void)	{ return new noiseHand(); }

//	construction/destruction
	noiseActor(void);
virtual	~noiseActor() {}

virtual	void 	sendDefaults(VHandler *);
virtual int	receiveMessage(const char * Message);

//	parameter setting members
	void	setCutoff(float f);
	void	setAllCutoff(float f, float t = 0.);
	void	setOrder(float f);
	void	setAllOrder(float f);

protected:
//	default parameters
	float	defaultCutoff, defaultOrder;

//	biographical info
	virtual ostream &dump(ostream &os, int tabs);

};	// end of class noiseActor

//===========================================================================
//	Find reasonable bounds and enforce them.
//
	static inline int CheckCutoff(float f) { return f >= 0.01 && f <= globs.SampleRate ; }

#endif // ndef _NOISE_H_
