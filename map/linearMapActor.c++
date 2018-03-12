#include "mapActor.h"

//	specifying the map
void LinearMapActor::setDomainAndRange(float iMin, float iMax, float oMin, float oMax)
{
	scale = (oMax - oMin) / (iMax - iMin);
	offset = oMin - (scale * iMin);
}
	
void LinearMapActor::setScaleAndOffset(float iscale, float ioffset)
{
	scale = iscale;
	offset = ioffset;
}
	
float LinearMapActor::map(float datum)
{
	return offset + (scale * datum);
}

int LinearMapActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("SetDomainAndRange"))
	{
		ifFFFF( iMin, iMax, oMin, oMax, setDomainAndRange(iMin, iMax, oMin, oMax) );
		return Uncatch();
	}
	
	if (CommandIs("SetScaleAndOffset"))
	{
		ifFF( scale, offset, setScaleAndOffset(scale, offset) );
		return Uncatch();
	}
	
	return MapActor::receiveMessage(Message);
}
