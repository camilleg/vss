#include "fm.h"

//===========================================================================
//	dso magic
//
#include "actorDso.h"
extern char* actorlist[]; char* actorlist[] = { "FmActor", "" };
DSO_DECLARE(fmActor, FmActor)

//===========================================================================
//		construction
//
//	Initialize the defaults for fm parameters, they will be
//	sent in sendDefaults().
//
fmActor::fmActor(void) : 
	defaultCarFreq( 100 ),
	defaultModIndex( 0 ),
	defaultCMratio( 1 ),
	VGeneratorActor() 
{
	setTypeName("FmActor");
}

//===========================================================================
//		sendDefaults
//
void 
fmActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	fmHand * h = (fmHand *)p;
	h->setCarrierFreq(defaultCarFreq, 0.);
	h->setModIndex(defaultModIndex, 0.);
	h->setCMratio(defaultCMratio, 0.);
}

//===========================================================================
//		receiveMessage
//
int
fmActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetAllCarFreq"))
    {
		ifFF(z,z2, setAllCarrierFreq(z, z2) );
		ifF(z, setAllCarrierFreq(z) );
		return Uncatch();
	}

	if (CommandIs("SetCarFreq"))
    {
		ifF(z, setCarrierFreq(z) );
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

	if (CommandIs("SetAllCMratio"))
    {
		ifFF(z,z2, setAllCMratio(z, z2) );
		ifF(z, setAllCMratio(z) );
		return Uncatch();
	}

	if (CommandIs("SetCMratio"))
    {
		ifF(z, setCMratio(z) );
		return Uncatch();
	}

	return VGeneratorActor::receiveMessage(Message);
}

//===========================================================================
//		setCarrierFreq
//
//	Set default  carrier freq for this actor.
//
void
fmActor::setCarrierFreq(float f)
{
	if (! CheckCarFreq(f)) 
		printf("fmActor got bogus carrier frequency %f.\n", f );
	else
		defaultCarFreq = f;
}

//===========================================================================
//		setAllCarrierFreq
//	
//	Call setCarrierFreq for all of my children.
//
void
fmActor::setAllCarrierFreq(float f, float t)
{
	if (!CheckCarFreq(f)) 
	{
		printf("fmActor got bogus carrier frequency %f.\n", f );
		return;
	}

	HandlerListIterator< fmHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setCarrierFreq(f, t);
	}

	defaultCarFreq = f;
}

//===========================================================================
//		setCMratio
//
//	Set default cm ratio for this actor.
//
void
fmActor::setCMratio(float f)
{
	if (! CheckCMratio(f)) 
		printf("fmActor got bogus cmratio %f.\n", f );
	else
		defaultCMratio = f;
}

//===========================================================================
//		setAllCMratio
//
//	Call setCMratio for all of my children.
//
void
fmActor::setAllCMratio(float f, float t)
{
	if (! CheckCMratio(f)) 
	{
		printf("fmActor got bogus cmratio %f.\n", f );
		return;
	}

	HandlerListIterator< fmHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setCMratio(f, t);
	}

	defaultCMratio = f;
}

//===========================================================================
//		setModIndex
//
//	Set default mod index for this actor.
//
void
fmActor::setModIndex(float f)
{
	if (! CheckModIndex(f)) 
		printf("fmActor got bogus mod index %f.\n", f );
	else
		defaultModIndex = f;
}

//===========================================================================
//		setAllModIndex	
//
//	Call setModIndex for all of my children.
//
void
fmActor::setAllModIndex(float f, float t)
{
	if (! CheckModIndex(f)) 
	{
		printf("fmActor got bogus mod index %f.\n", f );
		return;
	}

	HandlerListIterator< fmHand > it;
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
fmActor::dump(ostream &os, int tabs)
{
	VGeneratorActor::dump(os, tabs);

	indent(os, tabs) << "Carrier Freq: " << defaultCarFreq << endl;
	indent(os, tabs) << "Modulation Index: " << defaultModIndex << endl;
	indent(os, tabs) << "Carrier-Modulator ratio: " << defaultCMratio << endl;

	return os;
}
