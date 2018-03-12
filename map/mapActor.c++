//===========================================================================
//	This fragment of the vss renaissance brought to you by Kelly Fitz, 1997.
//===========================================================================

#include "mapActor.h"
extern const char* VSS_StripZeros(const char* ); // in vssSrv.c++

//===========================================================================
//		MapActor mapArray
//
int MapActor::mapArray(float * dataArray, int size)
{
	for (int i = 0; i < size; i++)
		dataArray[i] = map( dataArray[i] );
	return size;
}

//===========================================================================
//		MapActor mapAndSend
//
void MapActor::mapAndSend(char * mapThis)
{
	char message[512], copy[512], fstr[32];
	char * pch = mapThis;
// printf("\nMapActor building message from %s\n", mapThis);;
	
	//	find the first token and write it into the message
	//	the first token is everything before a '@'
	strcpy(copy, mapThis);
	pch = strtok(copy, "@");
	//	a null char has been inserted into copy at the position
	//	of the '@', so pch points to a terminated string of non @'s.
	strcpy(message, pch);
// printf("found %s...\n", pch);;
	//	if there are more tokens, the next one will be a number,
	//	white space delimited.
	while( NULL != (pch = strtok(NULL, " \t\r")) )
		{
		float x;
		if (1 == sscanf(pch, "%f", &x))
			{
			//	convert the indexed datum to a string and 
			//	append it to the message string
	// printf("@%f = %f\n", x, map(x));;
			sprintf(fstr, "%f ", map(x));
			strcat(message, fstr);
			
			//	look for another '@' delimited token
			if ( NULL != (pch = strtok(NULL, "@")) )
			{
	 //printf("found %s...\n", pch);;
				strcat(message, pch);
			}
			continue;
			}

		int i;
		if (1 == sscanf(pch, "*%d", &i))
			{
			// it was a *4 kind of arg.
		//	printf("using secret ninja [@*%d] trick in mapActor\n", i);;
			x = VrgzMG()[i];
			continue;
			}

		// Try to parse [%f %f ... %f] a la SscanfFloats,
		// accumulate the %f's into a buffer rgz, and send the buffer to mapArray().
		// HidimMapper overrides mapArray to do its special thing.
		{
		char ch;
		int cch;
		if (1 == sscanf(pch, " %c %n", &ch, &cch) && ch == '[')
			{
			pch += cch;
			float rgz[100]; // hardcoded limit
			int cz;
			for (cz=0; cz<100; )
				{
#ifdef VSS_WINDOWS
				// workaround for compiler bug
				{
				char* pchNext;
#ifdef DEBUG
				if (!*pch)
					{
					if (isDebug())
						printf("ZERO   ");;;;
					rgz[cz] = 0; pchNext = pch;
					}
#endif
				if (*pch == '*')
					{
				//	printf("using secret ninja\n");;;;
					rgz[cz] = VrgzMG()[(int)strtod(pch+1, &pchNext)];
					}
				else
					rgz[cz] = (float)strtod(pch, &pchNext);
				cch = pchNext - pch;
				}
				if (cch == 0)
#else
				if (1 != sscanf(pch, "%f %n", rgz+cz, &cch))
#endif
					{
					// Not a float.  End of list?
					//printf("End of list: <%s>\n", pch);;

					if (!*pch)
						{
						pch = strtok(NULL, " \t\r");
						if (!pch)
							{
							fprintf(stderr, "vss MapActor error: unexpected end of message in array of floats.\n");
							return;
							}
						continue;
						}

					if (1 != sscanf(pch, " %c %n", &ch, &cch) || ch != ']')
						{
						fprintf(stderr, "vss MapActor error: unexpected text in array of floats (\"%s\")\n", pch);
						return;
						}
					break; // Yes, end of list.
					}
				cz++;
				pch += cch;
				}
			if (cz >= 100)
				{
				fprintf(stderr, "vss MapActor warning: more than 100 floats in a mapArray() call.  Truncation possible.\n");
				}
			if (isDebug())
				{
				fprintf(stderr, "hidimMapper [");
				for (int j=0; j<cz; j++)
					fprintf(stderr, "%.2f ", rgz[j]);
				fprintf(stderr, "]  ");
				}
			strcat(message, "[");
			int czNew = mapArray(rgz, cz);
		//	if (isDebug())
		//		fprintf(stderr, "called mapArray, %d -> %d floats!\n", cz, czNew);
			for (int j=0; j<czNew; j++)
				{
				sprintf(fstr, "%f ", rgz[j]);
				strcat(message, fstr);
				}
			strcat(message, "]");
			strcpy(message, VSS_StripZeros(message)); // memory leak
			if (isDebug())
				fprintf(stderr, "->  %s\n", strchr(message, '['));
			continue;
			}
		}

		fprintf(stderr, "vss MapActor error: found non-number after \'@\' in message \"%s\".\n", mapThis);
		return;
		}

	actorMessageHandler( message );
}

//===========================================================================
//		MapActor receiveMessage
//
int 
MapActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("MapAndSend"))
	{
//		printf("gonna mapAndSend()\n");;
		ifM( msg, mapAndSend(msg) );
		return Uncatch();
	}
	
	return VActor::receiveMessage(Message);
}
