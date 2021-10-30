#include <iostream>
#include <cctype>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "platform.h"
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

#ifdef xxx_midi62_xxx
#include "vssMidicore.h"

static VSSMDevent vrgvssmdevent[100]; // match vssMidicore.c++'s size;;
static int vcvssmdevent;
static void AckMidi(int cb, char* pb)
{
	vcvssmdevent = cb;
	memcpy(vrgvssmdevent, pb, vcvssmdevent * sizeof(VSSMDevent));
}
VSSMDevent* MidiMsgsFromAckMidi(float* pcvssmdevent)
{
	*pcvssmdevent = vcvssmdevent;
	return vrgvssmdevent;
}
#endif

void clientMessageCall(char* Message)
{
#ifdef xxx_midi62_xxx
	// format of string: "AckMidiInputMsg 5 00ff00ff00",
	// but the hex digits are 012345689:;<=>? for consecutive ASCIIness.
	if (!strncmp(Message, "AckMidiInputMsg ", 16))
		{
		// decode args from ascii to binary
		int cb = atoi(Message += 16);
		if (cb < 0 || cb > 10000)
			{
LError:
			cerr << "Internal syntax error in AckMidiInputMsg\n";
			return;
			}
		char* pch = strchr(Message, ' ') + 1;
		int ib, bHi, bLo;
		char pb[10000];
		for (ib=0; ib<cb; ib++)
			{
			bHi = *(pch++) - '0';
			bLo = *(pch++) - '0';
			if (bHi < 0 || bHi >= 16 || bLo < 0 || bLo >= 16)
				goto LError;
			pb[ib] = bHi<<4 + bLo;
			}
		AckMidi(cb, pb);
		return;
		//;; CEntry.c++, look for "AckMidiInputMsg"
		}
	else
#endif

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
