#ifndef _BIQUAD_H_
#define _BIQUAD_H_

#include <cmath>

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

#include "checkBounds.h"

//===========================================================================
//		biquadFiltAlg 
//
//	class biquadFiltAlg is a state-space implementation of a second-order
//	"biquad" filter. Use it to realize classic second-order lowpass, 
//	bandpass, highpass, notch, and allpass (phase-shift) filters. The 
//	filter is implemented by mapping a continuous-time state-space 
//	realization into a digital one through the Bilinear Transform (BLT).
//
class biquadFiltAlg : public VAlgorithm
{
private:
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
//	access members
	float	getFrequency(void)	{ return wd / (2.0 * M_PI); }
	float	getResonance(void)	{ return Q; }
	float	getLowpassGain(void)	{ return Alp; }
	float	getBandpassGain(void)	{ return Abp; }
	float	getHighpassGain(void)	{ return Ahp; }
	float	getAllpassGain(void)	{ return Aap; }
	float	getNotchGain(void)	{ return An; }
	
//	parameter update members
	void	setFrequency(float f);
	void	setResonance(float Qi);
	void	setLowpassGain (float A);
	void	setBandpassGain (float A);
	void	setHighpassGain (float A);
	void	setAllpassGain (float A);
	void	setNotchGain (float A);

//	utility members
	void	computeCoef(void);

//	sample generation
	void	generateSamples(int);

//	construction/destruction
		biquadFiltAlg(void);
		~biquadFiltAlg();

};	// end of class biquadFiltAlg

//===========================================================================
//		biquadFiltHand 
//
//	class biquadFiltHand is a handler class for biquadFiltAlg.
//
class biquadFiltHand : public VHandler
{
private:
//	modulating parameters of biquadFiltAlg
	float frequency;
	float resonance;
	float LPgain;
	float BPgain;
	float HPgain;
	float APgain;
	float Ngain;

	enum { isetFrequency, isetResonance, isetLPGain, isetBPGain, isetHPGain, isetAPGain, isetNGain };

protected:
//	Algorithm access:
// 	Define a version of getAlg() that returns a pointer to biquadFiltAlg.
	biquadFiltAlg * getAlg(void)	{ return (biquadFiltAlg *) VHandler::getAlg(); }

public:
//	parameter modulation
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
	
//	damp amplitude changes
	float	dampingTime(void)	{ return 0.03; }

//	construction
	biquadFiltHand(biquadFiltAlg * alg = new biquadFiltAlg);

	virtual void actCleanup(void);

//	destruction
virtual	~biquadFiltHand() {}

//	message reception
	int receiveMessage(const char * Message);

};	// end of class biquadFiltHand

//===========================================================================
//		biquadFiltActor
//
//	class biquadFiltActor is a processor actor class for biquadFiltAlg
//
class biquadFiltActor : public VGeneratorActor
{
public:
virtual	VHandler * newHandler(void)	{ return new biquadFiltHand(); }

//	construction/destruction
	biquadFiltActor(void);
virtual	~biquadFiltActor() {}

virtual	void	sendDefaults(VHandler *);
virtual int	receiveMessage(const char * Message);

//	parameter setting members
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
//	default parameters
	float	defaultFrequency, defaultResonance;
	float	defaultLPgain, defaultBPgain, defaultHPgain, defaultAPgain, defaultNgain;

};	// end of class biquadFiltActor


#endif // ndef _BIQUAD_H_
