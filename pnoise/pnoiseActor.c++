#include "pnoise.h"

ACTOR_SETUP(pnoiseActor, PseudoNoiseActor)

pnoiseActor::pnoiseActor():
	VGeneratorActor(),
	defaultCutoff(1000.0),
	defaultModCutoff(1000.0),
	defaultModIndex(0.0)
{
	setTypeName("PseudoNoiseActor");
}

void pnoiseActor::sendDefaults(VHandler* p)
{
	VGeneratorActor::sendDefaults(p);
	pnoiseHand * h = (pnoiseHand *)p;
	h->setCutoff(defaultCutoff, 0.);
	h->setModCutoff(defaultModCutoff, 0.);
	h->setModIndex(defaultModIndex, 0.);
}

int pnoiseActor::receiveMessage(const char* Message)
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

void pnoiseActor::setCutoff(float f)
{
	if (!CheckCutoff(f))
		printf("pnoiseActor got bogus cutoff frequency %f.\n", f);
	else
		defaultCutoff = f;
}

void pnoiseActor::setAllCutoff(float f, float t)
{
	if (!CheckCutoff(f))
	{
		printf("pnoiseActor got bogus cutoff frequency %f.\n", f);
		return;
	}
	HandlerListIterator< pnoiseHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setCutoff(f, t);
	}
	defaultCutoff = f;
}

void pnoiseActor::setModCutoff(float f)
{
	if (!CheckCutoff(f))
		printf("pnoiseActor got bogus cutoff frequency %f.\n", f);
	else
		defaultModCutoff = f;
}

void pnoiseActor::setAllModCutoff(float f, float t)
{
	if (!CheckCutoff(f))
	{
		printf("pnoiseActor got bogus mod cutoff frequency %f.\n", f);
		return;
	}
	HandlerListIterator< pnoiseHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setModCutoff(f, t);
	}
	defaultModCutoff = f;
}

void pnoiseActor::setModIndex(float f)
{
	if (!CheckMod(f))
		printf("pnoiseActor got bogus mod index %f.\n", f);
	else
		defaultModIndex = f;
}

void pnoiseActor::setAllModIndex(float f, float t)
{
	if (!CheckMod(f))
	{
		printf("pnoiseActor got bogus mod index %f.\n", f);
		return;
	}
	HandlerListIterator< pnoiseHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setModIndex(f, t);
	}
	defaultModIndex = f;
}

ostream& pnoiseActor::dump(ostream &os, int tabs)
{
	VGeneratorActor::dump(os, tabs);
	indent(os, tabs) << "cutoff: " << defaultCutoff << endl;
	indent(os, tabs) << "mod cutoff: " << defaultModCutoff << endl;
	indent(os, tabs) << "mod index: " << defaultModIndex << endl;
	return os;
}
