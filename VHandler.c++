#include "VGenActor.h"
#include "VHandler.h"

VHandler* VHandler::rgDyingHandler [cDyingHandlerLim];
VHandler* VHandler::rgDyingHandler2[cDyingHandlerLim];
int VHandler::cDyingHandler  = 0;
int VHandler::cDyingHandler2 = 0;
VHandler** VHandler::pDyingHandler  = rgDyingHandler;
VHandler** VHandler::pDyingHandler2 = rgDyingHandler2;

#ifdef VSS_COMPILERBUG1
VAlgorithm* valgT = NULL;
#endif

//	Creating a handler without an algorithm instance to handle makes no 
//	sense. The default constructor for derived handler classes may specify
//	a default algorithm to be sent to the VHandler constructor as follows:
//		myHandlerType(myAlgType* a = new myAlgType) : VHandler(a) {}
//
VHandler::VHandler(VAlgorithm * const a) :
	zAmp(1.),
	zInputAmp(1.),
	fLinearEnv(0),
	input(NULL),
	valg(
#ifdef VSS_COMPILERBUG1
	valgT
#else
	a
#endif
	),
	parentHandle(-1.)
{
	setTypeName("VHandler");
	if (!valg)
		{
		printf("vss internal error: new Handler got a NULL Algorithm.\n");
		delete this;
		return;
		}
	if (!VAlgorithm::Verify(valg))
		{
		printf("vss internal error: new Handler got an invalid Algorithm.\n");
		delete this;
		}
}

//	Remove this handler from the parent's brood, and destroy 
//	the algorithm managed by this handler.
VHandler::~VHandler()
{
	pDyingHandler[cDyingHandler++] = this; // So this->FValid() will work correctly.
	//printf("\t\t\t\tcDyingHandler++\n");;
	VGeneratorActor * myParent = getParent();
	if (myParent != NULL) 
		myParent->removeChild(this);
	delete getAlg();
}

// Any entry point of a handler (act(), receiveMessage(), etc.)
// which does CommandIs("SetInput"), setInput(), getAlg()->setSource()
// or other such thing which makes use of a handler foo,
// must be preceded by a call to foo->FValid().
// Otherwise, foo may have been already delete()'d.
// This clever function works even after the destructor's been called,
// but for a limited time only (two calls to doActors()).
//
int VHandler::FValid(void)
{
	int i;
	for (i=0; i<cDyingHandler; i++)
		if (this == pDyingHandler[i])
			return 0;			// I've been deleted!
	for (i=0; i<cDyingHandler2; i++)
		if (this == pDyingHandler2[i])
			return 0;			// I've been deleted!
	// Phew, I still exist.
	return 1;
}

void VHandler::allAct(void)
{
	// Swap buffers for the FValid() test.
	if (pDyingHandler == rgDyingHandler)
		{
		pDyingHandler = rgDyingHandler2;
		pDyingHandler2 = rgDyingHandler;
		}
	else
		{
		pDyingHandler = rgDyingHandler;
		pDyingHandler2 = rgDyingHandler2;
		}

//	if (cDyingHandler!=0 || cDyingHandler2!=0) printf("\t\t\t\tcDyingHandler = %d, prev = %d\n", cDyingHandler, cDyingHandler2);;
	cDyingHandler2 = cDyingHandler;
	cDyingHandler = 0;
}

//	Control-rate behavior.
void VHandler::act(void)
{
	modpool.act(this);
	VActor::act();
}

//	Derived classes should override this member if they receive messages.
//	DON'T FORGET TO CALL THE PARENT'S RECEIVEMESSAGE() FOR MESSAGES YOU 
//	DON'T HANDLE.
int	VHandler::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("BeginNote"))
	{
		fprintf(stderr, "vss warning: you mean BeginSound not BeginNote.\n");
		strcpy(sscanf_cmd, "BeginSound");
		goto LBeginSound;
	}

	if (CommandIs("BeginNotePaused"))
	{
		fprintf(stderr, "vss warning: you mean BeginSoundPaused not BeginNotePaused.\n");
		strcpy(sscanf_cmd, "BeginSoundPaused");
		goto LBeginSound;
	}

	if (CommandIs("BeginSound") || CommandIs("BeginSoundPaused"))
	// re-attack the note, possibly with new initializers
	{
LBeginSound:
		restrike( sscanf_msg );
		setPause(CommandIs("BeginSoundPaused") ? true : false);
		return Catch();
	}

	if (CommandIs("SetPause"))
	{
		ifD(f, setPause(f) );
		return Uncatch();
	}

	if (CommandIs("SetMute"))
	{
		ifD(f, setMute(f) );
		return Uncatch();
	}

	if (CommandIs("SetInputAmp"))
	{
		ifFF(z,z2, setInputAmp(z, z2));
		ifF(z, setInputAmp(z));
		return Uncatch();
	}

	if (CommandIs("SetInputGain"))
	{
		ifFF(z,z2, setInputGain(z, z2));
		ifF( z, setInputGain(z) );
		return Uncatch();
	}

	if (CommandIs("InvertAmp"))
	{
		ifD(f, invertAmp(f) );
		return Uncatch();
	}

	if (CommandIs("SetAmp"))
	{
LSetAmp:
		ifFF(z,z2, setAmp(z, z2));
		ifF(z, setAmp(z));
		return Uncatch();
	}

	if (CommandIs("SetAmpl"))
	{
		fprintf(stderr, "vss warning: you mean SetAmp not SetAmpl.\n");
		goto LSetAmp;
	}

	if (CommandIs("SetGain"))
	{
		ifFF(z,z2, setGain(z, z2));
		ifF( z, setGain(z) );
		return Uncatch();
	}

	if (CommandIs("ScaleGain"))
	{
		ifFF(z,z2, scaleGain(z, z2));
		ifF(z, scaleGain(z));
		return Uncatch();
	}

	if (CommandIs("ScaleAmp"))
	{
		ifFF(z,z2, scaleAmp(z, z2));
		ifF(z, scaleAmp(z));
		return Uncatch();
	}

	if (CommandIs("SetPan"))
	{
		ifFF(z,z2, setPan(z, z2));
		ifF( z, setPan(z) );
		return Uncatch();
	}

	if (CommandIs("SetElev"))
	{
		ifFF(z,z2, setElev(z, z2));
		ifF( z, setElev(z) );
		return Uncatch();
	}

	if (CommandIs("SetDistance"))
	{
		ifFF(z,z2, setDistance(z, z2));
		ifF( z, setDistance(z) );
		return Uncatch();
	}

	if (CommandIs("SetDistanceHorizon"))
	{
		ifF( z, setDistanceHorizon(z) );
		return Uncatch();
	}

	if (CommandIs("SetXYZ"))
	{
		ifFFFF(x,y,z,z2, setXYZ(x,y,z, z2));
		ifFFF( x,y,z, setXYZ(x,y,z) );
		return Uncatch();
	}

	if (CommandIs("SetLinearEnv"))
	{
		ifD( lin, setLinear(lin) );
		return Uncatch();
	}

	if (CommandIs("SetInput"))
	{
		ifF(z, setInput(z) );
		ifNil(setInput());
	}

	if (CommandIs("SetNumChans"))
	{
		ifD(d, getAlg()->Nchan(d) );
		return Uncatch();
	}

	if (CommandIs("SetChannelAmps"))
	{
		ifFloatArray( rgz, cz, setChannelAmps(rgz, cz) );
		return Uncatch();
	}

	if (CommandIs("SetChannel"))
	{
		ifD( d, setChannel(d) );
		return Uncatch();
	}

	if (CommandIs("Debug"))
	{
		ifD(f, setDebug(f) );
		return Uncatch();
	}

	const auto tmp = VActor::receiveMessage(Message);
	if (delete_me() /* set by VActor::receiveMessage */)
          delete this;
	// We're called only at the end of a fooHand::receiveMessage,
	// which only forwards this return value,
	// and thus won't try to access anything in just-deleted *this.
	return tmp;
}

void VHandler::setDebug(int f)
{
	VActor::setDebug(f);
	getAlg()->setDebug(f);
}

//  Specify a source for ring modulation.
void VHandler::setInput(float hSrc)
{
	input = getByHandle( hSrc )->as_handler();
	if (!input || !input->FValid())
		{
		// That handler must have just got deleted.  Oh well.
		fprintf(stderr, "vss %s error: invalid input\n", typeName());
		input = NULL;
		getAlg()->setSource(NULL);
		return;
		}
	getAlg()->setSource( (VAlgorithm*)input->getAlg() );
}

void VHandler::setInput(void)
{
	input = NULL;
	getAlg()->setSource(NULL);
}

//	Derived classes that need to do something fancier to initialize 
//	themselves should override restrike(). In most cases, the derived
//	class should call its parent class' restrike() in addition to
//	performing its own state initialization.
void
VHandler::restrike(const char * inits_msg)
{
	VGeneratorActor * myParent = getParent();
	if (myParent == NULL)
	{
		printf("vss warning: VHandler can't find its parent, so it can't restrike.\n");
		return;
	}
	myParent->sendDefaults(this);
	myParent->parseInitializers(inits_msg, this);
}


void VHandler::SetAttribute(IParam iParam, float z)
{
	cerr << "vss warning: SetAttribute() unimplemented in handler " << typeName() << endl;
}

void VHandler::SetAttribute(IParam iParam, float* rgz)
{
	cerr << "vss warning: SetAttribute() unimplemented in handler " << typeName() << endl;
}

//===========================================================================
//		setGain, etc.
//		Send these values to the algorithm.

float VHandler::AdjustTime(float& time)
{
	if (time == timeDefault)
		time = getPause() ? 0. : dampingTime();
	else if (time < 0.)
		time = 0.;
	return time;
}

void
VHandler::setInputGain(float a, float time)
{
	getAlg()->setInputGain(a, AdjustTime(time));
}

void
VHandler::setInputAmp(float a, float time)
{
	if (a<0.)
		{
		fprintf(stderr, "vss warning: negative SetInputAmp amplitude %g rounded up to zero\n", a);
		a=0.;
		}
	getAlg()->setInputAmp( zInputAmp=a, AdjustTime(time));
}

void
VHandler::invertAmp(int f)
{
	getAlg()->invertAmp(f);
}

void
VHandler::setGain(float a, float time)
{
	getAlg()->setGain(a, AdjustTime(time));
}

void
VHandler::setAmp(float a, float time)
{
	if (a<0.)
		{
		fprintf(stderr, "vss warning: negative SetAmp amplitude %g rounded up to zero\n", a);
		a=0.;
		}
	getAlg()->setAmp( zAmp=a, AdjustTime(time) );
}

void
VHandler::scaleGain(float a, float time)
{
	getAlg()->scaleGain(a, AdjustTime(time));
}

void
VHandler::scaleAmp(float a, float time)
{
	if (a<0.)
		{
		fprintf(stderr, "vss warning: negative ScaleAmp amplitude %g rounded up to zero\n", a);  
		a=0.;
		}
	getAlg()->scaleAmp(a, AdjustTime(time));
}

void
VHandler::setPan(float a, float time)
{
	if (a < -1.0) a = -1.0;
	else if (a > 1.0) a = 1.0;
	getAlg()->setPan(a, AdjustTime(time));
}

void
VHandler::setElev(float a, float time)
{
	if (a < -1.0) a = -1.0;
	else if (a > 1.0) a = 1.0;
	getAlg()->setElev(a, AdjustTime(time));
}

void
VHandler::setDistance(float a, float time)
{
	if (a<0.) a=0.;
	getAlg()->setDistance(a, AdjustTime(time));
}

void VHandler::setDistanceHorizon(float a)
{
	getAlg()->setDistanceHorizon(a);
}

void VHandler::setXYZ(float x, float y, float z, float time)
{
	// Calls SetPan SetElev SetDistance, in cave coords.
	// Passes on "time" literally to SetPan SetElev SetDistance.

	AdjustTime(time);

	y -= 5.; // Origin is (0, 5, 0) in cave coords.

	float myPan;
	if (Nchans() == 2)
		{
		// A *very* crude approximation, so stereo stands a chance.
		// Pan hard left or right for sounds with azimuth > 45 degrees
		// from straight ahead or behind; linearly interpolate, in between.
		// Ignore y-coordinate.
		// (We shouldn't really:  for large |y|, pan should be near 0.)
		// Tweak:  sounds less than 6 feet away (ignoring y),
		// pan closer to centered the smaller their distance is.
		myPan = fabs(z) < .001 ? x : x / fabs(z);
		if (myPan < -1.) myPan = -1.;
		else if (myPan > 1.) myPan = 1.;
		float centerweighting = (6. - fsqrt(x*x + z*z)) / 6.;
		if (centerweighting > 0.)
			myPan *= (1 - centerweighting);
		}
	else
		myPan = atan2(x, -z) / M_PI;
	setPan(myPan, time);
	// Kinda bogus: can't distinguish elevations > .61 radians
	// (atan(1/sqrt(2)).
	float elevT = atan2(y, fhypot(x, z));
	if (elevT > .61548) elevT = .61548;
	else if (elevT < -.61548) elevT = -.61548;
	elevT /= .61548;
	setElev(elevT, time);
	setDistance(fsqrt(x*x + y*y + z*z), time);
}

void
VHandler::setChannel(int iChan)
{
	if (iChan<0 || iChan>Nchans())
		{
		fprintf(stderr, "vss error: SetChannel(%d) out of range 0..%d\n",
			iChan, Nchans()-1);
		return;
		}

	float rgz[MaxNumChannels] = {0};
	rgz[iChan] = 1.;
	setChannelAmps(rgz, Nchans());
}

void
VHandler::setChannelAmps(float* rgz, int cz)
{
	if (cz != Nchans())
		{
		fprintf(stderr, "vss error: SetChannelAmps got %d values instead of %d.\n",
			cz, Nchans());
		return;
		}
	getAlg()->setAmplsDirectly(1); // disable setPan etc.
	getAlg()->setPanAmps(rgz);
}

void
VHandler::setLinear(int fLin)
{
	getAlg()->setLinear( fLinearEnv = fLin );
}

// Start at zero, then ramp amplitudes up to what they are right now.
// This thing is nonorthogonal to the rest of this code.  Only gran/granHand.c++ uses it.
void
VHandler::RampUpAmps(float time)
{
	float a = zAmp;
	setAmp( 0., 0.);
	setAmp(a, time);
}

//	Print biographical info.
ostream & 
VHandler::dump(ostream &os, int tabs)
{
	VActor::dump(os, tabs);
	indent(os, tabs) << "Parent generator actor handle: " << parentHandle << endl;
	indent(os, tabs) << "Amp: " << zAmp << endl;
	return os;
}

//===========================================================================
// VModulator stuff.

VFloatParam::VFloatParam(float oldVal, float newVal, float modTime) :
	VModulator(),
	dstVal(newVal)
{
	// special case, which is quite common actually
	if (oldVal == newVal)
		{
		// Don't bother to initialize dstSamp, because when fDone, only newVal is used.
		fDone = 1;
		return;
		}

	const float modSamps = modTime * globs.SampleRate;
	slope = modSamps < 1. ? 0. : (newVal - oldVal) / modSamps;
#ifdef DEBUG
	if (oldVal != oldVal)
		cerr << "vss internal error: VFloatParam::VFloatParam(oldval) bogus, probably from an uninitialized member variable.\n";
	printf("\t\tVFloatParam::VFloatParam(%g %g %g)\n", oldVal, newVal, modTime);
#endif
	dstSamp = globs.SampleCount + modSamps;
}

float VFloatParam::currentValue(void)
{
	if (dstSamp <= globs.SampleCount)
		{
		// We're finished.
		fDone = 1;
		return dstVal;
		}
	return dstVal - ((double)(dstSamp - globs.SampleCount) * slope);
}

float VFloatArrayElement::currentValue(void)
{
	if (dstSamp <= globs.SampleCount)
		{
		// We're finished.
		fDone = 1;
		return dstVal;
		}
	return dstVal - ((double)(dstSamp - globs.SampleCount) * slope);
}

VFloatArrayElement::VFloatArrayElement(float oldVal, float newVal, float modTime) :
	VModulator(),
	dstVal(newVal)
{
	// special case, which is quite common actually
	if (oldVal == newVal)
		{
		dstSamp = -1000000;
		fDone = 1;
		return;
		}

	float modSamps = modTime * globs.SampleRate;
	slope = modSamps < 1. ? 0. : (newVal - oldVal) / modSamps;
	dstSamp = (long)((float)(globs.SampleCount) + modSamps);
}

VFloatArray::VFloatArray(int sizeArg, const float* oldVals, const float* newVals, float modTime) :
	VModulator(),
	size(sizeArg),
	dstVals(new float[sizeArg]),
	curVals(new float[sizeArg]),
	slopes(new float[sizeArg])
{
	FloatCopy(dstVals, newVals, size);

	// special case, which is quite common actually: old=new
	if (!memcmp(oldVals, newVals, size*sizeof(float)))
		{
		dstSamp = -1000000;
		fDone = 1;
		return;
		}

	float modSamps = modTime * globs.SampleRate;
	for (int i=0; i<size; i++)
		slopes[i] = modSamps < 1. ? 0. :
			(newVals[i] - oldVals[i]) / modSamps;
	dstSamp = (long)((float)(globs.SampleCount) + modSamps);
}

VFloatArray::~VFloatArray()
{
	delete [] dstVals;
	delete [] curVals;
	delete [] slopes;
}

float* VFloatArray::currentValue(void)
{
	// If our time is up, set the slope to zero and return dstVals.
	if (dstSamp <= globs.SampleCount)
		{
		fDone = 1;
		return dstVals;
		}
	for (int i=0; i<size; i++)
		{
		curVals[i] = dstVals[i] -
			((double)(dstSamp - globs.SampleCount) * slopes[i]);
		}
	return curVals;
}

int VFloatParam::SetAttribute(VHandler* phandler, IParam iParam)
{
	// This if() is the only difference between classes VFloatParam and VFloatArrayElement.
	if (!iParam.FOnlyI())
		{
		cerr << "vss internal error in VFloatParam::SetAttribute()\n";
		return 0;
		}
	phandler->SetAttribute(iParam, currentValue());
	return !fDone;
}

int VFloatArrayElement::SetAttribute(VHandler* phandler, IParam iParam)
{
	// This if() is the only difference between classes VFloatParam and VFloatArrayElement.
	if (iParam.Iz() < 0)
		{
		cerr << "vss internal error in VFloatArrayElement::SetAttribute()\n";
		return 0;
		}
	phandler->SetAttribute(iParam, currentValue());
	return !fDone;
}

int VFloatArray::SetAttribute(VHandler* phandler, IParam iParam)
{
	if (iParam.Cz() < 0)
		{
		cerr << "vss internal error in VFloatArray::SetAttribute()\n";
		return 0;
		}
	phandler->SetAttribute(iParam, currentValue());
	return !fDone;
}

void VModulatorPool::insert(int iParam,
	float duration, float zCur, float zEnd)
{
	IParam i(iParam);
	insertPrep(i);
	auto p = (VModulator*)new VFloatParam(zCur, zEnd, duration);
	m.insert(make_pair(i, p));
}

void VModulatorPool::insert(int iParam,
	float duration, int iz, float zCur, float zEnd)
{
	IParam i(iParam, iz);
	insertPrep(i);
	auto p = (VModulator*)new VFloatArrayElement(zCur, zEnd, duration);
	m.insert(make_pair(i, p));
}

void VModulatorPool::insert(int iParam,
	float duration, int cz, const float* rgzCur, const float* rgzEnd)
{
	IParam i(iParam, cz);
	insertPrep(i);
	auto p = (VModulator*)new VFloatArray(cz, rgzCur, rgzEnd, duration);
	m.insert(make_pair(i, p));
}

//===========================================================================
// VModulatorPool stuff.

void VModulatorPool::insertPrep(const IParam iparam)
{
	for (Modmap::iterator i = m.begin(); i != m.end(); ++i)
		{
		if (i->first == iparam)
			{
			delete i->second;
			m.erase(i);
			// At most one of these can exist, so we might as well return.
			return;
			}
		}
}

void VModulatorPool::act(VHandler* phandler)
{
	SanityCheck(phandler);
	Modmap::iterator i;
	for (i = m.begin(); i != m.end(); )
		{
		// i->first is an IParam
		// i->second is a VModulator*

		if (!i->second->SetAttribute(phandler, i->first))
			{
			// It finished modulating, so we can clean it up.
			// Can't increment i after i's been erase()'d, so do it before.
			Modmap::iterator j = i;
			++i;
			delete j->second;
			m.erase(j);
			}
		else
			++i;
		}
	SanityCheck(phandler);
}

void VModulatorPool::SanityCheck(VHandler* phandler)
{
	// ;;Make sure that two guys aren't modulating the same thing.

	// Do other checking here too.
	if (!phandler)
		cerr << "vss internal error: VModulatorPool::SanityCheck() null phandler\n";
}

