#include "mapActor.h"

HidimMapActor::HidimMapActor() :
	dimLo(0),
	dimHi(0),
	cpt(0),
	rgzLo(NULL),
	rgzHi(NULL)
{
}

HidimMapActor::~HidimMapActor()
{
	if (rgzLo)
		delete [] rgzLo;
	if (rgzHi)
		delete [] rgzHi;
}

//	LoadFile
//
//	Read a map file with the following format:
//	# of dimensions of low space
//	# of dimensions of high space
//	# of points
//	floating-point coords of high space
//	floating-point coords of low space

void HidimMapActor::loadFile(char* szFile)
{
	FILE* pf = fopen(szFile, "rb");
	int i;
	if (!pf)
		{
		fprintf(stderr, "vss error: HidimMapActor LoadFile can't open file \"%s\"\n",
			szFile);
		return;
		}
	if (3 != fscanf(pf, "%d %d %d", &dimLo, &dimHi, &cpt))
		{
		fprintf(stderr, "vss error: HidimMapActor LoadFile syntax error in file \"%s\"\n",
			szFile);
		goto LAbort;
		}

	// Validity check
	if (!setDims(dimLo, dimHi) || !setNumPoints(cpt) || !FValid())
		return;

	// Read in the data points.

	if (rgzHi)
		delete [] rgzHi;
	rgzHi = new float[cpt * dimHi];
	for (i=0; i < cpt * dimHi; i++)
		if (1 != fscanf(pf, "%f ", &rgzHi[i]))
			{
			fprintf(stderr, "vss error: HidimMapActor LoadFile syntax error in file \"%s\"\n",
				szFile);
			goto LAbort;
			}

	if (rgzLo)
		delete [] rgzLo;
	rgzLo = new float[cpt * dimLo];
	for (i=0; i < cpt * dimLo; i++)
		if (1 != fscanf(pf, "%f ", &rgzLo[i]))
			{
			fprintf(stderr, "vss error: HidimMapActor LoadFile syntax error in file \"%s\"\n",
				szFile);
			goto LAbort;
			}
	fclose(pf);
	mymap.Init(cpt, dimLo, dimHi, rgzLo, rgzHi);
	return;

LAbort:
	if (rgzLo)
		{
		delete [] rgzLo;
		rgzLo = NULL;
		}
	if (rgzHi)
		{
		delete [] rgzHi;
		rgzHi = NULL;
		}
	dimLo = dimHi = cpt = 0;
	fclose(pf);
}

int HidimMapActor::FValid(void)
{
	if (cpt < dimLo + 1)
		{
		fprintf(stderr, "vss error: # of points (%d) < low dimension (%d) + 1\n",
			cpt, dimLo);
		return 0;
		}
	return 1;
}

int HidimMapActor::setDims(int loDim, int hiDim)
{
	if (loDim >= hiDim)
		{
		fprintf(stderr, "vss error: HidimMapActor SetDims loDim not < hiDim\n");
		return 0;
		}
	if (loDim <= 0 || hiDim <= 0)
		{
		fprintf(stderr, "vss error: HidimMapActor SetDims dimension nonpositive\n");
		return 0;
		}
	
	dimLo = loDim;
	dimHi = hiDim;
	return 1;
}

int HidimMapActor::setNumPoints(int num)
{
	if (num <= 0)
		{
		fprintf(stderr, "vss error: HidimMapActor SetNumPoints nonpositive\n");
		return 0;
		}
	cpt = num;
	return 1;
}

void HidimMapActor::computeLowPoints(void)
{
	if (isDebug())
		printf("HidimMapActor::computeLowPoints()\n");
	if (rgzLo)
		delete [] rgzLo;
	rgzLo = NULL;
	if (!FValid())
		return;
	const int cz = cpt * dimLo;
	rgzLo = new float[cz];

	if (rgzHi && rgzLo)
		{
		mymap.GA();
		for (int ipt = 0; ipt < cpt; ipt++)
		for (int idim = 0; idim < dimLo; idim++)
			rgzLo[ipt*dimLo + idim] = mymap.PzQ(ipt)[idim];

		if (isDebug())
			printf("HidimMapActor::computeLowPoints calls mymap.Init(%d %d %d)\n",
				cpt, dimLo, dimHi);
		mymap.Init(cpt, dimLo, dimHi, rgzLo, rgzHi);
		}
}

void HidimMapActor::setLowPoints(int cz, float* rgz)
{
	if (isDebug())
		printf("HidimMapActor::setLowPoints(%d)\n", cz);
	if (rgzLo)
		delete [] rgzLo;
	rgzLo = NULL;
	if (!FValid())
		return;
	if (cz != cpt * dimLo)
		{
		fprintf(stderr, "vss error: HidimMapActor SetLowPoints array: %d entries, should be %d\n", cz, cpt * dimLo);
		return;
		}
	rgzLo = new float[cz];

	// Instead of a straight FloatCopy, throw a little randomness in here
	// to avoid degeneracies when 4 points lie on a circle.
	// FloatCopy(rgzLo, rgz, cz);
	for (int i=0; i<cz; i++)
		rgzLo[i] = rgz[i] + (drand48() - .5) * .00008;

	if (rgzHi && rgzLo)
		{
		if (isDebug())
			printf("HidimMapActor::setLowPoints calls mymap.Init(%d %d %d)\n",
				cpt, dimLo, dimHi);
		mymap.Init(cpt, dimLo, dimHi, rgzLo, rgzHi);
		}
}

void HidimMapActor::setHighPoints(int cz, float* rgz)
{
	if (isDebug())
		printf("HidimMapActor::setHighPoints(%d)\n", cz);
	if (rgzHi)
		delete [] rgzHi;
	rgzHi = NULL;
	if (!FValid())
		return;
	if (cz != cpt * dimHi)
		{
		fprintf(stderr, "vss error: HidimMapActor SetHighPoints array: %d entries, should be %d\n", cz, cpt * dimHi);
		return;
		}
	rgzHi = new float[cz];
	FloatCopy(rgzHi, rgz, cz);
	if (rgzHi && rgzLo)
		{
		if (isDebug())
			printf("HidimMapActor::setHighPoints calls mymap.Init(%d %d %d)\n",
				cpt, dimLo, dimHi);
		mymap.Init(cpt, dimLo, dimHi, rgzLo, rgzHi);
		}
}

// Send the data to mymap, and stuff the result in place.
// return a string to be stuffed into the string mapAndSend is building?
int HidimMapActor::mapArray(float * dataArray, int size)
{
	if (!rgzLo || !rgzHi || cpt == 0)
		// no error message, that happened already probably.
		return 0;

	if (size < dimLo)
		{
		fprintf(stderr, "vss error: HidimMapActor mapArray too short (%d, should be %d).\n", size, dimLo);
		return 0;
		}

//	if (isDebug())
//		printf("HidimMapActor::mapArray checkpoint 0\n");

	// Copy any extra args happening to come along for the ride
	// to the end of the array.
	// This is useful if there are some args we want to pass to a
	// message group which are NOT to be mapped.
	int i;
	for (i=dimLo; i<size; i++)
		dataArray[dimHi+i-dimLo] = dataArray[i];

	MCPoint pt(iMaxMCPoint);

//	if (isDebug())
//		printf("HidimMapActor::mapArray checkpoint 1, dimLo=%d\n", dimLo);

	// Map dataArray to pt, then copy back into dataArray.
	switch(dimLo)
		{
	default:
		fprintf(stderr, "vss error: HidimMapActor dimLo should be 2 or 3, not %d\n",
			dimLo);
		return 0;
	case 2:
		mymap.MCPointFromXY(pt, dataArray[0], dataArray[1]);
		break;
	case 3:
		mymap.MCPointFromXYZ(pt, dataArray[0], dataArray[1], dataArray[2]);
		break;
		}

//	if (isDebug())
//		printf("HidimMapActor::mapArray checkpoint 2\n");

	for (i=0; i<dimHi; i++)
		dataArray[i] = pt[i];

//	if (isDebug())
//		printf("HidimMapActor::mapArray checkpoint 3\n");

	return dimHi + (size - dimLo);
}

float HidimMapActor::map(float)
{
	fprintf(stderr, "vss error: HidimMapActor::map() should never be called.\n");
	// Though I suppose it could be called if dimLo == 1.
	return 0.0f;
}

int HidimMapActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("LoadFile"))
		{
		ifS( szFile, loadFile(szFile) );
		return Uncatch();
		}

	if (CommandIs("SetDims"))
		{
		ifDD( loDim, hiDim, setDims(loDim, hiDim) );
		return Uncatch();
		}

	if (CommandIs("SetNumPoints"))
		{
		ifD( num, setNumPoints(num) );
		return Uncatch();
		}

	if (CommandIs("SetLowPoints"))
		{
		ifFloatArray( rgz, cz, setLowPoints(cz, rgz) );
		return Uncatch();
		}

	if (CommandIs("SetHighPoints"))
		{
		ifFloatArray( rgz, cz, setHighPoints(cz, rgz) );
		return Uncatch();
		}

	if (CommandIs("UseGeneticAlgorithm"))
		{
		ifNil( mymap.SetSammon(0) );
		}

	if (CommandIs("UseSammonsMapping"))
		{
		ifNil( mymap.SetSammon(1) );
		}

	if (CommandIs("ComputeLowPoints"))
		{
		ifNil( computeLowPoints() );
		}

	return MapActor::receiveMessage(Message);
}
