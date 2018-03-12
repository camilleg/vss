#ifndef _PNOISE_H_
#define _PNOISE_H_

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

//===========================================================================
//		noiseAlg 
//
//	Simple filtered-noise generation (using sample-and-hold with
//	linear interpolation).
//
//	Camille Goudeseune, Carlos Ricci.
//

class noiseAlg : public VAlgorithm
{
private:
//	synthesis parameters
	float durMax;		// controls spectral width of pseudonoise
	float moddurMax;	// controls spectral width of random modulation noise
	float modIndex;		// controls amount of random modulation (output non-periodicity)

//	internal synthesis stateholders
	float durMaxInv;	// 1.0 / durMax
	float moddurMaxInv;	// 1.0 / moddurMax
	float dur;		// timer for pseudonoise sample/hold
	float moddur;		// timer for random modulation sample/hold
	int   iRand;		// pseudonoise table index, private to SRandom()
	float sampValue, sampValuePrev;
	float mod, modPrev;

//	utility inlines
inline	float SRandom(void);
	float Lerp(float a, float b, float t) { return a * (1-t) + b * t; }

public:
//	access members
	float	getCutoff(void)		{ return 1.0f / durMax; }
	float	getModCutoff(void)	{ return 1.0f / moddurMax; }
	float	getModIndex(void)	{ return modIndex; }

//	parameter update members
	void	setCutoff(float);
	void	setModCutoff(float);
	void	setModIndex(float);

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
	float modCutoff;
	float modIndex;

	enum { isetCutoff, isetModCutoff, isetModIndex };
	
//	Algorithm access:
// 	Define a version of getAlg() that returns a pointer to noiseAlg.
protected:
	noiseAlg * getAlg(void)	{ return (noiseAlg *) VHandler::getAlg(); }

//	parameter access
public:
	float	getCutoff(void)		{ return getAlg()->getCutoff(); }
	float	getModCutoff(void)	{ return getAlg()->getModCutoff(); }
	float	getModIndex(void)	{ return getAlg()->getModIndex(); }

//	parameter modulation
	void SetAttribute(IParam iParam, float z);

	void setCutoff(float z, float  t = timeDefault)
		{ modulate(isetCutoff, fCutoff, z, AdjustTime(t)); }
	void setModCutoff(float z, float  t = timeDefault)
		{ modulate(isetModCutoff, modCutoff, z, AdjustTime(t)); }
	void setModIndex(float z, float  t = timeDefault)
		{ modulate(isetModIndex, modIndex, z, AdjustTime(t)); }

//	damp amplitude changes
	float	dampingTime(void)	{ return 0.03; }

//	construction
	noiseHand(noiseAlg * alg = new noiseAlg);
		
//	destruction
	virtual	~noiseHand() {}

	int receiveMessage(const char * Message);

};	// end of class noiseHand

//===========================================================================
//		pnoiseActor
//
//	class pnoiseActor is a generator actor class for noiseAlg
//
class pnoiseActor : public VGeneratorActor
{
public:
virtual	VHandler * newHandler(void)	{ return new noiseHand(); }

//	construction/destruction
public:
	pnoiseActor(void);
virtual	~pnoiseActor() {}

virtual	void 	sendDefaults(VHandler *);
virtual int		receiveMessage(const char * Message);

//	parameter setting members
	void	setCutoff(float f);
	void	setAllCutoff(float f, float t = 0.);
	void	setModCutoff(float f);
	void	setAllModCutoff(float f, float t = 0.);
	void	setModIndex(float f);
	void	setAllModIndex(float f, float t = 0.);

//	default parameters
protected:
	float	defaultCutoff, defaultModCutoff, defaultModIndex;

//	biographical info
	virtual ostream &dump(ostream &os, int tabs);

};	// end of class pnoiseActor

//===========================================================================
//	Find reasonable bounds and enforce them.
//
	static inline int CheckCutoff(float f) { return f >= 0.01 && f <= globs.SampleRate; }
	static inline int CheckMod(float f) { return f>=0. && f <= 10.; }

#endif // ndef _PNOISE_H_
