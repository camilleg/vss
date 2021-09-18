#include "pnoise.h"

pnoiseHand::pnoiseHand(pnoiseAlg* alg): VHandler(alg)
{
	setTypeName("pnoiseHand");
}

int	pnoiseHand::receiveMessage(const char* Message)
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

void pnoiseHand::SetAttribute(IParam iParam, float z)
{
	if (!iParam.FOnlyI()) {
		printf("pnoiseHandler got bogus element-of-float-array-index %d.\n", iParam.i);
		return;
	}
	switch (iParam.i) {
	case isetCutoff:
		if (!CheckCutoff(z))
			printf("pnoiseHandler got bogus cutoff frequency %f.\n", z);
		else
			getAlg()->setCutoff(fCutoff = z);
		break;
	case isetModCutoff:
		if (!CheckCutoff(z))
			printf("pnoiseHandler got bogus mod cutoff frequency %f.\n", z);
		else
			getAlg()->setModCutoff(modCutoff = z);
		break;
	case isetModIndex:
		if (!CheckMod(z))
			printf("pnoiseHandler got bogus mod index %f.\n", z);
		else
			getAlg()->setModIndex(modIndex = z);
		break;
	default:
		printf("pnoiseHandler got bogus float-index %d.\n", iParam.i);
	}
}
