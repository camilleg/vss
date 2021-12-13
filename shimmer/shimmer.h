#pragma once
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

class shimmerAlg : public VAlgorithm
{
public:
	enum { cPartial = 30 }; // For more than that, use an inverse FFT.  Public, for CheckNumPartials().

private:
	int cPartialCur;
	float walkspeed;
	float avgfreq;
	float range;
	float freq;			// base frequency, in Hz
	ulong tStep;
	const float zAmpl = 0.3f;	// one ampl for all partials, for now
	float rgphase[cPartial];	// phase accumulators of partials
	float rgfreq[cPartial];		// frequencies of partials
	float rgwalk[cPartial];

public:
	float getFreq() const { return freq; }

    void	setNumPartials(int);
    void	setFreq(float);
	void	setWalkspeed(float);
	void	setAvgFreq(float);
	void	setRange(float);

 	float 	Lerp(float a, float b, float t) { return a*(1.0f-t) + b*t ; }
 	float 	Lerp(int i, float a, float *tab) { return (1.0f-a)*tab[i] + a*tab[i+1]; }
	void WrapAccSep(float& Phase, int& iPhase, float& fPhase);

	//	static wavetable initialization
	void InitShimmerSintab();

	shimmerAlg();
	~shimmerAlg();
private:
	float* ComputeMyNextSample();
	void generateSamples(int);
};

class shimmerHand : public VHandler
{
	float freq;			// base frequency
	float walkspeed;
	float avgfreq;
	float range;
	enum { isetFreq, isetWalkspeed, isetAvgFreq, isetRange };

protected:
	shimmerAlg* getAlg() { return (shimmerAlg*)VHandler::getAlg(); }

public:
	float getFreq() { return getAlg()->getFreq(); }

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

	float dampingTime() { return 0.02; }

	shimmerHand(shimmerAlg* alg = new shimmerAlg);
	~shimmerHand() {}
	int receiveMessage(const char*);
};

class shimmerActor : public VGeneratorActor
{
public:
	VHandler* newHandler() { return new shimmerHand(); }
	shimmerActor();
	~shimmerActor() {}
	void sendDefaults(VHandler*);
	int	receiveMessage(const char*);

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

protected:
	int  	defaultNumPartials;
	float	defaultFreq;
	float	defaultWalkspeed;
	float	defaultAvgFreq;
	float	defaultRange;

	ostream& dump(ostream&, int);
};

static inline int CheckNumPartials(int w) 	{ return w >= 0. && w <= shimmerAlg::cPartial; }
static inline int CheckFreq(float f) 	{ return f >= 1. && f <= 20000.; }
// static inline int CheckAmpl(float f) { return f>=0. && f <= 10.; }
static inline int CheckWalkspeed(float f) 	{ return f >= 0. && f <= 1.; }
static inline int CheckAvgfreq(float f) 	{ return f >= 1. && f <= 20000.; }
static inline int CheckRange(float f) 	{ return f >= 1.001 && f <= 100.; }
