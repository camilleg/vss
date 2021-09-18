#include "amplAnalyzer.h"

// Constructor.

AmplAlg::AmplAlg(void) :
	VAlgorithm(),
	fOneShot(0),
	fContinuous(0),
	wDuration(0),
	iDuration(0),
	zMax(0.0),
	zMaxPrev(0.0)
{
	szMG[0] = '\0';
}

// Destructor.

AmplAlg::~AmplAlg()
{
}

// Silently accept durations < csamp=128 samples;
// no advantage in sending messages faster than actors can process them!
void AmplAlg::setRate(float z)
{
	fContinuous = (z > 0.);
	if (fContinuous)
		{
		wDuration = (int)(globs.SampleRate / z);
		if (wDuration < csamp)
			wDuration = csamp;
		}
	else
		{
		// give it some value, for safety
		wDuration = 0;
		}
}

void AmplAlg::setUserFloat(float z)
{
	zUserFloat = z;
}

void AmplAlg::setMG(const char* sz)
{
	strcpy(szMG, sz);
}

void AmplAlg::SendAnalysis(void)
{
	// It's questionable to call actorMessageHandler() from
	// within a VAlgorithm::generateSamples, as it could
	// confuse priorities of message handling with sample
	// computation.  But let's see what happens.

	if (!*szMG)
		return;		// Name of message group is not yet specified!

	// Don't send any data if the data didn't change enough.
	// In this case, enough is .1 dB, which is a scalar of about 1.033.
	if (zMaxPrev != 0.)
		{
		float ratio = zMax / zMaxPrev;
		if (ratio < 1.033 && ratio > 1./1.033)
			return;
		}
	else if (zMax == 0.)
		return;

	char szCmd[200];
	sprintf(szCmd, "SendData %s [ %f %f ]", szMG, zMax, zUserFloat);
	zMaxPrev = zMax;
	actorMessageHandler(szCmd);
}

void AmplAlg::generateSamples(int howMany)
{
	// Assumes mono input.
	// Similar to PitchAlg::generateSamples().

	if (!source || (!fOneShot && !fContinuous))
		goto LDone;

	isamp = 0;
	for (int j = 0; j < howMany; ++j)
		{
	//	// This test isn't needed, if csamp == howMany.
	//	if (isamp >= csamp)
	//		{
	//		PerformAnalysis();
	//		fOneShot = 0;
	//		break;
	//		}
		rgsamp[isamp++] = (*source)[j][0];
		}
	PerformAnalysis();

	if (fOneShot)
		{
		sprintf(szCliRetval, "%.4f", zMax);
		ReturnStringToClient(szCliRetval);
		fOneShot = 0;
		}
	if (fContinuous)
		{
		iDuration += howMany;
		if (iDuration >= wDuration)
			{
			SendAnalysis();
			iDuration -= wDuration;
			}
		}

LDone:
	// Doesn't emit any sound.
	for (int j = 0; j < howMany; ++j)
		ClearSample(j);
}

void AmplAlg::AnalyzeOneShot(void)
{
//	// This test is needed only if csamp > howMany in generateSamples().
//	if (fOneShot)
//		{
//		fprintf(stderr, "vss error: AmplAlg already doing a one-shot measurement.\n");
//		return;
//		}

	fOneShot = 1;
}

// Perform an analysis on rgsamp[0 .. csamp-1].
void AmplAlg::PerformAnalysis(void)
{
	zMax = 0.;
	for (int i=0; i<csamp; i++)
		{
		float zT = fabs(rgsamp[i]);
		if (zT > zMax)
			zMax = zT;
		}
}
