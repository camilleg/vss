#include "biquad.h"

//===========================================================================
//		construction
//
//	Initialize the defaults for process parameters, they will be
//	sent in sendDefaults().
//
biquadFiltActor::biquadFiltActor(void) : 
	VGeneratorActor(),
	defaultFrequency( 1000. ),
	defaultResonance( 1.0 ),
	defaultLPgain( 1. ),
	defaultBPgain( 0. ),
	defaultHPgain( 0. ),
	defaultAPgain( 0. ),
	defaultNgain( 0. )
{
	setTypeName("BiQuadFilterActor");
}

//===========================================================================
//		sendDefaults
//
void 
biquadFiltActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	biquadFiltHand * h = (biquadFiltHand *)p;
	h->setFrequency(defaultFrequency, 0.);
	h->setResonance(defaultResonance, 0.);
	h->setLPGain(defaultLPgain, 0.);
	h->setBPGain(defaultBPgain, 0.);
	h->setHPGain(defaultHPgain, 0.);
	h->setAPGain(defaultAPgain, 0.);
	h->setNGain(defaultNgain, 0.);
}

//===========================================================================
//		receiveMessage
//
int
biquadFiltActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetAllFrequency"))
	{
		ifFF(z,z2, setAllFrequency(z, z2) );
		ifF(z, setAllFrequency(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllResonance"))
	{
		ifFF(z,z2, setAllResonance(z, z2) );
		ifF(z, setAllResonance(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllLowPassGain"))
	{
		ifFF(z,z2, setAllLPGain(z, z2) );
		ifF(z, setAllLPGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllBandPassGain"))
	{
		ifFF(z,z2, setAllBPGain(z, z2) );
		ifF(z, setAllBPGain(z) );
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

	if (CommandIs("SetAllNotchGain"))
	{
		ifFF(z,z2, setAllNGain(z, z2) );
		ifF(z, setAllNGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetFrequency"))
	{
		ifF(z, setFrequency(z) );
		return Uncatch();
	}

	if (CommandIs("SetResonance"))
	{
		ifF(z, setResonance(z) );
		return Uncatch();
	}

	if (CommandIs("SetLowPassGain"))
	{
		ifF(z, setLPGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetBandPassGain"))
	{
		ifF(z, setBPGain(z) );
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

	if (CommandIs("SetNotchGain"))
	{
		ifF(z, setNGain(z) );
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
biquadFiltActor::setFrequency(float f)
{
	if (! CheckFrequency(f)) 
		printf("biquadFiltActor got bogus corner frequency %f.\n", f );
	else
		defaultFrequency = f;
}

//===========================================================================
//		setAllFrequency	
//
//	Call setFrequency for all of my children.
//
void
biquadFiltActor::setAllFrequency(float f, float t)
{
	if (! CheckFrequency(f)) 
	{
		printf("biquadFiltActor got bogus corner frequency %f.\n", f );
		return;
	}

	HandlerListIterator< biquadFiltHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setFrequency(f, t);
	}

	defaultFrequency = f;
}

//===========================================================================
//		setResonance
//
//	Set default resonance for this actor.
//
void
biquadFiltActor::setResonance(float f)
{
	if (! CheckResonance(f)) 
		printf("biquadFiltActor got bogus resonance %f.\n", f );
	else
		defaultResonance = f;
}

//===========================================================================
//		setAllResonance	
//
//	Call setResonance for all of my children.
//
void
biquadFiltActor::setAllResonance(float f, float t)
{
	if (! CheckResonance(f)) 
	{
		printf("biquadFiltActor got bogus resonance %f.\n", f );
		return;
	}

	HandlerListIterator< biquadFiltHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setResonance(f, t);
	}

	defaultResonance = f;
}

//===========================================================================
//		setLPGain
//
//	Set default lowpass transfer gain for this actor.
//
void
biquadFiltActor::setLPGain(float f)
{
	if (! CheckGain(f)) 
		printf("biquadFiltActor got bogus filter gain %f.\n", f );
	else
		defaultLPgain = f;
}

//===========================================================================
//		setAllLPGain	
//
//	Call setLPGain for all of my children.
//
void
biquadFiltActor::setAllLPGain(float f, float t)
{
	if (! CheckGain(f)) 
	{
		printf("biquadFiltActor got bogus filter gain %f.\n", f );
		return;
	}

	HandlerListIterator< biquadFiltHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setLPGain(f, t);
	}

	defaultLPgain = f;
}

//===========================================================================
//		setBPGain
//
//	Set default bandpass transfer gain for this actor.
//
void
biquadFiltActor::setBPGain(float f)
{
	if (! CheckGain(f)) 
		printf("biquadFiltActor got bogus filter gain %f.\n", f );
	else
		defaultBPgain = f;
}

//===========================================================================
//		setAllBPGain	
//
//	Call setBPGain for all of my children.
//
void
biquadFiltActor::setAllBPGain(float f, float t)
{
	if (! CheckGain(f)) 
	{
		printf("biquadFiltActor got bogus filter gain %f.\n", f );
		return;
	}

	HandlerListIterator< biquadFiltHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setBPGain(f, t);
	}

	defaultBPgain = f;
}

//===========================================================================
//		setHPGain
//
//	Set default highpass transfer gain for this actor.
//
void
biquadFiltActor::setHPGain(float f)
{
	if (! CheckGain(f)) 
		printf("biquadFiltActor got bogus filter gain %f.\n", f );
	else
		defaultHPgain = f;
}

//===========================================================================
//		setAllHPGain	
//
//	Call setHPGain for all of my children.
//
void
biquadFiltActor::setAllHPGain(float f, float t)
{
	if (! CheckGain(f)) 
	{
		printf("biquadFiltActor got bogus filter gain %f.\n", f );
		return;
	}

	HandlerListIterator< biquadFiltHand > it;
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
biquadFiltActor::setAPGain(float f)
{
	if (! CheckGain(f)) 
		printf("biquadFiltActor got bogus filter gain %f.\n", f );
	else
		defaultAPgain = f;
}

//===========================================================================
//		setAllAPGain	
//
//	Call setAPGain for all of my children.
//
void
biquadFiltActor::setAllAPGain(float f, float t)
{
	if (! CheckGain(f)) 
	{
		printf("biquadFiltActor got bogus filter gain %f.\n", f );
		return;
	}

	HandlerListIterator< biquadFiltHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setAPGain(f, t);
	}

	defaultAPgain = f;
}

//===========================================================================
//		setNGain
//
//	Set default notch transfer gain for this actor.
//
void
biquadFiltActor::setNGain(float f)
{
	if (! CheckGain(f)) 
		printf("biquadFiltActor got bogus filter gain %f.\n", f );
	else
		defaultNgain = f;
}

//===========================================================================
//		setAllNGain	
//
//	Call setNGain for all of my children.
//
void
biquadFiltActor::setAllNGain(float f, float t)
{
	if (! CheckGain(f)) 
	{
		printf("biquadFiltActor got bogus filter gain %f.\n", f );
		return;
	}

	HandlerListIterator< biquadFiltHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setNGain(f, t);
	}

	defaultNgain = f;
}

