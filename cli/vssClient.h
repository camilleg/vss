#ifndef __VSSCLIENT_H__
#define __VSSCLIENT_H__
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#	include <arpa/inet.h>

#define hNil (-1.f)
typedef void *OBJ;

#ifndef TRUE
#	define TRUE 1
#	define FALSE 0
#endif
#ifndef PI
#	define PI (3.14159265358979323)
#endif

#	define cchmm 5120 
	typedef struct mm
	{
	char fRetval;
	char rgch[cchmm];
	} mm;

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C" {
#endif
int AUDinit(const char *fileName);
void AUDterminate(int fileHandle);
void AUDreset(void);
float AUDupdateSimple(char *messageGroupName, int numFloats, float *floatArray);
float AUDupdateSimpleFloats(char* szActor, int numFloats, ...);
float AUDupdateFloats(int fdT, char* szActor, int numFloats, ...);
float AUDupdate(int fileHandle, char *messageGroupName, int numFloats, float *floatArray);
void AUDupdateTwo(int theFirst, int theSecond, char *messageGroupName, int numFloats, float *floatArray);
void AUDupdateMany(int numHandles, int * handleArray, char *messageGroupName, int numFloats, float *floatArray);
void AUDqueue(int, float*, float);
void AUDflushQueue(int, char *, int fPreserveQueueData );
void actorMessage(char* messagename);
void actorMessageF(char* messagename, float f);
void actorMessageFD(char* messagename, float f, int d);
void actorMessageFDD(char* messagename, float f, int d1, int d2);
void actorMessageFS(char* messagename, float f, const char* sz);
float actorMessageRetval(char* messagename);
float actorGetReply(void);
const char* actorGetReplyData(void);
void AUDEnableNoMessageGroupWarning(int fEnable);
float createActor(const char* actorType); 
void deleteActor(const float actorHandle); 
void setActorActive(const float actorHandle, const int active); 
void dumpActor(const float actorHandle); 
void dumpActors(); 
float beginNote(const float hactor); /* obsolete, use beginSound() */
float beginSound(const float hactor);
void killSoundServer();
#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
} 
#endif

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
const char* GetVssLibVersion(void); 
const char* GetVssLibDate(void);

extern int fdMidi; 
typedef struct
	{
	int cb;
	char rgb[4];
	} VSSMDevent;
VSSMDevent* GetMidiMessages(float* pcmsg, float hMidiActor);

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
}
#endif

#endif		/* __VSSCLIENT_H__ */
