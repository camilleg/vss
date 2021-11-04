#pragma once
#include <cmath>
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

#include "checkBounds.h"

//	class biquadFiltAlg is a state-space implementation of a second-order
//	"biquad" filter. Use it to realize classic second-order lowpass, 
//	bandpass, highpass, notch, and allpass (phase-shift) filters. The 
//	filter is implemented by mapping a continuous-time state-space 
//	realization into a digital one through the Bilinear Transform (BLT).
//
class biquadFiltAlg : public VAlgorithm
{
//	internal synthesis stateholders
	float	w0;				// prewarped analog corner frequency, rad/sec
	float	a0, a1, b0, b1, b2;		// prewarped analog filter coefficients
	float	A0, A1, zB0, B1, B2;		// BLT-mapped digital filter coefficients
	float	state[MaxNumChannels];		// second-order state variable, current
	float	stateZi1[MaxNumChannels];	// second-order state variable, next
	float	stateZi2[MaxNumChannels];	// second-order state variable, last
	float	output[MaxNumChannels];		// output temporary stateholder

//	synthesis parameters
	float	wd;				// desired analog corner frequency, rad/sec
	float	Q;				// desired analog Q, or resonance factor
	float	Alp, Abp, Ahp, Aap, An;		// Low-Band-High-Allpass and Notch gains

public:
	float getFrequency() const { return wd / (2.0 * M_PI); }
	float getResonance() const { return Q; }
	float getLowpassGain() const { return Alp; }
	float getBandpassGain() const { return Abp; }
	float getHighpassGain() const { return Ahp; }
	float getAllpassGain() const { return Aap; }
	float getNotchGain() const { return An; }
	
	void	setFrequency(float f);
	void	setResonance(float Qi);
	void	setLowpassGain (float A);
	void	setBandpassGain (float A);
	void	setHighpassGain (float A);
	void	setAllpassGain (float A);
	void	setNotchGain (float A);

	void	computeCoef();
	void	generateSamples(int);
	biquadFiltAlg();
	~biquadFiltAlg();
};

class biquadFiltHand : public VHandler
{
	float frequency;
	float resonance;
	float LPgain;
	float BPgain;
	float HPgain;
	float APgain;
	float Ngain;
	enum { isetFrequency, isetResonance, isetLPGain, isetBPGain, isetHPGain, isetAPGain, isetNGain };

protected:
	biquadFiltAlg* getAlg() { return (biquadFiltAlg*)VHandler::getAlg(); }

public:
	void SetAttribute(IParam iParam, float z);
	void setFrequency(float z, float t = timeDefault)
		{ modulate(isetFrequency, frequency, z, AdjustTime(t)); }
	void setResonance(float z, float t = timeDefault)
		{ modulate(isetResonance, resonance, z, AdjustTime(t)); }
	void setLPGain(float z, float t = timeDefault)
		{ modulate(isetLPGain, LPgain, z, AdjustTime(t)); }
	void setBPGain(float z, float t = timeDefault)
		{ modulate(isetBPGain, BPgain, z, AdjustTime(t)); }
	void setHPGain(float z, float t = timeDefault)
		{ modulate(isetHPGain, HPgain, z, AdjustTime(t)); }
	void setAPGain(float z, float t = timeDefault)
		{ modulate(isetAPGain, APgain, z, AdjustTime(t)); }
	void setNGain(float z, float t = timeDefault)
		{ modulate(isetNGain, Ngain, z, AdjustTime(t)); }
	
	float dampingTime() { return 0.03; }
	biquadFiltHand(biquadFiltAlg* alg = new biquadFiltAlg);
	void actCleanup();
	~biquadFiltHand() {}
	int receiveMessage(const char*);
};

class biquadFiltActor : public VGeneratorActor
{
public:
	VHandler* newHandler() { return new biquadFiltHand(); }
	biquadFiltActor();
	~biquadFiltActor() {}
	void sendDefaults(VHandler*);
	int receiveMessage(const char*);

	void	setFrequency(float f);
	void	setResonance(float f);
	void	setLPGain(float f);
	void	setBPGain(float f);
	void	setHPGain(float f);
	void	setAPGain(float f);
	void	setNGain(float f);
	void	setAllFrequency(float f, float t = 0.);
	void	setAllResonance(float f, float t = 0.);
	void	setAllLPGain(float f, float t = 0.);
	void	setAllBPGain(float f, float t = 0.);
	void	setAllHPGain(float f, float t = 0.);
	void	setAllAPGain(float f, float t = 0.);
	void	setAllNGain(float f, float t = 0.);

protected:
	float	defaultFrequency, defaultResonance;
	float	defaultLPgain, defaultBPgain, defaultHPgain, defaultAPgain, defaultNgain;
};
