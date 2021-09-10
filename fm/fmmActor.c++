#include "fmm.h"

//===========================================================================
//		construction
//
//	Initialize the defaults for fmm parameters, they will be
//	sent in sendDefaults().
//
fmmActor::fmmActor(void) :
	VGeneratorActor(),

	default1RatioMode( 1 ), 
	default1CarFreq( 100 ),
	default1ModFreq( 100 ),
	default1CMratio( 1 ),
	default1ModIndex( 0 ),
	default1CarFeedback( 0 ),
	default1ModFeedback( 0 ),

	default2RatioMode( 1 ), 
	default2CarFreq( 100 ),
	default2ModFreq( 100 ),
	default2CMratio( 1 ),
	default2CCratio( 1.0 ),
	default2CCModIndex( 1.0 ),
	default2ModIndex( 0 ),
	default2CarFeedback( 0 ),
	default2ModFeedback( 0 ),

	defaultLowpassGain( 1 ),
	defaultHighpassGain( 1 )
{
	setTypeName("FmmActor");
}

//===========================================================================
//		sendDefaults
//
void 
fmmActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	fmmHand * h = (fmmHand *)p;

	h->set1RatioMode(default1RatioMode);
	h->set1CarrierFreq(default1CarFreq, 0.);
	h->set1ModulatorFreq(default1ModFreq, 0.);
	h->set1CMratio(default1CMratio, 0.);
	h->set1ModIndex(default1ModIndex, 0.);
	h->set1CarFeedback(default1CarFeedback, 0.);
	h->set1ModFeedback(default1ModFeedback, 0.);

	h->set2RatioMode(default2RatioMode);
	h->set2CarrierFreq(default2CarFreq, 0.);
	h->set2CCratio(default2CCratio, 0.);
	h->set2CCModIndex(default2CCModIndex, 0.);
	h->set2ModulatorFreq(default2ModFreq, 0.);
	h->set2CMratio(default2CMratio, 0.);
	h->set2ModIndex(default2ModIndex, 0.);
	h->set2CarFeedback(default2CarFeedback, 0.);
	h->set2ModFeedback(default2ModFeedback, 0.);

	h->setLowpassGain(defaultLowpassGain, 0.);
	h->setHighpassGain(defaultHighpassGain, 0.);

}

//===========================================================================
//		receiveMessage
//
int
fmmActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("Set1AllRatioMode"))
	{
		ifF(f, set1AllRatioMode(f) );
		ifNil( set1AllRatioMode() );
	}

	if (CommandIs("Set1RatioMode"))
	{
		ifF(f, set1RatioMode(f) );
		ifNil( set1RatioMode() );
	}

	if (CommandIs("Set1AllCarFreq"))
	{
		ifFF(z,z2, set1AllCarrierFreq(z, z2) );
		ifF(z, set1AllCarrierFreq(z) );
		return Uncatch();
	}

	if (CommandIs("Set1CarFreq"))
	{
		ifF(z, set1CarrierFreq(z) );
		return Uncatch();
	}

	if (CommandIs("Set1AllModFreq"))
	{
		ifFF(z,z2, set1AllModulatorFreq(z, z2) );
		ifF(z, set1AllModulatorFreq(z) );
		return Uncatch();
	}

	if (CommandIs("Set1ModFreq"))
	{
		ifF(z, set1ModulatorFreq(z) );
		return Uncatch();
	}

	if (CommandIs("Set1AllCMratio"))
	{
		ifFF(z,z2, set1AllCMratio(z, z2) );
		ifF(z, set1AllCMratio(z) );
		return Uncatch();
	}

	if (CommandIs("Set1CMratio"))
	{
		ifF(z, set1CMratio(z) );
		return Uncatch();
	}

	if (CommandIs("Set1AllMCratio"))
	{
		ifFF(z,z2, set1AllMCratio(z, z2) );
		ifF(z, set1AllMCratio(z) );
		return Uncatch();
	}

	if (CommandIs("Set1MCratio"))
	{
		ifF(z, set1MCratio(z) );
		return Uncatch();
	}

	if (CommandIs("Set1AllModIndex"))
	{
		ifFF(z,z2, set1AllModIndex(z, z2) );
		ifF(z, set1AllModIndex(z) );
		return Uncatch();
	}

	if (CommandIs("Set1ModIndex"))
	{
		ifF(z, set1ModIndex(z) );
		return Uncatch();
	}

	if (CommandIs("Set1AllCarFeedback"))
	{
		ifFF(z,z2, set1AllCarFeedback(z, z2) );
		ifF(z, set1AllCarFeedback(z) );
		return Uncatch();
	}

	if (CommandIs("Set1CarFeedback"))
	{
		ifF(z, set1CarFeedback(z) );
		return Uncatch();
	}

	if (CommandIs("Set1AllModFeedback"))
	{
		ifFF(z,z2, set1AllModFeedback(z, z2) );
		ifF(z, set1AllModFeedback(z) );
		return Uncatch();
	}

	if (CommandIs("Set1ModFeedback"))
	{
		ifF(z, set1ModFeedback(z) );
		return Uncatch();
	}

	if (CommandIs("Set2AllRatioMode"))
	{
		ifF(f, set2AllRatioMode(f) );
		ifNil( set2AllRatioMode() );
	}

	if (CommandIs("Set2RatioMode"))
	{
		ifF(f, set2RatioMode(f) );
		ifNil( set2RatioMode() );
	}

	if (CommandIs("Set2AllCarFreq"))
	{
		ifFF(z,z2, set2AllCarrierFreq(z, z2) );
		ifF(z, set2AllCarrierFreq(z) );
		return Uncatch();
	}

	if (CommandIs("Set2CarFreq"))
	{
		ifF(z, set2CarrierFreq(z) );
		return Uncatch();
	}

	if (CommandIs("Set2AllCCratio"))
	{
		ifFF(z,z2, set2AllCCratio(z, z2) );
		ifF(z, set2AllCCratio(z) );
		return Uncatch();
	}

	if (CommandIs("Set2CCratio"))
	{
		ifF(z, set2CCratio(z) );
		return Uncatch();
	}

	if (CommandIs("Set2CCModIndex"))
	{
		ifF(z, set2CCModIndex(z) );
		return Uncatch();
	}

	if (CommandIs("Set2AllModFreq"))
	{
		ifFF(z,z2, set2AllModulatorFreq(z, z2) );
		ifF(z, set2AllModulatorFreq(z) );
		return Uncatch();
	}

	if (CommandIs("Set2ModFreq"))
	{
		ifF(z, set2ModulatorFreq(z) );
		return Uncatch();
	}

	if (CommandIs("Set2AllCMratio"))
	{
		ifFF(z,z2, set2AllCMratio(z, z2) );
		ifF(z, set2AllCMratio(z) );
		return Uncatch();
	}

	if (CommandIs("Set2CMratio"))
	{
		ifF(z, set2CMratio(z) );
		return Uncatch();
	}

	if (CommandIs("Set2AllMCratio"))
	{
		ifFF(z,z2, set2AllMCratio(z, z2) );
		ifF(z, set2AllMCratio(z) );
		return Uncatch();
	}

	if (CommandIs("Set2MCratio"))
	{
		ifF(z, set2MCratio(z) );
		return Uncatch();
	}

	if (CommandIs("Set2AllModIndex"))
	{
		ifFF(z,z2, set2AllModIndex(z, z2) );
		ifF(z, set2AllModIndex(z) );
		return Uncatch();
	}

	if (CommandIs("Set2ModIndex"))
	{
		ifF(z, set2ModIndex(z) );
		return Uncatch();
	}

	if (CommandIs("Set2AllCarFeedback"))
	{
		ifFF(z,z2, set2AllCarFeedback(z, z2) );
		ifF(z, set2AllCarFeedback(z) );
		return Uncatch();
	}

	if (CommandIs("Set2CarFeedback"))
	{
		ifF(z, set2CarFeedback(z) );
		return Uncatch();
	}

	if (CommandIs("Set2AllModFeedback"))
	{
		ifFF(z,z2, set2AllModFeedback(z, z2) );
		ifF(z, set2AllModFeedback(z) );
		return Uncatch();
	}

	if (CommandIs("Set2ModFeedback"))
	{
		ifF(z, set2ModFeedback(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllLowpassGain"))
	{
		ifFF(z,z2, setAllLowpassGain(z, z2) );
		ifF(z, setLowpassGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetLowpassGain"))
	{
		ifF(z, setLowpassGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllHighpassGain"))
	{
		ifFF(z,z2, setAllHighpassGain(z, z2) );
		ifF(z, setHighpassGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetHighpassGain"))
	{
		ifF(z, setHighpassGain(z) );
		return Uncatch();
	}

	return VGeneratorActor::receiveMessage(Message);
}

//===========================================================================
//		setRatioMode
//
//	Set default ratio-frequency mode for this actor.
//
void fmmActor::set1RatioMode(float f)
{
	default1RatioMode = f;
}
void fmmActor::set2RatioMode(float f)
{
	default2RatioMode = f;
}

//===========================================================================
//		setAllRatioMode
//	
//	Call setRatioMode for all of my children, and
//	set default ratio-frequency mode for this actor.
//
void fmmActor::set1AllRatioMode(float f)
{
	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set1RatioMode(f);
	}

	default1RatioMode = f;
}
void fmmActor::set2AllRatioMode(float f)
{
	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set2RatioMode(f);
	}

	default2RatioMode = f;
}

//===========================================================================
//		setCarrierFreq
//
//	Set default carrier freq for this actor.
//
void fmmActor::set1CarrierFreq(float f)
{
	if (! CheckFreq(f)) 
		printf("fmmActor got bogus carrier frequency %f.\n", f );
	else
		default1CarFreq = f;
}
void fmmActor::set2CarrierFreq(float f)
{
	if (! CheckFreq(f)) 
		printf("fmmActor got bogus carrier frequency %f.\n", f );
	else
		default2CarFreq = f;
}

//===========================================================================
//		setAllCarrierFreq
//	
//	Call setCarrierFreq for all of my children, and
//	set default carrier freq for this actor.
//
void fmmActor::set1AllCarrierFreq(float f, float t)
{
	if (!CheckFreq(f)) 
	{
		printf("fmmActor got bogus carrier frequency %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set1CarrierFreq(f, t);
	}

	default1CarFreq = f;
}
void fmmActor::set2AllCarrierFreq(float f, float t)
{
	if (!CheckFreq(f)) 
	{
		printf("fmmActor got bogus carrier frequency %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set2CarrierFreq(f, t);
	}

	default2CarFreq = f;
}

//===========================================================================
//		setModulatorFreq
//
//	Set default modulator freq for this actor.
//
void fmmActor::set1ModulatorFreq(float f)
{
	if (! CheckFreq(f)) 
		printf("fmmActor got bogus modulator frequency %f.\n", f );
	else
		default1ModFreq = f;
}
void fmmActor::set2ModulatorFreq(float f)
{
	if (! CheckFreq(f)) 
		printf("fmmActor got bogus modulator frequency %f.\n", f );
	else
		default2ModFreq = f;
}

//===========================================================================
//		setAllModulatorFreq
//	
//	Call setModulatorFreq for all of my children, and
//	set default modulator freq for this actor.
//
void fmmActor::set1AllModulatorFreq(float f, float t)
{
	if (!CheckFreq(f)) 
	{
		printf("fmmActor got bogus modulator frequency %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set1ModulatorFreq(f, t);
	}

	default1ModFreq = f;
}
void fmmActor::set2AllModulatorFreq(float f, float t)
{
	if (!CheckFreq(f)) 
	{
		printf("fmmActor got bogus modulator frequency %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set2ModulatorFreq(f, t);
	}

	default2ModFreq = f;
}

//===========================================================================
//		setCMratio
//
//	Set default cm ratio for this actor.
//
void fmmActor::set1CMratio(float f)
{
	f = FMM_NiceRatio(f);
	if (! CheckCMratio(f)) 
		printf("fmmActor got bogus cmratio %f.\n", f );
	else
		default1CMratio = f;
}
void fmmActor::set2CMratio(float f)
{
	f = FMM_NiceRatio(f);
	if (! CheckCMratio(f)) 
		printf("fmmActor got bogus cmratio %f.\n", f );
	else
		default2CMratio = f;
}

//===========================================================================
//		setAllCMratio
//
//	Call setCMratio for all of my children, and
//	set default cm ratio for this actor.
//
void fmmActor::set1AllCMratio(float f, float t)
{
	f = FMM_NiceRatio(f);
	if (! CheckCMratio(f)) 
	{
		printf("fmmActor got bogus cmratio %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set1CMratio(f, t);
	}

	default1CMratio = f;
}
void fmmActor::set2AllCMratio(float f, float t)
{
	f = FMM_NiceRatio(f);
	if (! CheckCMratio(f)) 
	{
		printf("fmmActor got bogus cmratio %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set2CMratio(f, t);
	}

	default2CMratio = f;
}

//===========================================================================
//		set2CCratio
//
//	Set default mc ratio for this actor.
//
void fmmActor::set2CCratio(float f)
{
	f = FMM_NiceRatio(f);
	if (! CheckCMratio(f)) 
		printf("fmmActor got bogus ratio %f.\n", f );
	else
		default2CCratio = f;
}
void fmmActor::set2AllCCratio(float f, float t)
{
	f = FMM_NiceRatio(f);
	if (! CheckCMratio(f)) 
	{
		printf("fmmActor got bogus ratio %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set2CCratio(f, t);
	}

	default2CCratio = f;
}

//===========================================================================
//		set2CCModIndex
//
//	Set default mod index between the 2 carriers, for this actor.
//
void fmmActor::set2CCModIndex(float f)
{
	if (! CheckIndex(f)) 
		printf("fmmActor got bogus mod index %f.\n", f );
	else
		default2CCModIndex = f;
}
void fmmActor::set2AllCCModIndex(float f, float t)
{
	if (! CheckIndex(f)) 
	{
		printf("fmmActor got bogus mod index %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set2CCModIndex(f, t);
	}

	default2CCModIndex = f;
}

//===========================================================================
//		setMCratio
//
//	Set default mc ratio for this actor.
//
void fmmActor::set1MCratio(float f)
{
	f = FMM_NiceRatio(f);
	float finv = 1.0f / f;
	if (! CheckCMratio(finv)) 
		printf("fmmActor got bogus mcratio %f.\n", f );
	else
		default1CMratio = finv;
}
void fmmActor::set2MCratio(float f)
{
	f = FMM_NiceRatio(f);
	float finv = 1.0f / f;
	if (! CheckCMratio(finv)) 
		printf("fmmActor got bogus mcratio %f.\n", f );
	else
		default2CMratio = finv;
}

//===========================================================================
//		setAllMCratio
//
//	Call setMCratio for all of my children, and
//	set default mc ratio for this actor.
//
void fmmActor::set1AllMCratio(float f, float t)
{
	f = FMM_NiceRatio(f);
	float finv = 1.0f / f;
	if (! CheckCMratio(finv)) 
	{
		printf("fmmActor got bogus mcratio %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set1MCratio(f, t);
	}

	default1CMratio = finv;
}
void fmmActor::set2AllMCratio(float f, float t)
{
	float finv = 1.0f / f;
	if (! CheckCMratio(finv)) 
	{
		printf("fmmActor got bogus mcratio %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set2MCratio(f, t);
	}

	default2CMratio = finv;
}

//===========================================================================
//		setModIndex
//
//	Set default mod index for this actor.
//
void fmmActor::set1ModIndex(float f)
{
	if (! CheckIndex(f)) 
		printf("fmmActor got bogus mod index %f.\n", f );
	else
		default1ModIndex = f;
}
void fmmActor::set2ModIndex(float f)
{
	if (! CheckIndex(f)) 
		printf("fmmActor got bogus mod index %f.\n", f );
	else
		default2ModIndex = f;
}

//===========================================================================
//		setAllModIndex	
//
//	Call setModIndex for all of my children, and
//	set default mod index for this actor.
//
void fmmActor::set1AllModIndex(float f, float t)
{
	if (! CheckIndex(f)) 
	{
		printf("fmmActor got bogus mod index %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set1ModIndex(f, t);
	}

	default1ModIndex = f;
}
void fmmActor::set2AllModIndex(float f, float t)
{
	if (! CheckIndex(f)) 
	{
		printf("fmmActor got bogus mod index %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set2ModIndex(f, t);
	}

	default2ModIndex = f;
}

//===========================================================================
//		setCarFeedback
//
//	Set default carrier feedback for this actor.
//
void fmmActor::set1CarFeedback(float f)
{
	if (! CheckFeedback(f)) 
		printf("fmmActor got bogus carrier feedback %f.\n", f );
	else
		default1CarFeedback = f;
}
void fmmActor::set2CarFeedback(float f)
{
	if (! CheckFeedback(f)) 
		printf("fmmActor got bogus carrier feedback %f.\n", f );
	else
		default2CarFeedback = f;
}

//===========================================================================
//		setAllCarFeedback	
//
//	Call setCarFeedback for all of my children, and
//	set default carrier feedback for this actor.
//
void fmmActor::set1AllCarFeedback(float f, float t)
{
	if (! CheckFeedback(f)) 
	{
		printf("fmmActor got bogus carrier feedback %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set1CarFeedback(f, t);
	}

	default1CarFeedback = f;
}
void fmmActor::set2AllCarFeedback(float f, float t)
{
	if (! CheckFeedback(f)) 
	{
		printf("fmmActor got bogus carrier feedback %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set2CarFeedback(f, t);
	}

	default2CarFeedback = f;
}

//===========================================================================
//		setModFeedback
//
//	Set default modulator feedback for this actor.
//
void fmmActor::set1ModFeedback(float f)
{
	if (! CheckFeedback(f)) 
		printf("fmmActor got bogus modulator feedback %f.\n", f );
	else
		default1ModFeedback = f;
}
void fmmActor::set2ModFeedback(float f)
{
	if (! CheckFeedback(f)) 
		printf("fmmActor got bogus modulator feedback %f.\n", f );
	else
		default2ModFeedback = f;
}

//===========================================================================
//		setAllModFeedback	
//
//	Call setModFeedback for all of my children, and
//	set default modulator feedback for this actor.
//
void fmmActor::set1AllModFeedback(float f, float t)
{
	if (! CheckFeedback(f)) 
	{
		printf("fmmActor got bogus modulator feedback %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set1ModFeedback(f, t);
	}

	default1ModFeedback = f;
}
void fmmActor::set2AllModFeedback(float f, float t)
{
	if (! CheckFeedback(f)) 
	{
		printf("fmmActor got bogus modulator feedback %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->set2ModFeedback(f, t);
	}

	default2ModFeedback = f;
}


//===========================================================================
//		setLowpassGain, setHighpassGain
//

void fmmActor::setLowpassGain(float f)
{
	if (! CheckFilterGain(f)) 
		printf("fmmActor got bogus filter gain %f.\n", f );
	else
		defaultLowpassGain = f;
}
void fmmActor::setHighpassGain(float f)
{
	if (! CheckFilterGain(f)) 
		printf("fmmActor got bogus filter gain %f.\n", f );
	else
		defaultHighpassGain = f;
}

//===========================================================================
//		setAllLowpassGain, setAllHighpassGain
//
void fmmActor::setAllLowpassGain(float f, float t)
{
	if (! CheckFilterGain(f)) 
	{
		printf("fmmActor got bogus filter gain %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setLowpassGain(f, t);
	}

	defaultLowpassGain = f;
}
void fmmActor::setAllHighpassGain(float f, float t)
{
	if (! CheckFilterGain(f)) 
	{
		printf("fmmActor got bogus filter gain %f.\n", f );
		return;
	}

	HandlerListIterator< fmmHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setHighpassGain(f, t);
	}

	defaultHighpassGain = f;
}

//===========================================================================
//	dump
//
ostream &
fmmActor::dump(ostream &os, int tabs)
{
	VGeneratorActor::dump(os, tabs);

	indent(os, tabs) << "operator 1: " << endl;
	indent(os, tabs) << "Ratio-Frequency Mode: " << default1RatioMode << endl;
	indent(os, tabs) << "Carrier Freq: " << default1CarFreq << endl;
	indent(os, tabs) << "Modulator Freq: " << default1ModFreq << endl;
	indent(os, tabs) << "Carrier-Modulator ratio: " << default1CMratio << endl;
	indent(os, tabs) << "Modulation Index: " << default1ModIndex << endl;
	indent(os, tabs) << "Carrier Feedback: " << default1CarFeedback << endl;
	indent(os, tabs) << "Modulator Feedback: " << default1ModFeedback << endl;

	indent(os, tabs) << "operator 2: " << endl;
	indent(os, tabs) << "Ratio-Frequency Mode: " << default2RatioMode << endl;
	indent(os, tabs) << "Carrier Freq: " << default2CarFreq << endl;
	indent(os, tabs) << "Modulator Freq: " << default2ModFreq << endl;
	indent(os, tabs) << "Carrier-Modulator ratio: " << default2CMratio << endl;
	indent(os, tabs) << "Modulation Index: " << default2ModIndex << endl;
	indent(os, tabs) << "Carrier Feedback: " << default2CarFeedback << endl;
	indent(os, tabs) << "Modulator Feedback: " << default2ModFeedback << endl;

	indent(os, tabs) << "Low-pass gain: " << defaultLowpassGain << endl;
	indent(os, tabs) << "High-pass gain: " << defaultHighpassGain << endl;

	return os;
}

// Round z to to the nearest small-integer ratio (1 through 8).
extern float FMM_NiceRatio(float z)
{
#if 0
	const int cz = 43;
	static float rgz[cz] =
		{ 1./8, 1./7, 1./6, 1./5, 1./4, 2./7, 1./3, 3./8, 2./5, 3./7,
		  1./2, 4./7, 3./5, 5./8, 2./3, 3./4, 5./7, 4./5, 5./6, 6./7, 7./8,
		  1.,
		  8./7, 7./6, 6./5, 5./4, 7./5, 4./3, 3./2, 8./5, 5./3, 7./4,
		  2., 7./3, 5./2, 8./3, 3., 7./2, 4., 5., 6., 7., 8.
		};

	// naive algorithm for now
	float distMin = 1e9;
	int iMin = -1;
	for (int i=0; i<cz; i++)
		{
		float dist = fabs(z - rgz[i]);
		if (dist < distMin)
			{
			distMin = dist;
			iMin = i;
			}
		}
	return (iMin < 0) ? 1. : rgz[iMin];
#else
	return z;
#endif
}
