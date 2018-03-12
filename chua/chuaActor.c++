#include "chua.h"

ACTOR_SETUP(chuaActor, ChuaActor)

//===========================================================================
//		construction
//
//	Initialize the defaults for chua parameters, they will be
//	sent in sendDefaults().
//

int chuaActor::initialized = 0;

chuaActor::chuaActor(void) : 
	VGeneratorActor(),
	defaultR( 1523 ),
	defaultR0( 11.2 ),
	defaultC1( 4.7e-2 ),
	defaultC2( 1.0 ),
	defaultL( 0.00574 ),
	defaultBPH1( -6. ),
	defaultBPH2( 6. ),
	defaultBP1( 1. ),
	defaultBP2( .5966 ),
	defaultM0( -.5 ),
	defaultM1( -.8 ),
	defaultM2( 4. * 1.0e3 ),
	defaultM3( 4. * 1.0e3 )
{
	setTypeName("ChuaActor");
}

//===========================================================================
//		sendDefaults
//
void 
chuaActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	chuaHand * h = (chuaHand *) p;

	h->setR( defaultR , 0. );
	h->setR0( defaultR0 , 0. );
	h->setC1( defaultC1 , 0. );
	h->setC2( defaultC2 , 0. );
	h->setL( defaultL , 0. );
	h->setBPH1( defaultBPH1 , 0. );
	h->setBPH2( defaultBPH2 , 0. );
	h->setBP1( defaultBP1 , 0. );
	h->setBP2( defaultBP2 , 0. );
	h->setM0( defaultM0 , 0. );
	h->setM1( defaultM1 , 0. );
	h->setM2( defaultM2 , 0. );
	h->setM3( defaultM3 , 0. );
}

//===========================================================================
//		receiveMessage
//
int
chuaActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("ResetAllChuaState"))
	{
		resetAllChuaState();
		return Uncatch();
	}

	if (CommandIs("SetAllChuaR"))
	{
		ifFF(z, z2, setAllR(z, z2) );
		ifF(z, setAllR(z) );
		return Uncatch();
	}

	if (CommandIs("SetChuaR"))
	{
		ifF(z, setR(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllChuaR0"))
	{
		ifFF(z, z2, setAllR0(z, z2) );
		ifF(z, setAllR0(z) );
		return Uncatch();
	}

	if (CommandIs("SetChuaR0"))
	{
		ifF(z, setR0(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllChuaC1"))
	{
		ifFF(z, z2, setAllC1(z, z2) );
		ifF(z, setAllC1(z) );
		return Uncatch();
	}

	if (CommandIs("SetChuaC1"))
	{
		ifF(z, setC1(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllChuaC2"))
	{
		ifFF(z, z2, setAllC2(z, z2) );
		ifF(z, setAllC2(z) );
		return Uncatch();
	}

	if (CommandIs("SetChuaC2"))
	{
		ifF(z, setC2(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllChuaL"))
	{
		ifFF(z, z2, setAllL(z, z2) );
		ifF(z, setAllL(z) );
		return Uncatch();
	}

	if (CommandIs("SetChuaL"))
	{
		ifF(z, setL(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllChuaBPH1"))
	{
		ifFF(z, z2, setAllBPH1(z, z2) );
		ifF(z, setAllBPH1(z) );
		return Uncatch();
	}

	if (CommandIs("SetChuaBPH1"))
	{
		ifF(z, setBPH1(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllChuaBPH2"))
	{
		ifFF(z, z2, setAllBPH2(z, z2) );
		ifF(z, setAllBPH2(z) );
		return Uncatch();
	}

	if (CommandIs("SetChuaBPH2"))
	{
		ifF(z, setBPH2(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllChuaBP1"))
	{
		ifFF(z, z2, setAllBP1(z, z2) );
		ifF(z, setAllBP1(z) );
		return Uncatch();
	}

	if (CommandIs("SetChuaBP1"))
	{
		ifF(z, setBP1(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllChuaBP2"))
	{
		ifFF(z, z2, setAllBP2(z, z2) );
		ifF(z, setAllBP2(z) );
		return Uncatch();
	}

	if (CommandIs("SetChuaBP2"))
	{
		ifF(z, setBP2(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllChuaM0"))
	{
		ifFF(z, z2, setAllM0(z, z2) );
		ifF(z, setAllM0(z) );
		return Uncatch();
	}

	if (CommandIs("SetChuaM0"))
	{
		ifF(z, setM0(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllChuaM1"))
	{
		ifFF(z, z2, setAllM1(z, z2) );
		ifF(z, setAllM1(z) );
		return Uncatch();
	}

	if (CommandIs("SetChuaM1"))
	{
		ifF(z, setM1(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllChuaM2"))
	{
		ifFF(z, z2, setAllM2(z, z2) );
		ifF(z, setAllM2(z) );
		return Uncatch();
	}

	if (CommandIs("SetChuaM2"))
	{
		ifF(z, setM2(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllChuaM3"))
	{
		ifFF(z, z2, setAllM3(z, z2) );
		ifF(z, setAllM3(z) );
		return Uncatch();
	}

	if (CommandIs("SetChuaM3"))
	{
		ifF(z, setM3(z) );
		return Uncatch();
	}

	return VGeneratorActor::receiveMessage(Message);
}


//===========================================================================
//              resetAllChuaState
//
void
chuaActor::resetAllChuaState(void)
{
	HandlerListIterator< chuaHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->resetChuaState();
	}
}

//===========================================================================
//              setR
//
void
chuaActor::setR(float f)
{
//	if (! CheckMIDIVal(f))
//		printf("chuaActor got bogus MIDI-like value %f\n" );
//	else
		defaultR = f;
}

//===========================================================================
//              setAllR
//
void
chuaActor::setAllR(float f, float t)
{
	HandlerListIterator< chuaHand > it;
    for (it = children.begin(); it != children.end(); it++)
    {
		(*it)->setR(f, t);
	}

	defaultR = f;
}

//===========================================================================
//              setR0
//
void
chuaActor::setR0(float f)
{
//	if (! CheckMIDIVal(f))
//		printf("chuaActor got bogus MIDI-like value %f\n" );
//	else
		defaultR0 = f;
}

//===========================================================================
//              setAllR0
//
void
chuaActor::setAllR0(float f, float t)
{
    HandlerListIterator< chuaHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setR0(f, t);
	}

	defaultR0 = f;
}

//===========================================================================
//              setC1
//
void
chuaActor::setC1(float f)
{
//	if (! CheckMIDIVal(f))
//		printf("chuaActor got bogus MIDI-like value %f\n" );
//	else
		defaultC1 = f;
}

//===========================================================================
//              setAllC1
//
void
chuaActor::setAllC1(float f, float t)
{
    HandlerListIterator< chuaHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setC1(f * 1.0e-7, t);
	}

	defaultC1 = f * 1.0e-7;
}

//===========================================================================
//              setC2
//
void
chuaActor::setC2(float f)
{
//	if (! CheckMIDIVal(f))
//		printf("chuaActor got bogus MIDI-like value %f\n" );
//	else
		defaultC2 = f * 1.0e-7;
}

//===========================================================================
//              setAllC2
//
void
chuaActor::setAllC2(float f, float t)
{
    HandlerListIterator< chuaHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setC2(f * 1.0e-7, t);
	}

	defaultC2 = f * 1.0e-7;
}

//===========================================================================
//              setL
//
void
chuaActor::setL(float f)
{
//	if (! CheckMIDIVal(f))
//		printf("chuaActor got bogus MIDI-like value %f\n" );
//	else
		defaultL = f;
}

//===========================================================================
//              setAllL
//
void
chuaActor::setAllL(float f, float t)
{
    HandlerListIterator< chuaHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setL(f, t);
	}

	defaultL = f;
}

//===========================================================================
//              setBPH1
//
void
chuaActor::setBPH1(float f)
{
//	if (! CheckMIDIVal(f))
//		printf("chuaActor got bogus MIDI-like value %f\n" );
//	else
		defaultBPH1 = f;
}

//===========================================================================
//              setAllBPH1
//
void
chuaActor::setAllBPH1(float f, float t)
{
    HandlerListIterator< chuaHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setBPH1(f, t);
	}

	defaultBPH1 = f;
}

//===========================================================================
//              setBPH2
//
void
chuaActor::setBPH2(float f)
{
//	if (! CheckMIDIVal(f))
//		printf("chuaActor got bogus MIDI-like value %f\n" );
//	else
		defaultBPH2 = f;
}

//===========================================================================
//              setAllBPH2
//
void
chuaActor::setAllBPH2(float f, float t)
{
    HandlerListIterator< chuaHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setBPH2(f, t);
	}

	defaultBPH2 = f;
}

//===========================================================================
//              setBP1
//
void
chuaActor::setBP1(float f)
{
//	if (! CheckMIDIVal(f))
//		printf("chuaActor got bogus MIDI-like value %f\n" );
//	else
		defaultBP1 = f;
}

//===========================================================================
//              setAllBP1
//
void
chuaActor::setAllBP1(float f, float t)
{
    HandlerListIterator< chuaHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setBP1(f, t);
	}

	defaultBP1 = f;
}

//===========================================================================
//              setBP2
//
void
chuaActor::setBP2(float f)
{
//	if (! CheckMIDIVal(f))
//		printf("chuaActor got bogus MIDI-like value %f\n" );
//	else
		defaultBP2 = f;
}

//===========================================================================
//              setAllBP2
//
void
chuaActor::setAllBP2(float f, float t)
{
    HandlerListIterator< chuaHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setBP2(f, t);
	}

	defaultBP2 = f;
}

//===========================================================================
//              setM0
//
void
chuaActor::setM0(float f)
{
//	if (! CheckMIDIVal(f))
//		printf("chuaActor got bogus MIDI-like value %f\n" );
//	else
		defaultM0 = f * 1.0e-3;
}

//===========================================================================
//              setAllM0
//
void
chuaActor::setAllM0(float f, float t)
{
    HandlerListIterator< chuaHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setM0(f * 1.0e-3, t);
	}

	defaultM0 = f * 1.0e-3;
}

//===========================================================================
//              setM1
//
void
chuaActor::setM1(float f)
{
//	if (! CheckMIDIVal(f))
//		printf("chuaActor got bogus MIDI-like value %f\n" );
//	else
		defaultM1 = f * 1.0e-3;
}

//===========================================================================
//              setAllM1
//
void
chuaActor::setAllM1(float f, float t)
{
    HandlerListIterator< chuaHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setM1(f * 1.0e-3, t);
	}

	defaultM1 = f * 1.0e-3;
}

//===========================================================================
//              setM2
//
void
chuaActor::setM2(float f)
{
//	if (! CheckMIDIVal(f))
//		printf("chuaActor got bogus MIDI-like value %f\n" );
//	else
		defaultM2 = f * 1.0e-3;
}

//===========================================================================
//              setAllM2
//
void
chuaActor::setAllM2(float f, float t)
{
    HandlerListIterator< chuaHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setM2(f * 1.0e-3, t);
	}

	defaultM2 = f * 1.0e-3;
}

//===========================================================================
//              setM3
//
void
chuaActor::setM3(float f)
{
//	if (! CheckMIDIVal(f))
//		printf("chuaActor got bogus MIDI-like value %f\n" );
//	else
		defaultM3 = f * 1.0e-3;
}

//===========================================================================
//              setAllM3
//
void
chuaActor::setAllM3(float f, float t)
{
    HandlerListIterator< chuaHand > it;
    for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setM3(f * 1.0e-3, t);
	}

	defaultM3 = f * 1.0e-3;
}

