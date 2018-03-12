#include "process.h"

//===========================================================================
//	processAlg constructor
//
processAlg::processAlg(void) :
	VAlgorithm(),
	modIndex(1.)
{
}

//===========================================================================
//	processAlg destructor
//
processAlg::~processAlg()
{
}

//===========================================================================
//	processAlg generateSamples
//
//
void
processAlg::generateSamples(int howMany)
{
	// This is merely a 1-channel to 1-channel example.
	for (int j = 0; j < howMany; j++)
	{
		if (source == NULL)
			Output(0., j);
		else
			Output((*source)[j][0] * modIndex, j);
	}
}
