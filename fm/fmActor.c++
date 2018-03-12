#include "fm.h"

//===========================================================================
//		construction
//
//	Initialize the defaults for fm parameters, they will be
//	sent in sendDefaults().
//
fmActor::fmActor(void) :
	VGeneratorActor(),
	defaultRatioMode( 1 ), 
	defaultCarFreq( 100 ),
	defaultModFreq( 100 ),
	defaultCMratio( 1 ),
	defaultModIndex( 0 ),
	defaultCarFeedback( 0 ),
	defaultModFeedback( 0 )
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
	h->setRatioMode(defaultRatioMode);
	h->setCarrierFreq(defaultCarFreq, 0.);
	h->setModulatorFreq(defaultModFreq, 0.);
	h->setCMratio(defaultCMratio, 0.);
	h->setModIndex(defaultModIndex, 0.);
	h->setCarFeedback(defaultCarFeedback, 0.);
	h->setModFeedback(defaultModFeedback, 0.);
}

//===========================================================================
//		receiveMessage
//
int
fmActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetAllRatioMode"))
	{
		ifF(f, setAllRatioMode(f) );
		ifNil( setAllRatioMode() );
//		return Uncatch();
	}

	if (CommandIs("SetRatioMode"))
	{
		ifF(f, setRatioMode(f) );
		ifNil( setRatioMode() );
//		return Uncatch();
	}

	if (CommandIs("SetAllCarFreq"))
	{
		ifFF(z,z2, setAllCarrierFreq(z, z2) );
		ifF(z, setAllCarrierFreq(z) );
		return Uncatch();
	}

	if (CommandIs("SetFreq"))
	{
		ifF(z, setCarrierFreq(z) );
		return Uncatch();
	}

	if (CommandIs("SetCarFreq"))
	{
		ifF(z, setCarrierFreq(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllModFreq"))
	{
		ifFF(z,z2, setAllModulatorFreq(z, z2) );
		ifF(z, setAllModulatorFreq(z) );
		return Uncatch();
	}

	if (CommandIs("SetModFreq"))
	{
		ifF(z, setModulatorFreq(z) );
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

	if (CommandIs("SetAllMCratio"))
	{
		ifFF(z,z2, setAllMCratio(z, z2) );
		ifF(z, setAllMCratio(z) );
		return Uncatch();
	}

	if (CommandIs("SetMCratio"))
	{
		ifF(z, setMCratio(z) );
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

	if (CommandIs("SetAllCarFeedback"))
	{
		ifFF(z,z2, setAllCarFeedback(z, z2) );
		ifF(z, setAllCarFeedback(z) );
		return Uncatch();
	}

	if (CommandIs("SetCarFeedback"))
	{
		ifF(z, setCarFeedback(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllModFeedback"))
	{
		ifFF(z,z2, setAllModFeedback(z, z2) );
		ifF(z, setAllModFeedback(z) );
		return Uncatch();
	}

	if (CommandIs("SetModFeedback"))
	{
		ifF(z, setModFeedback(z) );
		return Uncatch();
	}

	return VGeneratorActor::receiveMessage(Message);
}

//===========================================================================
//		setRatioMode
//
//	Set default ratio-frequency mode for this actor.
//
void
fmActor::setRatioMode(float f)
{
	defaultRatioMode = f;
}

//===========================================================================
//		setAllRatioMode
//	
//	Call setRatioMode for all of my children, and
//	set default ratio-frequency mode for this actor.
//
void
fmActor::setAllRatioMode(float f)
{
	HandlerListIterator< fmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setRatioMode(f);
	}

	defaultRatioMode = f;
}

//===========================================================================
//		setCarrierFreq
//
//	Set default carrier freq for this actor.
//
void
fmActor::setCarrierFreq(float f)
{
	if (! CheckFreq(f)) 
		printf("fmActor got bogus carrier frequency %f.\n", f );
	else
		defaultCarFreq = f;
}

//===========================================================================
//		setAllCarrierFreq
//	
//	Call setCarrierFreq for all of my children, and
//	set default carrier freq for this actor.
//
void
fmActor::setAllCarrierFreq(float f, float t)
{
	if (!CheckFreq(f)) 
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
//		setModulatorFreq
//
//	Set default modulator freq for this actor.
//
void
fmActor::setModulatorFreq(float f)
{
	if (! CheckFreq(f)) 
		printf("fmActor got bogus modulator frequency %f.\n", f );
	else
		defaultModFreq = f;
}

//===========================================================================
//		setAllModulatorFreq
//	
//	Call setModulatorFreq for all of my children, and
//	set default modulator freq for this actor.
//
void
fmActor::setAllModulatorFreq(float f, float t)
{
	if (!CheckFreq(f)) 
	{
		printf("fmActor got bogus modulator frequency %f.\n", f );
		return;
	}

	HandlerListIterator< fmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setModulatorFreq(f, t);
	}

	defaultModFreq = f;
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
//	Call setCMratio for all of my children, and
//	set default cm ratio for this actor.
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
//		setMCratio
//
//	Set default mc ratio for this actor.
//
void
fmActor::setMCratio(float f)
{
	float finv = 1.0f / f;
	if (! CheckCMratio(finv)) 
		printf("fmActor got bogus mcratio %f.\n", f );
	else
		defaultCMratio = finv;
}

//===========================================================================
//		setAllMCratio
//
//	Call setMCratio for all of my children, and
//	set default mc ratio for this actor.
//
void
fmActor::setAllMCratio(float f, float t)
{
	float finv = 1.0f / f;
	if (! CheckCMratio(finv)) 
	{
		printf("fmActor got bogus mcratio %f.\n", f );
		return;
	}

	HandlerListIterator< fmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setMCratio(f, t);
	}

	defaultCMratio = finv;
}

//===========================================================================
//		setModIndex
//
//	Set default mod index for this actor.
//
void
fmActor::setModIndex(float f)
{
	if (! CheckIndex(f)) 
		printf("fmActor got bogus mod index %f.\n", f );
	else
		defaultModIndex = f;
}

//===========================================================================
//		setAllModIndex	
//
//	Call setModIndex for all of my children, and
//	set default mod index for this actor.
//
void
fmActor::setAllModIndex(float f, float t)
{
	if (! CheckIndex(f)) 
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
//		setCarFeedback
//
//	Set default carrier feedback for this actor.
//
void
fmActor::setCarFeedback(float f)
{
	if (! CheckFeedback(f)) 
		printf("fmActor got bogus carrier feedback %f.\n", f );
	else
		defaultCarFeedback = f;
}

//===========================================================================
//		setAllCarFeedback	
//
//	Call setCarFeedback for all of my children, and
//	set default carrier feedback for this actor.
//
void
fmActor::setAllCarFeedback(float f, float t)
{
	if (! CheckFeedback(f)) 
	{
		printf("fmActor got bogus carrier feedback %f.\n", f );
		return;
	}

	HandlerListIterator< fmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setCarFeedback(f, t);
	}

	defaultCarFeedback = f;
}

//===========================================================================
//		setModFeedback
//
//	Set default modulator feedback for this actor.
//
void
fmActor::setModFeedback(float f)
{
	if (! CheckFeedback(f)) 
		printf("fmActor got bogus modulator feedback %f.\n", f );
	else
		defaultModFeedback = f;
}

//===========================================================================
//		setAllModFeedback	
//
//	Call setModFeedback for all of my children, and
//	set default modulator feedback for this actor.
//
void
fmActor::setAllModFeedback(float f, float t)
{
	if (! CheckFeedback(f)) 
	{
		printf("fmActor got bogus modulator feedback %f.\n", f );
		return;
	}

	HandlerListIterator< fmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setModFeedback(f, t);
	}

	defaultModFeedback = f;
}

//===========================================================================
//	dump
//
ostream &
fmActor::dump(ostream &os, int tabs)
{
	VGeneratorActor::dump(os, tabs);

	indent(os, tabs) << "Ratio-Frequency Mode: " << defaultRatioMode << endl;
	indent(os, tabs) << "Carrier Freq: " << defaultCarFreq << endl;
	indent(os, tabs) << "Modulator Freq: " << defaultModFreq << endl;
	indent(os, tabs) << "Carrier-Modulator ratio: " << defaultCMratio << endl;
	indent(os, tabs) << "Modulation Index: " << defaultModIndex << endl;
	indent(os, tabs) << "Carrier Feedback: " << defaultCarFeedback << endl;
	indent(os, tabs) << "Modulator Feedback: " << defaultModFeedback << endl;

	return os;
}
