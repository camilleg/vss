//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1997.
//===========================================================================

#include "ring.h"

//===========================================================================
//		construction
//
ringmodHand::ringmodHand( ringmodAlg * alg ):
	VHandler( alg ),
	modFreq(440.),
	modIndex(1.)
{ 
	setTypeName("ringmodHand"); 
}

//===========================================================================
//		receiveMessage
//
int	
ringmodHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);
	
	if (CommandIs("SetModFreq"))
	{
		ifFF(z,z2, setModFreq(z, z2) );
		ifF(z, setModFreq(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetModIndex"))
	{
		ifFF(z,z2, setModIndex(z, z2) );
		ifF(z, setModIndex(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetModInput"))
	{
		ifF(z, setModInput(z) );
		ifNil(setModInput());
	}
	
	return VHandler::receiveMessage(Message);
}

//===========================================================================
//		setModInput
//
//	Specify a modulation source for ring modulation.
//
void
ringmodHand::setModInput(float hSrc)
{
	VHandler * h = getByHandle( hSrc )->as_handler();
	if (!h || !h->FValid())
		{
		fprintf(stderr, "vss %s error: invalid mod input\n", typeName());
		inputMod = NULL;
		getAlg()->setModSource( NULL );
		return;
		}
	getAlg()->setModSource( (VAlgorithm*)h->getAlg() );
}
void
ringmodHand::setModInput(void)
{
	inputMod = NULL;
	getAlg()->setModSource(NULL);
}

void ringmodHand::SetAttribute(IParam iParam, float z)
{
	if (iParam.FOnlyI())
		{
		switch (iParam.i)
			{
		case isetModFreq:
			if (!CheckModFreq(z))
				printf("ringmodHand got bogus carrier freq %f.\n", z);
			else
				getAlg()->setModFreq(modFreq = z);
			break;
		case isetModIndex:
			if (!CheckModIndex(z))
				printf("ringmodHand got bogus mod index %f.\n", z);
			else
				getAlg()->setModIndex(modIndex = z);
			break;
		default:
			printf("vss error: ringmodHandler got bogus float-index %d.\n", iParam.i);
			}
		}
	else
		printf("vss error: ringmodHandler got bogus element-of-float-array-index %d.\n", iParam.i);
}

void ringmodHand::actCleanup(void)
{
	// If our source got deleted, clean up after it.

	if (input && !input->FValid())
		{
		input = NULL;
		getAlg()->setSource(NULL);
		}
	if (inputMod && !inputMod->FValid())
		{
		inputMod = NULL;
		getAlg()->setModSource(NULL);
		}
}
