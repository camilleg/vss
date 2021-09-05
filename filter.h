#pragma once
#include "vssSrv.h"

//	Class filter is a state-space implementation of a first-order
//	pole/zero pair. Use it to realize first-order lowpass, highpass, and
//	allpass (phase-shift) filters. The filter is implemented by mapping a
//	continuous-time state-space realization into a digital one through the
//	Bilinear Transform (BLT).

class filter
{
private:
//	Internal synthesis stateholders.
	float za0, zb0, zb1;		// prewarped analog filter coefficients
	float zA0, zB0, zB1;		// BLT-mapped digital filter coefficients
	float stateZi1;				// first-order state variable
	float input, output;		// main filter input, output

//	Synthesis parameters.
	float w0;			// desired analog corner frequency, rad/sec
	float Alp, Ahp, Aap;		// Lowpass, Highpass, and Allpass gains

public:
//	Parameter access members.
	float getFrequency(void)	const { return w0 / (2.0 * M_PI); }
	float getLowpassGain(void)	const { return Alp; }
	float getHighpassGain(void)	const { return Ahp; }
	float getAllpassGain(void)	const { return Aap; }

//	Parameter update members.
	void setFrequency(float f)
		{
		const float fNyquist = globs.SampleRate / 2.0;
		if (f > fNyquist)
			f = fNyquist; // clamp at Nyquist for stability
		w0 = f * 2.0 * M_PI;
		za0 = tan(w0 * globs.OneOverSR / 2.0);
		computeCoef();
		}
	void setLowpassGain  (float A) { Alp = A; computeCoef(); }
	void setHighpassGain (float A) { Ahp = A; computeCoef(); }
	void setAllpassGain  (float A) { Aap = A; computeCoef(); }
	void setHiAllLopassGain  (float H, float A, float L)
		{ Ahp=H; Aap=A; Alp=L; computeCoef(); }

//	Input/output access members.
	float	getOutput(void)		{ return output; }
	void	setInput(float In)	{ input = In; }

//	Utility members.
	void computeCoef(void)
		{
		// za0 is computed in setFrequency()
		zb0 = za0 * (Alp - Aap);
		zb1 = 		 Ahp + Aap;
		zA0 = (za0 - 1.)  / (za0 + 1.);
		zB0 = (zb0 - zb1) / (za0 + 1.);
		zB1 = (zb0 + zb1) / (za0 + 1.);
		}

//	Sample generation.
	void computeSamp(void)
		{
		float state = input - zA0 * stateZi1;	// state equation
		output = zB0 * stateZi1 + zB1 * state;	// output equation
		stateZi1 = state;						// state update
		}

//	Construction/destruction.
	filter(void) :
	stateZi1(0.0),
	input(0.0),
	output(0.0),
	w0(2.0 * M_PI * 1000.0),
	Alp(1.0),
	Ahp(0.0),
	Aap(0.0)
		{ setFrequency(1000.); }
	~filter() {}
};
