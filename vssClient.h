/* vssClient.h  --  File created by machine.  Do not modify. */
 /* Built on Wed May 5 12:31:36 CDT 1999 */
#ifndef __VSSCLIENT_H__
#define __VSSCLIENT_H__
#define NO_LOCAL_INCLUDES
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#ifndef __VSS__SYNTH_H__
#define __VSS__SYNTH_H__
#include <climits>
typedef void *OBJ;
#ifndef TRUE
  #define TRUE 1
  #define FALSE 0
#endif
#ifndef PI
  #define PI (3.14159265358979323)
#endif
#endif

#if !defined(actorMsg_h)
#define actorMsg_h
#if !defined(NO_LOCAL_INCLUDES)
  #include "_synth.h"
  #include <stdarg.h>
#endif
#ifdef VSS_IRIX
  #define cchmm 5120 
  typedef struct mm
  {
  char fRetval;
  char rgch[cchmm];
  } mm;
#endif
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
float actorMessageRetval(char* messagename);
float actorGetReply(void);
const char* actorGetReplyData(void);
void AUDEnableNoMessageGroupWarning(int fEnable);
float createActor(const char* actorType); 
void deleteActor(const float actorHandle); 
void setActorActive(const float actorHandle, const int active); 
void dumpActor(const float actorHandle); 
void dumpActors(); 
float beginNote(const float hactor);
void killSoundServer();
#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
} 
#endif
#endif

#ifndef __CLI_MSG_H_
#define __CLI_MSG_H_
#if !defined(NO_LOCAL_INCLUDES)
  #include "_synth.h" 
#endif
#define hNil (-1.f)
#ifdef VSS_IRIX
  #include <arpa/inet.h>
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
#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
}
#endif
#endif

#ifndef __CLIENT_H__
#define __CLIENT_H__
#ifdef VSS_IRIX
#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C" {
#endif
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
#endif
#endif 
#endif
