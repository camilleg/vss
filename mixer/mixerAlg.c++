#include "mixer.h"

//===========================================================================
//	mixerAlg constructor
//
mixerAlg::mixerAlg(void) :
	VAlgorithm(),
	channelNum(0),
	numInputs(MaxNumInput),
	matrix(0)
{
	Nchan(globs.nchansVSS);
	for (int i=0; i<MaxNumInput; i++)
	{
		fader[i]=0.;
		source[i]=NULL;
	}
}

//===========================================================================
//	mixerAlg destructor
//
mixerAlg::~mixerAlg()
{
}

//===========================================================================
//  mixerAlg set matrix amplitudes
//
void mixerAlg::setMatrixMode(int z)
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
	int temp;
	for (int i=0; i<MaxNumInput; i++)
	for (int j=0; j<MaxNumInput; j++)
	{
		temp = i*MaxNumInput+j;
		if (lin[temp]!=1000)
			faderm[i][j]=lin[temp];
	}
}

//===========================================================================
//	mixerAlg generateSamples
//
void
mixerAlg::generateSamples(int howMany)
{
	int nchans = globs.nchansVSS;
	float out[MaxNumChannels];
	for (int s=0; s<howMany; s++) 
	{
		for (int c=0; c<nchans; c++) out[c]=0.;
	 	if (!matrix)
		{
			for (int i=0; i<MaxNumInput; i++)
				if (source[i]!=NULL)
					for (int c=0; c<nchans; c++)
						out[c] += source[i]->Input(s,c) * fader[i];
			OutputNchan(out,s);
		}
		else
		{
			for (int i=0; i<numInputs; i++)
				if (source[i]!=NULL)
					for (int j=0; j<nchans; j++)
						out[j] += source[i]->Input(s,0) * faderm[i][j];
			OutputNchan(out,s);
		}
	}
}
