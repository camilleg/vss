#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <netdb.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define NEED_RGSZMESSAGE
#include "cliMsg.h"

static OBJ* vpobj;
#define	MAX_NUM_SERVERS	100
static OBJ vobjArray[MAX_NUM_SERVERS];
static int numServers = 0;
int	currentServerHandle = 0;

typedef struct
{
	struct sockaddr_in addr;
	int len;
	int sockfd;
	int channel;
} desc;

#define fFalse 0
#define fTrue 1

#ifdef VSS_WINDOWS_OLDWAY
#include <windows.h>
#include <winsock.h>
#endif

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
OBJ BgnMsgsend(const char *szHostname, int channel)
{
	struct sockaddr_in  cl_addr;
	int  sockfd;
	desc *o = (desc *)malloc(sizeof(desc));
	if (!o)
		return (OBJ)0;

#ifdef VSS_WINDOWS_OLDWAY
	{
	WORD wVersionRequested;WSADATA wsaData;int err; 
	wVersionRequested = MAKEWORD( 2, 0 ); 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 )
		/* Tell the user that we couldn't find a usable WinSock DLL. */
		goto LCleanup;

	/* Confirm that the WinSock DLL supports 2.0.
	 * Note that if the DLL supports versions greater
	 * than 2.0 in addition to 2.0, it will still return
	 * 2.0 in wVersion since that is the version we requested. */
	if (LOBYTE( wsaData.wVersion ) != 2 ||
		HIBYTE( wsaData.wVersion ) != 0 )
		{
		/* Tell the user that we couldn't find a usable WinSock DLL. */
		WSACleanup();
LCleanup:
		free(o);
		return NULL;
		}
	/* The WinSock DLL is acceptable. Proceed. */
	}
#endif

	o->channel = channel;
	memset((char *)&o->addr, 0, sizeof(o->addr));
	o->addr.sin_family = AF_INET;
	o->addr.sin_addr.s_addr = inet_addr(szHostname);
	o->addr.sin_port = htons(channel);
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
		{
		memset((char *)&cl_addr, 0, sizeof(cl_addr));
		cl_addr.sin_family = AF_INET;
		cl_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		cl_addr.sin_port = htons(0);
		if (bind(sockfd, (struct sockaddr *)&cl_addr,
			sizeof(cl_addr)) < 0)
			{
			perror("can't bind");
			close(sockfd);
			sockfd = -1;
			}
		}
	else
		printf("unable to make socket\n");

#ifdef VSS_WINDOWS
	/* Make the socket nonblocking, using the impoverished ioctl in Windows.
	 * Don't let Windows manage the socket with its own message pump technique!
	 */
	fcntl(sockfd, F_SETFL, FNDELAY); /* Non-blocking I/O (man 5 fcntl) */
	{
	int f = 1;
	if (ioctl(sockfd, FIONBIO, &f) < 0) perror("ioctl failed");
#if defined(SET_SOCK_SND_BUF_SIZE)
	setsockopt(sockfd, F_SETFL, FNDELAY);
#endif
	}
#endif
	
	o->sockfd = sockfd;
	o->len = sizeof(o->addr);
#ifndef VSS_WINDOWS
	fcntl(sockfd, F_SETFL, FNDELAY); // Non-blocking I/O
#endif
#if defined(SET_SOCK_SND_BUF_SIZE)
	setsockopt(sockfd, F_SETFL, FNDELAY);
#endif
#ifdef NOISY
	printf("opened send socket fd %d on channel %d, obj %p\n",
		o->sockfd, o->channel, (void*)o);
#endif
	return o;
}

static void EndMsgsend(OBJ obj)
{
	close(((desc*)obj)->sockfd);
	free(obj);
}

static int sendudp(struct sockaddr_in *sp, int sockfd, long count, void  *b)
{
#ifdef NOISY
	printf("sendto \"%s\", fd=%d, cb=%ld,\t",
		((mm*)b)->rgch, sockfd, count);
	/* sockaddr_in defined in /usr/include/netinet/in.h */
	printf("port=%d, ipaddr=%x\n",
		(int)ntohs(sp->sin_port),
		(int)ntohl(sp->sin_addr.s_addr));
#endif
	if (sendto(sockfd, (const char*)b, (int)count, 0, (const struct sockaddr*)sp, sizeof(*sp)) != count)
		return fFalse;
	return fTrue;
}

/* This doesn't waste cpu, it just moves some of the parsing
   from the server side to the client side. */

void VSS_StripZerosInPlace(char* sz)
{
	char szT[sizeof(mm) + 5];
	char* pchSrc = sz;
	char* pchDst = szT;

	for ( ; ; /* *pchDst=='.' or ' '*/ *pchDst++ = *pchSrc++)
		{
		while (*pchSrc && *pchSrc != '.')
			*pchDst++ = *pchSrc++;

		if (!*pchSrc) /* end of string */
			break;

		/* We've reached a decimal point. */

		/* If it's NOT preceded by a space or a digit,
		 * don't abbreviate as it's probably NOT a floating-point number.
		 */
		if (!(pchSrc[-1] == ' ' || (pchSrc[-1]>='0' && pchSrc[-1]<='9')))
			continue;

#ifdef FANCY_AND_GENERAL_WAY
		/* Is it followed by zeros and whitespace? */
		while (*pchSrc == '0')
			pchSrc++;
		/* we've reached the end of any 0's following the decimal point */
		if (*pchSrc == ' ' || !*pchSrc)
			/* Aha! we reached a space.  Don't bother copying the ".000000". */
			continue;
#endif

		/* This way is faster, safer, and works almost all the time. */
		if (!strncmp(pchSrc, ".000000 ", 8))
			{
			pchSrc += 7;
			/* now *pchSrc == ' ' */
			continue;
			}
		if (!strcmp(pchSrc, ".000000"))
			/* end of string, just discard it. */
			break;
		}

	/* Copy back into original string. */
	*pchDst = '\0';
	strcpy(sz, szT);
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void MsgsendObj(OBJ obj, struct sockaddr_in *paddr, mm* pmm)
{
	desc* o = (desc *)obj;
	if (!obj)
		return;

	/* Strip trailing zeros for shorter more legible messages. */
	/* in vi:  s/\.[0]* / /g */

	VSS_StripZerosInPlace(pmm->rgch);

	if (paddr == NULL)
		paddr = &o->addr;
#ifdef VERBOSE
printf("\t\033[32msend %s \"%s\"\033[0m\n", pmm->fRetval ? "RET" : "   ", pmm->rgch);
#endif
	if(!sendudp(paddr, o->sockfd, (long)strlen(pmm->rgch)+1+1, pmm))
		/* the second +1 is for the fRetval field. */
		perror("VSS client error: MsgsendObj send failed");
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void Msgsend(struct sockaddr_in *paddr, mm* pmm)
{
	if (vpobj != NULL)
		MsgsendObj(*vpobj, paddr, pmm);
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void MsgsendArgs1(struct sockaddr_in *paddr, mm* pmm,
				const char* msg, float z0)
{
	sprintf(pmm->rgch, "%s %f", msg, z0);
	Msgsend(paddr, pmm);
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void MsgsendArgs2(struct sockaddr_in *paddr, mm* pmm,
				const char* msg, float z0, float z1)
{
	sprintf(pmm->rgch, "%s %f %f", msg, z0, z1);
	Msgsend(paddr, pmm);
}

static struct timeval vtimeout = { 2L, 500000L };

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void SetReplyTimeout(float z)
{
	vtimeout.tv_sec = (long)z;
	vtimeout.tv_usec = (long)(1000000. * (z - (float)(long)z));
	fprintf(stderr, "VSS client remark: timeout set to %g seconds.\n", z);
}

static int FMsgrcvCore(OBJ pv)
{
	desc *o;
	struct sockaddr_in  cl_addr;
	int r;
	fd_set read_fds;
	struct timeval timeout = { 2L, 500000L };

#	define MAXMESG 32768
	static char rgchT[MAXMESG];

	if (vpobj == NULL && pv == NULL)
		return fFalse;

	o = (desc *)(pv ? pv : *vpobj);
	FD_ZERO(&read_fds);
	FD_SET(o->sockfd, &read_fds);            
	/* Reassign the struct before every select() call, the manpage advises. */
	timeout.tv_sec = vtimeout.tv_sec;
	timeout.tv_usec = vtimeout.tv_usec;

	r = select(o->sockfd+1, &read_fds, (fd_set *)0, (fd_set *)0, &timeout);
	if (r <= 0)
		return fFalse;

	if (FD_ISSET(o->sockfd, &read_fds))
		{
		int fGotSomething = 0;
		int n = 0;
		unsigned clilen;
		while (clilen = sizeof(cl_addr),
			(n=recvfrom(o->sockfd, rgchT, MAXMESG, 0,
			(struct sockaddr*)&cl_addr, &clilen)) >0)
			{
			fGotSomething = 1;
			if (!pv)
				/* really a 3.0 client, not a 2.3 client */
				clientMessageCall(rgchT+1/*strip off fRetval with +1*/);
			}
		if (!fGotSomething && n < 0)
			{
#ifdef VSS_WINDOWS
			/* Don't complain: VSS probably just isn't running, that's all */
#else
			if (errno != ECONNREFUSED && errno != EWOULDBLOCK)
				perror("VSS client: internal error, FMsgrcv recvfrom() failed");
#endif
			return fFalse;
			}
		return fTrue;
		}
	/* This should not happen */
	fprintf(stderr, "VSS client: internal error 4\n");
	return fFalse;
}

int FMsgrcv()
{
	return FMsgrcvCore(NULL);
}

/* Secret back door so we can be a vss 2.3 client also, e.g. vssKill. */
int FMsgrcv23(void* pv)
{
	return FMsgrcvCore(pv);
}

#ifdef UNDER_CONSTRUCTION
struct hostent* schmoo(char* szHostname)
{
//	struct in_addr addr;
//	addr.S_un.S_addr = inet_addr(szHostname /*"141.142.220.33"*/);
    struct sockaddr_in sin;
//    u_long addr;

    sin.sin_family = AF_INET;
    if((sin.sin_addr.s_addr = inet_addr(szHostname)) != -1)
        return gethostbyaddr((const char*)&sin, 4, AF_INET);
	else
        return NULL;
}
#endif

int wSendChannel = 7999;

/* set up the UDP connection.  szHostname defaults to localhost. */
static int FInitUdp(OBJ* pobj, char* szHostname, int wChannel)
{
	if (!szHostname || !*szHostname)
		{
		char* szEnv = getenv("SOUNDSERVER");
		szHostname = szEnv ? szEnv : "127.0.0.1";
		char* pch = strchr(szHostname, ':');
		if (pch)
			{
			const int w = atoi(pch+1);
			if (w > 0 && w < 1000000)
				{
				// What's after the colon is a port number.
				if (w < 1000 || w > 65535)
					fprintf(stderr, "VSS client error: port number %d is out of range 1000 to 65535.  Using default %d instead.\n",
						w, wChannel);
				else
					{
					wChannel = w;
					wSendChannel = w;
					}
				}
			else
				fprintf(stderr, "VSS client error: host (and port?) name \"%s\" with colon unrecognized.\n", szHostname);
			}
		}

	if (isalpha(szHostname[0]))
		{
#ifdef VSS_WINDOWS
		fprintf(stderr, "VSS client error: Sorry, Windows needs a dotted-quad address, not a name\n");
#else
		struct hostent* myHostent = /*schmoo*/gethostbyname(szHostname);
		if (!myHostent)
			{
			fprintf(stderr, "VSS client error: host name \"%s\" doesn't exist?\n",
				szHostname);
			/*
				This "obsolete" (according to man pages on Linux)
				function prints an error message associated with
				the current value of h_errno on stderr.
			*/
#ifndef VSS_WINDOWS_OLDWAY
			herror("VSS client error: probably an invalid value for $SOUNDSERVER");
#endif
			return fFalse;
			}

		char** cp = myHostent->h_addr_list;
		char ipAddr[16];
		strcpy(ipAddr, inet_ntoa(*(struct in_addr *)(*cp)));
		szHostname = ipAddr;
#endif
		}

	*pobj = BgnMsgsend(szHostname, wChannel);
	return *pobj != 0;
}

static float vhnote = hNil;
static char vszDataReply[cchmm] = {0};
static int AckPrint = 0;
static void AckNote(float returnVal)
{
	if(AckPrint)
		printf("AckNote got %g.\n", returnVal);
	/* stuff it in this global */
	vhnote = returnVal;
}

static void AckDataReply(char* returnVal)
{
	if(AckPrint)
		printf("AckDataReply got \"%s\".\n", returnVal);
	/* stuff it in this global */
	strncpy(vszDataReply, returnVal, cchmm-1);
	vszDataReply[cchmm-1] = '\0';
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void setAckPrint(int flag)
{
	AckPrint = flag;
}

float HnoteFromAckNote()
{
	return vhnote;
}

const char* SzFromDataReply()
{
	return vszDataReply;
}

#include "vssClient_int.h"

/* Send a Ping msg without args. */
#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
int PingSoundServer()
{
	mm mmT;
	mmT.fRetval = 0; // Ignored.
	sprintf(mmT.rgch, "Ping");
        Msgsend(NULL, &mmT);
        return FMsgrcv();
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void AUDterminateImplicit();

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void EndSoundServer()
{
	if (!vpobj)
		{
		fprintf(stderr, "VSS client error: Sound server not attached!\n\t(Did you call EndSoundServer() more than once?)\n");
		return;
		}
	AUDterminateImplicit();
	EndMsgsend(*vpobj);
	*vpobj = NULL;
	vpobj = NULL;
	currentServerHandle = 0; /* Null handle */
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void EndAllSoundServers()
{
	int i;
	for (i = 0; i < numServers; i++)
		{
		vpobj = vobjArray + i;
		if (*vpobj)
			EndSoundServer();
		}
	numServers = 0;
}

/* Returns a server handle, or on error the null handle 0. */
static int BeginSoundServerCore(char* hostName)
{
	if (numServers >= MAX_NUM_SERVERS)
		{
		fprintf(stderr, "vss client error: Too many servers running to open another on %s.\n",
			hostName && *hostName ? hostName : "this host");
		return 0;
		}

	vpobj = NULL;
	if (!FInitUdp(vobjArray + numServers, hostName, wSendChannel))
		goto LAbort;

	vpobj = vobjArray + numServers;
	if (!PingSoundServer())
		{
LAbort:
		printf("VSS client error: no vss running on %s",
			hostName && *hostName ? hostName : getenv("SOUNDSERVER") ? getenv("SOUNDSERVER") : "this host");
		if (wSendChannel != 7999)
			printf(", port %d", wSendChannel);
		printf("\n");
		return 0;
		}

	currentServerHandle = ++numServers;
	return currentServerHandle;
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
int BeginSoundServerAt(char* hostName)
{
	return BeginSoundServerCore(hostName);
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
int BeginSoundServer()
{
	return BeginSoundServerCore(NULL);
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
int SelectSoundServer(int serverHandle)
{
	if (serverHandle > numServers)	return fFalse;
	if (serverHandle < 1)		return fFalse;
	/* handles range from 1 to MAX_NUM_SERVERS */

	vpobj = vobjArray + serverHandle - 1;
	currentServerHandle = serverHandle;
	return fTrue;
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void clientMessageCall(char* Message)
{
	if (!strncmp(Message, "AckNoteMsg ", 11))
		{
		float z = 0.;
		if (1 != sscanf(Message+11, "%f", &z))
			{
			fprintf(stderr, "vss client error: bad args to AckNoteMsg (%s)", Message);
			return;
			}
		AckNote(z);
		}

	else if (!strncmp(Message, "DataReply ", 10))
		{
#if 0
		// This test isn't needed, because lack of args is ok here, it's general-purpose.
		#define cchMax 2000
		char sz[cchMax];
		if (1 != sscanf(Message+10, "%[^]", sz))
			{
			fprintf(stderr, "vss client error: bad args to DataReply (string missing?) (%s)", Message);
			return;
			}
#endif
		AckDataReply(Message+10);
		}

	else
		fprintf(stderr, "vss client error: unrecognized message \"%s\" passed to clientMessageCall()\n", Message);
}
