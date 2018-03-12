//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1996.
//===========================================================================

#include "fm.h"

//===========================================================================
//	for wavetable computation
//
#include <cmath>
#define SINTABSZ 256
float	FMsintab[SINTABSZ+1];
int	flagFMsintab = 0;

//===========================================================================
//	for run-time optimization of generateSamples
//
#define MODI_MIN 0.005

//===========================================================================
//	fmAlg constructor
//
//	Besides other inits, also initialize the lookup wavetable, if not already
//
fmAlg::fmAlg(void) :
	VAlgorithm(),
	carFreq(100.),
	modFreq(100.),
	cmRatio(1.),
	ratioMode(1),
	modIndex(1.),
	carFeedback(0.),
	modFeedback(0.),
	carPhase(0.),
	carDPhase(0.),
	modPhase(0.),
	modDPhase(0.),
	modIndOPhs(0.),
	carFbOPhs(0.),
	modFbOPhs(0.),
	lastCarVal(0.),
	lastModVal(0.)
{
	if (flagFMsintab == 0) InitFMsintab();
}

//===========================================================================
//	fmAlg destructor
//
fmAlg::~fmAlg()
{
}

//===========================================================================
//	fmAlg initialize static wavetable
//
void
fmAlg::InitFMsintab(void)
{
	for (int i = 0; i <= SINTABSZ; i++)
		FMsintab[i] = sinf(i * 2.0f * (float)(M_PI / SINTABSZ));
	flagFMsintab = 1;
}

//===========================================================================
//	fmAlg wrap phase accumulator, and separate into wrapped integer and fractional parts
//
//	float	Phase	Phase accumulator is wrapped, in=place, to int table size STABSZ
//
//	int	iPhase	Integer part of Phase wrapped to STABSZ
//	float	fPhase	Fractional part of Phase
//
//	Integer and fractional parts are separated out for direct use by Lerp() table 
//	lookups. Wrapped total phase is reconstructed and put back into Phase.
//
inline void 
fmAlg::WrapAccSep(float &Phase, int &iPhase, float &fPhase)
{
	iPhase = (int)Phase;		// extract integer floor of Phase
	Phase -= (float)iPhase;		// strip off int part, leave fractional part

	fPhase = Phase;			// store fractional part
	iPhase &= (SINTABSZ-1);		// wrap and store integer part
	Phase += (float)iPhase;		// reconstruct and store the whole wrapped phase
}

//===========================================================================
//	fmAlg wrap phase accumulator
//
//	Argument float Phase is wrapped, in-place, to int table size STABSZ, 
//	preserving fractional part of argument
//
inline void 
fmAlg::WrapAcc(float &Phase)
{
	int iPhase = (int)Phase;	// extract integer floor of Phase

	Phase -= (float)iPhase;		// strip off int part, leave fractional part
	iPhase &= (SINTABSZ-1);		// wrap integer part
	Phase += (float)iPhase;		// reconstruct the whole wrapped phase
} 

//===========================================================================
//	fmAlg separate totalized phase into wrapped integer and fractional parts
//
//	Input:	float	Phase
//
//	Output:	int	iPhase, the integer part of Phase wrapped to STABSZ
//	        float	fPhase, the fractional part of Phase
//
//	Integer and fractional parts are separated out for direct use by Lerp() table 
//	lookups. Case (Phase < 0.0) is tested and separation adjusted to always yield 
//	(fPhase > 0.0).
//
inline void 
fmAlg::WrapTot(float Phase, int &iPhase, float &fPhase)
{
	// extract unidirectional integer floor of Phase
	iPhase = (Phase < 0.0f) ? ((int)Phase - 1) : (int)Phase;

	fPhase = Phase - (float)iPhase;		// strip off int part, get fractional part
	iPhase &= (SINTABSZ-1);			// wrap integer part
} 

//===========================================================================
//	fmAlg generateSamples
//
void
fmAlg::generateSamples(int howMany)
{
	float totPhase;	// total phase summer (TPS), in units of samples
	int	iTotPhase;	// integer part of TPS
	float fTotPhase;// fractional part of TPS

	if ((fabsf(carFeedback) < MODI_MIN) && (fabsf(modFeedback) < MODI_MIN))
	{
		for (int j = 0; j < howMany; j++)
		{
			// update, wrap modulator phase accumulator
			modPhase += modDPhase;
			WrapAccSep(modPhase, iTotPhase, fTotPhase);

			// deliver wrapped modulator phase
			lastModVal = Lerp(iTotPhase, fTotPhase, FMsintab);

			// update, wrap carrier phase accumulator
			carPhase += carDPhase;
			WrapAcc(carPhase);

			// add mod output to carrier phase, wrap it, deliver it
			totPhase = carPhase + modIndOPhs*lastModVal;
			WrapTot(totPhase, iTotPhase, fTotPhase);
			// lastCarVal must be maintained for popless carFeedback turn-on
			lastCarVal = Lerp(iTotPhase, fTotPhase, FMsintab);

			Output(lastCarVal, j);
		}
	}
	else
	{
		for (int j = 0; j < howMany; j++)
		{
			// update, wrap carrier and modulator phase accumulators
			carPhase += carDPhase;
			WrapAcc(carPhase);

			modPhase += modDPhase;
			WrapAcc(modPhase);
		
			// add mod feedback to modulator phase, wrap it and deliver it
			totPhase = modPhase + modFbOPhs*lastModVal;
			WrapTot(totPhase, iTotPhase, fTotPhase);
			lastModVal = Lerp(iTotPhase, fTotPhase, FMsintab);

			// add car feedback + mod output to carrier phase, wrap it, deliver it
			totPhase = carPhase + carFbOPhs*lastCarVal + modIndOPhs*lastModVal;
			WrapTot(totPhase, iTotPhase, fTotPhase);
			lastCarVal = Lerp(iTotPhase, fTotPhase, FMsintab);

			Output(lastCarVal, j);
		}
	}
}

//===========================================================================
//	Utilities for scaling frequency and phase offsets
//
//	scale natural frequency in Hz to units of "samples"
static inline 	float 	freqToDPhase(float fHz) { return fHz * globs.OneOverSR * SINTABSZ ; } 

//	scale modulation phase offset to units of "samples"
static inline 	float 	modIToOPhase(float Xind) { return Xind * SINTABSZ / (2.0f * M_PI) ; } 

//===========================================================================
//	fmAlg setCarrierFreq
//
void
fmAlg::setCarrierFreq(float car)
{
	carFreq = car;
	carDPhase = freqToDPhase(carFreq);

//	ratio-frequency mode: modFreq tracks changes in carFreq
	if (ratioMode == 1) {
		modFreq = carFreq / cmRatio;
		modDPhase = freqToDPhase(modFreq);
	}
}

//===========================================================================
//	fmAlg setModulatorFreq
//
void
fmAlg::setModulatorFreq(float mod)
{
	modFreq = mod;
	modDPhase = freqToDPhase(modFreq);
}

//===========================================================================
//	fmAlg setCMratio
//
void
fmAlg::setCMratio(float newCM)
{
	cmRatio = newCM;

	modFreq = carFreq / cmRatio;
	modDPhase = freqToDPhase(modFreq);
}

//===========================================================================
//	fmAlg setRatioMode
//
void
fmAlg::setRatioMode(int newMode)
{
	ratioMode = newMode;
}

//===========================================================================
//	fmAlg setModIndex
//
void
fmAlg::setModIndex(float newI)
{
	modIndex = newI;
	modIndOPhs = modIToOPhase(modIndex);
//	fprintf(stderr, "fmAlg got new modIndex %f\n", modIndex);
}

//===========================================================================
//	fmAlg setCarFeedback
//
void
fmAlg::setCarFeedback(float newCFB)
{
	carFeedback = newCFB;
	carFbOPhs = modIToOPhase(carFeedback);
//	fprintf(stderr, "fmAlg got new carFeedback %f\n", carFeedback);
}

//===========================================================================
//	fmAlg setModFeedback
//
void
fmAlg::setModFeedback(float newMFB)
{
	modFeedback = newMFB;
	modFbOPhs = modIToOPhase(modFeedback);
//	fprintf(stderr, "fmAlg got new modFeedback %f\n", modFeedback);
}
