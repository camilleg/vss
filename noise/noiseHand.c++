//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1996.
//===========================================================================

#include "noise.h"

//===========================================================================
//		construction
//
noiseHand::noiseHand(noiseAlg * alg):
	VHandler( alg )
{ 
	setTypeName("noiseHand"); 
}

//===========================================================================
//		receiveMessage
//
int	
noiseHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);
	
	if (CommandIs("SetCutoff"))
	{
		ifFF(z,z2, setCutoff(z, z2) );
		ifF(z, setCutoff(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetOrder"))
	{
		ifF(z, setOrder(z) );
		return Uncatch();
	}
	
	return VHandler::receiveMessage(Message);
}

void noiseHand::SetAttribute(IParam iParam, float z)
{
	if (iParam.FOnlyI())
		{
		switch (iParam.i)
			{
		case isetCutoff:
			if (!CheckCutoff(z))
				printf("noiseHandler got bogus cutoff frequency %f.\n", z);
			else
				getAlg()->setCutoff(fCutoff = z);
			break;
		default:
			printf("vss error: noiseHandler got bogus float-index %d.\n", iParam.i);
			break;
			}
		}
	else
		{
		printf("vss error: addHandler got bogus element-of-float-array-index %d.\n", iParam.i);
		}
}
