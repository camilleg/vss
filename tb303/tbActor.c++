#include "tb303.h"

ACTOR_SETUP(tb303Actor, Tb303Actor)

//===========================================================================
//		construction
//
//	Initialize the defaults for tb303 parameters, they will be
//	sent in sendDefaults().
//
tb303Actor::tb303Actor(void) :
	VGeneratorActor(),
	defaultFreq( 110. ),
	defaultFilterCutoff( 0. ),
	defaultResonance( 0. ),
	defaultEnvMod( 0. ),
	defaultEnvDecay( 0. )
{
	setTypeName("Tb303Actor");
}

//===========================================================================
//		sendDefaults
//
void
tb303Actor::sendDefaults(VHandler * p)
{
	VGeneratorActor::sendDefaults(p);
	tb303Hand * h = (tb303Hand *)p;
	h->setFreq(defaultFreq, 0.);
	h->setFilterCutoff(defaultFilterCutoff, 0.);
	h->setResonance(defaultResonance, 0.);
	h->setEnvMod(defaultEnvMod, 0.);
	h->setEnvDecay(defaultEnvDecay, 0.);
}

//===========================================================================
//		receiveMessage
//
int tb303Actor::receiveMessage(const char * Message)
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

	if (CommandIs("SetAllFilterCutoff"))
		{
		ifFF(z,z2, setAllFilterCutoff(z, z2) );
		ifF(z, setAllFilterCutoff(z) );
		return Uncatch();
		}
	if (CommandIs("SetFilterCutoff"))
		{
		ifF(z, setFilterCutoff(z) );
		return Uncatch();
		}

	if (CommandIs("SetAllResonance"))
		{
		ifFF(z,z2, setAllResonance(z, z2) );
		ifF(z, setAllResonance(z) );
		return Uncatch();
		}
	if (CommandIs("SetResonance"))
		{
		ifF(z, setResonance(z) );
		return Uncatch();
		}

	if (CommandIs("SetAllEnvMod"))
		{
		ifFF(z,z2, setAllEnvMod(z, z2) );
		ifF(z, setAllEnvMod(z) );
		return Uncatch();
		}
	if (CommandIs("SetEnvMod"))
		{
		ifF(z, setEnvMod(z) );
		return Uncatch();
		}

	if (CommandIs("SetAllEnvDecay"))
		{
		ifFF(z,z2, setAllEnvDecay(z, z2) );
		ifF(z, setAllEnvDecay(z) );
		return Uncatch();
		}
	if (CommandIs("SetEnvDecay"))
		{
		ifF(z, setEnvDecay(z) );
		return Uncatch();
		}

	return VGeneratorActor::receiveMessage(Message);
}

void tb303Actor::setFreq(float z)
{
	if (! CheckFreq(z))
		printf("tb303Actor got bogus frequency %f.\n", z );
	else
		defaultFreq = z;
}

void tb303Actor::setFilterCutoff(float z)
{
	if (! CheckFilterCutoff(z))
		printf("tb303Actor got bogus FilterCutoff %f.\n", z );
	else
		defaultFilterCutoff = z;
}

void tb303Actor::setResonance(float z)
{
	if (! CheckResonance(z))
		printf("tb303Actor got bogus Resonance %f.\n", z );
	else
		defaultResonance = z;
}

void tb303Actor::setEnvMod(float z)
{
	if (! CheckEnvMod(z))
		printf("tb303Actor got bogus EnvMod %f.\n", z );
	else
		defaultEnvMod = z;
}

void tb303Actor::setEnvDecay(float z)
{
	if (! CheckEnvDecay(z))
		printf("tb303Actor got bogus EnvDecay %f.\n", z );
	else
		defaultEnvDecay = z;
}

void tb303Actor::setAllFreq(float z, float t)
{
	if (!CheckFreq(z))
		{
		printf("tb303Actor got bogus frequency %f.\n", z );
		return;
		}
	HandlerListIterator< tb303Hand > it;
    for (it = children.begin(); it != children.end(); it++)
		(*it)->setFreq(z, t);
	defaultFreq = z;
}

void tb303Actor::setAllFilterCutoff(float z, float t)
{
	if (!CheckFilterCutoff(z))
		{
		printf("tb303Actor got bogus FilterCutoff %f.\n", z );
		return;
		}
	HandlerListIterator< tb303Hand > it;
    for (it = children.begin(); it != children.end(); it++)
		(*it)->setFilterCutoff(z, t);
	defaultFreq = z;
}

void tb303Actor::setAllResonance(float z, float t)
{
	if (!CheckResonance(z))
		{
		printf("tb303Actor got bogus Resonance %f.\n", z );
		return;
		}
	HandlerListIterator< tb303Hand > it;
    for (it = children.begin(); it != children.end(); it++)
		(*it)->setResonance(z, t);
	defaultResonance = z;
}

void tb303Actor::setAllEnvMod(float z, float t)
{
	if (!CheckEnvMod(z))
		{
		printf("tb303Actor got bogus EnvMod %f.\n", z );
		return;
		}
	HandlerListIterator< tb303Hand > it;
    for (it = children.begin(); it != children.end(); it++)
		(*it)->setEnvMod(z, t);
	defaultEnvMod = z;
}

void tb303Actor::setAllEnvDecay(float z, float t)
{
	if (!CheckEnvDecay(z))
		{
		printf("tb303Actor got bogus EnvDecay %f.\n", z );
		return;
		}
	HandlerListIterator< tb303Hand > it;
    for (it = children.begin(); it != children.end(); it++)
		(*it)->setEnvDecay(z, t);
	defaultFreq = z;
}

//===========================================================================
//	dump
//
ostream &
tb303Actor::dump(ostream &os, int tabs)
{
	VGeneratorActor::dump(os, tabs);

	indent(os, tabs) << "Freq: " << defaultFreq << endl;

	return os;
}
