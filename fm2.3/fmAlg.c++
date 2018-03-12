//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1996.
//===========================================================================

#include "fm.h"
#include "platform.h"

//===========================================================================
// this code moved from dev/platform/*.c++
//
#define TPOW 14
#define STABSZ (1l<<TPOW)
#define LOGBASE2OFTABLEELEMENT 2

static float *Sinetab = 0;
static void SineFunction(int n, float *stab, int stride, float from, float to)
{
    int j;
    float f = (to-from)/n;
    for(j=0;j<n;++j)
        stab[j*stride] = sinf(from +j*f);
}
static int Makeoscsinetable(void)
{
    if(!Sinetab)
    {
        Sinetab = (float*)malloc(sizeof(float)*STABSZ);
        if(!Sinetab)
            return FALSE;
        SineFunction(STABSZ, Sinetab, 1, 0.0f, 2.0 * 3.14159265358979323);
    }
    return TRUE;
}


//===========================================================================
//	fmAlg constructor
//
fmAlg::fmAlg(void) :
	carFreq(100.),
	carPhase(0),
	carDPhase(0),
	modPhase(0),
	modDPhase(0),
	pfd(0.),
	cmRatio(1),
	VAlgorithm()
{
	Makeoscsinetable();
}

//===========================================================================
//	fmAlg destructor
//
fmAlg::~fmAlg()
{
}

//===========================================================================
//	fmAlg generateSamples
//
//	please excuse all this ugliness
//	it is all copied from vss2.3
//	yik§
//
#define TPOW 14
#define LOGBASE2OFTABLEELEMENT 2
#define wShift ((32 - TPOW) - LOGBASE2OFTABLEELEMENT)
#define STABSZ (1l<<TPOW)
#define FastSin(x) (Sinetab[(((ulong) x) >> wShift) & (STABSZ - 1)])
void
fmAlg::generateSamples(int howMany)
{
	double  modAmp = globs.OneOverSR * pfd * 
					(float)(1 << (wShift + TPOW));

	for (int j = 0; j < howMany; j++)
	{
		ulong phaseTot;
		carPhase += carDPhase;      /* automatic wraparound */
		modPhase += modDPhase;      /* automatic wraparound */
		phaseTot = carPhase + (ulong)(modAmp * FastSin(modPhase));
		Output(FastSin(phaseTot), j);
	}
}

//===========================================================================
//	DphaseFromFreq
//
#define DphaseMultiplierConstantGlobalThing \
			((float) STABSZ * (float) (1 << wShift))
inline 
ulong DphaseFromFreq(float zHz)
{
	return (ulong)(zHz * globs.OneOverSR * DphaseMultiplierConstantGlobalThing);
} 

//===========================================================================
//	fmAlg setCarrierFreq
//
void
fmAlg::setCarrierFreq(float car)
{
	carFreq = car;
	carDPhase = DphaseFromFreq(carFreq);
	modDPhase = DphaseFromFreq(modFreq());
}

//===========================================================================
//	fmAlg setModIndex
//
void
fmAlg::setModIndex(float newI)
{
	pfd = modFreq() * newI;
}

//===========================================================================
//	fmAlg setCMratio
//
void
fmAlg::setCMratio(float newCM)
{
	//	keep modIndex constant with changing cmRatio
	pfd *= cmRatio / newCM;
	cmRatio = newCM;
	modDPhase = DphaseFromFreq(modFreq());
}
