#include "fmm.h"

//	for wavetable computation
#include <cmath>
#define SINTABSZ 512
float	FMMsintab[SINTABSZ+1];
int	flagFMMsintab = 0;

fmmAlg::fmmAlg() :
	VAlgorithm(),
	ccRatio(1.), 
	ccModIndex(1.),
	ccModIndOPhs(0.0)
{
	if (flagFMMsintab == 0) InitFMMsintab();

	dcBlock1.setFrequency(25.0);
	dcBlock1.setHiAllLopassGain(1., 0., 0.);
	dcBlock2.setFrequency(25.0);
	dcBlock2.setHiAllLopassGain(1., 0., 0.);

	lopass.setHiAllLopassGain(.8, 0., 0.); // .8 not 1: extra hi rolloff
	hipass.setHiAllLopassGain(0., 0., 1.);
}

// initialize static wavetable
void fmmAlg::InitFMMsintab() {
	for (int i = 0; i <= SINTABSZ; i++)
		FMMsintab[i] = sinf(i * 2.0f * (float)(M_PI / SINTABSZ));
	flagFMMsintab = 1;
}

//	fmmAlg wrap phase accumulator, and separate into wrapped integer and fractional parts
//
//	float	Phase	Phase accumulator is wrapped, in=place, to int table size STABSZ
//
//	int	iPhase	Integer part of Phase wrapped to STABSZ
//	float	fPhase	Fractional part of Phase
//
//	Integer and fractional parts are separated out for direct use by Lerp() table 
//	lookups. Wrapped total phase is reconstructed and put back into Phase.
void fmmAlg::WrapAccSep(float& Phase, int& iPhase, float& fPhase) {
	iPhase = (int)Phase;		// extract integer floor of Phase
	Phase -= (float)iPhase;		// strip off int part, leave fractional part

	fPhase = Phase;			// store fractional part
	iPhase &= (SINTABSZ-1);		// wrap and store integer part
	Phase += (float)iPhase;		// reconstruct and store the whole wrapped phase
}

//	fmmAlg wrap phase accumulator
//	Argument float Phase is wrapped, in-place, to int table size STABSZ, 
//	preserving fractional part of argument
void fmmAlg::WrapAcc(float& Phase) {
	int iPhase = (int)Phase;	// extract integer floor of Phase
	Phase -= (float)iPhase;		// strip off int part, leave fractional part
	iPhase &= (SINTABSZ-1);		// wrap integer part
	Phase += (float)iPhase;		// reconstruct the whole wrapped phase
} 

//===========================================================================
//	fmmAlg separate totalized phase into wrapped integer and fractional parts
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
void fmmAlg::WrapTot(float Phase, int& iPhase, float& fPhase) {
	// extract unidirectional integer floor of Phase
	iPhase = (Phase < 0.0f) ? ((int)Phase - 1) : (int)Phase;

	fPhase = Phase - (float)iPhase;		// strip off int part, get fractional part
	iPhase &= (SINTABSZ-1);			// wrap integer part
} 

void fmmAlg::generateSamples(int howMany) {
	float totPhase;		// total phase summer (TPS), in units of samples
	float fTotPhase;	// fractional part of TPS
	int iTotPhase;		// integer part of TPS
	float t;			// temporary register

	for (int j = 0; j < howMany; j++)
		{
		// update, wrap carrier and modulator phase accumulators
		op1.carPhase += op1.carDPhase;
		WrapAcc(op1.carPhase);

		op1.modPhase += op1.modDPhase;
		WrapAcc(op1.modPhase);

		// add car feedback + mod output to carrier phase, wrap it, deliver it
		// Commented out feedback term for this operator, as it has little sonic effect.
		totPhase = op1.carPhase /*+ op1.carFbOPhs*op1.lastCarVal*/ + op1.modIndOPhs*op1.lastModVal;
		WrapTot(totPhase, iTotPhase, fTotPhase);
		op1.lastCarVal = Lerp(iTotPhase, fTotPhase, FMMsintab);

		// filter out any DC
		dcBlock1.setInput(op1.lastCarVal);
		dcBlock1.computeSamp();
		op1.lastCarVal = dcBlock1.getOutput();

		// Now send op1's output to modulate the modulation frequency of op2.

		op2.carPhase += op2.carDPhase;
		WrapAcc(op2.carPhase);

		// add mod feedback to modulator phase, wrap it and deliver it
		// Commented out feedback term for this operator, as it has little sonic effect.
		totPhase = op1.modPhase /*+ op1.modFbOPhs*op1.lastModVal*/;
		WrapTot(totPhase, iTotPhase, fTotPhase);
		op1.lastModVal = Lerp(iTotPhase, fTotPhase, FMMsintab);

		op2.modPhase += op2.modDPhase;					// Keeps on phasing as usual, of course.
		op2.modPhase += ccModIndOPhs * op1.lastCarVal;	// But also accumulate op1 into it.
		WrapAcc(op2.modPhase);

		totPhase = op2.modPhase + op2.modFbOPhs*op2.lastModVal;
		WrapTot(totPhase, iTotPhase, fTotPhase);
		op2.lastModVal = Lerp(iTotPhase, fTotPhase, FMMsintab);

		totPhase = op2.carPhase + op2.carFbOPhs*op2.lastCarVal + op2.modIndOPhs*op2.lastModVal;
		WrapTot(totPhase, iTotPhase, fTotPhase);
		t = Lerp(iTotPhase, fTotPhase, FMMsintab);

		// low-pass filter counteracts buzziness of feedback.
		lopass.setInput(t);
		lopass.computeSamp();
		t = lopass.getOutput();

		// high-pass filter if you don't want the fundamental.
		hipass.setInput(t);
		hipass.computeSamp();
		t = hipass.getOutput();

		// filter out any more DC
		dcBlock2.setInput(t);
		dcBlock2.computeSamp();
		op2.lastCarVal = dcBlock2.getOutput();

		Output(op2.lastCarVal, j);
		}
}

// Scale natural frequency in Hz to units of "samples."
static float freqToDPhase(float fHz) { return fHz * globs.OneOverSR * SINTABSZ; }
// Scale modulation phase offset to units of "samples."
static float modIToOPhase(float Xind) { return Xind * SINTABSZ / (2.0*M_PI); }

void fmmAlg::set2CCratio(float newCC)
{
	ccRatio = newCC;
	// ratio-frequency mode: modFreq tracks changes in carFreq
	op2.setCarrierFreq(op1.getCarrierFreq() * ccRatio);
}

void fmmAlg::set1CarrierFreq(float z)
{
	op1.setCarrierFreq(z);

	// ratio-frequency mode: modFreq tracks changes in carFreq
	op2.setCarrierFreq(z * ccRatio);

	// low-pass and high-pass filters track carrier frequency
	lopass.setFrequency(z * 8.f);
	hipass.setFrequency(z * 24.f);
}

void fmmAlg::set2CCModIndex(float newI)
{
	ccModIndex = newI;
	ccModIndOPhs = modIToOPhase(ccModIndex);
}

void fmmAlg::setLowpassGain(float z)
{
	lopass.setLowpassGain(z);
}

void fmmAlg::setHighpassGain(float z)
{
	hipass.setHighpassGain(z);
}

//	fmOperator member functions

void fmOperator::setCarrierFreq(float car)
{
	carFreq = car;
	carDPhase = freqToDPhase(carFreq);

//	ratio-frequency mode: modFreq tracks changes in carFreq
	if (ratioMode == 1) {
		modFreq = carFreq / cmRatio;
		modDPhase = freqToDPhase(modFreq);
	}
}

void fmOperator::setModulatorFreq(float mod)
{
	modFreq = mod;
	modDPhase = freqToDPhase(modFreq);
}

void fmOperator::setCMratio(float newCM)
{
	cmRatio = newCM;

	modFreq = carFreq / cmRatio;
	modDPhase = freqToDPhase(modFreq);
}

void fmOperator::setRatioMode(int newMode)
{
	ratioMode = newMode;
}

void fmOperator::setModIndex(float newI)
{
	modIndex = newI;
	modIndOPhs = modIToOPhase(modIndex);
}

void fmOperator::setCarFeedback(float newCFB)
{
	carFeedback = newCFB;
	carFbOPhs = modIToOPhase(carFeedback);
}

void fmOperator::setModFeedback(float newMFB)
{
	modFeedback = newMFB;
	modFbOPhs = modIToOPhase(modFeedback);
}
