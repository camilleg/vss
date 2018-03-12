#include "biquad.h"

//===========================================================================
//	biquadFiltAlg constructor
//
biquadFiltAlg::biquadFiltAlg(void) :
	VAlgorithm(),
	wd(2.0 * M_PI * 1000.0),
	Q(1.0),
	Alp(1.0),
	Abp(0.0),
	Ahp(0.0),
	Aap(0.0),
	An(0.0)
{
	computeCoef();
	for(int c=0; c<MaxNumChannels; c++) { state[c] = stateZi1[c] = stateZi2[c] = output[c] = 0.0; }
}

//===========================================================================
//	biquadFiltAlg destructor
//
biquadFiltAlg::~biquadFiltAlg()
{
}

//===========================================================================
//	biquadFiltAlg setFrequency
//
//	set the corner frequency in Hz
//
void
biquadFiltAlg::setFrequency(float f)
{
	wd = f * 2.0 * M_PI;
#ifdef VSS_SOLARIS	//	 no single-precision support
	w0 = tan(wd * globs.OneOverSR / 2.0);
#else
	w0 = tanf(wd * globs.OneOverSR / 2.0);
#endif
	a0 = w0*w0;
	a1 = w0/Q;
	computeCoef();
}

//===========================================================================
//	biquadFiltAlg setResonance
//
//	set the resonance, Q, in the classical sense
//	i.e. Q=0.5 => a critically-damped system
//
void
biquadFiltAlg::setResonance(float Qi)
{
	Q = Qi;
	a1 = w0/Q;
	computeCoef();
}

//===========================================================================
//	biquadFiltAlg setLowpassGain
//
//	set the gain of the lowpass response
//
void
biquadFiltAlg::setLowpassGain(float A)
{
	Alp = A;
	computeCoef();
}

//===========================================================================
//	biquadFiltAlg setBandpassGain
//
//	set the gain of the bandpass response
//
void
biquadFiltAlg::setBandpassGain(float A)
{
	Abp = A;
	computeCoef();
}

//===========================================================================
//	biquadFiltAlg setHighpassGain
//
//	set the gain of the highpass response
//
void
biquadFiltAlg::setHighpassGain(float A)
{
	Ahp = A;
	computeCoef();
}

//===========================================================================
//	biquadFiltAlg setAllpassGain
//
//	set the gain of the allpass response
//
void
biquadFiltAlg::setAllpassGain(float A)
{
	Aap = A;
	computeCoef();
}

//===========================================================================
//	biquadFiltAlg setNotchGain
//
//	set the gain of the notch response
//
void
biquadFiltAlg::setNotchGain(float A)
{
	An = A;
	computeCoef();
}

//===========================================================================
//	biquadFiltAlg computeCoef
//
//	compute the filter coefficients from the parameter collection
//
void
biquadFiltAlg::computeCoef(void)
{
	// a0 is computed in setFrequency()
	// a1 is computed in setFrequency() and in setResonance()
	b0 = a0 * (	Alp			+ Aap	+ An	);
	b1 = a1 * (		Abp	 	- Aap		);
	b2 =      (			Ahp	+ Aap	+ An	);
	
	float	Den  = (a0 + a1 + 1.0) ;
	A0 = (a0 - a1 + 1.0) / Den ;
	A1 = 2.0 * (a0 - 1.0) / Den ;
	zB0 = (b0 - b1 + b2) / Den ;
	B1 = 2.0 * (b0 - b2) / Den ;
	B2 = (b0 + b1 + b2) / Den ;
}

//===========================================================================
//	biquadFiltAlg generateSamples
//
//	# of output channels = # of input channels, always.
//	filters are in channel-parallel, no cross-terms.
//
void
biquadFiltAlg::generateSamples(int howMany)
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
			state[c] = source->Input(s, c) - A1*stateZi1[c] - A0*stateZi2[c];

			// output equation
			output[c] = zB0*stateZi2[c] + B1*stateZi1[c] + B2*state[c];

			// state update
			stateZi2[c] = stateZi1[c];
			stateZi1[c] = state[c];
		}
		// stuff output
		OutputNchan(output, s);			
	}
}
