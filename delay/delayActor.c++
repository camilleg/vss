#include "delay.h"

ACTOR_SETUP(delayActor, DelayActor)

//===========================================================================
//		construction
//
//	Initialize the defaults for delay parameters, they will be
//	sent in sendDefaults().
//
delayActor::delayActor(void) : 
	VGeneratorActor(),
	defaultDelay(0.)
{
	setTypeName("DelayActor");
}

//===========================================================================
//		sendDefaults
//
void 
delayActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	delayHand * h = (delayHand *)p;
	h->setDelay(defaultDelay, 0.);
	h->setFB(defaultFB, 0.);
}

//===========================================================================
//		receiveMessage
//
int
delayActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetAllDelay"))
    {
		ifFF(z,z2, setAllDelay(z, z2) );
		ifF(z, setAllDelay(z) );
		return Uncatch();
	}

	if (CommandIs("SetDelay"))
    {
		ifF(z, setDelay(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllFeedback"))
    {
		ifFF(z,z2, setAllFB(z, z2) );
		ifF(z, setAllFB(z) );
		return Uncatch();
	}

	if (CommandIs("SetFeedback"))
    {
		ifF(z, setFB(z) );
		return Uncatch();
	}

	return VGeneratorActor::receiveMessage(Message);
}

//===========================================================================
//		setDelay
//
//	Set default mod index for this actor.
//
void
delayActor::setDelay(float f)
{
	if (! CheckDelay(f)) 
		printf("delayActor got bogus delay length %f.\n", f );
	else
		defaultDelay = f;
}

void
delayActor::setFB(float f)
{
	if (! CheckFB(f)) 
		printf("delayActor got bogus feedback value %f.\n", f );
	else
		defaultFB = f;
}

//===========================================================================
//		setAllDelay	
//
//	Call setDelay for all of my children.
//
void
delayActor::setAllDelay(float f, float t)
{
	if (! CheckDelay(f)) 
	{
		printf("delayActor got bogus delay length %f.\n", f );
		return;
	}

	HandlerListIterator< delayHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setDelay(f, t);
	}

	defaultDelay = f;
}

void
delayActor::setAllFB(float f, float t)
{
	if (! CheckFB(f)) 
	{
		printf("delayActor got bogus feedback value %f.\n", f );
		return;
	}

	HandlerListIterator< delayHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setFB(f, t);
	}

	defaultFB = f;
}

