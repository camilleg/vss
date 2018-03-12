#ifndef _ORDER1_H_
#define _ORDER1_H_

#include <cmath>

#include "VAlgorithm.h"
#include "VHandler.h"
#include "VGenActor.h"

#include "checkBounds.h"

//===========================================================================
//		order1FiltAlg 
//
//	class order1FiltAlg is a state-space implementation of a first-order
//	pole/zero pair. Use it to realize first-order lowpass, highpass, and
//	allpass (phase-shift) filters. The filter is implemented by mapping a
//	continuous-time state-space realization into a digital one through the
//	Bilinear Transform (BLT).
//
class order1FiltAlg : public VAlgorithm
{
private:
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
//	access members
	float	getFrequency(void)	{ return w0 / (2.0 * M_PI); }
	float	getLowpassGain(void)	{ return Alp; }
	float	getHighpassGain(void)	{ return Ahp; }
	float	getAllpassGain(void)	{ return Aap; }
	
//	parameter update members
	void	setFrequency(float f);
	void	setLowpassGain (float A);
	void	setHighpassGain (float A);
	void	setAllpassGain (float A);

//	utility members
	void	computeCoef(void);

//	sample generation
	void	generateSamples(int);

//	construction/destruction
		order1FiltAlg(void);
		~order1FiltAlg();

};	// end of class order1FiltAlg

//===========================================================================
//		order1FiltHand 
//
//	class order1FiltHand is a handler class for order1FiltAlg.
//
class order1FiltHand : public VHandler
{
private:
//	modulating parameters of order1FiltAlg
	float frequency;
	float LPgain;
	float HPgain;
	float APgain;

	enum { isetFrequency, isetLPGain, isetHPGain, isetAPGain };

protected:
//	Algorithm access:
// 	Define a version of getAlg() that returns a pointer to order1FiltAlg.
	order1FiltAlg * getAlg(void)	{ return (order1FiltAlg *) VHandler::getAlg(); }

public:
//	parameter modulation
	void SetAttribute(IParam iParam, float z);
	void setFrequency(float z, float t = timeDefault)
		{ modulate(isetFrequency, frequency, z, AdjustTime(t)); }
	void setLPGain(float z, float t = timeDefault)
		{ modulate(isetLPGain, LPgain, z, AdjustTime(t)); }
	void setHPGain(float z, float t = timeDefault)
		{ modulate(isetHPGain, HPgain, z, AdjustTime(t)); }
	void setAPGain(float z, float t = timeDefault)
		{ modulate(isetAPGain, APgain, z, AdjustTime(t)); }
	
//	damp amplitude changes
	float	dampingTime(void)	{ return 0.03; }

//	construction
	order1FiltHand(order1FiltAlg * alg = new order1FiltAlg);

	virtual void actCleanup(void);

//	destruction
virtual	~order1FiltHand() {}

//	message reception
	int receiveMessage(const char * Message);

};	// end of class order1FiltHand

//===========================================================================
//		order1FiltActor
//
//	class order1FiltActor is a processor actor class for order1FiltAlg
//
class order1FiltActor : public VGeneratorActor
{
public:
virtual	VHandler * newHandler(void)	{ return new order1FiltHand(); }

//	construction/destruction
	order1FiltActor(void);
virtual	~order1FiltActor() {}

virtual	void	sendDefaults(VHandler *);
virtual int	receiveMessage(const char * Message);

//	parameter setting members
	void	setFrequency(float f);
	void	setLPGain(float f);
	void	setHPGain(float f);
	void	setAPGain(float f);
	void	setAllFrequency(float f, float t = 0.);
	void	setAllLPGain(float f, float t = 0.);
	void	setAllHPGain(float f, float t = 0.);
	void	setAllAPGain(float f, float t = 0.);

protected:
//	default parameters
	float	defaultFrequency;
	float	defaultLPgain, defaultHPgain, defaultAPgain;

};	// end of class order1FiltActor


#endif // ndef _ORDER1_H_
