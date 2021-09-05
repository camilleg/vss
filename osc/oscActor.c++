// ? gotta change poll() to select(): cygwin's poll() is NYI.

#include "./client.h"

#include "VActor.h"

#include <fcntl.h>
#include <map>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>

#ifdef VSS_IRIX
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#endif

using namespace std;

const int cbBuf = 5000;

class OSCActor : public VActor
{
private:
	OSCbuf buf;
	OSCbuf* pbuf;
	char* pbBuf;
	int m_sockfd;
	struct sockaddr_in cl_addr;
	struct sockaddr_in addr;
	int fMaybeDisconnected;
public:
	OSCActor();
	~OSCActor();

	void setAddr(char* sz);

	void appendInt(int w)
		{ OSC_writeIntArg(pbuf, w); }
	void appendString(char* sz)
		{ OSC_writeStringArg(pbuf, sz); }
	void appendFloat(float z)
		{ OSC_writeFloatArg(pbuf, z); }
	void appendFloats(int cz, float* rgz)
		{ OSC_writeFloatArgs(pbuf, cz, rgz); }

	void send(void);

	void sendAddrInt(char* sz, int d);
	void sendAddrFloat(char* sz, float z);
	void sendAddrString(char* sz, char* szArg);
	void sendAddrFloatFloat(char* sz, float z1, float z2);

	virtual int receiveMessage(const char*);

	void init(char *szHostname, int channel=2002);
	void term(void);
};

#ifdef VSS_LINUX
	typedef unsigned clilenType;
#else
	typedef int clilenType;
#endif

class ClientAddr
{
public:
	struct sockaddr_in cl_addr;
	clilenType clilen;
	int sockfd;
	ClientAddr(struct sockaddr_in a, clilenType b, int c) :
		cl_addr(a), clilen(b), sockfd(c) {}
};

class CmdActor
{
	int cArg;
	string* m_args; // types of the arguments (F,I,S -- float,int,string)
	string* m_string; // command to send to actor
	string* m_msg; // trailing args

public:
	CmdActor(char* argsArg, char* cmdArg, char* msgArg)
		{ cArg = strlen(argsArg);
		  m_args = new string(argsArg);
		  m_string = new string(cmdArg);
		  m_msg = new string(msgArg); }
	~CmdActor()
		{ delete m_args;
		  delete m_string;
		  delete m_msg; }

	const char* Args() const
		{ return m_args->c_str(); }
	const char* Cmd() const
		{ return m_string->c_str(); }
	const char* Msg() const
		{ return m_msg->c_str(); }

	int Argtype(int i) const
		{ if (i<0 || i>=cArg) return -1;
		  char c = Args()[i];
		  //printf("\n\nc == %c\n", c);;;;
		  return c=='I'?0 : c=='F'?1 : c=='S'?2 : -1; }
};

typedef map<string, CmdActor*> Dict;

class OSCServer : public VActor
{
	private:
		int udp_port;
		int m_sockfd;
		clilenType clilen;
		struct sockaddr_in cl_addr;
		#define maxclilen (sizeof(struct sockaddr_in))
		char szMsgAddr[500];
		char szMsg[5000];
		char mbuf[32768];

		Dict dict;

		char* DataAfterAlignedString(char *string, char *boundary);
		int IsNiceString(char *string, char *boundary);
		void Smessage(CmdActor* pca, void *v, int n, ClientAddr* returnAddr);
		CmdActor* ParseOSCPacket(char *buf, int n, ClientAddr* returnAddr);
		void ProcessPca(CmdActor *);

	public:
		OSCServer() :
			VActor(),
			udp_port(-1),
			m_sockfd(-1),
			clilen(maxclilen)
			{ *szMsgAddr = *szMsg = *mbuf = '\0'; }
		void init(int port);
		void term(void);
		void setAddr(char* addr, char* args, char* cmd, char* msg);
		void rmAddr(char* addr);

		virtual void act(void);
		virtual int receiveMessage(const char*);
};

ACTOR_SETUP(OSCActor, OSCActor)
ACTOR_SETUP(OSCServer, OSCServer)

//////////////////////////////////////////////////////////////////////////


OSCActor::OSCActor() :
	 VActor(),
	 pbuf(&buf),
	 m_sockfd(-1)
{
	setTypeName("OSCActor");
	pbBuf = new char[cbBuf];
	if (!pbBuf)
		{
		fprintf(stderr, "vss error: OSCActor out of memory, crash imminent.\n");
		pbBuf = NULL;
		}
	OSC_initBuffer(pbuf, cbBuf, pbBuf);
}

OSCActor::~OSCActor()
{
	delete [] pbBuf;
}

void OSCActor::init(char *szHostname, int channel)
{
	if (m_sockfd >= 0)
		{
		fprintf(stderr, "vss warning: ignoring extra OSCActor Init\n");
		return;
		}

	memset((char *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
#ifdef OLD_WAY
	addr.sin_addr.s_addr = inet_addr(szHostname);
#else
	{
	// szHostname can be "nlx2.ncsa.uiuc.edu" or "141.142.223.20"
	struct hostent *hostsEntry = gethostbyname(szHostname);
	if (!hostsEntry)
		{
		fprintf(stderr, "vss error: unknown host name \"%s\"\n", szHostname);
		return;
		}
	unsigned long addrT = *((unsigned long *) hostsEntry->h_addr_list[0]);
	addr.sin_addr.s_addr = addrT;
	}
#endif
	addr.sin_port = htons(channel);
	if ((m_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
		fprintf(stderr, "vss error: unable to make socket.  ");
		perror("\n");
		return;
		}

	memset((char *)&cl_addr, 0, sizeof(cl_addr));
	cl_addr.sin_family = AF_INET;
	cl_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	cl_addr.sin_port = htons(0);
	if (bind(m_sockfd, (struct sockaddr *)&cl_addr, sizeof(cl_addr)) < 0)
		{
		fprintf(stderr, "vss error: unable to bind socket.  ");
		perror("\n");
		close(m_sockfd);
		m_sockfd = -1;
		return;
		}

//	fcntl(m_sockfd, F_SETFL, FNDELAY); /* Non-blocking I/O (man 5 fcntl) */
	if (isDebug())
		printf("vss remark: OSCActor connected to %s:%d\n", szHostname, channel);

#ifdef SANITY_CHECK
	// CAST hello-world
	setAddr("/documentation/all-messages");
	send();
#endif
}

void OSCActor::term(void)
{
	close(m_sockfd);
	m_sockfd = -1;
}

void OSCActor::setAddr(char* sz)
{
	//strcpy(szAddr, sz);
	OSC_resetBuffer(pbuf);
	OSC_writeAddress(pbuf, sz);
}

void OSCActor::sendAddrInt(char* sz, int d)
{
	setAddr(sz);
	appendInt(d);
	send();
}

void OSCActor::sendAddrFloat(char* sz, float z)
{
	setAddr(sz);
	appendFloat(z);
	send();
}

void OSCActor::sendAddrString(char* sz, char* szArg)
{
	setAddr(sz);
	appendString(szArg);
	send();
}

void OSCActor::sendAddrFloatFloat(char* sz, float z1, float z2)
{
	setAddr(sz);
	appendFloat(z1);
	appendFloat(z2);
	send();
}

#ifdef VSS_LINUX
#define LINUX_PORT_HACK_SND (const struct sockaddr *)
#define LINUX_PORT_HACK_RCV (      struct sockaddr *)
#else
#define LINUX_PORT_HACK_SND (const sockaddr *)
#define LINUX_PORT_HACK_RCV (      sockaddr *)
#endif
#include <cerrno>

void OSCActor::send(void)
{
	if (m_sockfd < 0)
		{
		fprintf(stderr, "vss error: OSCActor not connected to an OSC server.\n");
		return;
		}

	int cb0 = OSC_packetSize(pbuf);
	char* pb = OSC_getPacket(pbuf);

//	int err=0; int errlen=4;
//	getsockopt(m_sockfd, SOL_SOCKET, SO_ERROR, &err, &errlen);
//	printf("before sendto, socket error status is %d %d\n", err, errlen);

	int cb = sendto(m_sockfd, pb, cb0, 0, LINUX_PORT_HACK_SND &addr, sizeof(addr));
	if (cb != cb0)
		{
		fprintf(stderr, "vss error: OSCActor failed to sendto() to OSC server.\n");
		if (isDebug())
			{
			if (cb>=0)
				fprintf(stderr, "%d of %d bytes sent, ", cb, cb0);
			//fprintf(stderr, "errno=%d: ", errno);
			if (errno != ECONNREFUSED || fMaybeDisconnected <= 20)
			perror("");
			}

		if (errno == ECONNREFUSED)
			{
			if (fMaybeDisconnected <= 20)
				cerr << "vss remark: the OSC server at that host/port may not be running.\n";
			fMaybeDisconnected++;
			if (fMaybeDisconnected == 20)
				cerr << "vss remark: no more remarks about Connection refused from OSC server.\n";
			}
		else
			fMaybeDisconnected = 0;
		}
}

int OSCActor::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("Init"))
		{
		ifSD(szHost, port, init(szHost, port));
		ifS(szHost, init(szHost));
		return Uncatch();
		}
	if (CommandIs("Term"))
		{
		ifNil( term() );
		}

	if (CommandIs("Addr"))
		{
		ifS(sz, setAddr(sz) );
		return Uncatch();
		}

	if (CommandIs("Int"))
		{
		ifD(w, appendInt(w) );
		return Uncatch();
		}

	if (CommandIs("String"))
		{
		ifS(sz, appendString(sz) );
		return Uncatch();
		}

	if (CommandIs("Float"))
		{
		ifF(z, appendFloat(z) );
		return Uncatch();
		}

	if (CommandIs("Floats"))
		{
		ifFloatArray(rgz, cz, appendFloats(cz, rgz) );
		return Uncatch();
		}

	if (CommandIs("Send"))
		{
		ifNil( send() );
		}

	// Convenient abbreviations

	if (CommandIs("AddrIntSend"))
		{
		ifSD(sz, w, sendAddrInt(sz, w) );
		return Uncatch();
		}

	if (CommandIs("AddrFloatSend"))
		{
		ifSF(sz, z, sendAddrFloat(sz, z) );
		return Uncatch();
		}

	if (CommandIs("AddrStringSend"))
		{
		ifSS(sz, szArg, sendAddrString(sz, szArg) );
		return Uncatch();
		}

	if (CommandIs("AddrFloatFloatSend"))
		{
		ifSFF(sz, z1, z2, sendAddrFloatFloat(sz, z1, z2) );
		return Uncatch();
		}

	return VActor::receiveMessage(Message);
}

//////////////////////////////////////////////////////////////////////////
//
// OSCServer is derived from Matt Wright's dumpOSC.c

void OSCServer::init(int port)
{
	if (m_sockfd >= 0)
		{
		fprintf(stderr, "vss warning: ignoring extra OSCServer Init\n");
		return;
		}
	if((m_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
		return;
		}
	struct sockaddr_in serv_addr;
	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port);
	if(bind(m_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		{
		perror("vss error: OSCServer unable to bind to port");
		m_sockfd = -1;
		return;
		}
	fcntl(m_sockfd, F_SETFL, FNDELAY);
	if (isDebug())
		printf("OSCServer listening on port %d.\n", port);
}

void OSCServer::term(void)
{
	close(m_sockfd);
	m_sockfd = -1;
}

char* OSCServer::DataAfterAlignedString(char *string, char *boundary)
{
#define STRING_ALIGN_PAD 4
	/* The argument is a block of data beginning with a string.  The
	   string has (presumably) been padded with extra null characters
	   so that the overall length is a multiple of STRING_ALIGN_PAD
	   bytes.  Return a pointer to the next byte after the null
	   byte(s).  The boundary argument points to the character after
	   the last valid character in the buffer---if the string hasn't
	   ended by there, something's wrong, so return 0. */

	if ((boundary - string) %4 != 0)
		{
		fprintf(stderr, "vss error: OSCServer Internal error: DataAfterAlignedString: bad boundary\n");
		return 0;
		}

	int i;
	for (i = 0; string[i] != '\0'; i++)
		{
		if (string + i >= boundary)
			{
			fprintf(stderr, "vss error: OSCServer got very long string.\n");
			return 0;
			}
		}

	// Now string[i] is the first null character.
	for (i++; (i % STRING_ALIGN_PAD) != 0; i++)
		{
		if (string + i >= boundary)
			{
			fprintf(stderr, "vss error: OSCServer got very long string\n");
			return 0;
			}
		if (string[i] != '\0')
			{
			fprintf(stderr, "vss error: OSCServer got incorrectly padded string\n");
			return 0;
			}
		}
	return string+i;
}

int OSCServer::IsNiceString(char *string, char *boundary)
{
	/* Arguments same as DataAfterAlignedString().  Is the given "string"
	   really a string?  I.e., is it a sequence of isprint() characters
	   terminated with 1-4 null characters to align on a 4-byte boundary? */

	if ((boundary - string) %4 != 0)
		{
		fprintf(stderr, "vss error: OSCServer got bad string boundary\n");
		return 0;
		}

	int i;
	for (i = 0; string[i] != '\0'; i++)
		{
		if (!isprint(string[i]))
			return 0;
		if (string + i >= boundary)
			return 0;
		}

	// If we made it this far, it's a null-terminated sequence of printing characters
	// in the given boundary.  Now we just make sure it's null padded...

	// Now string[i] is the first null character.
	for (i++; (i % STRING_ALIGN_PAD) != 0; i++)
		if (string[i] != '\0')
			return 0;

	return 1;
}

void OSCServer::Smessage(CmdActor* pca, void *v, int n, ClientAddr* /*returnAddr*/)
{
	char* string=NULL;
	char* nextString=NULL;
	char sz[40];

	/* Go through the arguments a word at a time */
	float* floats = (float*)v;
	int* ints = (int*)v;
	char* chars = (char*)v;

	int iArg=0;
	int fGuess = 0; // if true, we're crudely guessing what the type of
					// the next param is, by ruling out unlikely values.

	for (int i = 0; i<n/4; )
		{
		string = &chars[i*4];

		if (!fGuess)
			{
			int argtype = pca->Argtype(iArg++);
			if (argtype<0)
				cerr <<"vss warning: OSCServer args don't match the arg string \""
					 <<pca->Args() <<"\"\n";
		//	else
		//		printf("Argtype is %c\n", "IFS"[argtype]);

			if (argtype==0 || argtype==1)
				ints[i] = ntohl(ints[i]); // &ints[i] == &floats[i]
			switch (argtype)
				{
			case 0: goto LInt;
			case 1: goto LFloat;
			case 2: goto LString;
			default: fGuess = 1; // not an I, F, or S; or past end of Args().
				}
			}

		if  (ints[i] >= -1000 && ints[i] <= 1000000)
			{
LInt:
			sprintf(sz, "%d ", ints[i]);
			strcat(szMsg, sz);
		//	if (isDebug())
		//		printf(sz);
//			if (floats[i]==1. || floats[i]==-1. || floats[i]==2. ||
//				(fabs(floats[i])<1e-10 && ints[i]!=0))
//				cerr <<"vss warning: OSCServer may have got a float when expecting an int.\n";
	//		fprintf(stderr, "%x %x %x %x yoyoyo\n",
	//			(int)string[0],
	//			(int)string[1],
	//			(int)string[2],
	//			(int)string[3]);;;;
		//	fprintf(stderr, "(%g)\n", floats[i]);
			i++;
			}
		else if (floats[i] >= -1000.f && floats[i] <= 1000000.f &&
			   (floats[i]<=0.0f || floats[i] >= 0.000001f))
			{
LFloat:
			sprintf(sz, "%f ", floats[i]);
			strcat(szMsg, sz);
		//	if (isDebug())
		//		printf(sz);
			i++;
			}
		else if (IsNiceString(string, chars+n))
			{
LString:
			nextString = DataAfterAlignedString(string, chars+n);
			strcat(szMsg, string);
			strcat(szMsg, " ");
		//	if (isDebug())
		//		printf("\"%s\" ", string);
			i += (nextString-string) / 4;
			}
		else
			{
			fprintf(stderr, "vss warning: OSCServer unguessable argument type for 0x%x\n", ints[i]);
			i++;
			}
		}
//	if (isDebug())
//		{
//		printf("\n");
//		fflush(stdout);
//		}
}


CmdActor* OSCServer::ParseOSCPacket(char *buf, int n, ClientAddr* returnAddr)
{
#ifdef VERBOSE
	fprintf(stderr, "{ ParseOSCPacket\n");
#endif
	if (n%4 != 0)
		{
		fprintf(stderr, "vss error: OSCServer packet not a multiple of 4 bytes.\n");
		return NULL;
		}

	if (n >= 8 && !strncmp(buf, "#bundle", 8))
		{
		// bundle message
		if (n < 16)
			{
			fprintf(stderr, "vss error: OSCServer packet with too-small bundle message.\n");
			return NULL;
			}
#if 0
		// This is unused.
		unsigned long long timetag = ntohl(*((unsigned long long *)(buf+8)));
		if (isDebug())
			printf("[ %llx\n", timetag);
#endif
		int i = 16; /* Skip "#group\0" and time tag */
		while (i<n)
			{
			int size = ntohl(*((int *) (buf + i)));
			if ((size % 4) != 0)
				{
				fprintf(stderr, "vss error: OSCServer packet with size count %d in bundle (not a multiple of 4)\n", size);
				return NULL;
				}
			if ((size + i + 4) > n)
				{
				fprintf(stderr, "vss error: OSCServer packet with bad size count %d in bundle (only %d bytes left in entire bundle)", size, n-i-4);
				return NULL;
				}
			// recurse into bundle.  Works for simple cases.
#ifdef VERBOSE
			fprintf(stderr, "{ recurse: ProcessPca ParseOSCPacket <%s>\n", szMsg);
#endif
			*szMsg = '\0';
			ProcessPca(ParseOSCPacket(buf+i+4, size, returnAddr));
#ifdef VERBOSE
			fprintf(stderr, "} recurse: ProcessPca ParseOSCPacket <%s>\n", szMsg);
#endif
			*szMsg = '\0';
			i += 4 + size;
			}
		if (i != n)
			fprintf(stderr, "vss warning: internal error in OSCServer\n");
		if (isDebug())
			printf("]\n");
#ifdef VERBOSE
fprintf(stderr, "} ParseOSCPacket (bundle!)\n");
#endif
		return NULL;
		}
	else
		{
		// not a bundle message
		char* messageName = buf;
		strcpy(szMsgAddr, messageName);
		if (isDebug())
			printf("OSCServer got a \"%s\"\n", szMsgAddr);
		Dict::const_iterator it = dict.find(szMsgAddr);
		if (it == dict.end())
			{
			fprintf(stderr, "vss warning: OSCServer ignoring address \"%s\".\n",
				szMsgAddr);
			return NULL;
			}
		CmdActor* pca = it->second;
		char* args = DataAfterAlignedString(messageName, buf+n);
		if (args == 0)
			{
			fprintf(stderr, "vss error: OSCServer packet with bad message name string\n");
			return NULL;
			}
		int messageLen = args-messageName;
		Smessage(pca, (void *)args, n-messageLen, returnAddr);
#ifdef VERBOSE
fprintf(stderr, "} ParseOSCPacket \n");
#endif
		return pca;
		}
}

void OSCServer::act(void)
{
	{
	if (m_sockfd <= 0)
		// Couldn't listen to the network.  Nothing to do.
		return;
	struct pollfd pfd;
	pfd.fd = m_sockfd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	int r = poll(&pfd, 1, 0);
	if (r < 0)
		{
		// poll() failed
		perror("vss error: OSCServer::act() poll() failed.");
		return;
		}
	if (r<=0 || !(pfd.revents & POLLIN))
		// nothing to receive
		return;
	}

	// something to receive
	int n;
	while (clilen = maxclilen, (n = recvfrom(m_sockfd, mbuf, sizeof(mbuf)-2, 0, LINUX_PORT_HACK_RCV &cl_addr, &clilen)) > 0)
		{
		// Accumulate args into szMsg.
		ClientAddr ras(cl_addr, clilen, m_sockfd);
		*szMsg = '\0';
#ifdef VERBOSE
fprintf(stderr, "nonrecursive ParseOSCPacket() call:\n");
#endif
		CmdActor* pca = ParseOSCPacket(mbuf, n, &ras);
		ProcessPca(pca);
		if (pca)
			clilen = maxclilen;
		}
}

void OSCServer::ProcessPca(CmdActor* pca)
{
	if (!pca)
		return;

	// Pass on args to another actor.
	char sz[6000];
	sprintf(sz, "%s %s %s", pca->Cmd(), pca->Msg(), szMsg);
	if (isDebug())
		printf("OSCServer(%s) sending \"%s\" to VSS\n", szMsgAddr, sz);
	actorMessageHandler(sz);
}

void OSCServer::setAddr(char* addr, char* args, char* cmd, char* msg)
{
	const string s(addr);
	CmdActor* pca = new CmdActor(args, cmd, msg);
	// memory leak!  should delete these, when dict is destroyed.

	dict[s] = pca; // add to dictionary
}

void OSCServer::rmAddr(char* addr)
{
#ifdef VSS_IRIX_53
// g++ generates >1k-long identifiers which irix5.3's assembler chokes on.
// So it's a bad memory leak here, but at least we can limp along.
	if (isDebug())
		cerr <<"OSCServer \"" <<addr <<"\" NOT removed in irix5.3 version.\n";
#else
	const string s(addr);
	delete dict[s];
	dict.erase(s);
	if (isDebug())
		cerr <<"OSCServer \"" <<s <<"\" removed.\n";
#endif
}

int OSCServer::receiveMessage(const char* Message)
{
	CommandFromMessage(Message);

	if (CommandIs("Init"))
		{
		ifD(port, init(port));
		return Uncatch();
		}
	if (CommandIs("Term"))
		{
		ifNil( term() );
		}

	// Bind this address (addr,args) to the message (cmd,msg).
	if (CommandIs("AddrCmdActor"))
		{
		ifSSSM(addr, args, cmd, msg, setAddr(addr,args,cmd,msg) );
		return Uncatch();
		}

	if (CommandIs("AddrRemove"))
		{
		ifS(sz, rmAddr(sz) );
		return Uncatch();
		}

	return VActor::receiveMessage(Message);
}
