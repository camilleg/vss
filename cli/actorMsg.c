/**********************************************
 *                actorMsg.c                  * 
 *--------------------------------------------*
 *                                            *
 * Client-side communication for actors.      *
 *                                            *
 *--------------------------------------------*
 * Copyright (C) 1994,  Sumit Das             *
 **********************************************/
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
#define ExternC extern "C"
#else
#define ExternC
#endif

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
{
#endif
#include "actorMsg.h"
#include "cliMsg.h"
#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
}
#endif

int FMsgrcv(void);
float HnoteFromAckNote(void);
const char* SzFromDataReply(void);

static char szT[100];	/* for accumulating message strings */

/* These are shortcut C functions, wrappers for common ways to call actorMessage(). */

/* Create an actor on the server.  returns a handle. */
ExternC float createActor(const char* actorType)
{
	sprintf(szT, "Create %s", actorType);
	return actorMessageRetval(szT);
}

ExternC void deleteActor(const float actorHandle)
{
	sprintf(szT, "Delete %f", actorHandle);
	actorMessage(szT);
}

ExternC void setActorActive(const float actorHandle, const int active)
{
	sprintf(szT, "Active %f %d", actorHandle, active);
	actorMessage(szT);
}

ExternC void dumpActor(const float actorHandle)
{
	sprintf(szT, "Dump %f", actorHandle);
	actorMessage(szT);
}

ExternC void dumpActors()
{
	actorMessage("DumpAll");
}

ExternC void killSoundServer()
{
	actorMessage("KillServer");
}


/****************** Lower level stuff ********************/

#if 0
extern "C" float beginNoteCore(char* szCore)
extern "C" float beginNote(char* sz)
extern "C" float beginNote(const float actorHandle, char* szArgs)
#endif

/* just a simple test case. */
ExternC float beginSound(const float hactor)
{
	char sz[100];
	sprintf(sz, "BeginSound %f", hactor);
	return actorMessageRetval(sz);
}

ExternC float beginNote(const float hactor)
{
	return beginSound(hactor);
}

/***********************************************/

ExternC float actorGetReply(void)
{
	if(!FMsgrcv())
	{
		fprintf(stderr, "vss client error: timed out waiting for server\n");
		return -1.0;
	}
	return HnoteFromAckNote();
}

ExternC const char* actorGetReplyData(void)
{
	if (!FMsgrcv())
		{
		fprintf(stderr, "vss client error: timed out waiting for server\n");
		return NULL;
		}
	return SzFromDataReply();
}

/***********************************************/

ExternC void actorMessage(char* messagename)
{
/*** Send the message to the server ***/
	static mm mmT;
	if (strlen(messagename) > 1498)
		{
		fprintf(stderr, "vss client error: actorMessage() argument too long.\n");
		return;
		}
	strcpy(mmT.rgch, messagename);
	mmT.fRetval = 0;
	Msgsend(NULL, &mmT);
}

ExternC void actorMessageF(char* messagename, float f)
{
	char sz[1000];
	sprintf(sz, messagename, f);
	actorMessage(sz);
}

ExternC void actorMessageFD(char* messagename, float f, int d)
{
	char sz[1000];
	sprintf(sz, messagename, f, d);
	actorMessage(sz);
}

ExternC void actorMessageFDD(char* messagename, float f, int d1, int d2)
{
	char sz[1000];
	sprintf(sz, messagename, f, d1, d2);
	actorMessage(sz);
}

ExternC void actorMessageFS(char* messagename, float f, const char* sz)
{
	char szCmd[1000];
	sprintf(szCmd, messagename, f, sz);
	actorMessage(szCmd);
}

ExternC float actorMessageRetval(char* messagename)
{
/*** Send the message to the server ***/
	static mm mmT;
	if (strlen(messagename) > 1498)
		{
		fprintf(stderr, "vss client error: actorMessageRetval() argument too long.\n");
		return -1.0;
		}
	strcpy(mmT.rgch, messagename);
	mmT.fRetval = 1;
	Msgsend(NULL, &mmT);

	return actorGetReply();
}

ExternC const char* actorMessageReturnData(char* messagename)
{
/*** Send the message to the server ***/
	static mm mmT;
	if (strlen(messagename) > 1498)
		{
		fprintf(stderr, "vss client error: actorMessageReturnData() argument too long.\n");
		return NULL;
		}
	strcpy(mmT.rgch, messagename);
	mmT.fRetval = 1;
	Msgsend(NULL, &mmT);

	return actorGetReplyData();
}
