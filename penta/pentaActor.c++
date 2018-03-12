//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1996.
//===========================================================================

#include "pentaActor.h"

#ifdef VSS_WINDOWS
#define rint(_) (_)
#endif

ACTOR_SETUP(PentaActor, PentaActor)

//===========================================================================
//		construction
//
PentaActor::PentaActor() :
	fRecomputeDuration(0),
	zDuration(0.),
	zAmplScale(1.),
	zIrreg(0.),
	zAlpha(1.),
	wWidth(4),
	zValue(0.),
	zFreqLowest(220.),
	fSkipFirstTime(0),
	tPrev(1.),           // allow one second for setup time
	iPitchPrev(-1)
{
	setTypeName("PentaActor");
	szMG[0] = '\0';
	for (iPS = 0; iPS < 11; iPS++)
		{
		// mysterious secret formula of camille's own invention
		rgPS[iPS][0] = (0 + 7*iPS) % 12;
		rgPS[iPS][1] = (2 + 7*iPS) % 12;
		rgPS[iPS][2] = (5 + 7*iPS) % 12;
		rgPS[iPS][3] = (7 + 7*iPS) % 12;
		rgPS[iPS][4] = (9 + 7*iPS) % 12;
		}
	iPS = 0;
}

// Compute the next duration with the Xenakis-Poisson-Myhill-Goudeseune
// algorithm, no doubt published in some stats text I haven't dug up yet.
void PentaActor::RecomputeDuration()
{
	if (zIrreg <= 0)
		zDuration = zAlpha;
	else
		{
		const double r = exp(zIrreg);
		const double xMin = pow(r, r/(1.-r));
		const double xMax = xMin * r;
		zDuration = -zAlpha * log(  drand48() * (xMax - xMin) + xMin  );
		}
	fRecomputeDuration = 0;
}

//===========================================================================
//		act
//
// Send and delete all messages whose time has come.
//
void 
PentaActor::act()
{
	VActor::act();
	
	if (!*szMG)
		return; // not yet initialized with SetMessageGroup

	float tNow = currentTime(); // in seconds since vss started

	float timeSinceLastChime = tNow - tPrev;
	if (fRecomputeDuration)
		RecomputeDuration();
	if (timeSinceLastChime < zDuration)
		return;
	tPrev = tNow;

	// A hack.  Can you do this better?
	if (!fSkipFirstTime)
		{
		fSkipFirstTime = 1;
		return;
		}

	RecomputeDuration();

	// Choose a pitch in a 4-octave range, A to A,
	// avoiding extremes by an approximate Gaussian distribution,
	// giving pitches 0..47, 110Hz to 1760Hz, centered on E flat ~= 622Hz.

	int iPitch = 0;
	int i;
	for (i=0; i<wWidth; i++)
		iPitch += rand() % 48;
	iPitch = int(rint((float)iPitch / wWidth));

	// Then round it to the nearest of the five notes currently in use.

	int iPitchNearestAbove = -10000;
	for (i = iPitch; i < 48; i++)
		{
		if (i % 12 == rgPS[iPS][0] ||
			i % 12 == rgPS[iPS][1] ||
			i % 12 == rgPS[iPS][2] ||
			i % 12 == rgPS[iPS][3] ||
			i % 12 == rgPS[iPS][4])
			{
			iPitchNearestAbove = i;
			break;
			}
		}

	int iPitchNearestBelow = -10000;
	for (i = iPitch; i >= 0; i--)
		{
		if (i % 12 == rgPS[iPS][0] ||
			i % 12 == rgPS[iPS][1] ||
			i % 12 == rgPS[iPS][2] ||
			i % 12 == rgPS[iPS][3] ||
			i % 12 == rgPS[iPS][4])
			{
			iPitchNearestBelow = i;
			break;
			}
		}

	iPitch = (abs(iPitch - iPitchNearestAbove) <
			  abs(iPitch - iPitchNearestBelow)) ?
			  iPitchNearestAbove : iPitchNearestBelow;
	if (iPitch == iPitchPrev)
		{
		// Don't do two in a row, because it sounds nervous.
		// Choose the other guy, then.
		iPitch = (abs(iPitch - iPitchNearestAbove) <
				  abs(iPitch - iPitchNearestBelow)) ?
				  iPitchNearestBelow : iPitchNearestAbove;
		}
	if (iPitch == iPitchPrev)
		{
		// iPitchNearestBelow == iPitchNearestAbove.
		// Just skip this note, to avoid the repeat.
		return;
		}

	if (iPitch < 0)
		// Just skip this note (this case is very rare).
		return;

	iPitchPrev = iPitch;

	// Compute its amplitude, fading out the extremes like Shepard tones.

	const double db = -50. * fabs(iPitch/48. - .5) // -25 to 0 to -25.
	  + 6. * drand48() - 3;
	const double zAmpl = zAmplScale * pow(1.065, db); // 1.065 is approximate.

//for (i=0;i<iPitch*3/2;i++) printf(" "); printf("%.2f\n", zAmpl/zAmplScale);

	// Emit a note.

	char szCmd[256];
	sprintf(szCmd, "SendData %s [ %f %f %f %f ]",
		szMG, zFreqLowest * pow(2., iPitch/12.), zAmpl, zValue, drand48()*1.8-.9);
	// Fourth value is an optional "pan" argument.
	actorMessageHandler(szCmd);
}


void PentaActor::setMG(char* sz)
{
	strncpy(szMG, sz, sizeof(szMG)-1);
}

void PentaActor::setIrreg(float z)
{
	if (z < 0.)
		z = 0.;
	zIrreg = z;
	fRecomputeDuration = 1;
}

void PentaActor::setDensity(float z)
{
	if (z <= 0.)
		return;
	zAlpha = 1/z;
	fRecomputeDuration = 1;
}

void PentaActor::setHueExact(int i)
{
	iPS = i % 12;
}

void PentaActor::setHue(float z)
{
	iPS = int(rint(fabs(z) * 12)) % 12;
}

void PentaActor::setValue(float z)
{
	zValue = z;
}

void PentaActor::setAmp(float z)
{
	zAmplScale = z;
}

void PentaActor::setWidth(float z)
{
	if (z < .01)
		z = .01;
	else if (z > 1.)
		z = 1.;
	wWidth = int(rint(1./z));
}

void PentaActor::setLowestFreq(float z)
{
	if (z < 20.)
		z = 20.;
	else if (z > 44100./5.)
		z = 44100./5.;
	zFreqLowest = z;
}

//===========================================================================
//		receiveMessage
//
int 
PentaActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetMessageGroup"))
	{
		ifS( szName, setMG(szName) );
		return Uncatch();
	}

	if (CommandIs("SetIrregularity"))
	{
		ifF( z, setIrreg(z) );
		return Uncatch();
	}

	if (CommandIs("SetHueExact"))
	{
		ifD( i, setHueExact(i) );
		return Uncatch();
	}

	if (CommandIs("SetHue"))
	{
		ifF( z, setHue(z) );
		return Uncatch();
	}

	if (CommandIs("SetSaturation"))
	{
		ifF( z, setDensity(z) );
		return Uncatch();
	}

	if (CommandIs("SetValue"))
	{
		ifF( z, setValue(z) );
		return Uncatch();
	}

	if (CommandIs("SetAmp"))
	{
		ifF( z, setAmp(z) );
		return Uncatch();
	}

	if (CommandIs("SetWidth"))
	{
		ifF( z, setWidth(z) );
		return Uncatch();
	}

	if (CommandIs("SetLowestFreq"))
	{
		ifF( z, setLowestFreq(z) );
		return Uncatch();
	}

	return VActor::receiveMessage(Message);
}
