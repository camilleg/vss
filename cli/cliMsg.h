/* cliMsg.h -- send data over udp packets */

#ifndef __CLI_MSG_H_
#define __CLI_MSG_H_

#define hNil (-1.f)

/* urp.. defined in actorMsg.h as well as in here. */
#ifndef actorMsg_h
typedef void *OBJ;

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

void MsgsendObj(OBJ obj, struct sockaddr_in *paddr, mm* pmm);
OBJ BgnMsgsend(char *szHostname, int channel);

void setAckPrint(int flag);
int BeginSoundServer(void);
int BeginSoundServerAt(char * hostName);
int SelectSoundServer(int serverHandle);
void EndSoundServer(void);
void EndAllSoundServers(void);
int PingSoundServer(void);
void Msgsend(struct sockaddr_in *paddr, mm* pmm);
void MsgsendArgs1(struct sockaddr_in *paddr, mm* pmm, const char* msg,
					float z0);
void MsgsendArgs2(struct sockaddr_in *paddr, mm* pmm, const char* msg,
					float z0, float z1);
void clientMessageCall(char* Message);
int WMsgFromSz(char *szMsg);
const char* SzMsgFromW(int wMsg);

const char* GetVssLibVersion(void); /* from vssBuild.c++ */
const char* GetVssLibDate(void);

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
}
#endif

#endif /* __CLI_MSG_H_ */