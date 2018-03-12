#include "fm.h"

//===========================================================================
//		construction
//
fmHand::fmHand(fmAlg * alg):
	VHandler( alg )
{ 
	setTypeName("fmHand"); 
}

//===========================================================================
//		receiveMessage
//
int	
fmHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);
	
	if (CommandIs("SetCarFreq"))
	{
		ifFF(z,z2, setCarrierFreq(z, z2) );
		ifF(z, setCarrierFreq(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetCMratio"))
	{
		ifFF(z,z2, setCMratio(z, z2) );
		ifF(z, setCMratio(z) );
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

void fmHand::SetAttribute(IParam iParam, float z)
{
	if (iParam.FOnlyI())
		{
		switch (iParam.i)
			{
		case isetCarrierFreq:
			if (!CheckCarFreq(z))
				printf("fmHandler got bogus carrier freq %f.\n", z);
			else
				getAlg()->setCarrierFreq(carFreq = z);
			break;
		case isetModIndex:
			if (!CheckModIndex(z))
				printf("fmHandler got bogus mod index %f.\n", z);
			else
				getAlg()->setModIndex(modIndex = z);
			break;
		case isetCMratio:
			if (!CheckCMratio(z))
				printf("fmHandler got bogus cmratio %f.\n", z);
			else
				getAlg()->setCMratio(cmRatio = z);
			break;
		default:
			printf("vss error: fmHandler got bogus float-index %d.\n", iParam.i);
			}
		}
	else
		printf("vss error: fmHandler got bogus element-of-float-array-index %d.\n", iParam.i);
}
