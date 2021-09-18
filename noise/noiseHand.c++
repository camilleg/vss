#include "noise.h"

noiseHand::noiseHand(noiseAlg* alg): VHandler(alg)
{ 
	setTypeName("noiseHand"); 
}

int	noiseHand::receiveMessage(const char* Message)
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
	if (!iParam.FOnlyI()) {
		printf("noiseHandler got bogus element-of-float-array-index %d.\n", iParam.i);
		return;
	}
	switch (iParam.i) {
	case isetCutoff:
		if (!CheckCutoff(z))
			printf("noiseHandler got bogus cutoff frequency %f.\n", z);
		else
			getAlg()->setCutoff(fCutoff = z);
		break;
	default:
		printf("noiseHandler got bogus float-index %d.\n", iParam.i);
	}
}
