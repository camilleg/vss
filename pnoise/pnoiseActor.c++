#include "pnoise.h"

ACTOR_SETUP(pnoiseActor, PseudoNoiseActor)

//===========================================================================
//		construction
//
//	Initialize the defaults for noise parameters, they will be
//	sent in sendDefaults().
//
pnoiseActor::pnoiseActor(void) : 
	VGeneratorActor(),
	defaultCutoff( 1000. ),
	defaultModCutoff( 1000. ),
	defaultModIndex( 0. )
{
	setTypeName("PseudoNoiseActor");
}

//===========================================================================
//		sendDefaults
//
void 
pnoiseActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	noiseHand * h = (noiseHand *)p;
	h->setCutoff(defaultCutoff, 0.);
	h->setModCutoff(defaultModCutoff, 0.);
	h->setModIndex(defaultModIndex, 0.);
}

//===========================================================================
//		receiveMessage
//
int
pnoiseActor::receiveMessage(const char * Message)
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

	if (CommandIs("SetAllModCutoff"))
	{
		ifFF(z,z2, setAllModCutoff(z, z2) );
		ifF(z, setAllModCutoff(z) );
		return Uncatch();
	}

	if (CommandIs("SetModCutoff"))
    	{
		ifF(z, setModCutoff(z) );
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
//		setCutoff
//
//	Set default cutoff freq for this actor.
//
void
pnoiseActor::setCutoff(float f)
{
	if (!CheckCutoff(f)) 
		printf("pnoiseActor got bogus cutoff frequency %f.\n", f );
	else
		defaultCutoff = f;
}

//===========================================================================
//		setAllCutoff
//	
//	Call setCutoff for all of my children.
//
void
pnoiseActor::setAllCutoff(float f, float t)
{
	if (!CheckCutoff(f)) 
	{
		printf("pnoiseActor got bogus cutoff frequency %f.\n", f );
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
//		setModCutoff
//
//	Set default mod cutoff freq for this actor.
//
void
pnoiseActor::setModCutoff(float f)
{
	if (!CheckCutoff(f)) 
		printf("pnoiseActor got bogus cutoff frequency %f.\n", f );
	else
		defaultModCutoff = f;
}

//===========================================================================
//		setAllCutoff
//	
//	Call setModCutoff for all of my children.
//
void
pnoiseActor::setAllModCutoff(float f, float t)
{
	if (!CheckCutoff(f)) 
	{
		printf("pnoiseActor got bogus mod cutoff frequency %f.\n", f );
		return;
	}

	HandlerListIterator< noiseHand > it;
    	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setModCutoff(f, t);
	}

	defaultModCutoff = f;
}

//===========================================================================
//		setModIndex
//
//	Set default mod index for this actor.
//
void
pnoiseActor::setModIndex(float f)
{
	if (! CheckMod(f)) 
		printf("pnoiseActor got bogus mod index %f.\n", f );
	else
		defaultModIndex = f;
}

//===========================================================================
//		setAllModIndex	
//
//	Call setModIndex for all of my children.
//
void
pnoiseActor::setAllModIndex(float f, float t)
{
	if (! CheckMod(f)) 
	{
		printf("pnoiseActor got bogus mod index %f.\n", f );
		return;
	}

	HandlerListIterator< noiseHand > it;
    	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setModIndex(f, t);
	}

	defaultModIndex = f;
}

//===========================================================================
//	dump
//
ostream &
pnoiseActor::dump(ostream &os, int tabs)
{
	VGeneratorActor::dump(os, tabs);

	indent(os, tabs) << "Cutoff Frequency: " << defaultCutoff << endl;
	indent(os, tabs) << "Modulator Cutoff Frequency: " << defaultModCutoff << endl;
	indent(os, tabs) << "Modulation Index: " << defaultModIndex << endl;

	return os;
}
