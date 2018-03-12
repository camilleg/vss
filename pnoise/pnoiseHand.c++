#include "pnoise.h"

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
	
	if (CommandIs("SetModCutoff"))
	{
		ifFF(z,z2, setModCutoff(z, z2) );
		ifF(z, setModCutoff(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetModIndex"))
	{
		ifFF(z,z2, setModIndex(z, z2) );
		ifF(z, setModIndex(z) );
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
		case isetModCutoff:
			if (!CheckCutoff(z))
				printf("noiseHandler got bogus mod cutoff frequency %f.\n", z);
			else
				getAlg()->setModCutoff(modCutoff = z);
			break;
		case isetModIndex:
			if (!CheckMod(z))
				printf("noiseHandler got bogus mod index %f.\n", z);
			else
				getAlg()->setModIndex(modIndex = z);
			break;
		default:
			printf("vss error: noiseHandler got bogus float-index %d.\n", iParam.i);
			}
		}
	else
		printf("vss error: noiseHandler got bogus element-of-float-array-index %d.\n", iParam.i);
}
