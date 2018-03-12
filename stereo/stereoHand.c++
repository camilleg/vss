#include "stereo.h"

//===========================================================================
//		construction
//
stereoHand::stereoHand( stereoAlg * alg ):
	VHandler( alg )
{ 
	setTypeName("stereoHand"); 
}

//===========================================================================
//		receiveMessage
//
int	
stereoHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);
	
	if (CommandIs("SetPan"))
	{
		ifFF(z,z2, setPan(z, z2) );
		ifF(z, setPan(z) );
		return Uncatch();
	}

	return VHandler::receiveMessage(Message);
}

//===========================================================================
//		setPan
//
void
stereoHand::SetAttribute(IParam iParam, float z)
{
	if (iParam.FOnlyI())
		{
		switch (iParam.i)
			{
		case isetPan:
			if (!CheckPan(z))
				printf("stereoHand got bogus pan value %f.\n", z);
			else
				getAlg()->setPan(pan = z);
			break;
		default:
			printf("vss error: stereoHandler got bogus float-index %d.\n", iParam.i);
			}
		}
	else
		printf("vss error: stereoHandler got bogus element-of-float-array-index %d.\n", iParam.i);
}

void stereoHand::actCleanup(void)
{
	// If our source got deleted, clean up after it.
	if (input && !input->FValid())
		{
		input = NULL;
		getAlg()->setSource(NULL);
		}
}
