#ifndef _SHIMMER_H_
#define _SHIMMER_H_

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

//===========================================================================
//		shimmerAlg 
//
//	Description.
//
//	Camille Goudeseune
//

class shimmerAlg : public VAlgorithm
{
public:
	enum { cPartial = 30 }; // Only 30.  For any more than that, use an inverse FFT!

private:
//	synthesis parameters
	int cPartialCur;
	float walkspeed;
	float avgfreq;
	float range;
	float freq;			// base frequency, in Hz
	ulong tStep;
	#define zAmpl (0.3f)	// one ampl for all partials, for now
	float rgphase[cPartial];	// phase accumulators of partials
	float rgfreq[cPartial];		// frequencies of partials
	float rgwalk[cPartial];

public:
//	access members
	float	getFreq(void)		{ return freq; }

//	parameter update members
    void	setNumPartials(int);
    void	setFreq(float);
	void	setWalkspeed(float);
	void	setAvgFreq(float);
	void	setRange(float);

//	utility members
 	float 	Lerp(float a, float b, float t) { return a*(1.0f-t) + b*t ; }
 	float 	Lerp(int i, float a, float *tab) { return (1.0f-a)*tab[i] + a*tab[i+1]; }
inline 	void 	WrapAccSep(float &Phase, int &iPhase, float &fPhase);

//	static wavetable initialization
	void	InitShimmerSintab(void);

//	construction/destruction
		shimmerAlg(void);
		~shimmerAlg();

private:
	inline float* ComputeMyNextSample(void);
	void generateSamples(int howMany);

};	// end of class shimmerAlg

//===========================================================================
//		shimmerHand 
//
//	class shimmerHand is a handler class for shimmerAlg.
//
class shimmerHand : public VHandler
{
//	modulating parameters of shimmerAlg
private:
	float freq;			// base frequency
	float walkspeed;
	float avgfreq;
	float range;

	enum { isetFreq, isetWalkspeed, isetAvgFreq, isetRange };

protected:
//	Algorithm access:
// 	Define a version of getAlg() that returns a pointer to shimmerAlg.
	shimmerAlg * getAlg(void)	{ return (shimmerAlg *) VHandler::getAlg(); }

public:
//	parameter access
	float	getFreq(void)		{ return getAlg()->getFreq(); }

//	parameter modulation
	void SetAttribute(IParam iParam, float z);
	void setFreq(float z, float  t = timeDefault)
		{ modulate(isetFreq, freq, z, AdjustTime(t)); }
	void setWalkspeed(float z, float  t = timeDefault)
		{ modulate(isetWalkspeed, walkspeed, z, AdjustTime(t)); }
	void setAvgFreq(float z, float  t = timeDefault)
		{ modulate(isetAvgFreq, avgfreq, z, AdjustTime(t)); }
	void setRange(float z, float  t = timeDefault)
		{ modulate(isetRange, range, z, AdjustTime(t)); }
	void setNumPartials(int c);

//	damp amplitude changes
	float	dampingTime(void)	{ return 0.02; }

//	construction
	shimmerHand(shimmerAlg * alg = new shimmerAlg);
		
//	destruction
virtual	~shimmerHand() {}

	int receiveMessage(const char * Message);

};	// end of class shimmerHand

//===========================================================================
//		shimmerActor
//
//	class shimmerActor is a generator actor class for shimmerAlg
//
class shimmerActor : public VGeneratorActor
{
public:
virtual	VHandler * newHandler(void)	{ return new shimmerHand(); }

//	construction/destruction
	shimmerActor(void);
virtual	~shimmerActor() {}

virtual	void sendDefaults(VHandler *);
virtual int	receiveMessage(const char * Message);

//	parameter setting members
	void	setNumPartials(int c);
	void	setAllNumPartials(int c);
	void	setFreq(float f);
	void	setAllFreq(float f, float t=0.);
	void	setWalkspeed(float f);
	void	setAllWalkspeed(float f, float t=0.);
	void	setAvgFreq(float f);
	void	setAllAvgFreq(float f, float t=0.);
	void	setRange(float f);
	void	setAllRange(float f, float t=0.);

//	default parameters
protected:
	int  	defaultNumPartials;
	float	defaultFreq;
	float	defaultWalkspeed;
	float	defaultAvgFreq;
	float	defaultRange;

//	biographical info
	virtual ostream &dump(ostream &os, int tabs);

};	// end of class shimmerActor

//===========================================================================
//	Bounds checking.
//
static inline int CheckNumPartials(int w) 	{ return w >= 0. && w <= shimmerAlg::cPartial; }
static inline int CheckFreq(float f) 	{ return f >= 1. && f <= 20000.; }
// static inline int CheckAmpl(float f) { return f>=0. && f <= 10.; }
static inline int CheckWalkspeed(float f) 	{ return f >= 0. && f <= 1.; }
static inline int CheckAvgfreq(float f) 	{ return f >= 1. && f <= 20000.; }
static inline int CheckRange(float f) 	{ return f >= 1.001 && f <= 100.; }

#endif // ndef _SHIMMER_H_
