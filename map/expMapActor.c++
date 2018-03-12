#include "mapActor.h"

//	default ExpMapActor is an identity map.
ExpMapActor::ExpMapActor(void)
{
	i0 = o0 = 0.0;
	i1 = o1 = 1.0;
	expBase = 1.0;
}

//	specifying the map
void ExpMapActor::setMapBounds(float setIn0, float setIn1, float setOut0, float setOut1, float setBase)
{ 
	i0 = setIn0; 
	i1 = setIn1; 
	o0 = setOut0; 
	o1 = setOut1;
	expBase = setBase;
}
	
float ExpMapActor::map(float datum)
{
	if (expBase == 1.0) 
		return o0;

	float kerpow;
	
	if ( i0 < i1 )
		{
		if ( o0 < o1 )
			{			
			kerpow = pow(expBase, ( (datum - i0) / (i1 - i0) ) );
			return o0 + ( (o1 - o0) * (kerpow - 1.0) / (expBase - 1.0) );
			}
		else
			{			
			kerpow = pow(expBase, ( (datum - i1) / (i0 - i1) ) );
			return o1 + ( (o0 - o1) * (kerpow - 1.0) / (expBase - 1.0) );
			}
		}
	else
		{			
		if ( o0 < o1 )
			{			
			kerpow = pow(expBase, ( (datum - i0) / (i1 - i0) ) );
			return o1 + ( (o0 - o1) * (kerpow - 1.0) / (expBase - 1.0) );
			}
		else
			{			
			kerpow = pow(expBase, ( (datum - i1) / (i0 - i1) ) );
			return o0 + ( (o1 - o0) * (kerpow - 1.0) / (expBase - 1.0) );
			}
		}
/*
(kerpow - 1.0) / (expBase - 1.0) is like an exponential wavetable
that goes from 0 to 1.  If i1 > i0, then it is read in the forward
direction (0 to 1). If i1 < i0  then it is read in the reverse
direction (1 to 0). If o1 > o0, then the wavetable-like thing is
added to o0. If o1 < o0, then the wavetable-like thing is
subtracted from o0

i.e. the sign of the output range determines whether the output
increases or decreases with increasing input. The sign of the input
range dtermines the direction of concavity of the curve.
*/
}

int ExpMapActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetMapBounds"))
	{
		ifFFFFF( i0, i1, o0, o1, base, setMapBounds(i0, i1, o0, o1, base) );
		return Uncatch();
	}
	
	return MapActor::receiveMessage(Message);
}
