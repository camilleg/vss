#include "distance.h"

//===========================================================================
//	dso magic
//
#include "actorDso.h"
extern char* actorlist[]; char* actorlist[] = { "DistanceActor", "" };
DSO_DECLARE(distanceActor, DistanceActor)

//===========================================================================
//		construction
//
//	Initialize the defaults for distance parameters, they will be
//	sent in sendDefaults().
//
distanceActor::distanceActor(void) :
	defaultDistance( 0. ), 
	VGeneratorActor() 
{
	setTypeName("distanceActor");
}

//===========================================================================
//		sendDefaults
//
void 
distanceActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	distanceHand * h = (distanceHand *)p;
	h->setDistance(defaultDistance, 0.);
}

//===========================================================================
//		receiveMessage
//
int
distanceActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetAllDistance"))
	{
		ifFF(z,z2, setAllDistance(z, z2) );
		ifF(z, setAllDistance(z) );
		return Uncatch();
	}

	if (CommandIs("SetDistance"))
	{
		ifF(z, setDistance(z) );
		return Uncatch();
	}

	return VGeneratorActor::receiveMessage(Message);
}

//===========================================================================
//		setDistance
//
//	Set default distance for this actor.
//
void
distanceActor::setDistance(float f)
{
	if (! CheckDist(f)) 
		printf("distanceActor got bogus distance %f.\n", f );
	else
		defaultDistance = f;
}

//===========================================================================
//		setAllDistance
//	
//	Call setDistance for all of my children, and
//	set default distance for this actor.
//
void
distanceActor::setAllDistance(float f, float t)
{
	if (!CheckDist(f)) 
	{
		printf("distanceActor got bogus distance %f.\n", f );
		return;
	}

	HandlerListIterator< distanceHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setDistance(f, t);
	}

	defaultDistance = f;
}

