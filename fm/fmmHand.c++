#include "fmm.h"

//===========================================================================
//		construction
//
fmmHand::fmmHand(fmmAlg * alg):
	VHandler( alg )
{
	setTypeName("fmmHand");
}

//===========================================================================
//		receiveMessage
//
int
fmmHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("Set1CarFreq"))
	{
		ifFF(z,z2, set1CarrierFreq(z, z2) );
		ifF(z, set1CarrierFreq(z) );
		return Uncatch();
	}

	if (CommandIs("Set1ModFreq"))
	{
		ifFF(z,z2, set1ModulatorFreq(z, z2) );
		ifF(z, set1ModulatorFreq(z) );
		return Uncatch();
	}

	if (CommandIs("Set1CMratio"))
	{
		ifFF(z,z2, set1CMratio(z, z2) );
		ifF(z, set1CMratio(z) );
		return Uncatch();
	}

	if (CommandIs("Set1MCratio"))
	{
		ifFF(z,z2, set1MCratio(z, z2) );
		ifF(z, set1MCratio(z) );
		return Uncatch();
	}

	if (CommandIs("Set1ModIndex"))
	{
		ifFF(z,z2, set1ModIndex(z, z2) );
		ifF(z, set1ModIndex(z) );
		return Uncatch();
	}

	if (CommandIs("Set1CarFeedback"))
	{
		ifFF(z,z2, set1CarFeedback(z, z2) );
		ifF(z, set1CarFeedback(z) );
		return Uncatch();
	}

	if (CommandIs("Set1ModFeedback"))
	{
		ifFF(z,z2, set1ModFeedback(z, z2) );
		ifF(z, set1ModFeedback(z) );
		return Uncatch();
	}

	if (CommandIs("Set1RatioMode"))
	{
		ifF( f, set1RatioMode(f) );
		ifNil( set1RatioMode() );
//		return Uncatch();
	}

	if (CommandIs("Set2CarFreq"))
	{
		ifFF(z,z2, set2CarrierFreq(z, z2) );
		ifF(z, set2CarrierFreq(z) );
		return Uncatch();
	}

	if (CommandIs("Set2CCratio"))
	{
		ifFF(z,z2, set2CCratio(z, z2) );
		ifF(z, set2CCratio(z) );
		return Uncatch();
	}

	if (CommandIs("Set2CCModIndex"))
	{
		ifFF(z,z2, set2CCModIndex(z, z2) );
		ifF(z, set2CCModIndex(z) );
		return Uncatch();
	}

	if (CommandIs("Set2ModFreq"))
	{
		ifFF(z,z2, set2ModulatorFreq(z, z2) );
		ifF(z, set2ModulatorFreq(z) );
		return Uncatch();
	}

	if (CommandIs("Set2CMratio"))
	{
		ifFF(z,z2, set2CMratio(z, z2) );
		ifF(z, set2CMratio(z) );
		return Uncatch();
	}

	if (CommandIs("Set2MCratio"))
	{
		ifFF(z,z2, set2MCratio(z, z2) );
		ifF(z, set2MCratio(z) );
		return Uncatch();
	}

	if (CommandIs("Set2ModIndex"))
	{
		ifFF(z,z2, set2ModIndex(z, z2) );
		ifF(z, set2ModIndex(z) );
		return Uncatch();
	}

	if (CommandIs("Set2CarFeedback"))
	{
		ifFF(z,z2, set2CarFeedback(z, z2) );
		ifF(z, set2CarFeedback(z) );
		return Uncatch();
	}

	if (CommandIs("Set2ModFeedback"))
	{
		ifFF(z,z2, set2ModFeedback(z, z2) );
		ifF(z, set2ModFeedback(z) );
		return Uncatch();
	}

	if (CommandIs("Set2RatioMode"))
	{
		ifF( f, set2RatioMode(f) );
		ifNil( set2RatioMode() );
	}

	if (CommandIs("SetLowpassGain"))
	{
		ifFF(z,z2, setLowpassGain(z, z2) );
		ifF(z, setLowpassGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetHighpassGain"))
	{
		ifFF(z,z2, setHighpassGain(z, z2) );
		ifF(z, setHighpassGain(z) );
		return Uncatch();
	}

	return VHandler::receiveMessage(Message);
}

void fmmHand::SetAttribute(IParam iParam, float z)
{
	if (iParam.FOnlyI())
		{
		switch (iParam.i)
			{
		case iset1CarrierFreq:
			if (!CheckFreq(z))
				printf("fmmHandler got bogus carrier freq %f.\n", z);
			else
				getAlg()->set1CarrierFreq(carFreq1 = z);
			break;
		case iset1ModulatorFreq:
			if (!CheckFreq(z))
				printf("fmmHandler got bogus carrier freq %f.\n", z);
			else
				getAlg()->set1ModulatorFreq(modFreq1 = z);
			break;
		case iset1CMratio:
			z = FMM_NiceRatio(z);
			if (!CheckCMratio(z))
				printf("fmmHandler got bogus cmratio %f.\n", z);
			else
				getAlg()->set1CMratio(cmRatio1 = z);
			break;
		case iset1ModIndex:
			if (!CheckIndex(z))
				printf("fmmHandler got bogus mod index %f.\n", z);
			else
				getAlg()->set1ModIndex(modIndex1 = z);
			break;
		case iset1CarFeedback:
			if (!CheckFeedback(z))
				printf("fmmHandler got bogus car feedback value %f.\n", z);
			else
				getAlg()->set1CarFeedback(carFeedback1 = z);
			break;
		case iset1ModFeedback:
			if (!CheckFeedback(z))
				printf("fmmHandler got bogus mod feedback value %f.\n", z);
			else
				getAlg()->set1ModFeedback(modFeedback1 = z);
			break;
		case iset2CarrierFreq:
			if (!CheckFreq(z))
				printf("fmmHandler got bogus carrier freq %f.\n", z);
			else
				getAlg()->set2CarrierFreq(carFreq2 = z);
			break;
		case iset2CCratio:
			z = FMM_NiceRatio(z);
			if (!CheckCMratio(z))
				printf("fmmHandler got bogus ratio %f.\n", z);
			else
				getAlg()->set2CCratio(ccRatio = z);
			break;
		case iset2CCModIndex:
			if (!CheckIndex(z))
				printf("fmmHandler got bogus mod index %f.\n", z);
			else
				getAlg()->set2CCModIndex(ccmodIndex = z);
			break;
		case iset2ModulatorFreq:
			if (!CheckFreq(z))
				printf("fmmHandler got bogus carrier freq %f.\n", z);
			else
				getAlg()->set2ModulatorFreq(modFreq2 = z);
			break;
		case iset2CMratio:
			z = FMM_NiceRatio(z);
			if (!CheckCMratio(z))
				printf("fmmHandler got bogus cmratio %f.\n", z);
			else
				getAlg()->set2CMratio(cmRatio2 = z);
			break;
		case iset2ModIndex:
			if (!CheckIndex(z))
				printf("fmmHandler got bogus mod index %f.\n", z);
			else
				getAlg()->set2ModIndex(modIndex2 = z);
			break;
		case iset2CarFeedback:
			if (!CheckFeedback(z))
				printf("fmmHandler got bogus car feedback value %f.\n", z);
			else
				getAlg()->set2CarFeedback(carFeedback2 = z);
			break;
		case iset2ModFeedback:
			if (!CheckFeedback(z))
				printf("fmmHandler got bogus mod feedback value %f.\n", z);
			else
				getAlg()->set2ModFeedback(modFeedback2 = z);
			break;
		case isetLowpassGain:
			if (!CheckFilterGain(z))
				printf("fmmHandler got bogus filter gain value %f.\n", z);
			else
				getAlg()->setLowpassGain(lowpassGain = z);
			break;
		case isetHighpassGain:
			if (!CheckFilterGain(z))
				printf("fmmHandler got bogus filter gain value %f.\n", z);
			else
				getAlg()->setHighpassGain(highpassGain = z);
			break;
		default:
			printf("vss error: fmmHandler got bogus float-index %d.\n", iParam.i);
			}
		}
	else
		printf("vss error: fmmHandler got bogus element-of-float-array-index %d.\n", iParam.i);
}
