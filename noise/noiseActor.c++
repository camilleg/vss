#include "noise.h"

ACTOR_SETUP(noiseActor, NoiseActor)

noiseActor::noiseActor():
	VGeneratorActor(),
	defaultCutoff(500.0),
	defaultOrder(1)
{
	setTypeName("NoiseActor");
}

void noiseActor::sendDefaults(VHandler* p)
{
	VGeneratorActor::sendDefaults(p);
	noiseHand * h = (noiseHand *)p;
	h->setCutoff(defaultCutoff, 0.);
	h->setOrder(defaultOrder);
}

int noiseActor::receiveMessage(const char* Message)
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

void noiseActor::setCutoff(float f)
{
	if (!CheckCutoff(f))
		printf("noiseActor got bogus cutoff frequency %f.\n", f);
	else
		defaultCutoff = f;
}

void noiseActor::setAllCutoff(float f, float t)
{
	if (!CheckCutoff(f))
	{
		printf("noiseActor got bogus cutoff frequency %f.\n", f);
		return;
	}
	HandlerListIterator< noiseHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setCutoff(f, t);
	}
	defaultCutoff = f;
}

void noiseActor::setOrder(float f)
{
	defaultOrder = f;
}

void noiseActor::setAllOrder(float f)
{
	HandlerListIterator< noiseHand > it;
	for (it = children.begin(); it != children.end(); it++)
	{
		(*it)->setOrder(f);
	}
	defaultOrder = f;
}

ostream& noiseActor::dump(ostream &os, int tabs)
{
	VGeneratorActor::dump(os, tabs);
	indent(os, tabs) << "cutoff: " << defaultCutoff << endl;
	indent(os, tabs) << "order: " << defaultOrder << endl;
	return os;
}
