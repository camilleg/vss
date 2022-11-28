#include "mixer.h"

mixerAlg::mixerAlg():
	VAlgorithm(),
	channelNum(0),
	numInputs(MaxNumInput),
	matrix(false)
{
	Nchan(Nchans());
	for (int i=0; i<MaxNumInput; ++i) {
		fader[i] = 0.0;
		source[i] = nullptr;
	}
}

mixerAlg::~mixerAlg()
{
}

// Set matrix amplitudes.
void mixerAlg::setMatrixMode(bool z)
{
	matrix = z;
	if (matrix)
	{
		float panAmps[MaxNumChannels];
		for (int i=0; i<MaxNumChannels; i++)
			panAmps[i] = 1.;
		setPanAmps(panAmps);
	}
}

void mixerAlg::setMatrixAmp(float * lin)
{
	for (int i=0; i<MaxNumInput; i++)
	for (int j=0; j<MaxNumInput; j++)
	{
		const auto temp = i*MaxNumInput+j;
		if (lin[temp]!=1000)
			faderm[i][j]=lin[temp];
	}
}

void mixerAlg::generateSamples(int howMany)
{
	const auto nchans = Nchans();
	for (int s=0; s<howMany; s++) 
	{
		float out[MaxNumChannels] = {};
		if (matrix) {
			for (int i=0; i<numInputs; i++)
				if (source[i])
					for (int j=0; j<nchans; j++)
						out[j] += source[i]->Input(s,0) * faderm[i][j];
		} else {
			for (int i=0; i<MaxNumInput; i++)
				if (source[i])
					for (int c=0; c<nchans; c++)
						out[c] += source[i]->Input(s,c) * fader[i];
		}
		OutputNchan(out,s);
	}
}
