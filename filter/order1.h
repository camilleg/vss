#pragma once
#include <cmath>
#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

#include "checkBounds.h"

//	class order1FiltAlg is a state-space implementation of a first-order
//	pole/zero pair. Use it to realize first-order lowpass, highpass, and
//	allpass (phase-shift) filters. The filter is implemented by mapping a
//	continuous-time state-space realization into a digital one through the
//	Bilinear Transform (BLT).
//
class order1FiltAlg : public VAlgorithm
{
//	internal synthesis stateholders
	float	a0, b0, b1;			// prewarped analog filter coefficients
	float	A0, zB0, B1;			// BLT-mapped digital filter coefficients
	float	state[MaxNumChannels];		// first-order state variable, current
	float	stateZi1[MaxNumChannels];	// first-order state variable, last
	float	output[MaxNumChannels];		// output temporary stateholder

//	synthesis parameters
	float	w0;			// desired analog corner frequency, rad/sec
	float	Alp, Ahp, Aap;		// Lowpass, Highpass, and Allpass gains

public:
	float getFrequency() const { return w0 / (2.0 * M_PI); }
	float getLowpassGain() const { return Alp; }
	float getHighpassGain() const { return Ahp; }
	float getAllpassGain() const { return Aap; }
	
	void	setFrequency(float f);
	void	setLowpassGain (float A);
	void	setHighpassGain (float A);
	void	setAllpassGain (float A);

	void	computeCoef();
	void	generateSamples(int);
	order1FiltAlg();
	~order1FiltAlg();
};

class order1FiltHand : public VHandler
{
	float frequency, LPgain, HPgain, APgain;
	enum { isetFrequency, isetLPGain, isetHPGain, isetAPGain };

protected:
	order1FiltAlg* getAlg() { return (order1FiltAlg*)VHandler::getAlg(); }

public:
	void SetAttribute(IParam iParam, float z);
	void setFrequency(float z, float t = timeDefault)
		{ modulate(isetFrequency, frequency, z, AdjustTime(t)); }
	void setLPGain(float z, float t = timeDefault)
		{ modulate(isetLPGain, LPgain, z, AdjustTime(t)); }
	void setHPGain(float z, float t = timeDefault)
		{ modulate(isetHPGain, HPgain, z, AdjustTime(t)); }
	void setAPGain(float z, float t = timeDefault)
		{ modulate(isetAPGain, APgain, z, AdjustTime(t)); }
	
	float dampingTime() { return 0.03; }

	order1FiltHand(order1FiltAlg* alg = new order1FiltAlg);
	void actCleanup();
	~order1FiltHand() {}
	int receiveMessage(const char*);
};

class order1FiltActor : public VGeneratorActor
{
public:
	VHandler* newHandler() { return new order1FiltHand(); }
	order1FiltActor();
	~order1FiltActor() {}
	void sendDefaults(VHandler *);
	int receiveMessage(const char*);

	void	setFrequency(float f);
	void	setLPGain(float f);
	void	setHPGain(float f);
	void	setAPGain(float f);
	void	setAllFrequency(float f, float t = 0.);
	void	setAllLPGain(float f, float t = 0.);
	void	setAllHPGain(float f, float t = 0.);
	void	setAllAPGain(float f, float t = 0.);

protected:
	float defaultFrequency, defaultLPgain, defaultHPgain, defaultAPgain;
};
