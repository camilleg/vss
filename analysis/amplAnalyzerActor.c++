#include "amplAnalyzer.h"

AmplActor::AmplActor() :
	VGeneratorActor(),
	defaultRate(0.)
{
	setTypeName("AmplAnalyzer");
}

void AmplActor::sendDefaults(VHandler* p)
{
	VGeneratorActor::sendDefaults(p);
	AmplHand* h = (AmplHand*)p;
	h->setRate(defaultRate);
}

void AmplActor::act()
{
	VGeneratorActor::act();
}

int AmplActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);
	if (CommandIs("SetRate"))
		{
		ifF( z, setRate(z) );
		return Uncatch();
		}
	return VGeneratorActor::receiveMessage(Message);
}

void AmplActor::setRate(float z)
{
	if (!CheckRate(z))
		{
		fprintf(stderr, "AmplActor got bogus SetRate %f.\n", z);
		return;
		}
	defaultRate = z;
}

ostream& AmplActor::dump(ostream &os, int tabs)
{
	VGeneratorActor::dump(os, tabs);
	indent(os, tabs) << "rate : " << defaultRate << endl;
	return os;
}
