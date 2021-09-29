#pragma once
#include "VAlgorithm.h"
#include "VGenActor.h"
#include "VHandler.h"

class fmAlg : public VAlgorithm
{
//	synthesis parameters
	float	carFreq;	// carrier frequency, Hz
	float	modFreq;	// modulator frequency, Hz
	float	cmRatio;	// car/mod frequency ratio
	int	ratioMode;	// ratio or independent freq mode (1=ratio mode)
	float	modIndex;	// modulation index ("beta" for phase modulation)
	float	carFeedback;	// carrier feedback amplitude
	float	modFeedback;	// modulator feedback amplitude

//	internal synthesis stateholders
	float	carPhase;	// carrier phase accumulator (CPA), in units of "samples"
	float	carDPhase;	// carrier phase increment, in "samples"
	float   modPhase;	// modulator phase accumulator (MPA), in units of samples
	float   modDPhase;	// modulator phase increment, in samples
	float	modIndOPhs;	// modIndex, scaled to units of "samples"
	float	carFbOPhs;	// carrier feedback amplitude,  scaled to units of samples
	float	modFbOPhs;	// modulator feedback amplitude,  scaled to units of samples
	float	lastCarVal;	// carrier feedback signal: last carrier output value
	float	lastModVal;	// modulator feedback signal: last modulator output value

public:
	float	getCarrierFreq()	{ return carFreq; }
	float	getModulatorFreq()	{ return modFreq; }
	float	getCMratio()		{ return cmRatio; }
	int		getRatioMode()		{ return ratioMode; }
	float	getModIndex()		{ return modIndex; }
	float	getCarFeedback()	{ return carFeedback; }
	float	getModFeedback()	{ return modFeedback; }

	void	setCarrierFreq(float);
	void	setModulatorFreq(float);
	void	setCMratio(float);
	void	setRatioMode(int);
	void	setModIndex(float);
	void	setCarFeedback(float);
	void	setModFeedback(float);

//	utility members
 	float 	Lerp(float a, float b, float t) { return a*(1.0f-t) + b*t ; }
 	float 	Lerp(int i, float a, float *tab) { return (1.0f-a)*tab[i] + a*tab[i+1]; }
	inline void WrapAccSep(float &Phase, int &iPhase, float &fPhase);
	inline void WrapAcc(float &Phase);
	inline void WrapTot(float Phase, int &iPhase, float &fPhase);

	void	generateSamples(int);

//	static wavetable initialization
	void	InitFMsintab();

	fmAlg();
	~fmAlg();
};

class fmHand : public VHandler
{
//	modulating parameters of fmAlg
	float carFreq;
	float modFreq;
	float cmRatio;
	float modIndex;
	float carFeedback;
	float modFeedback;

	enum {
		isetCarrierFreq,
		isetModulatorFreq,
		isetCMratio,
		isetModIndex,
		isetCarFeedback,
		isetModFeedback };

protected:
	fmAlg* getAlg()	{ return (fmAlg*)VHandler::getAlg(); }

public:
	float getCarrierFreq()	{ return getAlg()->getCarrierFreq(); }
	float getModulatorFreq()	{ return getAlg()->getModulatorFreq(); }
	float getCMratio()	{ return getAlg()->getCMratio(); }
	float getModIndex()	{ return getAlg()->getModIndex(); }
	float getCarFeedback()	{ return getAlg()->getCarFeedback(); }
	float getModFeedback()	{ return getAlg()->getModFeedback(); }
	int getRatioMode()	{ return getAlg()->getRatioMode(); }

//	parameter modulation
	void SetAttribute(IParam iParam, float z);
	void setCarrierFreq(float z, float  t = timeDefault)
		{ modulate(isetCarrierFreq, carFreq, z, AdjustTime(t)); }
	void setModulatorFreq(float z, float  t = timeDefault)
		{ modulate(isetModulatorFreq, modFreq, z, AdjustTime(t)); }
	void setCMratio(float z, float t = timeDefault)
		{ modulate(isetCMratio, cmRatio, z, AdjustTime(t)); }
	void setMCratio(float z, float t = timeDefault)
		{ setCMratio(1./z, t); }
	void setModIndex(float z, float t = timeDefault)
		{ modulate(isetModIndex, modIndex, z, AdjustTime(t)); }
	void setCarFeedback(float z, float t = timeDefault)
		{ modulate(isetCarFeedback, carFeedback, z, AdjustTime(t)); }
	void setModFeedback(float z, float t = timeDefault)
		{ modulate(isetModFeedback, modFeedback, z, AdjustTime(t)); }

//	parameter setting
	void setRatioMode(float z = 1.)
		{ getAlg()->setRatioMode((int)z); }

//	damp amplitude changes
	float	dampingTime()	{ return 0.03; }

	int receiveMessage(const char*);

	fmHand(fmAlg* alg = new fmAlg);
	~fmHand() {}
};

class fmActor : public VGeneratorActor
{
public:
	VHandler* newHandler() { return new fmHand(); }
	fmActor();
	~fmActor() {}

	void sendDefaults(VHandler*);
	int receiveMessage(const char*);

	void	setRatioMode(float f = 1.);
	void	setAllRatioMode(float f = 1.);
	void	setCarrierFreq(float f);
	void	setAllCarrierFreq(float f, float t = 0.);
	void	setModulatorFreq(float f);
	void	setAllModulatorFreq(float f, float t = 0.);
	void	setCMratio(float f);
	void	setAllCMratio(float f, float t = 0.);
	void	setMCratio(float f);
	void	setAllMCratio(float f, float t = 0.);
	void	setModIndex(float f);
	void	setAllModIndex(float f, float t = 0.);
	void	setCarFeedback(float f);
	void	setAllCarFeedback(float f, float t = 0.);
	void	setModFeedback(float f);
	void	setAllModFeedback(float f, float t = 0.);

protected:
	float	defaultRatioMode, defaultCarFreq, defaultModFreq, defaultCMratio;
	float	defaultModIndex, defaultCarFeedback, defaultModFeedback;

	ostream &dump(ostream &os, int tabs);
};

#include "boundscheckers.h"
