#include "delay.h"

//===========================================================================
delayAlg::delayAlg(void) :
	VAlgorithm(),
	delaySeconds(0.)
{
	// +5 chickenfactor; * 2 means 2 seconds max.
	length = (long)(globs.SampleRate + 5) * 2 * MaxNumChannels;
	for (int i=0;i<MaxNumChannels; i++)
		inputs[i] = new float[length];

	clear();
	inPoint = 0;
	outPoint = length >> 1;
}

//===========================================================================
delayAlg::~delayAlg()
{
	for (int i=0;i<MaxNumChannels; i++)
		delete [] inputs[i];
}

//===========================================================================
void delayAlg::setDelay(float lag)
{
	delaySeconds = lag *= globs.SampleRate;
	float outputPointer = inPoint - lag + 2; // outPoint chases inpoint
	while (outputPointer<0)
		outputPointer += length;			// modulo table length
	outPoint = (long) outputPointer;		// Integer part of delay
	alpha = 1.0 + outPoint - outputPointer;	// fractional part of delay
	if (alpha<0.1)
		{
		outputPointer += 1.0;	// Hack to avoid pole/zero cancellation.
		outPoint += 1;			// Keeps allpass delay in range of .1 to 1.1
		alpha += 1.0;
		}
	coeff = (1.0 - alpha) / (1.0 + alpha); // coefficient for all pass
}

void delayAlg::setFB(float z)
{
	if (z<0.) z=0.;
	if (z>.9999) z=.9999;
	zFB = z;
}

//===========================================================================
void delayAlg::clear(void)
{
	for (int i=0; i<MaxNumChannels; i++)
		ZeroFloats(inputs[i], length);
	ZeroFloats(lastIn, MaxNumChannels);
	ZeroFloats(lastOutput, MaxNumChannels);
}

//===========================================================================
//	delayAlg generateSamples
//
//	# of output channels = # of input channels, always.
//	Delays are in channel-parallel, no cross-terms.
//
void
delayAlg::generateSamples(int howMany)
{
	int s;
	int nchans = source->Nchan();
	Nchan(nchans);

	if (nchans == 1)
		{
		// Optimized common case.
		for (s = 0; s < howMany; s++)
			{
			inputs[0][inPoint] = source->Input(s,0) + zFB * lastOutput[0];
			inPoint++;
			if (inPoint == length)
				inPoint -= length;
			float temp0 = inputs[0][outPoint];
			outPoint++;
			if (outPoint == length)
				outPoint -= length;
			lastOutput[0] = -coeff * lastOutput[0];
			lastOutput[0] += lastIn[0] + (coeff * temp0);
			lastIn[0] = temp0;
			Output(lastOutput[0], s);
			}
		}
	else
		{
		int c;
		float temp[MaxNumChannels] = {0};

		for (s = 0; s < howMany; s++)
			{
			for (c=0; c<nchans; c++)
				inputs[c][inPoint] = source->Input(s, c) + zFB * lastOutput[c];
			for (c=0; c<nchans; c++)
				temp[c] = inputs[c][outPoint];

			inPoint++;
			if (inPoint >= length)
				inPoint -= length;
			outPoint++;
			if (outPoint >= length)
				outPoint -= length;

			for (c=0; c<nchans; c++)
				{
				lastOutput[c] = -coeff * lastOutput[c] +
								lastIn[c] + (coeff * temp[c]);
				lastIn[c] = temp[c];
				}
			OutputNchan(lastOutput, s);
			}
		}
}
