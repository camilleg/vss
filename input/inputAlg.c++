#include "input.h"

inputAlg::inputAlg(void) :
	VAlgorithm()
{
}

inputAlg::~inputAlg()
{
}

void inputAlg::generateSamples(int howMany)
{
	const float* pz = VssInputBuffer();
	if (!pz)
		return;
	int nchansIn = NchansIn();
	Nchan(nchansIn);

#ifdef VSS_WINDOWS

	static int dpz = 0;
	pz += dpz;
	dpz += howMany;
	if (dpz >= MaxSampsPerBuffer)
		dpz = 0;

	for (int i = 0; i < howMany; i++)
		Output(pz[i], i);

#else

//	static float zMaxPrev = 0.f;
	float zMax = 0.f;
	for (int i = 0; i < howMany; i++)
		{
		float rgz[MaxNumChannels];
		for (int j = 0; j < nchansIn; j++)
			{
			float z = rgz[j] = pz[i*MaxNumChannels + j];
			if (z > zMax)
				zMax = z;
			else if (-z > zMax)
				zMax = -z;
			}
		OutputNchan(rgz, i);
		}

	if (zMax >= 0.9995)
		cerr <<"vss warning: input too loud (hard clipping).\n";

//	{
//	static int _=0;
//	if (_<9) _++; else { _=0;
//	printf("\t\t\t\t\tinp: M %6.4f    dM %6.4f\n", zMax, fabs(zMax-zMaxPrev));
//	}
//	}
//	zMaxPrev = zMax;

#endif
}
