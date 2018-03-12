#include "mapActor.h"

//	default SegmentMapActor is an identity map.
SegmentMapActor::SegmentMapActor()
{
	numPoints = 2;
	breakPtsX = new float[numPoints];
	breakPtsY = new float[numPoints];
	scale = new float[numPoints-1];
	offset = new float[numPoints-1];
	 
	breakPtsX[0] = 0.0;	// xlo
	breakPtsX[1] = 1.0;	// xhi
	breakPtsY[0] = 0.0;	// ylo
	breakPtsY[1] = 1.0;	// yhi
	
	scale[0] = 1.0;		// multiplicative identity
	offset[0] = 0.0;	// additive identity
}
SegmentMapActor::~SegmentMapActor() 
{
	delete [] breakPtsX;
	delete [] breakPtsY;
	delete [] scale;
	delete [] offset;
}

//	specifying the map
void SegmentMapActor::setBreakpoints(float *breakData, int inumPoints) 
{
	if ( 0 != inumPoints%2 )
	{
		printf("setBreakPoints requires an even number of floats\n");
		return;
	}

	numPoints = inumPoints/2;

	if ( numPoints == 1 )
	{
		printf("setBreakPoints requires at least two pairs of floats\n");
		return;
	}


	delete [] breakPtsX;
	delete [] breakPtsY;
	delete [] scale;
	delete [] offset;
	breakPtsX = new float[numPoints];
	breakPtsY = new float[numPoints];
	scale = new float[numPoints-1];
	offset = new float[numPoints-1];
	
	int i, ii;

	for (i=0, ii=0; i < numPoints; i++, ii+=2)
	{
		breakPtsX[i] = breakData[ii];
		breakPtsY[i] = breakData[ii+1];
//	Flag breakpoints that are out of monotonic order in x
		if((i > 0) && (breakPtsX[i] < breakPtsX[i-1]))
			fprintf(stderr, "SegmentMapActor: breakpoints are out of x-order (%f, %f)!!\nAre you sure you want to do that?\n",
				breakPtsX[i-1], breakPtsX[i]);
	}
	
	for(i=0; i < numPoints-1; i++)
	{
		scale[i] = (breakPtsY[i+1] - breakPtsY[i]) / (breakPtsX[i+1] - breakPtsX[i]);
		offset[i] = breakPtsY[i] - (scale[i] * breakPtsX[i]);
	}
}		

float SegmentMapActor::map(float datum)	
{
// 		find the right segment by matching the x-region into
// 		which datum falls, using a...
// 		dumb linear search: advance through the x values
// 		until datum is straddled
// 			sorry, replace with a "better" search if you like, 
//			but also realize that (numPoints < 10), usually
	int i;
	for (i=0; i < numPoints; i++)
	{
		if (breakPtsX[i] > datum) break;	// straddled, done
		if (breakPtsX[i] == datum)
			return breakPtsY[i];		// VERY special case
	}
	i--;					// back index off to match segment index
	if (i < 0) i=0;				// limit segment index to the extreme
	if (i > numPoints-2) i=numPoints-2;	// 	described segments

	return offset[i] + (scale[i] * datum); 
}

int SegmentMapActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetBreakpoints"))
	{
		ifFloatArray( breakdata, count, setBreakpoints(breakdata, count) );
		return Uncatch();
	}
	
	return MapActor::receiveMessage(Message);
}
