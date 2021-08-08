//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1996.
//===========================================================================

#include "noise.h"

ACTOR_SETUP(noiseActor, NoiseActor)

//===========================================================================
//		construction
//
//	Initialize the defaults for noise parameters, they will be
//	sent in sendDefaults().
//
noiseActor::noiseActor(void) : 
	VGeneratorActor(),
	defaultCutoff( 500. ),
	defaultOrder( 1 )
{
	setTypeName("NoiseActor");
}

//===========================================================================
//		sendDefaults
//
void 
noiseActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	noiseHand * h = (noiseHand *)p;
	h->setCutoff(defaultCutoff, 0.);
	h->setOrder(defaultOrder);
}

//===========================================================================
//		receiveMessage
//
int
noiseActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetAllCutoff"))
    {
		ifFF(z,z2, setAllCutoff(z, z2) );
		ifF(z, setAllCutoff(z) );
		return Uncatch();
	}

	if (CommandIs("SetCutoff"))
    {
		ifF(z, setCutoff(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllOrder"))
    {
		ifF(z, setAllOrder(z) );
		return Uncatch();
	}

	if (CommandIs("SetOrder"))
    {
		ifF(z, setOrder(z) );
		return Uncatch();
	}

	return VGeneratorActor::receiveMessage(Message);
}

//===========================================================================
//		setCutoff
//
//	Set default cutoff frequency for this actor.
//
void
noiseActor::setCutoff(float f)
{
	if (! CheckCutoff(f)) 
		printf("noiseActor got bogus cutoff frequency %f.\n", f );
	else
		defaultCutoff = f;
}

//===========================================================================
//		setAllCutoff
//	
//	Call setCutoff for all of my children.
//
void
noiseActor::setAllCutoff(float f, float t)
{
	if (!CheckCutoff(f)) 
	{
		printf("noiseActor got bogus cutoff frequency %f.\n", f );
		return;
	}

	HandlerListIterator< noiseHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setCutoff(f, t);
	}

	defaultCutoff = f;
}

//===========================================================================
//		setOrder
//
//	Set default filter order for this actor.
//
void
noiseActor::setOrder(float f)
{
	defaultOrder = f;
}

//===========================================================================
//		setAllOrder	
//
//	Call setOrder for all of my children.
//
void
noiseActor::setAllOrder(float f)
{
	HandlerListIterator< noiseHand > it;

    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setOrder(f);
	}

	defaultOrder = f;
}

//===========================================================================
//	dump
//
ostream &
noiseActor::dump(ostream &os, int tabs)
{
	VGeneratorActor::dump(os, tabs);

	indent(os, tabs) << "Cutoff: " << defaultCutoff << endl;
	indent(os, tabs) << "Order: " << defaultOrder << endl;

	return os;
}
