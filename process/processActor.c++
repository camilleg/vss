#include "process.h"

ACTOR_SETUP(processActor, ProcessActor)

//===========================================================================
//		construction
//
//	Initialize the defaults for process parameters, they will be
//	sent in sendDefaults().
//
processActor::processActor(void) : 
	VGeneratorActor(),
	defaultModIndex( 1 )
{
	setTypeName("ProcessActor");
}

//===========================================================================
//		sendDefaults
//
void 
processActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	processHand * h = (processHand *)p;
	h->setModIndex(defaultModIndex, 0.);
}

//===========================================================================
//		receiveMessage
//
int
processActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

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
//		setModIndex
//
//	Set default mod index for this actor.
//
void
processActor::setModIndex(float f)
{
	if (! CheckModIndex(f)) 
		printf("processActor got bogus mod index %f.\n", f );
	else
		defaultModIndex = f;
}

//===========================================================================
//		setAllModIndex	
//
//	Call setModIndex for all of my children.
//
void
processActor::setAllModIndex(float f, float t)
{
	if (! CheckModIndex(f)) 
	{
		printf("processActor got bogus mod index %f.\n", f );
		return;
	}

	HandlerListIterator< processHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setModIndex(f, t);
	}

	defaultModIndex = f;
}

