#include "order1.h"

//===========================================================================
//		construction
//
//	Initialize the defaults for process parameters, they will be
//	sent in sendDefaults().
//
order1FiltActor::order1FiltActor(void) : 
	VGeneratorActor(),
	defaultFrequency( 1000. ),
	defaultLPgain( 1. ),
	defaultHPgain( 0. ),
	defaultAPgain( 0. )
{
	setTypeName("Order1FilterActor");
}

//===========================================================================
//		sendDefaults
//
void 
order1FiltActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	order1FiltHand * h = (order1FiltHand *)p;
	h->setFrequency(defaultFrequency, 0.);
	h->setLPGain(defaultLPgain, 0.);
	h->setHPGain(defaultHPgain, 0.);
	h->setAPGain(defaultAPgain, 0.);
}

//===========================================================================
//		receiveMessage
//
int
order1FiltActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetAllFrequency"))
	{
		ifFF(z,z2, setAllFrequency(z, z2) );
		ifF(z, setAllFrequency(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllLowPassGain"))
	{
		ifFF(z,z2, setAllLPGain(z, z2) );
		ifF(z, setAllLPGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllHighPassGain"))
	{
		ifFF(z,z2, setAllHPGain(z, z2) );
		ifF(z, setAllHPGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllAllPassGain"))
	{
		ifFF(z,z2, setAllAPGain(z, z2) );
		ifF(z, setAllAPGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetFrequency"))
	{
		ifF(z, setFrequency(z) );
		return Uncatch();
	}

	if (CommandIs("SetLowPassGain"))
	{
		ifF(z, setLPGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetHighPassGain"))
	{
		ifF(z, setHPGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllPassGain"))
	{
		ifF(z, setAPGain(z) );
		return Uncatch();
	}

	return VGeneratorActor::receiveMessage(Message);
}

//===========================================================================
//		setFrequency
//
//	Set default corner frequency for this actor.
//
void
order1FiltActor::setFrequency(float f)
{
	if (! CheckFrequency(f)) 
		printf("order1FiltActor got bogus corner frequency %f.\n", f );
	else
		defaultFrequency = f;
}

//===========================================================================
//		setAllFequency	
//
//	Call setFrequency for all of my children.
//
void
order1FiltActor::setAllFrequency(float f, float t)
{
	if (! CheckFrequency(f)) 
	{
		printf("order1FiltActor got bogus corner frequency %f.\n", f );
		return;
	}

	HandlerListIterator< order1FiltHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setFrequency(f, t);
	}

	defaultFrequency = f;
}

//===========================================================================
//		setLPGain
//
//	Set default lowpass transfer gain for this actor.
//
void
order1FiltActor::setLPGain(float f)
{
	if (! CheckGain(f)) 
		printf("order1FiltActor got bogus filter gain %f.\n", f );
	else
		defaultLPgain = f;
}

//===========================================================================
//		setAllLPGain	
//
//	Call setLPGain for all of my children.
//
void
order1FiltActor::setAllLPGain(float f, float t)
{
	if (! CheckGain(f)) 
	{
		printf("order1FiltActor got bogus filter gain %f.\n", f );
		return;
	}

	HandlerListIterator< order1FiltHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setLPGain(f, t);
	}

	defaultLPgain = f;
}

//===========================================================================
//		setHPGain
//
//	Set default highpass transfer gain for this actor.
//
void
order1FiltActor::setHPGain(float f)
{
	if (! CheckGain(f)) 
		printf("order1FiltActor got bogus filter gain %f.\n", f );
	else
		defaultHPgain = f;
}

//===========================================================================
//		setAllHPGain	
//
//	Call setHPGain for all of my children.
//
void
order1FiltActor::setAllHPGain(float f, float t)
{
	if (! CheckGain(f)) 
	{
		printf("order1FiltActor got bogus filter gain %f.\n", f );
		return;
	}

	HandlerListIterator< order1FiltHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setHPGain(f, t);
	}

	defaultHPgain = f;
}

//===========================================================================
//		setAPGain
//
//	Set default allpass transfer gain for this actor.
//
void
order1FiltActor::setAPGain(float f)
{
	if (! CheckGain(f)) 
		printf("order1FiltActor got bogus filter gain %f.\n", f );
	else
		defaultAPgain = f;
}

//===========================================================================
//		setAllAPGain	
//
//	Call setAPGain for all of my children.
//
void
order1FiltActor::setAllAPGain(float f, float t)
{
	if (! CheckGain(f)) 
	{
		printf("order1FiltActor got bogus filter gain %f.\n", f );
		return;
	}

	HandlerListIterator< order1FiltHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setAPGain(f, t);
	}

	defaultAPgain = f;
}

