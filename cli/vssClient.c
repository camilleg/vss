#include <stdio.h>
#include <string.h>
#include "vssClient_int.h"
#include "cliMsg.h"

int FMsgrcv(void);
float HnoteFromAckNote(void);

VSSMDevent* MidiMsgsFromAckMidi(float* pcvssmdevent);

VSSMDevent* GetMidiMessages(float* pcmsg, float hMidiActor)
{
	static mm mmT;
	if (!pcmsg)
		{
		fprintf(stderr, "GetMidiMessages: null *pcmsg\n");
		return NULL;
		}
	/*xx30 mirror structure of CEntry.c++'s actorMessage()*/
	sprintf(mmT.rgch, "MidiReceive %f", hMidiActor);
	Msgsend(NULL, &mmT);
	if (!FMsgrcv())				/* AckMidiInputMsg */
		{
		*pcmsg = 0.;
		fprintf(stderr, "MidiInputMsg didn't get a reply\n");
		return NULL;
		}
	return MidiMsgsFromAckMidi(pcmsg);
}

