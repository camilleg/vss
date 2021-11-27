#include <cstdio>
#include <cstring>

#include "vssMsg.h"

static float vhnote;
static int AckPrint=1; //;; 0
static void AckNote(float returnVal)
{
	if(AckPrint)
		printf("AckNote got %f.\n", returnVal);
	vhnote = returnVal;
}

void setAckPrint(int flag)
{
	AckPrint = flag;
}

float HnoteFromAckNote()
{
	return vhnote;
}

void clientMessageCall(char* Message)
{
	if (!strncmp(Message, "AckNoteMsg ", 11))
		{
		float z = 0.;
		if (1 != sscanf(Message, "AckNoteMsg %f", &z))
			{
			fprintf(stderr, "bad args to AckNoteMsg (%s)", Message);
			return;
			}
		AckNote(z);
		}
	else
		{
		fprintf(stderr, "vss error: clientMessageCall(): Bad message \"%s\"\n",
			Message);
		if (strstr(Message, "0000000"))
			fprintf(stderr, "           (connection attempted from a pre-3.0 client?)\n");
		}
}
