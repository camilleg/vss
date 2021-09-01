#include "tb303.h"

analogHand::analogHand(analogAlg * alg):
	VHandler( alg )
{
	setTypeName("analogHand");
}

int	analogHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetFreq"))
		{
		ifFF(z,z2, setFreq(z, z2) );
		ifF(z, setFreq(z) );
		return Uncatch();
		}

	if (CommandIs("SetFilterCutoff"))
		{
		ifFF(z,z2, setFilterCutoff(z, z2) );
		ifF(z, setFilterCutoff(z) );
		return Uncatch();
		}

	if (CommandIs("SetResonance"))
		{
		ifFF(z,z2, setResonance(z, z2) );
		ifF(z, setResonance(z) );
		return Uncatch();
		}

	if (CommandIs("SetEnvMod"))
		{
		ifFF(z,z2, setEnvMod(z, z2) );
		ifF(z, setEnvMod(z) );
		return Uncatch();
		}

	if (CommandIs("SetEnvDecay"))
		{
		ifFF(z,z2, setEnvDecay(z, z2) );
		ifF(z, setEnvDecay(z) );
		return Uncatch();
		}

	return VHandler::receiveMessage(Message);
}

void analogHand::SetAttribute(IParam iParam, float z)
{
	if (iParam.FOnlyI())
		{
		switch (iParam.i)
			{
		case isetFreq:
			if (!CheckFreq(z))
				printf("analogActor Handler got bogus frequency %f.\n", z);
			else
				getAlg()->setFreq(zFreq=z);
			break;
		case isetFilterCutoff:
			if (!CheckFilterCutoff(z))
				printf("analogActor Handler got bogus FilterCutoff %f.\n", z);
			else
				getAlg()->setFilterCutoff(zFilterCutoff=z);
			break;
		case isetResonance:
			if (!CheckResonance(z))
				printf("analogActor Handler got bogus Resonance %f.\n", z);
			else
				getAlg()->setResonance(zResonance=z);
			break;
		case isetEnvMod:
			if (!CheckEnvMod(z))
				printf("analogActor Handler got bogus EnvMod %f.\n", z);
			else
				getAlg()->setEnvMod(zEnvMod=z);
			break;
		case isetEnvDecay:
			if (!CheckEnvDecay(z))
				printf("analogActor Handler got bogus EnvDecay %f.\n", z);
			else
				getAlg()->setEnvDecay(zEnvDecay=z);
			break;
		default:
			printf("vss error: analogActor Handler got bogus float-index %d.\n", iParam.i);
			}
		}
	else
		printf("vss error: analogActor Handler got bogus float-index %d.\n", iParam.i);
}
