#include "process.h"

//===========================================================================
//		construction
//
processHand::processHand( processAlg * alg ):
	VHandler( alg )
{ 
	setTypeName("processHand"); 
}

//===========================================================================
//		receiveMessage
//
int	
processHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);
	
	if (CommandIs("SetModIndex"))
	{
		ifFF(z,z2, setModIndex(z, z2) );
		ifF(z, setModIndex(z) );
		return Uncatch();
	}

	return VHandler::receiveMessage(Message);
}

void processHand::SetAttribute(IParam iParam, float z)
{
	if (iParam.FOnlyI())
		{
		switch (iParam.i)
			{
		case isetModIndex:
			if (!CheckModIndex(z))
				printf("processHand got bogus mod index %f.\n", z);
			else
				getAlg()->setModIndex(modIndex = z);
			break;
		default:
			printf("vss error: processHand got bogus float-index %d.\n", iParam.i);
			}
		}
	else
		printf("vss error: processHand got bogus element-of-float-array-index %d.\n", iParam.i);
}

void processHand::actCleanup(void)
{
	// If our source got deleted, clean up after it.
	if (input && !input->FValid())
		{
		input = NULL;
		getAlg()->setSource(NULL);
		}
}
