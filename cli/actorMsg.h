#if !defined(actorMsg_h)
#define actorMsg_h

#if !defined(NO_LOCAL_INCLUDES)
typedef void* OBJ;
#include <stdarg.h>
#endif

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C" {
#endif

// Actor messages

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

// Shortcuts
 
float	createActor(const char* actorType); // returns a handle
void	deleteActor(const float handle);
void	setActorActive(const float handle, const int active);
void	dumpActor(const float handle); // vss prints this actor
void	dumpActors(void); // vss lists all actors
float	beginNote(const float hactor);
void	killSoundServer(void);

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
} /* extern "C" */
#endif

#endif
