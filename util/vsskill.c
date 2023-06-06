// Connect to vss and kill it.
// On IRIX, also kill any vss older than version 3.0.

#include "vssClient.h"

#ifdef __sgi

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <bstring.h>
#include <sys/prctl.h>

/* from htm/htm.h */
#define cchmm23 32
#define czmm23 128
typedef struct mm23
{
	char name[cchmm23];
	float param[czmm23];
} mm23;

/* from vssMsg.c++ */
typedef struct
{
	struct sockaddr_in addr;
	int len;
	int sockfd;
	int channel;
} desc;

int sendudp(struct sockaddr_in *sp, int sockfd, long count, void  *b)
{
	return sendto(sockfd, b, (int)count, 0, sp, sizeof(*sp)) == count;
}

int FInitUdp23(OBJ* pobj, int wChannel)
{
	char ipAddr[16];
	char* szHostname = getenv("SOUNDSERVER") ?
		getenv("SOUNDSERVER") : "127.0.0.1";
	struct hostent * myHostent = gethostbyname(szHostname);
	char **cp = myHostent->h_addr_list;
	sprintf(ipAddr, "%s", inet_ntoa(*(struct in_addr *)(*cp)));
	szHostname = ipAddr;
	*pobj = BgnMsgsend(szHostname, wChannel);
	return *pobj != 0;
}

extern int FMsgrcv23(void*); /* defined in 3.0 libsnd.a */

int BeginSoundServer23()
{
	const int wSendChannel = 7999; // from vss 2.x vssMsg.c++
	OBJ vobj;
	if (!FInitUdp23(&vobj, wSendChannel))
		return 0;
	OBJ* vpobj = &vobj;
	mm23 mmT;
	mmT.name[0] = '_';
	mmT.name[1] = '\0';
	const int PingMsg = -6; // from vss 2.x vssMsg.h
	((int*)mmT.name)[1] = PingMsg;

	/* Msgsend(NULL, &mmT, 0); */
		{
		desc* o = (desc*)*vpobj;
		struct sockaddr_in* paddr = &o->addr;
		if(!sendudp(paddr, o->sockfd, cchmm23 + sizeof(float), &mmT))
			return 0;
		}

	/*
	 *  This will work, but may call clientMessageCall() with a spurious string
	 *  as an argument. But who cares, vss will die in nanoseconds anyway.
	 */
	return FMsgrcv23(vobj);
}

void killSoundServer23()
{
	const int A_KillServer = 74; // from vss 2.x ACTOR/actorMessages.h
	// Implement actorMessage(A_KillServer, "").
	mm23 mmT;
	mmT.name[0] = 'A';
	((int*)(mmT.name))[1] = A_KillServer;
	char* formatPtr = mmT.name + sizeof(int)*2;
	*formatPtr = '\0';
	// unsigned char* paramPtr = mmT.param;
	// Msgsend(NULL, &mmT, ((float *)paramPtr)-mmT.param+1);
	// MsgsendObj(vpobj, NULL, (mm*)&mmT, 1);
		{
		desc* o = *vpobj;
		struct sockaddr_in* paddr = &o->addr;
		if (!sendudp(paddr, o->sockfd, cchmm23 + sizeof(float), &mmT))
			printf("vssKill: 2.x send failed\n");
		}
}

int pid2 = -1, pid3 = -1;
int fDone = 0;
int fKilled = 0;

void killvss2(void* pv)
{
	if (BeginSoundServer23()) {
		killSoundServer23();
		kill(pid3, SIGKILL);
		fKilled = 1;
	}
	fDone++;
}

void killvss3(void* pv)
{
	if (BeginSoundServer()) {
		killSoundServer();
		kill(pid2, SIGKILL);
		fKilled = 1;
	}
	fDone++;
}

int main()
{
	/*
	 *  If vss3.0 is running, it complains about 2.3 kill messages.
	 *  If vss2.x is running, it doesn't complain about 3.0 kill messages.
	 *  So give the 3.0 kill messages a head start.
	 */
	pid3 = sproc(killvss3, PR_SALL);
	sginap(5L);
	pid2 = sproc(killvss2, PR_SALL);
	while (!fKilled && fDone < 2)
		sginap(10L);
	return 0;
}

#else

int main()
{
	if (BeginSoundServer())
		killSoundServer();
	return 0;
}

#endif
