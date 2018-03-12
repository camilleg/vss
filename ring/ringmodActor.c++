//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1997.
//===========================================================================

#include "ring.h"

ACTOR_SETUP(ringmodActor, RingModActor)

//===========================================================================
//		construction
//
//	Initialize the defaults for ringmod parameters, they will be
//	sent in sendDefaults().
//
ringmodActor::ringmodActor(void) : 
	VGeneratorActor(),
	defaultModFreq( 100 ),
	defaultModIndex( 1 )
{
	setTypeName("RingModActor");
}

//===========================================================================
//		sendDefaults
//
void 
ringmodActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	ringmodHand * h = (ringmodHand *)p;
	h->setModFreq(defaultModFreq, 0.);
	h->setModIndex(defaultModIndex, 0.);
}

//===========================================================================
//		receiveMessage
//
int
ringmodActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetAllModFreq"))
    {
		ifFF(z,z2, setAllModFreq(z, z2) );
		ifF(z, setAllModFreq(z) );
		return Uncatch();
	}

	if (CommandIs("SetModFreq"))
    {
		ifF(z, setModFreq(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllModIndex"))
    {
		ifFF(z,z2, setAllModIndex(z, z2) );
		ifF(z, setAllModIndex(z) );
		return Uncatch();
	}

	if (CommandIs("SetModIndex"))
    {
		ifF(z, setModIndex(z) );
		return Uncatch();
	}

	return VGeneratorActor::receiveMessage(Message);
}

//===========================================================================
//		setModFreq
//
//	Set default modulating freq for this actor.
//
void
ringmodActor::setModFreq(float f)
{
	if (! CheckModFreq(f)) 
		printf("ringmodActor got bogus modulating frequency %f.\n", f );
	else
		defaultModFreq = f;
}

//===========================================================================
//		setAllModFreq
//	
//	Call setModFreq for all of my children.
//
void
ringmodActor::setAllModFreq(float f, float t)
{
	if (!CheckModFreq(f)) 
	{
		printf("ringmodActor got bogus modulating frequency %f.\n", f );
		return;
	}

	HandlerListIterator< ringmodHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setModFreq(f, t);
	}

	defaultModFreq = f;
}

//===========================================================================
//		setModIndex
//
//	Set default mod index for this actor.
//
void
ringmodActor::setModIndex(float f)
{
	if (! CheckModIndex(f)) 
		printf("ringmodActor got bogus mod index %f.\n", f );
	else
		defaultModIndex = f;
}

//===========================================================================
//		setAllModIndex	
//
//	Call setModIndex for all of my children.
//
void
ringmodActor::setAllModIndex(float f, float t)
{
	if (! CheckModIndex(f)) 
	{
		printf("ringmodActor got bogus mod index %f.\n", f );
		return;
	}

	HandlerListIterator< ringmodHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setModIndex(f, t);
	}

	defaultModIndex = f;
}

