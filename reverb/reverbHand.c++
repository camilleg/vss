#include "reverb.h"
#include <fstream>			// For preset file

//===========================================================================
//		construction
//
reverbHand::reverbHand( reverbAlg * alg ):
	VHandler( alg )
{ 
	setTypeName("reverbHand");
}

//===========================================================================
//		receiveMessage
//
int	
reverbHand::receiveMessage(const char * Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetIdle"))
	{
		ifF(z, setIdle(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetPreset"))
	{
		ifS( s, setPreset(s) );
		return Uncatch();
	}
	
	if (CommandIs("SetPresetFile"))
	{
		ifS( s, setPresetFile(s) );
		return Uncatch();
	}
	
	if (CommandIs("SetPresetNum"))
	{
		ifD( z, setPresetNum(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetMix"))
	{
		ifFF(z,z2, setRevMix(z, z2) );
		ifF(z, setRevMix(z) );
		return Uncatch();
	}

	if (CommandIs("SetGain"))
	{
		ifFF(z,z2, setRevGain(z, z2) );
		ifF(z, setRevGain(z) );
		return Uncatch();
	}

	if (CommandIs("SetPole"))
	{
		ifFF(z,z2, setRevPole(z, z2) );
		ifF(z, setRevPole(z) );
		return Uncatch();
	}

	if (CommandIs("SetBright"))
	{
		ifFF(z,z2, setRevBright(z, z2) );
		ifF(z, setRevBright(z) );
		return Uncatch();
	}

	if (CommandIs("SetEarlyRefNum"))
	{
		ifD(z, setEarlyRefNum(z) );
		return Uncatch();
	}

	if (CommandIs("SetEarlyRefDelay"))
	{
		ifFloatArray(rgz, cz, setEarlyRefDelay(cz, rgz) );
		return Uncatch();
	}

	if (CommandIs("SetEarlyRefCoeff"))
	{
		ifFloatArray(rgz, cz, setEarlyRefCoeff(cz, rgz) );
		return Uncatch();
	}

	if (CommandIs("SetCombNum"))
	{
		ifD(z, setCombNum(z) );
		return Uncatch();
	}

	if (CommandIs("SetTime"))
	{
		ifFF(z,z2, setRevTime(z, z2) );
		ifF(z, setRevTime(z) );
		return Uncatch();
	}
	
	if (CommandIs("SetDampRatio"))
	{
		ifFF(z,z2, setDampRatio(z, z2) );
		ifF(z, setDampRatio(z) );
		return Uncatch();
	}

	if (CommandIs("SetCombDelay"))
	{
		ifFloatArray(rgz, cz, setCombDelay(cz, rgz) );
		return Uncatch();
	}

	if (CommandIs("SetAllPassNum"))
	{
		ifD(z, setAllPassNum(z) );
		return Uncatch();
	}

	if (CommandIs("SetAllPassDelay"))
	{
		ifFloatArray(rgz, cz, setAllPassDelay(cz, rgz) );
		return Uncatch();
	}

	return VHandler::receiveMessage(Message);
}

//===========================================================================
//		setIdle: skip the calculation
//
void reverbHand::setIdle(float z)
{
	getAlg()->setIdle(z ? 1 : 0);
}

//===========================================================================
//		Preset
//
void reverbHand::setPreset(char * pre)
{
	int temp;
	if (!strcmp(pre,"Default")) temp = DEFAULT;
	else if (!strcmp(pre,"SmallRoom")) temp = SMALLROOM;
	else if (!strcmp(pre,"Hall")) temp = HALL;
	else if (!strcmp(pre,"Echo")) temp = ECHO1;
	else if (!strcmp(pre,"Canyon")) temp = CANYON;
	else
	{
		printf("reverbHand got bogus preset %s.\n",pre);
		return;
	}
	getAlg()->setPreset(temp);
}

void reverbHand::setPresetNum(int pre)
{
	if ( pre < 0 || pre >= MAXPRE )
	{
		printf("reverbHand got bogus preset number %d. ",pre);
		printf("Valid range is [0,4].\n");
		return;
	}
	getAlg()->setPreset(pre);
}

void reverbHand::setPresetFile(char * prefile)
{
	ifstream inFile(prefile, ios::in);
	if (!inFile)
	{
		printf("Error in opening preset file %s.\n",prefile);
		return;
	}
        static float f[50];
	int num,i;
	int error = 0;
	printf("reverbHand loading preset file %s ... ",prefile);
	for (;;)
	{
		inFile >> f[0]; if (!CheckRevMix(f[0])) break;
		inFile >> f[1]; if (!CheckRevGain(f[1])) break;
		inFile >> f[2]; if (!CheckRevTime(f[2])) break;
		inFile >> f[3]; if (!CheckRevBright(f[3])) break;
		inFile >> f[4]; if (!CheckDampRatio(f[4])) break;
		inFile >> f[5]; if (!CheckRevPole(f[5])) break;
		inFile >> f[6];
		inFile >> f[7];
		inFile >> f[8];
		inFile >> f[9];

		inFile >> f[10]; if (!CheckEarlyRefNum(int(f[10]))) break;
		num = (int)f[10]+11;
		for (i=11;i<num;i++)
		{
			inFile >> f[i];
			if (!CheckEarlyRefDelay(f[i])) {  error = 1; break; }
		}
		if (error) break;
		for (i=num;i<21;i++) inFile >> f[i];

		num += 10;
		for (i=21;i<num;i++)
		{
			inFile >> f[i];
			if (!CheckEarlyRefCoeff(f[i])) {  error = 1; break; }
		}
		if (error) break;
		for (i=num;i<30;i++) inFile >> f[i];

		inFile >> f[30]; if (!CheckCombNum(int(f[30]))) break;
		num = (int)f[30]+31;
		for (i=31;i<num;i++)
		{
			inFile >> f[i];
			if (!CheckCombDelay(f[i])) {  error = 1; break; }
		}
		if (error) break;
		for (i=num;i<40;i++) inFile >> f[i];
	
		inFile >> f[40]; if (!CheckAllPassNum(int(f[40]))) break;
		num = (int)f[40]+41;
			for (i=41;i<num;i++)
		{
			inFile >> f[i];
			if (!CheckAllPassDelay(f[i])) {  error = 1; break; }
		}
		if (error) break;

//		for (i=0;i<50;i++) printf("%d %f\n",i,f[i]);
		printf("Preset data loaded.\n");
		getAlg()->setPara(f);	
		inFile.close();
		return;
	}
	printf("Error in preset data.\n\t");
	printf("Please check documentation for bounds.\n");
	inFile.close();
}

//===========================================================================
//		Global parameters
//
void reverbHand::SetAttribute(IParam iParam, float z)
{
	if (iParam.FOnlyI())
		{
		switch (iParam.i)
			{
		case isetRevMix:
			if (!CheckRevMix(z))
				{
				printf("reverbHand got bogus rev mix %f. ", z);
				printf("Valid range is [0,1].\n");
				}
			else
				getAlg()->setRevMix(revMix = z);
			break;
		case isetRevGain:
			if (!CheckRevGain(z))
				{
				printf("reverbHand got bogus reverberation gain %f. ", z);
				printf("Valid range is (0,10].\n");
				}
			else
				getAlg()->setRevGain(revGain = z);
			break;
		case isetRevTime:
			if (!CheckRevTime(z))
			{
				printf("reverbHand got bogus reverberation time %f. ", z);
				printf("Valid range is (0,1].\n");
			}
			else
				getAlg()->setT60(t60 = z);
			break;
		case isetRevBright:
			if (!CheckRevBright(z))
				{
				printf("reverbHand got bogus reverberation brightness %f. ", z);
				printf("Valid range is (0,1].\n");
				}
			else
				getAlg()->setBW(BW = z);
			break;
		case isetRevPole:
			if (!CheckRevPole(z))
				{
				printf("reverbHand got bogus pole %f. ", z);
				printf("Valid range is [-0.5,0.5].\n");
				}
			else
				getAlg()->setPole(pole = z);
			break;
		case isetDampRatio:
			if (!CheckDampRatio(z))
				{
				printf("reverbHand got bogus damp ratio %f. ", z);
				printf("Valid range is [1,30].\n");
				}
			else
				getAlg()->setDampRatio(dampRatio = z);
			break;
		default:
			printf("vss error: reverbHand got bogus float-index %d.\n", iParam.i);
			}
		}
	else
		printf("vss error: reverbHand got bogus element-of-float-array-index %d.\n", iParam.i);
}

//===========================================================================
//		Early reflections
//
void reverbHand::setEarlyRefNum(int z)
{
	if (!CheckEarlyRefNum(z))
	{
		printf("reverbHand got bogus early reflection number %d. ", z);
		printf("Valid range is [0,%d].\n",MaxEarlyRefNum);
	}
	else	getAlg()->setEarlyRefNum( z );
}

void reverbHand::setEarlyRefDelay(int cz, float* rgz)
{
	int temp;
	temp = getAlg()->getEarlyRefNum();
	if (cz < temp)
	{
		printf("reverbHand got less(%d) early reflection delay. ",cz);
		printf("There should be %d.\n",temp);
		return;
	}	
	if (cz > temp)
	{
		printf("reverbHand got more(%d) early reflection delay. ",cz);
		printf("Only take %d of them.\n",temp);
	}
	for (int i=0; i<temp; i++)
		if (!CheckEarlyRefDelay(rgz[i]))
		{
			printf("reverbHand got bogus early reflection delay %f.",rgz[i]);
			printf("Valid range is [0.1,500]ms.\n");
			return;
		}			
	getAlg()->setEarlyRefDelay(rgz);
}

void reverbHand::setEarlyRefCoeff(int cz, float* rgz)
{
	int temp;
	temp = getAlg()->getEarlyRefNum();
	if (cz < temp)
	{
		printf("reverbHand got less(%d) early reflection coefficients. ",cz);
		printf("There should be %d.\n",temp);
		return;
	}	
	if (cz > temp)
	{
		printf("reverbHand got more(%d) early reflection coefficients. ",cz);
		printf("Only take %d of them.\n",temp);
	}
	for (int i=0; i<temp; i++)
		if (!CheckEarlyRefCoeff(rgz[i]))
		{
			printf("reverbHand got bogus early reflection coefficient %f. ",rgz[i]);
			printf("Valid range is [0,1].\n");
			return;
		}			
	getAlg()->setEarlyRefCoeff(rgz);
}

//===========================================================================
//		Comb filters
//
void reverbHand::setCombNum(int z)
{
	if (!CheckCombNum(z))
	{
		printf("reverbHand got bogus comb filter number %d. ", z);
		printf("Valid range is [0,%d].\n", MaxCombNum);
	}
	else	getAlg()->setCombNum( z );
}

void reverbHand::setCombDelay(int cz, float* rgz)
{
	int temp;
	temp = getAlg()->getCombNum();
	if (cz < temp)
	{
		printf("reverbHand got less(%d) comb filter delay. ",cz);
		printf("There should be %d.\n",temp);
		return;
	}	
	if (cz > temp)
	{
		printf("reverbHand got more(%d) comb filter delay. ",cz);
		printf("Only take %d of them.\n",temp);
	}
	for (int i=0; i<temp; i++)
		if (!CheckCombDelay(rgz[i]))
		{
			printf("reverbHand got bogus comb filter delay %f. ",rgz[i]);
			printf("Valid range is [0.1,100]ms.\n");
			return;
		}			
	getAlg()->setCombDelay(rgz);
}

//===========================================================================
//		All-pass filters
//
void reverbHand::setAllPassNum(int z)
{
	if (!CheckAllPassNum(z))
	{
		printf("reverbHand got bogus all-pass filter number %d. ",z);
		printf("Valid range is [0,%d].\n", MaxAllPassNum);
	}
	else	getAlg()->setAllPassNum( z );
}

void reverbHand::setAllPassDelay(int cz, float* rgz)
{
	int temp;
	temp = getAlg()->getAllPassNum();
	if (cz < temp)
	{
		printf("reverbHand got less(%d) all-pass filter delay. ",cz);
		printf("There should be %d.\n",temp);
		return;
	}	
	if (cz > temp)
	{
		printf("reverbHand got more(%d) all-pass filter delay. ",cz);
		printf("Only take %d of them.\n",temp);
	}
	for (int i=0; i<temp; i++)
		if (!CheckAllPassDelay(rgz[i]))
		{
			printf("reverbHand got bogus all-pass filter delay %f. ",rgz[i]);
			printf("Valid range is [0.1,10]ms.\n");
			return;
		}			
	getAlg()->setAllPassDelay(rgz);
}

//===========================================================================
//		Source detection
//
void reverbHand::actCleanup(void)
{
    // If our source got deleted, clean up after it.
    if (input && !input->FValid())
        {
        input = NULL;
        getAlg()->setSource(NULL);
        }
}
