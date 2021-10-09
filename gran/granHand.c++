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
	strcpy(directoryName, *dir ? dir : ".");
}

//	also set the sample step, because the algorithm resets it
//	to the sample rate ratio when it reads a new file.
void
granHand::setFile(char * fileName)
{
	auto myParent = (granActor*)getParent();
	if (!myParent)
	{
		fprintf(stderr, "vss error: granHand::setFile has no parent.\n");
		return;
	}
	const auto newFile = myParent->loadFile(directoryName, fileName);
	if (!newFile)
		return;
	getAlg()->setFile(newFile);
	getAlg()->setSampleStep( sampleStep );
	setRebound(myParent->Rebound());
	setSpread(myParent->Spread());

	// I'd do this in the constructor, but we need a file first.
	setStart(myParent->Start());
	const auto sec = myParent->Dur();
	setDur(sec > 0.0 ? sec : 0.01);

	setSlope(myParent->Slope());
}

void granHand::setSampleStep(float z)
{
	if (!CheckSampleStep(z)) {
		fprintf(stderr, "vss: granHand ignoring out-of-range playback rate %f.\n", z);
		return;
	}
	sampleStep = z;
}

void granHand::setRanges()
{
	auto myParent = (granActor*)getParent();
	if (!myParent)
	{
		fprintf(stderr, "vss error: granHand::setRanges has no parent.\n");
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
	if (dur <= 0.0)
	{
		fprintf(stderr, "vss: granHand ignoring nonpositive duration %g.\n", dur);
		return;
	}
	if (mySlope > 20.0)
		{
#ifdef TOO_NOISY_FOR_PRODUCTION_RUNS
		fprintf(stderr, "vss: granHand zeroing overlong slope %g\n", mySlope);
#endif
		mySlope = 0.0;
		VHandler::RampUpAmps(0.0);
		getAlg()->setSlope(0.0);
		}

	if (mySlope > dur/2.0)
		{
		// This grain wouldn't get up to max amplitude, because of the envelope.
#ifdef TOO_NOISY_FOR_PRODUCTION_RUNS
		fprintf(stderr,
			"vss: granHand::setDur duration %g < twice slope length %g.\n         Grain will be extended to %g.\n",
			dur, mySlope, mySlope*2.0);
#endif
		// dur is >0, so mySlope must be too.  So dur will still be >0 after this change.
		dur = mySlope*2.0;
		}
	myDur = dur;
	getAlg()->setDur( myDur );
}

void granHand::setSlope( float slope )
{
	if (slope > myDur/2.0)
		{
#ifdef TOO_NOISY_FOR_PRODUCTION_RUNS
		fprintf(stderr, "vss: granHand::setSlope reduced from %g to half-duration %g\n", slope, myDur/2.0);
#endif
		slope = myDur/2.0;
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
