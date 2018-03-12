//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1996.
//===========================================================================

#include "add.h"

ACTOR_SETUP(addActor, AddActor)

//===========================================================================
//		construction
//
//	Initialize the defaults for add parameters, they will be
//	sent in sendDefaults().
//
addActor::addActor(void) : 
	VGeneratorActor(),
	defaultFreq( 200. )
{
	setTypeName("AddActor");

//	default the sound to a sinewave
	for (int i = 0; i < addAlg::cPartial; i++)
	{
		defaultIthAmpl[i] = 0.0;
		defaultIthFD[i] = 0.0;
	}
	defaultIthAmpl[0] = 1.0;
}

//===========================================================================
//		sendDefaults
//
void 
addActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	addHand * h = (addHand *)p;
	h->setFreq(defaultFreq, 0.);
	for (int i=0; i < addAlg::cPartial; i++)
	{
		h->setIthAmpl(i, defaultIthAmpl[i], 0.);
		h->setIthFD(i, defaultIthFD[i], 0.);
	}
}


//===========================================================================
//		receiveMessage
//
int
addActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetAllFreq"))
	{
		ifFF(z,z2, setAllFreq(z, z2) );
		ifF(z, setAllFreq(z) );
		return Uncatch();
	}

	if (CommandIs("SetFreq"))
	{
		ifF(z, setFreq(z) );
		return Uncatch();
	}


	if (CommandIs("SetAmplPartials"))
	{
		ifFloatArray(rgz, cz, setAmplPartials(cz, rgz) );
		return Uncatch();
	}

	if (CommandIs("SetAllIthAmpl"))
	{
		ifDFF(i,z,z2, setAllIthAmpl(i, z, z2) );
		ifDF(i,z, setAllIthAmpl(i, z) );
		return Uncatch();
	}

	if (CommandIs("SetIthAmpl"))
	{
		ifDF(i, z, setIthAmpl(i, z) );
		return Uncatch();
	}

	if (CommandIs("SetFDPartials"))
	{
		ifFloatArray(rgz, cz, setFDPartials(cz, rgz) );
		return Uncatch();
	}

	if (CommandIs("SetAllIthFD"))
	{
		ifDFF(i,z,z2, setAllIthFD(i, z, z2) );
		ifDF(i,z, setAllIthFD(i, z) );
		return Uncatch();
	}

	if (CommandIs("SetIthFD"))
	{
		ifDF(i, z, setIthFD(i, z) );
		return Uncatch();
	}

	return VGeneratorActor::receiveMessage(Message);
}

//===========================================================================
//		setFreq
//
//	Set default fundamental frequency for this actor.
//
void
addActor::setFreq(float f)
{
	if (! CheckFreq(f)) 
		printf("addActor got bogus fundamental frequency %f.\n", f );
	else
		defaultFreq = f;
}

//===========================================================================
//		setAllFreq
//	
//	Call setFreq for all of my children.
//
void
addActor::setAllFreq(float f, float t)
{
	if (!CheckFreq(f)) 
	{
		printf("addActor got bogus fundamental frequency %f.\n", f );
		return;
	}

	HandlerListIterator< addHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setFreq(f, t);
	}

	defaultFreq = f;
}

//===========================================================================
//		setAmplPartials
//
void
addActor::setAmplPartials(int cz, float* rgz)
{
	if (cz > addAlg::cPartial)
	{
		printf("addActor got %d partial amplitudes; the max is %d", cz, addAlg::cPartial);
		return;
	}

	int i;
//	check and fill default amplitudes
	for (i = 0; i < cz; i++)
	{
		if (!CheckAmpl(rgz[i]))
		{ 
			printf("addActor got bogus %dth amplitude %f.\n", i, rgz[i]);
			return;
		}

	}
	for (i = 0; i < cz; i++)
		defaultIthAmpl[i] = rgz[i];

//	fill remaining amplitudes with zeros
	for (i = cz; i < addAlg::cPartial; i++)
		defaultIthAmpl[i] = 0.0;
}

//===========================================================================
//		setIthAmpl
//
//	Set default Ith Amplitude for this actor.
//
void
addActor::setIthAmpl(int i, float f)
{
	if (!CheckAmpl(f)) 
		printf("addActor got bogus amplitude %f.\n", f );
	else
		defaultIthAmpl[i] = f;
}

//===========================================================================
//		setAllIthAmpl	
//
//	Call setIthAmpl for all of my children.
//
void
addActor::setAllIthAmpl(int i, float f, float t)
{
	if (!CheckAmpl(f)) 
	{
		printf("addActor got bogus amplitude %f.\n", f );
		return;
	}

	HandlerListIterator< addHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setIthAmpl(i, f, t);
	}

	defaultIthAmpl[i] = f;
}

//===========================================================================
//		setFDPartials
//
void
addActor::setFDPartials(int cz, float* rgz)
{
	if (cz > addAlg::cPartial)
	{
		printf("addActor got %d partial frequency deviation; the max is %d", cz, addAlg::cPartial);
		return;
	}

	int i;
//	check and fill default FD
	for (i = 0; i < cz; i++)
	{
		if (!CheckFD(rgz[i]))
		{ 
			printf("addActor got bogus %dth frequency deviation %f.\n", i, rgz[i]);
			return;
		}

	}
	for (i = 0; i < cz; i++)
		defaultIthFD[i] = rgz[i];

//	fill remaining FD with 1
	for (i = cz; i < addAlg::cPartial; i++)
		defaultIthFD[i] = 1.0;
}

//===========================================================================
//		setIthFD
//
//	Set default Ith FDitude for this actor.
//
void
addActor::setIthFD(int i, float f)
{
	if (!CheckFD(f)) 
		printf("addActor got bogus frequency deviation %f.\n", f );
	else
		defaultIthFD[i] = f;
}

//===========================================================================
//		setAllIthFD	
//
//	Call setIthFD for all of my children.
//
void
addActor::setAllIthFD(int i, float f, float t)
{
	if (!CheckFD(f)) 
	{
		printf("addActor got bogus frequency deviation %f.\n", f );
		return;
	}

	HandlerListIterator< addHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setIthFD(i, f, t);
	}

	defaultIthFD[i] = f;
}

//===========================================================================
//	dump
//
ostream &
addActor::dump(ostream &os, int tabs)
{
	VGeneratorActor::dump(os, tabs);

	indent(os, tabs) << "Freq: " << defaultFreq << endl;
	for (int i=0; i<addAlg::cPartial; i++)
	indent(os, tabs+1) << "Partial amplitude: " << defaultIthAmpl[i] << endl;

	return os;
}

#if 0
#if defined(VSS_WINDOWS)
#include <windows.h>

extern "C" int WINAPI
mydll_init(HANDLE h, DWORD reason, void *foo)
{
	return 1;
}
#endif
#endif
