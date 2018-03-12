#ifndef _FM_H_
#define _FM_H_

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"


//===========================================================================
//		fmAlg 
//
//	class fmAlg is a test algorithm with sample generation
//	code copied from the old classFMnote.
//
class fmAlg : public VAlgorithm
{
private:
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
//	access members
	float	getCarrierFreq(void)	{ return carFreq; }
	float	getModulatorFreq(void)	{ return modFreq; }
	float	getCMratio(void)	{ return cmRatio; }
	int	getRatioMode(void)	{ return ratioMode; }
	float	getModIndex(void)	{ return modIndex; }
	float	getCarFeedback(void)	{ return carFeedback; }
	float	getModFeedback(void)	{ return modFeedback; }

//	parameter update members
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
inline 	void 	WrapAccSep(float &Phase, int &iPhase, float &fPhase);
inline 	void 	WrapAcc(float &Phase);
inline 	void 	WrapTot(float Phase, int &iPhase, float &fPhase);

//	sample generation
	void	generateSamples(int);

//	static wavetable initialization
	void	InitFMsintab(void);

//	construction/destruction
		fmAlg(void);
		~fmAlg();

};	// end of class fmAlg

//===========================================================================
//		fmHand 
//
//	class fmHand is a handler class for fmAlg.
//
class fmHand : public VHandler
{
private:
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
//	Algorithm access:
// 	Define a version of getAlg() that returns a pointer to fmAlg.
	fmAlg * getAlg(void)	{ return (fmAlg *) VHandler::getAlg(); }

public:
//	parameter access
	float getCarrierFreq(void)	{ return getAlg()->getCarrierFreq(); }
	float getModulatorFreq(void)	{ return getAlg()->getModulatorFreq(); }
	float getCMratio(void)	{ return getAlg()->getCMratio(); }
	float getModIndex(void)	{ return getAlg()->getModIndex(); }
	float getCarFeedback(void)	{ return getAlg()->getCarFeedback(); }
	float getModFeedback(void)	{ return getAlg()->getModFeedback(); }
	int getRatioMode(void)	{ return getAlg()->getRatioMode(); }

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
	float	dampingTime(void)	{ return 0.03; }

//	message handling
	int receiveMessage(const char * Message);

//	construction
	fmHand(fmAlg * alg = new fmAlg);
		
//	destruction
	virtual	~fmHand() {}

};	// end of class fmHand

//===========================================================================
//		fmActor
//
//	class fmActor is a generator actor class for fmAlg
//
class fmActor : public VGeneratorActor
{
public:
virtual	VHandler * newHandler(void)	{ return new fmHand(); }

//	construction/destruction
public:
	fmActor(void);
virtual	~fmActor() {}

virtual	void 	sendDefaults(VHandler *);
virtual int	receiveMessage(const char * Message);

//	parameter setting members
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

//	default parameters
protected:
	float	defaultRatioMode, defaultCarFreq, defaultModFreq, defaultCMratio;
	float	defaultModIndex, defaultCarFeedback, defaultModFeedback;

//	biographical info
virtual ostream &dump(ostream &os, int tabs);

};	// end of class fmActor


//===========================================================================
//	BOUNDS CHECKING IS VITAL TO OUR SURVIVAL!!!!!!!!!!!!!!!!!!!
//
//	Find reasonable bounds and enforce them.
//
static	inline	int	CheckFreq(float f) 	{ return f >= 0. && f < 20000.; }
static	inline	int	CheckCMratio(float f) 	{ return f > 1.0e-6 && f < 1.0e6; }
static	inline	int	CheckIndex(float f)	{ return f >= 0. && f < 1000.; }
static	inline	int	CheckFeedback(float f)	{ return f >= -1. && f <= 1.; }

#endif // ndef _FM_H_
