//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1996.
//===========================================================================

#include "chua.h"

//===========================================================================
//	chuaAlg constructor
//
chuaAlg::chuaAlg(void) :
	VAlgorithm()
{
	resetChuaNote();
}

//===========================================================================
//	chuaAlg destructor
//
chuaAlg::~chuaAlg()
{
}

//===========================================================================
//	chuaAlg generateSamples
//
//
void
chuaAlg::generateSamples(int howMany)
{
	int i;
/*
	Put some of these things in registers.
*/
	REGbp1 = BP1;
	REGbp2 = BP2;
	REGbph1 = BPH1;
	REGbph2 = BPH2;
	REGbpdiff =  REGbp2 - REGbp1;
/*
	Make dimensionless params from dimensions.
*/
	AA = C2 / C1;
	BB = C2 * R * R / L;
	MA = M1 * R;
	MB = M0 * R;
	rho = R0 / R;
	p5m = .5 * (MA - MB);
	g0BPH1 = MB * REGbph1 + p5m *
	        (fabs(REGbph1 + REGbp1) - fabs(REGbph1 - REGbp2) - REGbp1 + REGbp2);
	g0BPH2 = MB * REGbph2 + p5m *
	        (fabs(REGbph2 +REGbp1) - fabs(REGbph2 - REGbp2) - REGbp1 + REGbp2);
/*
	Okay, make the samples.
*/
	for(i = 0; i < howMany; i++) {
		rk4();
		Output(vector_pos[0], i);
	}
}


/////////////////////////////////////////
//      Class initialization stuff     //
/////////////////////////////////////////

void
chuaAlg::resetChuaNote(void)
{
//	fprintf(stderr, "Resetting CHUA circuit.\n" );
	resetControlForce();
	resetVector();
//	resetState();

	integStep = 500;
	tstep = 660. * globs.OneOverSR;
	spectStep = globs.SampleRate;
}


void
chuaAlg::resetControlForce(void)
{
	potentiometer = 0;
	Q = .05;
	tau = 15;
	cursamp = 0;
}


void
chuaAlg::resetVector(void)
{
        vector_pos[0] = 0.049746192893;
        vector_pos[1] = 0.04776632302;
        vector_pos[2] = 0.;
}


void
chuaAlg::resetState(void)
{
        R               = 1523.;
        R0              = 11.2;
        C1              = 4.7e-2 * 1.0e-7;
        C2              = 1.0 * 1.0e-7;
        L               = 5.74 * 1.0e-3;
        BPH1    	= -6;
        BPH2    	= 6;
        BP1             = 1.;
        BP2             = .5966;
        M0              = -.5 * 1.0e-3;
        M1              = -.8 * 1.0e-3;
        M2              = 4. * 1.0e-3;
        M3              = 4. * 1.0e-3;
}
