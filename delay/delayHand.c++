#include "delay.h"

//===========================================================================
//		construction
//
delayHand::delayHand( delayAlg * alg ):
	VHandler( alg )
{ 
	setTypeName("delayHand"); 
}

//===========================================================================
//		receiveMessage
//
int	
delayHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetDelay"))
	{
		ifFF(z,z2, setDelay(z, z2) );
		ifF(z, setDelay(z) );
		return Uncatch();
	}

	if (CommandIs("SetFeedback"))
	{
		ifFF(z,z2, setFB(z, z2) );
		ifF(z, setFB(z) );
		return Uncatch();
	}

	if (CommandIs("Clear"))
	{
		ifNil( clear() );
		return Catch();
	}

	return VHandler::receiveMessage(Message);
}

void delayHand::SetAttribute(IParam iParam, float z)
{
	if (iParam.FOnlyI())
		{
		switch (iParam.i)
			{
		case isetDelay:
			if (!CheckDelay(z))
				printf("delayHand got bogus delay length %f.\n", z);
			else
				getAlg()->setDelay(delaySeconds = z);
			break;
		case isetFB:
			if (!CheckFB(z))
				printf("delayHand got bogus delay length %f.\n", z);
			else
				getAlg()->setFB(zFB = z);
			break;
		default:
			printf("vss error: delayHandler got bogus float-index %d.\n", iParam.i);
			}
		}
	else
		printf("vss error: delayHandler got bogus element-of-float-array-index %d.\n", iParam.i);
}

void delayHand::clear(void)
{
	getAlg()->clear();
}

void delayHand::actCleanup(void)
{
	// If our source got deleted, clean up after it.
	if (input && !input->FValid())
		{
		input = NULL;
		getAlg()->setSource(NULL);
		}
}
