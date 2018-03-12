#include "amplAnalyzer.h"

// Constructor.

AmplHand::AmplHand(AmplAlg * alg):
	VHandler(alg),
	zRate(1000)
{
	setTypeName("AmplHand");
}

// receiveMessage

int AmplHand::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetMessageGroup"))
		{
		ifS( sz, setMG(sz) );
		return Uncatch();
		}

	if (CommandIs("SetRate"))
		{
		ifF( z, setRate(z) );
		return Uncatch();
		}

	if (CommandIs("Analyze"))
		{
		ifNil( Analyze() );
		}

	if (CommandIs("SetUserFloat"))
		{
		ifF( z, getAlg()->setUserFloat(z) );
		}

	return VHandler::receiveMessage(Message);
}

// Parameter-setting functions.

void AmplHand::setMG(const char* sz)
{
	getAlg()->setMG(sz);
}

void AmplHand::setRate(float z)
{
	if (!CheckRate(z))
		{
		fprintf(stderr, "AmplHand got bogus SetRate %f.\n", z);
		return;
		}
	zRate = z;
	getAlg()->setRate(zRate);
}

void AmplHand::Analyze(void)
{
	getAlg()->AnalyzeOneShot();
}

void AmplHand::actCleanup(void)
{
	// If our source got deleted, clean up after it.
	if (input && !input->FValid())
		{
		input = NULL;
		getAlg()->setSource(NULL);
		}
}

