#include "order1.h"

//===========================================================================
//	order1FiltAlg constructor
//
order1FiltAlg::order1FiltAlg(void) :
	VAlgorithm(),
	w0(2.0 * M_PI * 1000.0),
	Alp(1.0),
	Ahp(0.0),
	Aap(0.0)
{
	computeCoef();
	for(int c=0; c<MaxNumChannels; c++) { state[c] = stateZi1[c] = output[c] = 0.0; }
}

//===========================================================================
//	order1FiltAlg destructor
//
order1FiltAlg::~order1FiltAlg()
{
}

//===========================================================================
//	order1FiltAlg setFrequency
//
//	set the corner frequency in Hz
//
void
order1FiltAlg::setFrequency(float f)
{
	w0 = f * 2.0 * M_PI;
#ifdef VSS_SOLARIS	// no single-precision support
	a0 = tan(w0 * globs.OneOverSR / 2.0);
#else
	a0 = tanf(w0 * globs.OneOverSR / 2.0);
#endif
	computeCoef();
}

//===========================================================================
//	order1FiltAlg setLowpassGain
//
//	set the gain of the lowpass response
//
void
order1FiltAlg::setLowpassGain(float A)
{
	Alp = A;
	computeCoef();
}

//===========================================================================
//	order1FiltAlg setHighpassGain
//
//	set the gain of the highpass response
//
void
order1FiltAlg::setHighpassGain(float A)
{
	Ahp = A;
	computeCoef();
}

//===========================================================================
//	order1FiltAlg setAllpassGain
//
//	set the gain of the allpass response
//
void
order1FiltAlg::setAllpassGain(float A)
{
	Aap = A;
	computeCoef();
}

//===========================================================================
//	order1FiltAlg computeCoef
//
//	compute the filter coefficients from the parameter collection
//
void
order1FiltAlg::computeCoef(void)
{
	// a0 is computed in setFrequency()
	b0 = a0 * (	Alp		- Aap	);
	b1 = 			Ahp 	+ Aap	;
	
	A0 = (a0 - 1.)/(a0 + 1.);
	zB0 = (b0 - b1)/(a0 + 1.);
	B1 = (b0 + b1)/(a0 + 1.);
}

//===========================================================================
//	order1FiltAlg generateSamples
//
//
//	# of output channels = # of input channels, always.
//	filters are in channel-parallel, no cross-terms.
//
void
order1FiltAlg::generateSamples(int howMany)
{
	if (source == NULL)
	{
		ClearBuffer(howMany);
		return;
	}

	int nchans = source->Nchan();
	Nchan(nchans);
	
	for (int s = 0; s < howMany; s++)
	{
		for (int c=0; c<nchans; c++)
		{
			// state equation
			state[c] = source->Input(s, c) - A0 * stateZi1[c];
		
			// output equation
			output[c] = zB0 * stateZi1[c] + B1 * state[c];

			// state update
			stateZi1[c] = state[c];
		}
		// stuff output
		OutputNchan(output, s);
	}
}
