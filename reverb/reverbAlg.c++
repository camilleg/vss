#include "reverb.h"
#include "vssSrv.h"				// For globs.SampleRate
#include <cmath>				// For pow()

//===========================================================================
//	reverbAlg constructor
//
reverbAlg::reverbAlg(void) :
	VAlgorithm(),
	MaxT60(20.),
	idle(0)
{
	long length1 = 1 << int(ceil( log(globs.SampleRate*.5 ) / log(2.) ));
	long length2 = 1 << int(ceil( log(globs.SampleRate*.1 ) / log(2.) ));
	long length3 = 1 << int(ceil( log(globs.SampleRate*.01) / log(2.) ));

	int i;
	for (i=0; i<MaxEarlyRefNum; i++)
		delayLineEarlyRef[i] = new DLineNcopy(length1);
	for (i=0; i<MaxCombNum; i++)
	{
		delayLineComb[i] = new DLineNcopy(length2);
		combLastOut[i] = 0.;
	}
	for (i=0; i<MaxAllPassNum; i++)
	{
		delayLineAllPass[i] = new DLineNcopy(length3);
		allPassCoeff[i] = .7;
	}
	lastin1 = lastin2 = lastout1 = lastout2 = 0.;

	float temp[ParaNum] =
	{.5, 1., .3, .15, 5., 0.1, 0., 0., 0., 0.,
	2, 20., 18., 0., 0., 0., 0., 0., 0., 0.,
	2, .82, .63, 0., 0., 0., 0., 0., 0., 0.,
	4, 50., 56., 61., 68., 0., 0., 0., 0., 0.,
	1, 6., 0., 0.};
/*	{... 6, 20., 18., 20., 15., 19., 10., 0., 0., 0.,
	6, .82, .82, .63, .72, .53, .47, 0., 0., 0.,
	6, 50., 56., 61., 68., 72., 78., 0., 0., 0.,
	3, 6., 1.7, 0.63};
*/
	this->setPara(temp);
	Nchan(1);
}

//===========================================================================
//	reverbAlg destructor
//
reverbAlg::~reverbAlg()
{
	int i;
	for (i=0; i<MaxEarlyRefNum; i++) delete delayLineEarlyRef[i];
	for (i=0; i<MaxCombNum; i++) delete delayLineComb[i];
	for (i=0; i<MaxAllPassNum; i++) delete delayLineAllPass[i];
/*
	printf("revMix\trevGain\trevTime\trevBrt\tdampR\tpole\tER\tComb\tAP\n");
	printf("%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%d\t%d\t%d\n",
		revMix, revGain, t60/MaxT60, BW, dampRatio, pole, 
		earlyRefNum, combNum, allPassNum);
	for (i=0; i<earlyRefNum; i++)
		printf("Early reflection delay %f, coeff %f\n",
			earlyRefDelay[i],earlyRefCoeff[i]);
	for (i=0; i<combNum; i++)
		printf("Comb delay %f, coeff %f, damp %f\n",
			combDelay[i],combCoeff[i],combDamp[i]);
	for (i=0; i<allPassNum; i++) 
		printf("Allpass delay %f, coeff %f\n",
			allPassDelay[i],allPassCoeff[i]);
*/
}

//===========================================================================
//	reverbAlg set parameters: Presets
//
void reverbAlg::setPara(float * p)
{
		revMix = p[0];
		revGain = p[1];
		t60 = p[2]*MaxT60;
		BW = p[3];
		dampRatio = p[4];
		pole = p[5];
		earlyRefNum = int(p[10]);
		this->setEarlyRefDelay(p+11);
		this->setEarlyRefCoeff(p+21);
		combNum = int(p[30]);
		this->setCombDelay(p+31);
		allPassNum = int(p[40]);
		this->setAllPassDelay(p+41);
}

void reverbAlg::setPreset(float n)
{
	switch (int(n)) {
	case SMALLROOM:
		{ float temp1[ParaNum] =
		{.4, 1., .3, .15, 5., .25, 0., 0., 0., 0.,
		5, 20., 23., 30., 35., 39., 0., 0., 0., 0.,
		5, .82, .82, .63, .72, .53, 0., 0., 0., 0.,
		6, 50., 56., 61., 68., 72., 78., 0., 0., 0.,
		1, 6., 0., 0.};
		this->setPara(temp1);
		break; }
	case HALL:
		{ float temp2[ParaNum] =
		{.4, 2., .6, .15, 8., .25, 0., 0., 0., 0.,
		5, 20., 23., 30., 35., 39., 0., 0., 0., 0.,
		5, .82, .82, .63, .72, .53, 0., 0., 0., 0.,
		6, 50., 56., 61., 68., 72., 78., 0., 0., 0.,
		3, 6., 1.7, 0.63};
		this->setPara(temp2);
		break; }
	case ECHO1:
		{ float temp3[ParaNum] =
		{.5, 2., .5, .75, 10., .5, 0., 0., 0., 0.,
		1., 100., 0., 0., 0., 0., 0., 0., 0., 0.,
		1., 0.82, 0., 0., 0., 0., 0., 0., 0., 0.,
		1, 100., 0., 0., 0., 0.,
		0, 0., 0., 0.};
		this->setPara(temp3);
		break; }
	case CANYON:
		{ float temp4[ParaNum] =
		{.5, 3., .7, .1, 10., .45, 0., 0., 0., 0.,
		6, 500., 500., 450., 450., 400., 400., 0., 0., 0., 
		6, 0., .92, 0., .8, 0., .6, 0., 0., 0.,
		4, 50., 56., 61., 68., 0., 0., 0., 0., 0.,
		2, 6., 1.7, 0.};
		this->setPara(temp4);
		break; }
	default:
		{ float temp0[ParaNum] =
		{.5, 1., .3, .15, 5., 0.1, 0., 0., 0., 0.,
		2, 20., 18., 0., 0., 0., 0., 0., 0., 0.,
		2, .82, .63, 0., 0., 0., 0., 0., 0., 0.,
		4, 50., 56., 61., 68., 0., 0., 0., 0., 0.,
		1, 6., 0., 0.};
		this->setPara(temp0);
		break; }
	}
}

//===========================================================================
//	reverbAlg set parameters: Early reflections
//

void reverbAlg::setEarlyRefDelay(float * delay)
{
	for (int i=0; i<earlyRefNum; i++)
	{
		earlyRefDelay[i] = delay[i];
		delay[i] *= .001;
		delayLineEarlyRef[i]->setDelay(delay[i]);
	}
}

void reverbAlg::setEarlyRefCoeff(float * coeff)
{
	for (int i=0; i<earlyRefNum; i++)
		earlyRefCoeff[i] = coeff[i];
}

//===========================================================================
//	reverbAlg set parameters: Comb filters
//
void reverbAlg::setCombDelay(float * delay)
{
	for (int i=0; i<combNum; i++)
	{
		combDelay[i] = delay[i];
		delay[i] *= .001;
		delayLineComb[i]->setDelay(delay[i]);
	}
	this->setComb();
}

void reverbAlg::setComb()
{
	double delay, glow[MaxCombNum], ghi[MaxCombNum];
	for (int i=0; i<combNum; i++)
	{
		delay = combDelay[i] * .001 / t60;
		glow[i] = pow(0.001, delay);
		ghi[i] = pow(0.001, delay * dampRatio);
		combDamp[i] = (glow[i] - ghi[i]) / (glow[i] + ghi[i]);
    		combCoeff[i] = (1. - combDamp[i] ) * glow[i];
		combLastOut[i] = 0.;
	}
}

//===========================================================================
//	reverbAlg set parameters: All-pass filters
//
void reverbAlg::setAllPassDelay(float * delay)
{
	for (int i=0; i<allPassNum; i++)
	{
		allPassDelay[i] = delay[i];
		delay[i] *= .001;
		delayLineAllPass[i]->setDelay(delay[i]);
	}
}
		
//===========================================================================
//	reverbAlg generateSamples
//
void reverbAlg::generateSamples(int howMany)
{
	for (int s = 0; s < howMany; s++)
	{
		float out = source->Input(s,0);
		if (!idle)
			out = tick(out);
		Output(out, s, 0);
	}
}

float reverbAlg::tick(float in)
{
	float samp, temp, temp1, temp2;
	samp = in;
	temp = temp1 = temp2 = 0.;
	int i;

// BPF
	temp = samp - lastin2 + pole * lastout1;
	lastin2 = lastin1;
	lastin1 = samp;
	lastout1 = temp;
	samp = lastout1 * .5;

// Early reflections
	temp = samp;
	for (i=0; i<earlyRefNum; i++)
	{
		samp = delayLineEarlyRef[i]->tick(samp);
		temp += earlyRefCoeff[i] * samp;
	}
	samp = temp / (float(earlyRefNum) + 1.);

// Comb filters
	for (i=0; i<combNum; i++)
	{
		temp2 = delayLineComb[i]->tick
			(samp + combCoeff[i] * combLastOut[i]);
		combLastOut[i] = temp2+combDamp[i]*combLastOut[i];
		temp1 += temp2;
	}
	samp = temp1 / (float(combNum) + 1.);

// Allpass filters
	for (i=0; i<allPassNum; i++)
	{
		temp1 = delayLineAllPass[i]->lastOut();
		temp2 = allPassCoeff[i] * (samp - temp1);
		delayLineAllPass[i]->tick(samp + temp2);
		samp =  temp1 + temp2;
	}
	
// LPF
	temp = lastout2 + BW * (samp - lastout2);
	lastout2 = temp;
	samp = lastout2;

// Mix and gain
	samp *= revGain;
	samp = in + revMix * ( samp - in );	// (1-revMix)*in+revMix*samp

	return samp;
}

//==========================================================================
//	non-interpolating delay line 
//
DLineNcopy :: DLineNcopy(long len)
{
	length = len;
	lengthm1 = length - 1;
	if ( (inputs = new float[length]) == 0 )
		{ cerr << "Memory allocation error in reverbAlg.\n"; exit(-1); }
	clear();
	inPoint = 0;
	outPoint = 0;
	lastOutput = 0.;
}

DLineNcopy :: ~DLineNcopy()
{
    delete[] inputs;
}

void DLineNcopy :: clear()
{
    for (long i=0; i<length; i++) inputs[i] = 0.;
}

void DLineNcopy :: setDelay(float lagSec)
{
	long lagSample;
	lagSample = long (lagSec * globs.SampleRate);
					// integer part of delay in samples
	lagSample += !(lagSample%2);
	while (!this->isprime(lagSample)) lagSample += 2;
//	printf("prime %d\n",lagSample);
	outPoint = inPoint - lagSample;		// read chases write
	outPoint &= lengthm1;			// modulo maximum length
	clear();
}

float DLineNcopy :: tick(float samp)
{                                              
	inputs[inPoint++] = samp;
	inPoint &= lengthm1;
	lastOutput = inputs[outPoint++];
	outPoint &= lengthm1;
	return lastOutput;
}

int DLineNcopy :: isprime(int val)
{
	for (int i=3; i<(int)sqrt((double)val)+1; i+=2) 
		if (!(val%i)) return 0;		// even
	return 1;				// prime
}

#ifdef UNUSED

float reverbAlg::tickBPF(float in, int c)
{
	lastout1[c] = in - lastin2[c] + pole * lastout1[c];
	lastin2[c] = lastin1[c];
	lastin1[c] = in; 
	return lastout1[c] * .5;
}

float reverbAlg::tickLPF(float in, int c)
{
	lastout2[c] = lastout2[c] + BW * (in - lastout2[c]);
	return lastout2[c];
}	

float reverbAlg::tickEarlyRef(float in, int c)
{
	float temp;
	temp = in;
	for (int i=0; i<earlyRefNum; i++)
	{
		in = delayLineEarlyRef[c][i]->tick(in);
		temp += earlyRefCoeff[i] * in;
	}
	return temp;
}

float reverbAlg::tickComb(float in, int c)
{
	float temp1 = 0., temp2;
	for (int i=0; i<combNum; i++)
	{
		temp2 = delayLineComb[c][i]->tick
			(in + combCoeff[i] * combLastOut[c][i]);
		combLastOut[c][i] = temp2+combDamp[i]*combLastOut[c][i];
		temp1 += temp2;
	}
	return temp1 / float(combNum);
}

float reverbAlg::tickAllPass(float in, int c)
{
	float temp1, temp2;
	for (int i=0; i<allPassNum; i++)
	{
		temp1 = delayLineAllPass[c][i]->lastOut();
		temp2 = allPassCoeff[i] * (in - temp1);
		delayLineAllPass[c][i]->tick(in + temp2);
		in = temp1 + temp2;
	}
	return in;
}

#endif // UNUSED
