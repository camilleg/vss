#include "reverb.h"

ACTOR_SETUP(reverbActor, ReverbActor)

//===========================================================================
//		construction
//
//	Initialize the defaults for reverb parameters, they will be
//	sent in sendDefaults().
//
reverbActor::reverbActor(void) : 
	VGeneratorActor(),
	defaultRevTime( 0.3 ),
	defaultRevMix( 0.5 ),
	defaultRevBright( 0.15 ),
	defaultRevGain( 1. ),
	defaultRevPole( 0.1 ),
	defaultDampRatio( 5. )
{
	setTypeName("ReverbActor");
}

//===========================================================================
//		sendDefaults
//
void 
reverbActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	reverbHand * h = (reverbHand *)p;
	h->setRevTime(defaultRevTime, 0.);
	h->setRevMix(defaultRevMix, 0.);
	h->setRevGain(defaultRevGain, 0.);
	h->setRevBright(defaultRevBright, 0.);
	h->setRevPole(defaultRevPole, 0.);
	h->setDampRatio(defaultDampRatio, 0.);
}

//===========================================================================
//		receiveMessage
//
int
reverbActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetTime"))
    {
		ifF(z, setRevTime(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllTime"))
    {
		ifFF(z,z2, setAllRevTime(z, z2) );
		ifF(z, setAllRevTime(z) );
		return Uncatch();
	}

	if (CommandIs("SetMix"))
    {
		ifF(z, setRevMix(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllMix"))
    {
		ifFF(z,z2, setAllRevMix(z, z2) );
		ifF(z, setAllRevMix(z) );
		return Uncatch();
	}

	if (CommandIs("SetGain"))
    {
		ifF(z, setRevGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllGain"))
    {
		ifFF(z,z2, setAllRevGain(z, z2) );
		ifF(z, setAllRevGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetBright"))
    {
		ifF(z, setRevBright(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllBright"))
    {
		ifFF(z,z2, setAllRevBright(z, z2) );
		ifF(z, setAllRevBright(z) );
		return Uncatch();
	}

	if (CommandIs("SetPole"))
    {
		ifF(z, setRevPole(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllPole"))
    {
		ifFF(z,z2, setAllRevPole(z, z2) );
		ifF(z, setAllRevPole(z) );
		return Uncatch();
	}

	if (CommandIs("SetDampRatio"))
    {
		ifF(z, setDampRatio(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllDampRatio"))
    {
		ifFF(z,z2, setAllDampRatio(z, z2) );
		ifF(z, setAllDampRatio(z) );
		return Uncatch();
	}

	return VGeneratorActor::receiveMessage(Message);
}

//===========================================================================
//		setRevTime: Set default reverberation time for this actor.
//
void
reverbActor::setRevTime(float f)
{
	if (! CheckRevTime(f)) 
		printf("reverbActor got bogus reverberation time %f.\n", f );
	else
		defaultRevTime = f;
}

//===========================================================================
//		setAllRevTime: Call setRevTime for all of my children.
//
void
reverbActor::setAllRevTime(float f, float t)
{
	if (! CheckRevTime(f)) 
	{
		printf("reverbActor got bogus reverberation time %f.\n", f );
		return;
	}

	HandlerListIterator< reverbHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setRevTime(f, t);
	}

	defaultRevTime = f;
}

//===========================================================================
//		setRevMix: Set default rev mix for this actor.
//
void
reverbActor::setRevMix(float f)
{
	if (! CheckRevMix(f)) 
		printf("reverbActor got bogus rev mix %f.\n", f );
	else
		defaultRevMix = f;
}

//===========================================================================
//		setAllRevMix: Call setRevMix for all of my children.
//
void
reverbActor::setAllRevMix(float f, float t)
{
	if (! CheckRevMix(f)) 
	{
		printf("reverbActor got bogus rev mix %f.\n", f );
		return;
	}

	HandlerListIterator< reverbHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setRevMix(f, t);
	}

	defaultRevMix = f;
}

//===========================================================================
//		setRevGain: Set default rev gain for this actor.
//
void
reverbActor::setRevGain(float f)
{
	if (! CheckRevGain(f)) 
		printf("reverbActor got bogus rev gain %f.\n", f );
	else
		defaultRevGain = f;
}

//===========================================================================
//		setAllRevGain: Call setRevGain for all of my children.
//
void
reverbActor::setAllRevGain(float f, float t)
{
	if (! CheckRevGain(f)) 
	{
		printf("reverbActor got bogus rev gain %f.\n", f );
		return;
	}

	HandlerListIterator< reverbHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setRevGain(f, t);
	}

	defaultRevGain = f;
}

//===========================================================================
//		setRevBright: Set default rev bright for this actor.
//
void
reverbActor::setRevBright(float f)
{
	if (! CheckRevBright(f)) 
		printf("reverbActor got bogus rev bright %f.\n", f );
	else
		defaultRevBright = f;
}

//===========================================================================
//		setAllRevBright: Call setRevBright for all of my children.
//
void
reverbActor::setAllRevBright(float f, float t)
{
	if (! CheckRevBright(f)) 
	{
		printf("reverbActor got bogus rev bright %f.\n", f );
		return;
	}

	HandlerListIterator< reverbHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setRevBright(f, t);
	}

	defaultRevBright = f;
}

//===========================================================================
//		setRevPole: Set default rev pole for this actor.
//
void
reverbActor::setRevPole(float f)
{
	if (! CheckRevPole(f)) 
		printf("reverbActor got bogus rev pole %f.\n", f );
	else
		defaultRevPole = f;
}

//===========================================================================
//		setAllRevPole: Call setRevPole for all of my children.
//
void
reverbActor::setAllRevPole(float f, float t)
{
	if (! CheckRevPole(f)) 
	{
		printf("reverbActor got bogus rev pole %f.\n", f );
		return;
	}

	HandlerListIterator< reverbHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setRevPole(f, t);
	}

	defaultRevPole = f;
}

//===========================================================================
//		setDampRatio: Set default rev damp ratio for this actor.
//
void
reverbActor::setDampRatio(float f)
{
	if (! CheckDampRatio(f)) 
		printf("reverbActor got bogus damp ratio %f.\n", f );
	else
		defaultDampRatio = f;
}

//===========================================================================
//		setAllDampRatio: Call setDampRatio for all of my children.
//
void
reverbActor::setAllDampRatio(float f, float t)
{
	if (! CheckDampRatio(f)) 
	{
		printf("reverbActor got bogus damp ratio %f.\n", f );
		return;
	}

	HandlerListIterator< reverbHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setDampRatio(f, t);
	}

	defaultDampRatio = f;
}
