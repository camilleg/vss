/********************************************************************
 *                              actorMsg.h                          * 
 *------------------------------------------------------------------*
 *                                                                  *
 * Udp message codes for controlling actors, for inclusion in the   *
 * client side code.                                                *
 *                                                                  *
 * From the client, this protocol is accessed at the lowest level   *
 * through the actorMessage() call.
 *                                                                  *
 *------------------------------------------------------------------*
 * Copyright (C) 1994,  Sumit Das                                   *
 *                                                                  *
 * Redesigned June 1996 Camille Goudeseune.                         *
 *                                                                  *
 ********************************************************************/

#if !defined(actorMsg_h)
#define actorMsg_h

#if !defined(NO_LOCAL_INCLUDES)
typedef void* OBJ;
#include <stdarg.h>
#endif

#define cchmm 5120 /* 5x1024, like in dev/srv/vssglobals.h */
typedef struct mm
{
    char fRetval;
    /* Nonzero iff srv should return float to client in response to this message. */
    char rgch[cchmm];
    /* The ascii string containing the message. */
} mm;


#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C" {
#endif

/********* Actor Messages **********/

int 	AUDinit(const char *fileName);
void	AUDterminate(int fileHandle);
void	AUDreset(void);
float	AUDupdateSimple(char *messageGroupName, int numFloats, float *floatArray);
float	AUDupdateSimpleFloats(char* szActor, int numFloats, ...);
float	AUDupdateFloats(int fdT, char* szActor, int numFloats, ...);
float	AUDupdate(int fileHandle, char *messageGroupName, int numFloats, float *floatArray);
void	AUDupdateTwo(int theFirst, int theSecond, char *messageGroupName, int numFloats, float *floatArray);
void	AUDupdateMany(int numHandles, int * handleArray, char *messageGroupName, int numFloats, float *floatArray);
void	AUDqueue(int, float*, float);
void	AUDflushQueue(int, char *, int fPreserveQueueData /*=0*/);
void	actorMessage(char* messagename);
float	actorMessageRetval(char* messagename);
float	actorGetReply(void);
const char*	actorGetReplyData(void);
void	AUDEnableNoMessageGroupWarning(int fEnable);

/*xx30 void	addEventRet(float seqHandle, float when, float returnID, int message, char *formatString, ... );*/
/*xx30 void	addEvent(float seqHandle, float when, int message, char *formatString, ... );*/


/*****************************
 *** useful shortcut stuff ***
 *****************************/
 
float	createActor(const char* actorType);		/* Make one, return a handle */
void	deleteActor(const float actorHandle);		/* Delete actor with this handle */
void	setActorActive(const float actorHandle, const int active);	/* turn actor on or off */
void	dumpActor(const float actorHandle);		/* Print out (on server) one actor */
void	dumpActors();					/* Print out (on server) listing of all actors */
float	beginNote(const float hactor);
void	killSoundServer();

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
} /* extern "C" */
#endif

#endif
