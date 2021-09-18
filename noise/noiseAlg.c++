#include "noise.h"

noiseAlg::noiseAlg():
	VAlgorithm(),
	fCutoff(500.),
	order(1),
	decimationPeriod(637.8e-6),
	noizTimer(0.),
	sampValue(0.),
	sampValuePrev(0.)
{
}

void noiseAlg::generateSamples(int howMany)
{
	for (int j = 0; j < howMany; j++)
	{
		noizTimer += globs.OneOverSR;
		if (noizTimer > decimationPeriod)
		{
			sampValuePrev = sampValue;	// update delay element
			sampValue = drand48() - 0.5f;	// uniform distr noise over [-0.5, 0.5)
			noizTimer -= decimationPeriod;	// modulo timer ("phase wrapping")
		}
		if (order == 0)
			// zero-order hold: value held constant over decimation period
			Output(sampValue * 0.667f, j);	// scale by 2/3 to balance power wrt o=1
		else	
			// first-order hold: linear interp over decimation period
			Output(Lerp(sampValuePrev, sampValue, noizTimer*fCutoff), j);
	}
}

//	Set effective filter order. For time-width = decim-period = P:
//
//	Filter Order		Eff. impulse response	Eff. freq response
//	------------------------------------------------------------------
//	0 (Zero-Order-Hold)	Pulse, rect(t/P)	P*Sinc(P*f)
//	1 (First-Order-Hold)	Triangle, tri(t/P)	P*Sinc^2(P*f)
void noiseAlg::setOrder(int ord)
{
	order = ord;
}

//	Set decimation period. The inverse of this period is the frequency where
//	of the first zero occurs in the sinc or sinc^2 output spectra. The output
//	noise spectrum is lowpass in nature, with the -3dB point, or cuttoff frequency,
//	occurring as follows
//
//	Filter Order	-3dB point
//	----------------------------------
//	0		0.44295 * fCutoff
//	1		0.31890 * fCutoff
void noiseAlg::setCutoff(float fHz)
{
	fCutoff = fHz;
	decimationPeriod = 1.0f / fCutoff;
	// set hard-bound at SamplePeriod
	if (decimationPeriod < globs.OneOverSR)
		decimationPeriod = globs.OneOverSR;
}
