#include "mixer.h"

ACTOR_SETUP(mixerActor, MixerActor)

//===========================================================================
//		construction
//
//	Initialize the defaults for mixer parameters, they will be
//	sent in sendDefaults().
//
mixerActor::mixerActor(void) : 
	VGeneratorActor(),
	defaultFaderAmp( 0. )
{
	setTypeName("MixerActor");
}

//===========================================================================
//		sendDefaults
//
void 
mixerActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	mixerHand * h = (mixerHand *)p;
	for (int i=1; i<=MaxNumInput; i++)
	{
		h->setOneFaderAmp(i, defaultFaderAmp);
	}
}

//===========================================================================
//		receiveMessage
//
int
mixerActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetFaderAmp"))
    {
		ifF(z, setFaderAmp(z) );
		return Uncatch();
	}

	if (CommandIs("SetFaderGain"))
    {
		ifF(z, setFaderGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllFaderAmp"))
    {
		ifFF(z,z2, setAllFaderAmp(z, z2) );
		ifF(z, setAllFaderAmp(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllFaderGain"))
    {
		ifFF(z,z2, setAllFaderGain(z, z2) );
		ifF(z, setAllFaderGain(z) );
		return Uncatch();
	}

	return VGeneratorActor::receiveMessage(Message);
}

//===========================================================================
//		setFaderAmp
//
//	Set default linear fader value for this actor.
//
void
mixerActor::setFaderAmp(float f)
{
	if (! CheckFaderAmp(f)) 
		printf("mixerActor got bogus linear fader value %f.\n", f );
	else
		defaultFaderAmp = f;
}

//===========================================================================
//		setAllFaderAmp	
//
//	Call setFaderAmp for all of my children.
//
void
mixerActor::setAllFaderAmp(float f, float t)
{
	if (! CheckFaderAmp(f)) 
	{
		printf("mixerActor got bogus linear fader value %f.\n", f );
		return;
	}

	HandlerListIterator< mixerHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		for (int i=0; i<MaxNumInput; i++)
		{
			(*it)->setOneFaderAmp(i+1, f, t);
		}
	}

	defaultFaderAmp = f;
}

//===========================================================================
//		setFaderGain
//
//	Set default log fader value for this actor.
//
void
mixerActor::setFaderGain(float f)
{
	if (! CheckFaderGain(f)) 
		printf("mixerActor got bogus log fader value %f.\n", f );
	else
		defaultFaderAmp = pow(10.,f/20.);
}

//===========================================================================
//		setAllFaderGain
//	
//	Call setFaderGain for all of my children.
//
void
mixerActor::setAllFaderGain(float f, float t)
{
	if (!CheckFaderGain(f)) 
	{
		printf("mixerActor got bogus log fader value %f.\n", f );
		return;
	}

	HandlerListIterator< mixerHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		for (int i=0; i<MaxNumInput; i++)
		{
			(*it)->setChannelNum(i+1);
			(*it)->setOneFaderGain(i+1, f, t);
		}
	}
	defaultFaderAmp = pow(10.f, f/20.f);
}

