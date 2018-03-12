#include "stereo.h"

ACTOR_SETUP(stereoActor, StereoActor)

//===========================================================================
//		construction
//
//	Initialize the defaults for stereo parameters, they will be
//	sent in sendDefaults().
//
stereoActor::stereoActor(void) : 
	VGeneratorActor(),
	defaultPan( 0 )
{
	setTypeName("StereoActor");
}

//===========================================================================
//		sendDefaults
//
void 
stereoActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	stereoHand * h = (stereoHand *)p;
	h->setPan(defaultPan, 0.);
}

//===========================================================================
//		receiveMessage
//
int
stereoActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetAllPan"))
    {
		ifFF(z,z2, setAllPan(z, z2) );
		ifF(z, setAllPan(z) );
		return Uncatch();
	}

	if (CommandIs("SetPan"))
    {
		ifF(z, setPan(z) );
		return Uncatch();
	}

	return VGeneratorActor::receiveMessage(Message);
}

//===========================================================================
//		setPan
//
//	Set default mod index for this actor.
//
void
stereoActor::setPan(float f)
{
	if (! CheckPan(f)) 
		printf("stereoActor got bogus pan value %f.\n", f );
	else
		{
		defaultPan = f;
		}
}

//===========================================================================
//		setAllPan	
//
//	Call setPan for all of my children.
//
void
stereoActor::setAllPan(float f, float t)
{
	if (! CheckPan(f)) 
	{
		printf("stereoActor got bogus mod index %f.\n", f );
		return;
	}

	HandlerListIterator< stereoHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setPan(f, t);
	}

	defaultPan = f;
}

