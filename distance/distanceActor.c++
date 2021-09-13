#include "distance.h"

ACTOR_SETUP(distanceActor, DistanceActor)

distanceActor::distanceActor(void) :
	VGeneratorActor(),
	defaultDistance(0.0)
{
	setTypeName("distanceActor");
}

void 
distanceActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	distanceHand * h = (distanceHand *)p;
	h->setDistance(defaultDistance, 0.);
}

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

//	Set default distance for this actor.
void
distanceActor::setDistance(float f)
{
	if (! CheckDist(f)) 
		printf("distanceActor got bogus distance %f.\n", f );
	else
		defaultDistance = f;
}

//	Call setDistance for all of my children, and
//	set default distance for this actor.
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
