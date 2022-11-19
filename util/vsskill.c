/***************************************
 *               vssKill.c             *
 * Terminate the server.               *
 * Terminate pre-3.0 servers too.      *
 ***************************************/

#include "vssClient.h"

#ifdef __sgi

int fKilled = 0;
int fDone = 0;
int pid2 = -1, pid3 = -1;

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

#define A_KillServer 74 /* same as in vss 2.x ACTOR/actorMessages.h */
#define PingMsg (-6)    /* same as in vss 2.x vssMsg.h */
#define wSendChannel 7999 /* same as in vss 2.x vssMsg.c++ */

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

OBJ* vpobj = NULL;
mm23 mmT;
OBJ vobj;

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
	return (*pobj != 0);
}

extern int FMsgrcv23(void* pv); /* defined in 3.0 libsnd.a */

int BeginSoundServer23(void)
{
	vpobj = NULL;
	if (!FInitUdp23(&vobj, wSendChannel))
		return 0;
	vpobj = &vobj;
	mmT.name[0] = '_';
	mmT.name[1] = '\0';
	((int *)mmT.name)[1] = PingMsg;

	/* Msgsend(NULL, &mmT, 0); */
		{
		desc *o = (desc*)*vpobj;
		struct sockaddr_in *paddr = &o->addr;
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
	/* actorMessage(A_KillServer, ""); */
	{
	unsigned char   *paramPtr;
	char            *formatPtr;
	mm23 Thingy;

	Thingy.name[0] = 'A';
	((int *)(Thingy.name))[1] = A_KillServer;
	formatPtr = Thingy.name + sizeof(int)*2;
	paramPtr = (unsigned char *)Thingy.param;
	*formatPtr = '\0';


	/* Msgsend(NULL, &Thingy, ((float *)paramPtr)-Thingy.param+1); */
	/* MsgsendObj(vpobj, NULL, (mm*)&Thingy, 1); */
		{
		desc *o = (desc*)*vpobj;
		struct sockaddr_in *paddr = &o->addr;
		if(!sendudp(paddr, o->sockfd, cchmm23 + sizeof(float), &Thingy))
			printf("vssKill: 2.x send failed\n");
		}
	}
}

void killvss2(void *pv)
{
	if (!BeginSoundServer23())
		goto LDone;

	killSoundServer23();
	kill(pid3, SIGKILL);
	fKilled = 1;
LDone:
	fDone++;
}

void killvss3(void *pv)
{
	if (!BeginSoundServer())
		goto LDone;

	killSoundServer();
	kill(pid2, SIGKILL);
	fKilled = 1;
LDone:
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
