#include "gran.h"

granHand::granHand(granAlg * a) :
	VHandler(a),
	controlMin(0),
	controlMax(1),
	jumpMin(0),
	jumpMax(1),
	sampleStep(1.)
{
	setTypeName("granHand");
	directoryName[0] = '\0';
}

// Delete yourself if necessary.
void granHand::act(void)
{
	VHandler::act();	
	if (getAlg()->getPosition() >= getAlg()->getIntervalEnd())
		{
		// the sample has played past the end
		delete this;
		}
}

int granHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);
	
	if (CommandIs("SetFile"))
	{
		ifS( fname, setFile(fname) );
		return Uncatch();
	}
	
	if (CommandIs("SetDirectory"))
	{
		ifS( dname, setDirectory(dname) );
		return Uncatch();
	}

	if (CommandIs("SetDur"))
	{
		ifF( time, setDur( time ) );
		return Uncatch();
	}

	if (CommandIs("SetStart"))
	{
		ifF( _, setStart( _ ) );
		return Uncatch();
	}

	if (CommandIs("SetSlope"))
	{
		ifF( _, setSlope( _ ) );
		return Uncatch();
	}

	if (CommandIs("Rebound"))
	{
		ifF( _, setRebound( _ ) );
		return Uncatch();
	}

	if (CommandIs("Spread"))
	{
		ifF( _, setSpread( _ ) );
		return Uncatch();
	}

	if (CommandIs("SetPlaybackRate"))
	{
		ifF( _, setSampleStep( _ ) );
		return Uncatch();
	}

	return VHandler::receiveMessage(Message);
}

void granHand::setDirectory(char * dir)
{
	if (dir[0] != 0)
		strcpy(directoryName, dir);
	else
		strcpy(directoryName, ".");
}

//	also set the sample step, because the algorithm resets it
//	to the sample rate ratio when it reads a new file.
void
granHand::setFile(char * fileName)
{
	granActor * myParent = (granActor *)getParent();
	if (myParent == NULL)
	{
		fprintf(stderr, "vss error: granHand::setFile found itself with NULL parent!!\n");
		VActor::curtainCall(cout);;;;
		return;
	}

	sfile * newFile = myParent->loadFile(directoryName, fileName);
	if (newFile == NULL)
		return;
		
	getAlg()->setFile(newFile);
	getAlg()->setSampleStep( sampleStep );
	setRebound(myParent->Rebound());
	setSpread(myParent->Spread());

	// I'd do this in the constructor, but we need a file first.
	setStart(myParent->Start());
	const float sec = myParent->Dur();
	setDur(sec > 0.0 ? sec : 0.01);

	setSlope(myParent->Slope());
}

void granHand::setSampleStep(float z)
{
	if (!CheckSampleStep(z))
		fprintf(stderr, "granHand got bogus playback rate %f.\n", z);
	else
		sampleStep = z;
}

void granHand::setRanges(void)
{
	granActor * myParent = (granActor *)getParent();
	if (myParent == NULL)
	{
		fprintf(stderr, "vss error: granHand::setRanges found itself with NULL parent!!\n");
		VActor::curtainCall(cout);;;;
		return;
	}

	controlMin = myParent->ControlMin();
	controlMax = myParent->ControlMax();
	if (controlMin >= controlMax)
		{
		fprintf(stderr, "vss error: granHand controlMin >= controlMax (%g, %g)\n",
			controlMin, controlMax);
		controlMin = 0.;
		controlMax = 1.;
		}

	jumpMin = myParent->JumpMin();
	jumpMax = myParent->JumpMax();
	if (jumpMin > jumpMax)
		{
		fprintf(stderr, "vss error: granHand jumpMin > jumpMax (%g, %g)\n",
			jumpMin, jumpMax);
		jumpMax = jumpMin;
		}

	getAlg()->setRanges(controlMin,controlMax,jumpMin,jumpMax);
}

void granHand::setStart( float time )
{
	setRanges();
	getAlg()->setStart( time );
}

void granHand::setDur( float dur )
{
	if (mySlope > 20)
		{
#ifdef TOO_NOISY_FOR_PRODUCTION_RUNS
		fprintf(stderr, "vss emergency Ground Truth procedure: reducing slope from %g to 0\n", mySlope);
#endif
		mySlope = 0;
		VHandler::RampUpAmps(0);
		getAlg()->setSlope(0);
		}

	if (mySlope > dur/2)
		{
		// This grain wouldn't get up to max amplitude, because of the envelope.
#ifdef TOO_NOISY_FOR_PRODUCTION_RUNS
		fprintf(stderr,
			"vss warning: granHand::setDur duration %g < twice slope length %g.\n         Grain will be extended to %g.\n",
			dur, mySlope, mySlope*2);
#endif
		dur = mySlope*2;
		}
	if (dur <= 0.0)
	{
		fprintf(stderr, "vss: granHand::setDur %g must be positive.\n", dur);
		return;
	}
	myDur = dur;
	getAlg()->setDur( myDur );
}

void granHand::setSlope( float slope )
{
	if (slope > myDur/2)
		{
		// Clamp slope to be less than duration.
#ifdef TOO_NOISY_FOR_PRODUCTION_RUNS
		fprintf(stderr, "vss warning: granHand::setSlope clamped from %g to half-duration %g\n", slope, myDur/2);
#endif
		slope = myDur/2;
		}
	mySlope = slope;
	VHandler::RampUpAmps(slope); // this does the ramp-up.
	getAlg()->setSlope( slope ); // this will do the ramp-down.
}

void granHand::setRebound( float _ )
{
	getAlg()->setRebound(_);
}

void granHand::setSpread( float _ )
{
	getAlg()->setSpread(_);
}
