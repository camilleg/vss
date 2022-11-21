/* Send data over UDP packets. */

#pragma once

#define hNil (-1.f)

/* urp.. defined in actorMsg.h as well as in here. */
#ifndef actorMsg_h
typedef void* OBJ;

#define cchmm 5120
typedef struct mm
{
	char fRetval;
	/* Nonzero iff srv should return float to client in response to this message. */
	char rgch[cchmm];
	/* The ascii string containing the message. */
} mm;

#endif

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C" {
#endif

void MsgsendObj(OBJ, struct sockaddr_in*, mm*);
OBJ BgnMsgsend(const char *szHostname, int channel);

void setAckPrint(int flag);
int BeginSoundServer();
int BeginSoundServerAt(char* hostName);
int SelectSoundServer(int serverHandle);
void EndSoundServer();
void EndAllSoundServers();
int PingSoundServer();
void Msgsend(struct sockaddr_in*, mm*);
void MsgsendArgs1(struct sockaddr_in*, mm*, const char* msg, float z0);
void MsgsendArgs2(struct sockaddr_in*, mm*, const char* msg, float z0, float z1);
void clientMessageCall(char*);
int WMsgFromSz(char*);
const char* SzMsgFromW(int);

const char* GetVssLibVersion(); /* from vssBuild.c++ */
const char* GetVssLibDate();

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
}
#endif
