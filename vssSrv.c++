#ifdef VSS_WINDOWS
#include <windows.h>
#include <windowsx.h>
#endif

#include <sys/types.h>

#include <csignal>
#include <cstring>

#include <unistd.h>

#ifdef VSS_IRIX
#include <sys/resource.h> // for prctl()
#include <sys/prctl.h>
#endif

#ifdef VSS_LINUX
#include <sys/mman.h>
#endif

#include "platform.h"
#include "VActor.h"
#include "VHandler.h"
#include "VAlgorithm.h"
#include "vssSrv.h"
#include "vssMsg.h"

// for open(2)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <climits>

static OBJ udpDescObj = nullptr;

VSSglobals globs;

VSSglobals::VSSglobals()
{
	// Always update SampleRate and OneOverSR together.
	SampleRate = 44100.0f;
	OneOverSR = 1.0f / SampleRate;
#ifdef VSS_WINDOWS
	nchansVSS = nchansIn = nchansOut = 2;
#else
	nchansVSS = nchansIn = nchansOut = 1;
#endif
	SampleCount = 0L;
	ofile_enabled = false;
	liveaudio = true;
	smax = INT_MAX;
	hog = 0;
	lwm = 384;
	hwm = 1024;
	msecAntidropout = 0.;
	hostname = "127.0.0.1";
	udp_port = 7999;
	fdOfile = -1;
	vcbBufOfile = 0;
	vibBufOfile = 0;
	rgbBufOfile = nullptr;
	ofile[0] = '\0';
}

VSSglobals::~VSSglobals() {
	delete [] rgbBufOfile; // smart pointer? uniq pointer? (;;;; after schedulerMain args)
}

// Reply to a client's ping.
void PingServer(struct sockaddr_in *cl_addr)
{
	mm mmT;
	mmT.fRetval = 0;
#if 0
	const auto addr = inet_ntoa(cl_addr->sin_addr);
	cerr << "vss: ping from " << (strcmp(addr, "127.0.0.1") ? addr : "local client") << "\n";
#endif
	sprintf(mmT.rgch, "AckNoteMsg %f", hNil);
	MsgsendObj(udpDescObj, cl_addr, &mmT);
}

void FlushServer()
{
	VActor::WantToFlush();
}

void Srv_UpdateMasterVolume(float gain)
{
	VSS_SetGlobalAmplitude(ScalarFromdB(gain));
}

void AddToCreatedlist(const char*) {}

// Irix 5.3 and 6.2 needs sproc, everything else can use pthreads.
#if defined(VSS_IRIX_53) || defined (VSS_IRIX_62)
//|| defined (VSS_IRIX_63)

#include <sys/types.h>
#include <sys/prctl.h>

static void SynthThread(void *)
{
	schedulerMain();
}

#elif defined(VSS_WINDOWS)
#else // VSS_LINUX, etc.

#include <pthread.h>
#include <ctime>
#include <sys/resource.h>

static void* SynthThread(void *)
{
	// In IRIX don't bother with schedctl(), setpriority() suffices.
	if (setpriority(PRIO_PROCESS, 0, -10 /* -20 to 0 */) == 0)
		cerr << "vss: set priority to -10\n";
	schedulerMain();
	return NULL;
}
#endif

#ifdef VSS_IRIX
#include <sys/utsname.h>
#endif

int VSS_main(int argc,char *argv[])
{
#ifdef VSS_IRIX
	{
	struct utsname name;
	(void)uname(&name);
	char* sz = name.release;
#if defined VSS_IRIX_65
	if (!(sz[0]=='6' && sz[1]=='.' &&sz[2] >= '5'))
		{
		fprintf(stderr, "This copy of VSS requires IRIX 6.5 or greater.\n");
		return -1;
		}
#elif defined VSS_IRIX_63
	if (!(sz[0]=='6' && sz[1]=='.' &&sz[2] >= '3'))
		{
		fprintf(stderr, "This copy of VSS requires IRIX 6.3 or greater.\n");
		return -1;
		}
#elif defined VSS_IRIX_62
	if (!(sz[0]=='6' && sz[1]=='.' &&sz[2] == '2'))
		{
		fprintf(stderr, "This copy of VSS requires IRIX 6.2.\n");
		return -1;
		}
#endif
	}
#endif

	ParseArgs(argc, argv, &globs.udp_port, &globs.liveaudio,
		&globs.SampleRate, &globs.nchansVSS, &globs.nchansIn, &globs.nchansOut, &globs.hog, &globs.lwm, &globs.hwm, globs.ofile);
	globs.OneOverSR = 1.0 / globs.SampleRate;
	udpDescObj = BgnMsgsend(globs.hostname, globs.udp_port);
	if (!udpDescObj)
		return -1;

	fprintf(stderr, 
		"%s, built %s\n  (C) 2021 U. of Illinois Board of Trustees\n  github.com/camilleg/vss\n",
		GetVssLibVersion(),
		GetVssLibDate());

#if defined VSS_IRIX_53 || defined VSS_IRIX_62
//|| defined (VSS_IRIX_63)
	prctl(PR_SETEXITSIG, SIGINT);
	SynthThread(NULL);
#elif defined VSS_WINDOWS
	schedulerMain();
#else
	// VSS_IRIX_63, VSS_LINUX, etc.
	SynthThread(NULL);
#endif

	return 0;
}

void DumpServerStats()
{
	printf("[31m");
	printf("SR %g, 1/1/SR %g\n", globs.SampleRate, 1./globs.OneOverSR);
	printf("%s%s%s%swater=%d/%d  host=%s\n",
		globs.liveaudio ? "" : "NOT LIVE  ",
		*globs.ofile ? globs.ofile : "",
		globs.hog > 0 ? "HOG  " : "",
		globs.nchansVSS == 1 ? "" :
			globs.nchansVSS == 2 ? "stereo  " :
			globs.nchansVSS == 4 ? "quad  " :
			globs.nchansVSS == 8 ? "8-channel  " : "BOGUS_NCHANS  ",
		globs.lwm,
		globs.hwm,
		globs.hostname);

	printf("[0m\n");
}


// If >=1, print each message as it is received.
int printCommands = 0;

// Handle to return to client.
static float vzReturnToClient = hNil;
float ClientReturnVal()
{
	return vzReturnToClient;
}

// Data to return to client.
static char vszReturnToClient[cchmm] = {0};
const char* ClientReturnValString()
{
	if (vszReturnToClient[0] != '\0')
		return vszReturnToClient;
	// There was no string, so instead return the float.  -kel 12 Oct 99
	static char s[64];
	sprintf(s, "%g", vzReturnToClient);
	return s;
}

static struct sockaddr_in* vcl_addr;

void ReturnFloatMsgToClient(float z, const char* msg)
{
	mm mmT;
	mmT.fRetval = 0;

	if (!strcmp(msg, "AckMidiInputMsg"))
		{
#ifdef needed_eventually_for_xx30

		// send the msgs to the client, which should call FMsgrcv()
		// and MidiMsgsFromAckMidi() to get them.

		int cch;
		char* pch;
		int cb = z * sizeof(VSSMDevent);

		if (cb > 10000/2 - 30)
			{
			cerr << "buffer overflow in MidiGetMsgs()\n";
			return;
			}
		sprintf(mmT.rgch, "AckMidiInputMsg %d %n", cMsg, &cch);
		pch = mmT.rgch + cch;

		extern VSSMDevent* vpMsg; // in midiActor.c++
		for (ib=0; ib<cb; ib++)
			{
			int b = ((char*)vpMsg)[ib];
			pch[ib*2  ] = (b >> 4) + '0';
			pch[ib*2+1] = (b & 0xf) + '0';
			}

		// Encode the midi data as an ascii string.
		MsgsendObj(udpDescObj, vcl_addr, &mmT);
		//;; is this ok instead? MsgsendObj(NULL, &mmT);
#endif
		}
	else if (!strcmp(msg, "AckNoteMsg"))
		{
		sprintf(mmT.rgch, "%s %f", msg, z);
		//printf("replying on port=%d\n", ((struct sockaddr_in *)vcl_addr)->sin_port );;
		MsgsendObj(udpDescObj, vcl_addr, &mmT);
		}
	else
		cerr << "vss: ignored ReturnFloatMsgToClient with unexpected args: " << msg << "\n";
}

void ReturnSzMsgToClient(const char* sz, const char* msg)
{
	mm mmT;
	mmT.fRetval = 0;
	sprintf(mmT.rgch, "%s %s", msg, sz);
	MsgsendObj(udpDescObj, vcl_addr, &mmT);
}

// Called by scheduler.
extern void doActors(void)
{
	VHandler::allAct();
	VActor::allAct();
}

// Clean up any dangling pointers, in a second pass.
// Kind of like real-time garbage collection.
extern void doActorsCleanup(void)
{
	VActor::allActCleanup();
}

extern void deleteActors(void)
{
	VActor::flushActorList();
}

//===========================================================================
//		internal message handling
//	
//	actorMessageMM() returns 1 almost always, zero means that we received a
//	message that should cause vss to exit. Return whatever actorMessageHandler()
//	returns.

// Called only by AmplAlg::generateSamples.
void ReturnStringToClient(const char* sz)
{
	strncpy(vszReturnToClient, sz, cchmm-1);
	vszReturnToClient[cchmm-1] = '\0';
	ReturnSzMsgToClient(vszReturnToClient, "DataReply");
}

void ReturnFloatToClient(float aFloat)
{
	//	if it is a float to be returned, make sure the
	//	string isn't hanging around.
	//	-kel 12 Oct 99
	vszReturnToClient[0] = '\0';
	vzReturnToClient = aFloat; // in case we need to return it to client.
}

float vzMessageGroupRecentHandle = hNil;
float* PvzMessageGroupRecentHandle()
{
	return &vzMessageGroupRecentHandle;
}

static bool vfAlreadyLogged = false;

// Called by LiveTick or BatchTick.
int actorMessageMM(const void* pv, struct sockaddr_in* cl_addr)
{
	const mm* pmm = (const mm*)pv;
	const auto message = pmm->rgch;
	const auto fReturnToClient = pmm->fRetval != 0;
	// Save this for ReturnFloatMsgToClient, and rarely ReturnSzMsgToClient and PingServer.
	// todo: replace this global with an arg to those and to actorMessageHandler, actorMessageHandlerCore.
	vcl_addr = cl_addr;

	// Don't postpone the \n to append "= vzReturnToClient", because
	// diagnostics from actorMessageHandler() would then start mid-line.
	if (printCommands >= 1)
		cerr << message << "\n";

	vfAlreadyLogged = true;
	const auto liveOn = actorMessageHandler(message);
	vfAlreadyLogged = false;

	if (fReturnToClient) {
		// hNil may mean an AUDupdate() "SendData mgFoo [...]" that wants a "*?".
		if (vzReturnToClient == hNil)
			vzReturnToClient = vzMessageGroupRecentHandle;
		ReturnFloatMsgToClient(vzReturnToClient, "AckNoteMsg");
		if (printCommands >= 1)
			cerr << "  = " << vzReturnToClient << "\n";
	}
	return liveOn;
}

#include "actorlist.h"

VActor* dummy(void) { return NULL; }

VActor* newActor(const char* szName)
{
	// search for szName in map.  If found, call its new().
	for (int i=0; i<cactor; ++i) {
		if (!strcmp(szName, m[i].name)) {
			const auto p = m[i].pfn();
			if (!p)
				cerr << "vss: failed to create a \"" << szName << "\" actor.\n";
			return p;
		}
	}
	cerr << "vss: unrecognized actor \"" << szName << "\"\n";
	return NULL;
}

static void InternalCreateActor(const char* s)
{
	auto anActor = newActor(s);
	if (!anActor)
		{
		ReturnFloatToClient(hNil);
		return;
		}
	auto aHandle = anActor->handle();
	if (aHandle == hNil)
		cerr << "vss error: nil handle to new actor " << s << "\n";
	ReturnFloatToClient(aHandle);
}

static void InternalEnableOfile(int d, const char* s, int cbBuf=0)
{
	if (d && !*s)
		{
		cerr << "vss: ignored EnableOfile without filename.\n";
		return;
		}

	globs.ofile_enabled = d;
	cerr << "vss remark: file output " << (globs.ofile_enabled ? "enabled" : "disabled") << "\n";

	if (globs.ofile_enabled)
		OpenOfile(s, cbBuf);
	else
		CloseOfile(s);
}

static void InternalSetPrintCommands(int d)
{
	printCommands = d;
}

static void Internal_SetGear(const char* sz)
{
	if (!strcmp(sz, "PRNDL_Parked"))
		VSS_SetGear(0);
	else if (!strcmp(sz, "PRNDL_Low"))
		VSS_SetGear(1);
	else if (!strcmp(sz, "PRNDL_Drive"))
		VSS_SetGear(2);
	else
		{
		cerr <<"vss error: unexpected SetGear \"" <<sz <<"\"\n" <<
			"\tTry one of these instead: PRNDL_Parked PRNDL_Low PRNDL_Drive.\n";
		}
}

static int fKeepRunning = 1;

//===========================================================================
//		actorMessageHandlerCore
//
//	Handle messages that are not actor-specific.
//
extern void DumpServerStats(void);

#define SPECIAL_TEST_FOR_RAT
#ifdef SPECIAL_TEST_FOR_RAT
static void foo1(int index, float value, float* cheatarray)
{
	cheatarray[index] = value;
	//printf("\t\tvss special cheat: storing %f at %d\n", value, index);
}

static void foo2(float hMg, float value)
{
	char sz[100];
	sprintf(sz, "SendData %f [%f]", hMg, value);
	actorMessageHandler(sz);
	//printf("\t\tvss special cheat: %s\n", sz);
}

static void foo3(float hMg, float value, float value2)
{
	char sz[100];
	sprintf(sz, "SendData %f [%f %f]", hMg, value, value2);
	actorMessageHandler(sz);
}

static void foo4(float hMg, float value, int value2)
{
	char sz[100];
	sprintf(sz, "SendData %f [%f %d]", hMg, value, value2);
	actorMessageHandler(sz);
}
#endif

int actorMessageHandlerCore(const char* Message)
{
	CommandFromMessage(Message, 1);

	if (CommandIs("KillServer"))
		{ ifNil( fKeepRunning = 0 ); }

	if (CommandIs("Ping"))
		{ ifNil( PingServer(vcl_addr) ); }

	// Backwards compatible.  Does nothing.
	if (CommandIs("LoadDSO"))
		{ ifS(s, (void)0 ); return Uncatch(); }

	if (CommandIs("SetPrintCommands"))
		{ ifD(d, InternalSetPrintCommands(d) ); return Uncatch(); }

	if (CommandIs("EnableOfile"))
		{
		ifDSD(d,s,cbBuf, InternalEnableOfile(d, s, cbBuf) );
		ifDS(d,s, InternalEnableOfile(d, s) );
		ifD(d, InternalEnableOfile(d, "") );
		return Uncatch();
		}

	if (CommandIs("Create"))
		{ ifS(s, InternalCreateActor(s) ); return Uncatch(); }

	if (CommandIs("SetMasterGain"))
		{ ifF(f, Srv_UpdateMasterVolume(f) ); return Uncatch(); }

	if (CommandIs("DumpAll"))		// print list of actors
		{
		VActor::curtainCall(cout);
		DumpServerStats();
		return Catch();
		}

	if (CommandIs("DeleteAllActors"))	// Delete all actors (don't document this one for now)
		{ ifNil( VActor::flushActorList() ); }

	if (CommandIs("SetGear"))		// change behavior of scheduler
		{
		ifS(sz, Internal_SetGear(sz) );
		return Uncatch();
		}

#ifdef SPECIAL_TEST_FOR_RAT

	static float cheatarray[100];

	// rat sends "StoreValue 0 hTalkingGuy" to vss
	if (CommandIs("StoreValue"))
		{
		ifDF(index, value, foo1(index, value, cheatarray));
		}

	if (CommandIs("GetValue"))
		{
		ifDF(index, hMg, foo2(hMg, cheatarray[index]));
		ifDFD(index, hMg, z, foo4(hMg, cheatarray[index], z));
		ifDFF(index, hMg, z, foo3(hMg, cheatarray[index], z));
		}

	// audpanel's audfile sends "GetValue 0 hReverbActor"
	// to cause "SetInput hReverbActor hTalkingGuy" to get sent.
	if (CommandIs("GetValue"))
		{
		//ifDF(index, hActor, foo2(hActor, cheatarray[index]));
		}

#endif

	// Not a vss builtin message, so see if it's sent to
	// an individual actor instance.
	return 2;
}

// Returns 0 iff Message tells vss to quit.
int actorMessageHandler(const char* Message)
{
	vzReturnToClient = hNil;
	fKeepRunning = 1;
	const auto caught = actorMessageHandlerCore(Message);
	// May have set vzReturnToClient or fKeepRunning.
	if (!fKeepRunning)
		return 0;

	if (printCommands >= 2 && !vfAlreadyLogged)
		cerr << Message << " (internal)\n";
	vfAlreadyLogged = false; // This was the 2nd time.  Permit later msgs to be printed.

	switch(caught)
		{
	case 0:
		cerr << "vss: ignored message with garbled args: \"" << Message << "\"\n";
		return 1;
	case 1:
		return 1;
	case 2:
		break;
	default:
		cerr << "vss: actorMessageHandler internal error\n";
		return 1;
		}

	if (!*Message)
		{
		// CommandFromMessage() complained already.
		return 1;
		}

	// Process a message sent to an individual actor.
	float aHandle;
	if (1 != sscanf(Message, "%*s %f", &aHandle))
		{
		cerr << "vss: ignored message lacking actor handle \"" << Message << "\"\n";
		return 1;
		}

	VActor* anActor = VActor::getByHandle(aHandle);
	if(!anActor)
		{
		cerr << "vss: ignored message with unknown actor handle " << aHandle << ": \"" << Message << "\"\n";
		return 1;
		}

	if (!anActor->receiveMessage(Message))
		{
		// VActor::receiveMessage complained already.
		return 1;
		}

	if (anActor->delete_me() /* set by VActor::receiveMessage */)
		delete anActor;
	return 1;
}

extern void OpenOfile(const char * fileName, int cbBuf)
{
	if (globs.fdOfile >= 0)
		CloseOfile(globs.ofile);
	if (!*fileName)
		return;
	globs.fdOfile = open(fileName, O_CREAT/*|O_TRUNC*/|O_WRONLY, 0666);
	if (globs.fdOfile < 0)
		{
		perror(fileName);
		*globs.ofile = '\0';
		return;
		}
	cerr << "vss: writing to " << fileName << "\n";
	strcpy(globs.ofile, fileName);

	if (cbBuf > 0)
		cerr << "vss: buffering " << int(cbBuf/globs.SampleRate/globs.nchansVSS) << " seconds of output.\n";
	globs.vcbBufOfile = cbBuf;
	globs.vibBufOfile = 0;
	globs.rgbBufOfile = new char[cbBuf];
}

extern void CloseOfile(const char * fileName)
{
	if (!*fileName)
		fileName = globs.ofile;
	else if (strcmp(fileName, globs.ofile))
		{
		cerr << "vss: failed to close output file " << fileName << ". Current output file is " << globs.ofile << ".\n";
		return;
		}

	if (globs.fdOfile >= 0)
		{
		cerr << "vss: wrote to " << globs.ofile << ".\n";
		if (globs.vcbBufOfile>0 && globs.vibBufOfile>0)
			{
			// flush and free memory buffer.  Ignore return value, because we're closing things down anyways.
			(void)!write(globs.fdOfile, globs.rgbBufOfile, globs.vibBufOfile);
			globs.vcbBufOfile=0;
			globs.vibBufOfile=0;
			delete [] globs.rgbBufOfile;
			globs.rgbBufOfile = NULL;
			}
		close(globs.fdOfile);
		globs.fdOfile = -1;

#define VSS_SPECIAL_CONVERSION_FROM_RAW_TO_AIFF
#ifdef VSS_SPECIAL_CONVERSION_FROM_RAW_TO_AIFF
		// todo: nice cmd-line flag for this.
			{
			char szCmd[1000];
			sprintf(szCmd, "/usr/bin/sox -e signed-integer -b 16 -c %d -r %d -t raw %s -t aiff %s.aiff",
				globs.nchansVSS, (int)globs.SampleRate, globs.ofile, globs.ofile);
			(void)!system(szCmd);
			unlink(globs.ofile);
			cerr << "vss: created " << globs.ofile << ".aiff.\n";
			}
#endif

		*globs.ofile= '\0';
		}
}
