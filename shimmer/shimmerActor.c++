#include "shimmer.h"

ACTOR_SETUP(shimmerActor, ShimmerActor)

//===========================================================================
//		construction
//
//	Initialize the defaults for shimmer parameters, they will be
//	sent in sendDefaults().
//
shimmerActor::shimmerActor(void) : 
	VGeneratorActor(), 
	defaultNumPartials( 5 ),
	defaultFreq( 60. ),
	defaultWalkspeed( .1 ),
	defaultAvgFreq( 1000. ),
	defaultRange( 3 )
{
	setTypeName("ShimmerActor");
}

//===========================================================================
//		sendDefaults
//
void 
shimmerActor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	shimmerHand * h = (shimmerHand *)p;
	h->setNumPartials(defaultNumPartials);
	h->setFreq(defaultFreq, 0.);
	h->setWalkspeed(defaultWalkspeed, 0.);
	h->setAvgFreq(defaultAvgFreq, 0.);
	h->setRange(defaultRange, 0.);
}

//===========================================================================
//		receiveMessage
//
int
shimmerActor::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetAllNumPartials"))
	{
		ifD(w, setAllNumPartials(w) );
		return Uncatch();
	}
	if (CommandIs("SetNumPartials"))
	{
		ifD(w, setNumPartials(w) );
		return Uncatch();
	}

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

	if (CommandIs("SetAllWalkspeed"))
	{
		ifFF(z,z2, setAllWalkspeed(z, z2) );
		ifF(z, setAllWalkspeed(z) );
		return Uncatch();
	}
	if (CommandIs("SetWalkspeed"))
	{
		ifF(z, setWalkspeed(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllAvgFreq"))
	{
		ifFF(z,z2, setAllAvgFreq(z, z2) );
		ifF(z, setAllAvgFreq(z) );
		return Uncatch();
	}
	if (CommandIs("SetAvgFreq"))
	{
		ifF(z, setAvgFreq(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllRange"))
	{
		ifFF(z,z2, setAllRange(z, z2) );
		ifF(z, setAllRange(z) );
		return Uncatch();
	}
	if (CommandIs("SetRange"))
	{
		ifF(z, setRange(z) );
		return Uncatch();
	}

	return VGeneratorActor::receiveMessage(Message);
}

//===========================================================================
//		setNumPartials
//
//	Set default fundamental frequency for this actor.
//
void
shimmerActor::setNumPartials(int c)
{
	if (! CheckNumPartials(c)) 
		printf("shimmerActor got bogus number of partials %d.\n", c );
	else
		defaultNumPartials = c;
}

//===========================================================================
//		setAllNumPartials
//	
//	Call setNumPartials for all of my children.
//
void
shimmerActor::setAllNumPartials(int c)
{
	if (!CheckNumPartials(c)) 
		{
		printf("shimmerActor got bogus number of partials %d.\n", c );
		return;
		}
	HandlerListIterator< shimmerHand > it;
	for (it = children.begin(); it != children.end(); it++)
		(*it)->setNumPartials(c);
	defaultNumPartials = c;
}

//===========================================================================
//		setFreq
//
//	Set default fundamental frequency for this actor.
//
void
shimmerActor::setFreq(float f)
{
	if (! CheckFreq(f)) 
		printf("shimmerActor got bogus fundamental frequency %f.\n", f );
	else
		defaultFreq = f;
}

//===========================================================================
//		setAllFreq
//	
//	Call setFreq for all of my children.
//
void
shimmerActor::setAllFreq(float f, float t)
{
	if (!CheckFreq(f)) 
		{
		printf("shimmerActor got bogus fundamental frequency %f.\n", f );
		return;
		}
	HandlerListIterator< shimmerHand > it;
	for (it = children.begin(); it != children.end(); it++)
		(*it)->setFreq(f, t);
	defaultFreq = f;
}

//===========================================================================
//		setWalkspeed
//
//	Set default fundamental frequency for this actor.
//
void
shimmerActor::setWalkspeed(float f)
{
	if (! CheckWalkspeed(f)) 
		printf("shimmerActor got bogus walk speed %f.\n", f );
	else
		defaultWalkspeed = f;
}

//===========================================================================
//		setAllWalkspeed
//	
//	Call setWalkspeed for all of my children.
//
void
shimmerActor::setAllWalkspeed(float f, float t)
{
	if (!CheckWalkspeed(f)) 
		{
		printf("shimmerActor got bogus walk speed %f.\n", f );
		return;
		}
	HandlerListIterator< shimmerHand > it;
	for (it = children.begin(); it != children.end(); it++)
		(*it)->setWalkspeed(f, t);
	defaultWalkspeed = f;
}

//===========================================================================
//		setAvgFreq
//
//	Set default fundamental frequency for this actor.
//
void
shimmerActor::setAvgFreq(float f)
{
	if (! CheckAvgfreq(f)) 
		printf("shimmerActor got bogus average frequency %f.\n", f );
	else
		defaultAvgFreq = f;
}

//===========================================================================
//		setAllAvgFreq
//	
//	Call setAvgFreq for all of my children.
//
void
shimmerActor::setAllAvgFreq(float f, float t)
{
	if (!CheckAvgfreq(f)) 
		{
		printf("shimmerActor got bogus average frequency %f.\n", f );
		return;
		}
	HandlerListIterator< shimmerHand > it;
	for (it = children.begin(); it != children.end(); it++)
		(*it)->setAvgFreq(f, t);
	defaultAvgFreq = f;
}

//===========================================================================
//		setRange
//
//	Set default fundamental frequency for this actor.
//
void
shimmerActor::setRange(float f)
{
	if (! CheckRange(f)) 
		printf("shimmerActor got bogus frequency range %f.\n", f );
	else
		defaultRange = f;
}

//===========================================================================
//		setAllRange
//	
//	Call setRange for all of my children.
//
void
shimmerActor::setAllRange(float f, float t)
{
	if (!CheckRange(f)) 
		{
		printf("shimmerActor got bogus frequency range %f.\n", f );
		return;
		}
	HandlerListIterator< shimmerHand > it;
	for (it = children.begin(); it != children.end(); it++)
		(*it)->setRange(f, t);
	defaultRange = f;
}


//===========================================================================
//	dump
//
ostream &
shimmerActor::dump(ostream &os, int tabs)
{
	VGeneratorActor::dump(os, tabs);

	indent(os, tabs) << "NumPartials: " << defaultNumPartials << endl;
	indent(os, tabs) << "Freq: " << defaultFreq << endl;
	indent(os, tabs) << "Walkspeed: " << defaultWalkspeed << endl;
	indent(os, tabs) << "AvgFreq: " << defaultAvgFreq << endl;
	indent(os, tabs) << "Range: " << defaultRange << endl;

	return os;
}
