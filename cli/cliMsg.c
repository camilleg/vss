#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <netdb.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
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
int currentServerHandle = 0; // null value

typedef struct
{
	struct sockaddr_in addr;
	int len;
	int sockfd;
	int channel; // UDP port
} desc;

// Small enough to avoid fragmentation when MTU is typically 1500 bytes.
// Not a const int, because C not C++.
#define MAXMESG 500

#define fFalse 0
#define fTrue 1

#ifdef VSS_WINDOWS_OLDWAY
#include <windows.h>
#include <winsock.h>
#endif

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
// Caller is responsible for free()ing the return value.
OBJ BgnMsgsend(const char *hostname, int channel)
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
	o->addr.sin_addr.s_addr = inet_addr(hostname);
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

	fcntl(sockfd, F_SETFL, FNDELAY); // Non-blocking I/O
	o->sockfd = sockfd;
	o->len = sizeof(o->addr);
#ifdef NOISY
	printf("will send to %s:%d\n", inet_ntoa(o->addr.sin_addr), o->channel);
#endif
	return o;
}

static void EndMsgsend(OBJ obj)
{
	close(((desc*)obj)->sockfd);
	free(obj);
}

static int sendudp(struct sockaddr_in* sp, int sockfd, size_t count, const void* b)
{
#ifdef NOISY
	printf("sendto %s:%d '%s', cb=%lu\n", inet_ntoa(sp->sin_addr), ntohs(sp->sin_port), (const char*)b, count);
#endif
	return sendto(sockfd, b, count, 0, (const struct sockaddr*)sp, sizeof(*sp)) == count ? fTrue : fFalse;
}

// Shorten a message, at least for legibility when debugging.
void VSS_StripZerosInPlace(const char* sz)
{
#if 1
	// Do nothing.
#else
	// todo: rewrite, not in place, because sz might be a literal, which is const or acts like const.
	char szT[MAXMESG + 5];
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
#else
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
#endif
		}

	/* Copy back into original string. */
	*pchDst = '\0';
	strcpy(sz, szT);
#endif
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void MsgsendObj(OBJ obj, struct sockaddr_in* paddr, const char* msg)
{
	if (!obj)
		return;
	desc* o = (desc*)obj;
	if (!paddr)
		paddr = &o->addr;
	VSS_StripZerosInPlace(msg);
	const size_t cb = strlen(msg) + 1; // +1 is the terminating null.
#ifdef VERBOSE
printf("\t\033[32msend \"%s\"\033[0m\n", msg);
#endif
	if (!sendudp(paddr, o->sockfd, cb, msg))
		perror("VSS client error: MsgsendObj send failed");
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void Msgsend(struct sockaddr_in* paddr, const char* msg)
{
	if (vpobj)
		MsgsendObj(*vpobj, paddr, msg);
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void MsgsendArgs1(struct sockaddr_in* paddr, const char* msg, float z0)
{
	char sz[MAXMESG];
	sprintf(sz, "%s %f", msg, z0);
	Msgsend(paddr, sz);
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void MsgsendArgs2(struct sockaddr_in* paddr, const char* msg, float z0, float z1)
{
	char sz[MAXMESG];
	sprintf(sz, "%s %f %f", msg, z0, z1);
	Msgsend(paddr, sz);
}

static struct timeval vtimeout = { 2L, 500000L };

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void SetReplyTimeout(float z)
{
	vtimeout.tv_sec = z;
	vtimeout.tv_usec = 1000000.0 * (z - (long)z);
	fprintf(stderr, "VSS client remark: timeout set to %g seconds.\n", z);
}

int FMsgrcv(void)
{
	if (!vpobj)
		return fFalse;
	const desc* o = (desc*)(*vpobj);
	fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_SET(o->sockfd, &read_fds);            

	// Assign the struct before every select() call, the manpage advises.
	struct timeval timeout;
	timeout.tv_sec = vtimeout.tv_sec;
	timeout.tv_usec = vtimeout.tv_usec;

	const int r = select(o->sockfd+1, &read_fds, (fd_set *)0, (fd_set *)0, &timeout);
	if (r <= 0)
		return fFalse;
	if (!FD_ISSET(o->sockfd, &read_fds)) {
		fprintf(stderr, "VSS client: internal error 4\n"); // Should not happen.
		return fFalse;
	}

	bool fGotSomething = false;
	int n = 0;
	unsigned clilen;
	struct sockaddr_in cl_addr;
	char rgchT[MAXMESG];
	while (clilen = sizeof(cl_addr),
		(n = recvfrom(o->sockfd, rgchT, MAXMESG, 0, (struct sockaddr*)&cl_addr, &clilen)) > 0) {
		fGotSomething = true;
		// Accept messages from pre-2024 clients, which are prefixed with a byte that's now ignored.
		const char firstbyte = rgchT[0];
		const char* msg = firstbyte == 0x00 || firstbyte == 0x01 ? rgchT+1 : rgchT;
		clientMessageCall(msg);
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

static int FInitUdp(OBJ* pobj, const char* szHostname, int wChannel)
{
	if (!szHostname || !*szHostname)
		{
		char* szEnv = getenv("SOUNDSERVER");
		szHostname = szEnv ? szEnv : "127.0.0.1"; // Default to localhost.
		const char* pch = strchr(szHostname, ':');
		if (pch)
			{
			const int w = atoi(pch+1);
			if (w > 0 && w < 1000000)
				{
				// What's after the colon is a port number.
				if (w < 1000 || w > 65535)
					fprintf(stderr, "VSS client warning: port %d out of range 1000 to 65535, defaulting to %d.\n",
						w, wChannel);
				else
					{
					wChannel = w;
					wSendChannel = w;
					}
				}
			else
				fprintf(stderr, "VSS client error: expected host or host:port, not \"%s\".\n", szHostname);
			}
		}

	if (isalpha(szHostname[0]))
		{
#ifdef VSS_WINDOWS
		fprintf(stderr, "VSS client error: Windows needs a dotted-quad address, not a name.\n");
#else
		struct hostent* myHostent = /*schmoo*/gethostbyname(szHostname);
		if (!myHostent)
			{
			fprintf(stderr, "VSS client error: host name \"%s\" doesn't exist?\n",
				szHostname);
			herror("VSS client error: probably an invalid value for $SOUNDSERVER");
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
static char vszDataReply[MAXMESG] = {0};
static int AckPrint = 0;
static void AckNote(float returnVal)
{
	if(AckPrint)
		printf("AckNote got %g.\n", returnVal);
	/* stuff it in this global */
	vhnote = returnVal;
}

static void AckDataReply(const char* returnVal)
{
	if(AckPrint)
		printf("AckDataReply got \"%s\".\n", returnVal);
	strncpy(vszDataReply, returnVal, MAXMESG-1);
	vszDataReply[MAXMESG-1] = '\0';
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void setAckPrint(int flag)
{
	AckPrint = flag;
}

float HnoteFromAckNote(void)
{
	return vhnote;
}

const char* SzFromDataReply(void)
{
	return vszDataReply;
}

#include "vssClient_int.h"

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
int PingSoundServer(void)
{
        Msgsend(NULL, "Ping");
        return FMsgrcv();
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void AUDterminateImplicit(void);

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
void EndSoundServer(void)
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
void EndAllSoundServers(void)
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
static int BeginSoundServerCore(const char* hostName)
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
		printf("VSS client error: no vss on %s",
			hostName && *hostName ? hostName : getenv("SOUNDSERVER") ? getenv("SOUNDSERVER") : "localhost");
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
int BeginSoundServerAt(const char* hostName)
{
	return BeginSoundServerCore(hostName);
}

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C"
#endif
int BeginSoundServer(void)
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
void clientMessageCall(const char* Message)
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
