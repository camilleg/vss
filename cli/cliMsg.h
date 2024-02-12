/* Send data over UDP packets. */

#pragma once

#define hNil (-1.f)

/* urp.. defined in actorMsg.h as well as in here. */
#ifndef actorMsg_h
typedef void* OBJ;
#endif

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C" {
#endif

void MsgsendObj(OBJ, struct sockaddr_in*, const char*);
OBJ BgnMsgsend(const char *szHostname, int channel);

void setAckPrint(int flag);
int BeginSoundServer(void);
int BeginSoundServerAt(const char* hostName);
int SelectSoundServer(int serverHandle);
void EndSoundServer(void);
void EndAllSoundServers(void);
int PingSoundServer(void);
void Msgsend(struct sockaddr_in*, const char*);
void MsgsendArgs1(struct sockaddr_in*, const char*, float);
void MsgsendArgs2(struct sockaddr_in*, const char*, float, float);
void clientMessageCall(const char*);
int WMsgFromSz(char*);
const char* SzMsgFromW(int);

const char* GetVssLibVersion(void); /* from vssBuild.c++ */
const char* GetVssLibDate(void);

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
}
#endif
