#pragma once

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

#include "boundscheckers.h"
static inline int CheckFilterGain(float f) { return f >= 0. && f <= 10.; }

// For fmmAlg.
class fmOperator
{
public:
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

	float getCarrierFreq() { return carFreq; }

	void setCarrierFreq(float);
	void setModulatorFreq(float);
	void setCMratio(float);
	void setRatioMode(int);
	void setModIndex(float);
	void setCarFeedback(float);
	void setModFeedback(float);

	fmOperator() :
		carFreq(100.),
		modFreq(100.),
		cmRatio(1.),
		ratioMode(1),
		modIndex(1.),
		carFeedback(0.),
		modFeedback(0.),
		carPhase(0.),
		carDPhase(0.),
		modPhase(0.),
		modDPhase(0.),
		modIndOPhs(0.),
		carFbOPhs(0.),
		modFbOPhs(0.),
		lastCarVal(0.),
		lastModVal(0.)
		{}
};

class fmmAlg : public VAlgorithm
{
//	synthesis parameters
	fmOperator op1;
	fmOperator op2;
	float ccRatio;		// ratio between op1's and op2's carrier freqs.
	float ccModIndex;
	float ccModIndOPhs;
	float last12ModVal;	// modulator feedback signal: last modulator output value
	filter dcBlock1;
	filter dcBlock2;
	filter lopass;
	filter hipass;

public:
	void set1CarrierFreq(float z);
	void set1ModulatorFreq(float z)		{ op1.setModulatorFreq(z); }
	void set1CMratio(float z)			{ op1.setCMratio(z); }
	void set1RatioMode(int w)			{ op1.setRatioMode(w); }
	void set1ModIndex(float z)			{ op1.setModIndex(z); }
	void set1CarFeedback(float z)		{ op1.setCarFeedback(z); }
	void set1ModFeedback(float z)		{ op1.setModFeedback(z); }

	void set2CarrierFreq(float z)		{ op2.setCarrierFreq(z); }
	void set2CCratio(float z);
	void set2CCModIndex(float z);
	void set2ModulatorFreq(float z)		{ op2.setModulatorFreq(z); }
	void set2CMratio(float z)			{ op2.setCMratio(z); }
	void set2RatioMode(int w)			{ op2.setRatioMode(w); }
	void set2ModIndex(float z)			{ op2.setModIndex(z); }
	void set2CarFeedback(float z)		{ op2.setCarFeedback(z); }
	void set2ModFeedback(float z)		{ op2.setModFeedback(z); }

	void setLowpassGain(float z);
	void setHighpassGain(float z);

//	utility members
 	float 	Lerp(float a, float b, float t) { return a*(1.0f-t) + b*t ; }
 	float 	Lerp(int i, float a, float *tab) { return (1.0f-a)*tab[i] + a*tab[i+1]; }
  inline void WrapAccSep(float &Phase, int &iPhase, float &fPhase);
  inline void WrapAcc(float &Phase);
  inline void WrapTot(float Phase, int &iPhase, float &fPhase);

	void generateSamples(int);

//	static wavetable initialization
	void	InitFMMsintab(void);

	fmmAlg();
	~fmmAlg() {}

};

class fmmHand : public VHandler
{
//	modulating parameters of fmmAlg
	float carFreq1;
	float modFreq1;
	float cmRatio1;
	float modIndex1;
	float carFeedback1;
	float modFeedback1;

	float carFreq2;
	float ccRatio;		// for setting carFreq2
	float ccmodIndex;		// for setting carFreq2
	float modFreq2;
	float cmRatio2;
	float modIndex2;
	float carFeedback2;
	float modFeedback2;

	float lowpassGain;
	float highpassGain;

	enum {
		iset1CarrierFreq,
		iset1ModulatorFreq,
		iset1CMratio,
		iset1ModIndex,
		iset1CarFeedback,
		iset1ModFeedback,
		iset2CarrierFreq,
		iset2CCratio,
		iset2CCModIndex,
		iset2ModulatorFreq,
		iset2CMratio,
		iset2ModIndex,
		iset2CarFeedback,
		iset2ModFeedback,
		isetLowpassGain,
		isetHighpassGain
		};

protected:
	fmmAlg* getAlg() { return (fmmAlg*)VHandler::getAlg(); }

public:
//	parameter modulation

	void SetAttribute(IParam iParam, float z);

	void set1CarrierFreq(float z, float  t=timeDefault)
		{ modulate(iset1CarrierFreq, carFreq1, z, AdjustTime(t)); }
	void set1ModulatorFreq(float z, float  t=timeDefault)
		{ modulate(iset1ModulatorFreq, modFreq1, z, AdjustTime(t)); }
	void set1CMratio(float z, float t=timeDefault)
		{ modulate(iset1CMratio, cmRatio1, z, AdjustTime(t)); }
	void set1MCratio(float z, float t=timeDefault)
		{ set1CMratio(1./z, t); }
	void set1ModIndex(float z, float t=timeDefault)
		{ modulate(iset1ModIndex, modIndex1, z, AdjustTime(t)); }
	void set1CarFeedback(float z, float t=timeDefault)
		{ modulate(iset1CarFeedback, carFeedback1, z, AdjustTime(t)); }
	void set1ModFeedback(float z, float t=timeDefault)
		{ modulate(iset1ModFeedback, modFeedback1, z, AdjustTime(t)); }

	void set2CarrierFreq(float z, float  t=timeDefault)
		{ modulate(iset2CarrierFreq, carFreq2, z, AdjustTime(t)); }
	void set2CCratio(float z, float  t=timeDefault)
		{ modulate(iset2CCratio, ccRatio, z, AdjustTime(t)); }
	void set2CCModIndex(float z, float t=timeDefault)
		{ modulate(iset2CCModIndex, ccmodIndex, z, AdjustTime(t)); }
	void set2ModulatorFreq(float z, float  t=timeDefault)
		{ modulate(iset2ModulatorFreq, modFreq2, z, AdjustTime(t)); }
	void set2CMratio(float z, float t=timeDefault)
		{ modulate(iset2CMratio, cmRatio2, z, AdjustTime(t)); }
	void set2MCratio(float z, float t=timeDefault)
		{ set2CMratio(1./z, t); }
	void set2ModIndex(float z, float t=timeDefault)
		{ modulate(iset2ModIndex, modIndex2, z, AdjustTime(t)); }
	void set2CarFeedback(float z, float t=timeDefault)
		{ modulate(iset2CarFeedback, carFeedback2, z, AdjustTime(t)); }
	void set2ModFeedback(float z, float t=timeDefault)
		{ modulate(iset2ModFeedback, modFeedback2, z, AdjustTime(t)); }

	void setLowpassGain(float z, float t=timeDefault)
		{ modulate(isetLowpassGain, lowpassGain, z, AdjustTime(t)); }
	void setHighpassGain(float z, float t=timeDefault)
		{ modulate(isetHighpassGain, highpassGain, z, AdjustTime(t)); }

//	parameter setting
	void set1RatioMode(float z = 1.)
		{ getAlg()->set1RatioMode((int)z); }
	void set2RatioMode(float z = 1.)
		{ getAlg()->set2RatioMode((int)z); }

//	damp amplitude changes
	float dampingTime(void)	{ return 0.03; }

	int receiveMessage(const char * Message);

	fmmHand(fmmAlg* alg = new fmmAlg);
	~fmmHand() {}
};

class fmmActor : public VGeneratorActor
{
public:
	VHandler* newHandler() { return new fmmHand(); }
	fmmActor();
	~fmmActor() {}

	void sendDefaults(VHandler*);
	int	receiveMessage(const char*);

	void set1RatioMode(float f = 1.);
	void set1AllRatioMode(float f = 1.);
	void set1CarrierFreq(float f);
	void set1AllCarrierFreq(float f, float t = 0.);
	void set1ModulatorFreq(float f);
	void set1AllModulatorFreq(float f, float t = 0.);
	void set1CMratio(float f);
	void set1AllCMratio(float f, float t = 0.);
	void set1MCratio(float f);
	void set1AllMCratio(float f, float t = 0.);
	void set1ModIndex(float f);
	void set1AllModIndex(float f, float t = 0.);
	void set1CarFeedback(float f);
	void set1AllCarFeedback(float f, float t = 0.);
	void set1ModFeedback(float f);
	void set1AllModFeedback(float f, float t = 0.);

	void set2RatioMode(float f = 1.);
	void set2AllRatioMode(float f = 1.);
	void set2CarrierFreq(float f);
	void set2AllCarrierFreq(float f, float t = 0.);
	void set2CCratio(float f);
	void set2AllCCratio(float f, float t = 0.);
	void set2CCModIndex(float f);
	void set2AllCCModIndex(float f, float t = 0.);
	void set2ModulatorFreq(float f);
	void set2AllModulatorFreq(float f, float t = 0.);
	void set2CMratio(float f);
	void set2AllCMratio(float f, float t = 0.);
	void set2MCratio(float f);
	void set2AllMCratio(float f, float t = 0.);
	void set2ModIndex(float f);
	void set2AllModIndex(float f, float t = 0.);
	void set2CarFeedback(float f);
	void set2AllCarFeedback(float f, float t = 0.);
	void set2ModFeedback(float f);
	void set2AllModFeedback(float f, float t = 0.);

	void setAllLowpassGain(float f, float t = 0.);
	void setLowpassGain(float f);
	void setAllHighpassGain(float f, float t = 0.);
	void setHighpassGain(float f);

protected:
	float default1RatioMode, default1CarFreq, default1ModFreq, default1CMratio;
	float default1ModIndex, default1CarFeedback, default1ModFeedback;
	float default2RatioMode, default2CarFreq, default2ModFreq, default2CMratio;
	float default2CCratio, default2CCModIndex;
	float default2ModIndex, default2CarFeedback, default2ModFeedback;
	float defaultLowpassGain, defaultHighpassGain;

	ostream &dump(ostream &os, int tabs);
};

float FMM_NiceRatio(float z);
