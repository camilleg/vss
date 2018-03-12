#include "add.h"

addHand::addHand(addAlg * alg) : VHandler(alg)
{
	ZeroFloats(partials, addAlg::cPartial);
	ZeroFloats(fd, addAlg::cPartial);
	ZeroFloats(allfreq, addAlg::cPartial);
	setTypeName("addHand");
}

int	addHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetFreq"))
		{
		ifFF(z, t, setFreq(z, t));
		ifF(z, setFreq(z));
		return Uncatch();
		}

	if (CommandIs("SetAmplPartials"))
		{
		ifFloatArrayFloat(rgz, cz, t, setAmplPartials(cz, rgz, t));
		ifFloatArray(rgz, cz, setAmplPartials(cz, rgz));
		return Uncatch();
		}

	if (CommandIs("SetIthAmpl"))
		{
		ifDFF(i, z, t, setIthAmpl(i, z, t));
		ifDF(i, z, setIthAmpl(i, z));
		return Uncatch();
		}

	if (CommandIs("SetFDPartials"))
		{
		ifFloatArrayFloat(rgz, cz, t, setFDPartials(cz, rgz, t));
		ifFloatArray(rgz, cz, setFDPartials(cz, rgz));
		return Uncatch();
		}

	if (CommandIs("SetIthFD"))
		{
		ifDFF(i, z, t, setIthFD(i, z, t));
		ifDF(i, z, setIthFD(i, z));
		return Uncatch();
		}

	if (CommandIs("SetFreqPartials"))
		{
		ifFloatArrayFloat(rgz, cz, t, setFreqPartials(cz, rgz, t));
		ifFloatArray(rgz, cz, setFreqPartials(cz, rgz));
		return Uncatch();
		}

	if (CommandIs("SetIthFreq"))
		{
		ifDFF(i, z, t, setIthFreq(i, z, t));
		ifDF(i, z, setIthFreq(i, z));
		return Uncatch();
		}

	return VHandler::receiveMessage(Message);
}

// Set a parameter immediately.
//
// VModulators call these.
// modulate() calls them directly, if it had a zero modulation time.
//

void addHand::SetAttribute(IParam iParam, float z)
{
	if (iParam.FOnlyI())
		{
		switch (iParam.i)
			{
		case isetFreq:
			if (!CheckFreq(z))
				printf("vss error: addHandler got bogus freq %f.\n", z);
			else
				{
				getAlg()->setFreq(freq = z);
				// Also have to overwrite allfreq[], like addAlg.c++.
				// This kind of design is yucky, where a single float param
				// and an array both modify the same thing.
				for (int i=0; i<addAlg::cPartial; i++)
					allfreq[i] = z * (i+1);
				}
			break;

		default:
			printf("vss error: addHandler got bogus float-index %d.\n", iParam.i);
			break;
			}
		}
	else
		{
		int iz = iParam.Iz();
		switch (iParam.i)
			{
		case isetIthAmpl:
			if (!CheckAmpl(z))
				printf("vss error: addHandler got bogus amplitude %f.\n", z);
			else
				getAlg()->setIthAmpl(iz, partials[iz] = z);
			break;
		case isetIthFD:
			if (!CheckFD(z))
				printf("vss error: addHandler setIthFD got bogus frequency deviation %f.\n", z);
			else
				getAlg()->setIthFD(iz, fd[iz] = z);
			break;
		case isetIthFreq:
			if (!CheckFreq(z))
				printf("vss error: addHandler got bogus frequency %f.\n", z);
			else
				getAlg()->setIthFreq(iz, allfreq[iz] = z);
			break;

		default:
			printf("vss error: addHandler got bogus element-of-floatarray-index %d.\n", iParam.i);
			break;
			}
		}
}

void addHand::SetAttribute(IParam iParam, float* rgz)
{
	int cz = iParam.Cz();
	switch (iParam.i)
		{
	case isetAmplPartials:
	//	getAlg()->setAmplPartials(rgz); // This fails: rgz is too short!
		FloatCopy(partials, rgz, cz);
		getAlg()->setAmplPartials(partials);
		break;
	case isetFDPartials:
		FloatCopy(fd, rgz, cz);
		getAlg()->setFDPartials(fd);
		break;
	case isetFreqPartials:
		FloatCopy(allfreq, rgz, cz);
		getAlg()->setFreqPartials(allfreq);
		break;
	default:
		printf("vss error: addHandler got bogus floatarray-index %d.\n", iParam.i);
		break;
		}
}
