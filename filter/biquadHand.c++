#include "biquad.h"

//===========================================================================
//		construction
//
biquadFiltHand::biquadFiltHand( biquadFiltAlg * alg ):
	VHandler( alg )
{ 
	setTypeName("biquadFiltHand"); 
}

//===========================================================================
//		receiveMessage
//
int	
biquadFiltHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);
	
	if (CommandIs("SetFrequency"))
	{
		ifFF(z,z2, setFrequency(z, z2) );
		ifF(z, setFrequency(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetResonance"))
	{
		ifFF(z,z2, setResonance(z, z2) );
		ifF(z, setResonance(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetLowPassGain"))
	{
		ifFF(z,z2, setLPGain(z, z2) );
		ifF(z, setLPGain(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetBandPassGain"))
	{
		ifFF(z,z2, setBPGain(z, z2) );
		ifF(z, setBPGain(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetHighPassGain"))
	{
		ifFF(z,z2, setHPGain(z, z2) );
		ifF(z, setHPGain(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetAllPassGain"))
	{
		ifFF(z,z2, setAPGain(z, z2) );
		ifF(z, setAPGain(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetNotchGain"))
	{
		ifFF(z,z2, setNGain(z, z2) );
		ifF(z, setNGain(z) );
		return Uncatch();
	}

	return VHandler::receiveMessage(Message);
}

void biquadFiltHand::SetAttribute(IParam iParam, float z)
{
	if (iParam.FOnlyI())
		{
		switch (iParam.i)
			{
		case isetFrequency:
			if (!CheckFrequency(z))
				printf("biquadFiltHand got bogus filter frequency %f.\n", z);
			else
				getAlg()->setFrequency(frequency = z);
			break;

		case isetResonance:
			if (!CheckResonance(z))
				printf("biquadFiltHand got bogus filter resonance %f.\n", z);
			else
				getAlg()->setResonance(resonance = z);
			break;

		case isetLPGain:
			if (!CheckGain(z))
				printf("biquadFiltHand got bogus filter gain %f.\n", z);
			else
				getAlg()->setLowpassGain(LPgain = z);
			break;

		case isetBPGain:
			if (!CheckGain(z))
				printf("biquadFiltHand got bogus filter gain %f.\n", z);
			else
				getAlg()->setBandpassGain(BPgain = z);
			break;

		case isetHPGain:
			if (!CheckGain(z))
				printf("biquadFiltHand got bogus filter gain %f.\n", z);
			else
				getAlg()->setHighpassGain(HPgain = z);
			break;

		case isetAPGain:
			if (!CheckGain(z))
				printf("biquadFiltHand got bogus filter gain %f.\n", z);
			else
				getAlg()->setAllpassGain(APgain = z);
			break;

		case isetNGain:
			if (!CheckGain(z))
				printf("biquadFiltHand got bogus filter gain %f.\n", z);
			else
				getAlg()->setNotchGain(Ngain = z);
			break;

		default:
			printf("vss error: biquadFilterHandler got bogus float-index %d.\n", iParam.i);
			}
		}
	else
		printf("vss error: biquadFilterHandler got bogus element-of-float-array-index %d.\n", iParam.i);
}

void biquadFiltHand::actCleanup(void)
{
	// If our source got deleted, clean up after it.
	if (input && !input->FValid())
		{
		input = NULL;
		getAlg()->setSource(NULL);
		}
}
