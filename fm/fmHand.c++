#include "fm.h"

fmHand::fmHand(fmAlg* alg): VHandler(alg)
{
	setTypeName("fmHand"); 
}

int fmHand::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetFreq"))
	{
		ifFF(z,z2, setCarrierFreq(z, z2) );
		ifF(z, setCarrierFreq(z) );
		return Uncatch();
	}

	if (CommandIs("SetCarFreq"))
	{
		ifFF(z,z2, setCarrierFreq(z, z2) );
		ifF(z, setCarrierFreq(z) );
		return Uncatch();
	}

	if (CommandIs("SetModFreq"))
	{
		ifFF(z,z2, setModulatorFreq(z, z2) );
		ifF(z, setModulatorFreq(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetCMratio"))
	{
		ifFF(z,z2, setCMratio(z, z2) );
		ifF(z, setCMratio(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetMCratio"))
	{
		ifFF(z,z2, setMCratio(z, z2) );
		ifF(z, setMCratio(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetModIndex"))
	{
		ifFF(z,z2, setModIndex(z, z2) );
		ifF(z, setModIndex(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetCarFeedback"))
	{
		ifFF(z,z2, setCarFeedback(z, z2) );
		ifF(z, setCarFeedback(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetModFeedback"))
	{
		ifFF(z,z2, setModFeedback(z, z2) );
		ifF(z, setModFeedback(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetRatioMode"))
	{
		ifF( f, setRatioMode(f) );
		ifNil( setRatioMode() );
//		return Uncatch();
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
			if (!CheckFreq(z))
				printf("fmHandler got bogus carrier freq %f.\n", z);
			else
				getAlg()->setCarrierFreq(carFreq = z);
			break;
		case isetModulatorFreq:
			if (!CheckFreq(z))
				printf("fmHandler got bogus modulator freq %f.\n", z);
			else
				getAlg()->setModulatorFreq(modFreq = z);
			break;
		case isetCMratio:
			if (!CheckCMratio(z))
				printf("fmHandler got bogus cmratio %f.\n", z);
			else
				getAlg()->setCMratio(cmRatio = z);
			break;
		case isetModIndex:
			if (!CheckIndex(z))
				printf("fmHandler got bogus mod index %f.\n", z);
			else
				getAlg()->setModIndex(modIndex = z);
			break;
		case isetCarFeedback:
			if (!CheckFeedback(z))
				printf("fmHandler got bogus car feedback value %f.\n", z);
			else
				getAlg()->setCarFeedback(carFeedback = z);
			break;
		case isetModFeedback:
			if (!CheckFeedback(z))
				printf("fmHandler got bogus mod feedback value %f.\n", z);
			else
				getAlg()->setModFeedback(modFeedback = z);
			break;
		default:
			printf("vss error: fmHandler got bogus float-index %d.\n", iParam.i);
			}
		}
	else
		printf("vss error: fmHandler got bogus element-of-float-array-index %d.\n", iParam.i);
}
