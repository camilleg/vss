#pragma once
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

//	Simple brute-force add generation (using linearly interpolated 
//	sinewave table lookups).
//	Camille Goudeseune, Carlos Ricci.

class addAlg : public VAlgorithm
{
public:
	enum { cPartial = 30 }; /*const int*/ 	// Only 30.  For any more than that, use an inverse FFT!

private:
//	synthesis parameters
	float freq;					// base frequency, in Hz
	float freqSamples;			// base frequency, in samples
	float rgzAmpl[cPartial];	// amplitudes of partials
	float rgphase[cPartial];	// phase accumulators of partials
	float rgfreq[cPartial];		// frequencies of partials
	float rgzfd[cPartial];		// frequency deviations of partials

public:
	float	getFreq()			{ return freq; }
	float	getIthAmpl(int i)	{ return rgzAmpl[i]; }
	float	getIthFD(int i)		{ return rgzfd[i]; }

    void	setFreq(float);
    void	setAmplPartials(float*);
    void	setIthAmpl(int, float);
	void	setFDPartials(float*);
	void	setIthFD(int, float);
	void	setFreqPartials(float*);
	void	setIthFreq(int, float);

//	utility members
 	float 	Lerp(float a, float b, float t) { return a*(1.0f-t) + b*t ; }
 	float 	Lerp(int i, float a, float *tab) { return (1.0f-a)*tab[i] + a*tab[i+1]; }
inline 	void 	WrapAccSep(float &Phase, int &iPhase, float &fPhase);

	void	generateSamples(int);

//	static wavetable initialization
	void	InitAddSintab();

	addAlg();
	~addAlg();
};

class addHand : public VHandler
{
//	modulating parameters of addAlg
	float freq;			// base frequency
	float partials[addAlg::cPartial]; 	// amplitudes of partials
	float fd[addAlg::cPartial]; 	// frequency deviation of partials
	float allfreq[addAlg::cPartial]; 	// individual frequency of partials

protected:
	addAlg* getAlg() { return (addAlg*)VHandler::getAlg(); }

public:
	enum {
		isetFreq,
		isetIthAmpl,
		isetAmplPartials,
		isetFDPartials,
		isetIthFD,
		isetFreqPartials,
		isetIthFreq
		};
	void SetAttribute(IParam iParam, float);
	void SetAttribute(IParam iParam, const float*);

	float	getFreq()			{ return getAlg()->getFreq(); }
	float	getIthAmpl(int i)	{ return getAlg()->getIthAmpl(i); }

	void	setFreq(float z, float  t = timeDefault)
			{ modulate(isetFreq, freq, z, AdjustTime(t)); }
	void	setIthAmpl(int i, float z, float  t = timeDefault)
			{ modulate(isetIthAmpl, i, partials[i], z, AdjustTime(t)); }
	void	setAmplPartials(int cz, float* rgz, float t=timeDefault)
			{ modulate(isetAmplPartials, cz, partials, rgz, AdjustTime(t)); }
	void	setFDPartials(int cz, float* rgz, float t=timeDefault)
			{ modulate(isetFDPartials, cz, fd, rgz, AdjustTime(t)); }
	void	setIthFD(int i, float z, float  t = timeDefault)
			{ modulate(isetIthFD, i, fd[i], z, AdjustTime(t)); }
	void	setFreqPartials(int cz, float* rgz, float t=timeDefault)
			{ modulate(isetFreqPartials, cz, allfreq, rgz, AdjustTime(t)); }
	void	setIthFreq(int i, float z, float  t = timeDefault)
			{ modulate(isetIthFreq, i, allfreq[i], z, AdjustTime(t)); }

	float	dampingTime() { return 0.03; }

	addHand(addAlg* alg = new addAlg());
	~addHand() {}
	int receiveMessage(const char * Message);
};

//	class addActor is a generator actor class for addAlg
class addActor : public VGeneratorActor
{
public:
	VHandler* newHandler() { return new addHand(); }
	addActor();
	~addActor() {}

	void sendDefaults(VHandler*);
	int	receiveMessage(const char*);

	void	setFreq(float f);
	void	setAllFreq(float f, float t = 0.);
	void	setAmplPartials(int cz, float* rgz);
	void	setIthAmpl(int i, float f);
	void	setAllIthAmpl(int i, float f, float t = 0.);
	void	setFDPartials(int cz, float* rgz);
	void	setIthFD(int i, float f);
	void	setAllIthFD(int i, float f, float t = 0.);

protected:
	float	defaultFreq;
	float	defaultIthAmpl[addAlg::cPartial];
	float	defaultIthFD[addAlg::cPartial];

	ostream &dump(ostream &os, int tabs);
};

	static	inline int CheckFreq(float f) { return f >= 0. && f <= 24000.; }
	static	inline int CheckAmpl(float f) { return f>=0. && f <= 10.; }
	static	inline int CheckFD(float f) { return f>=-1. && f <= 1.; }
