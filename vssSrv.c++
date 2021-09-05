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

static OBJ udpDescObj = NULL;	// used to send udp messages

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
	rgbBufOfile = NULL;
	ofile[0] = '\0';
}

VSSglobals::~VSSglobals() {
	if (rgbBufOfile)
		delete [] rgbBufOfile;
}

static int sample(int length,  float* out, int nchans)
{
	VAlgorithmList::iterator it;
	for ( it = VAlgorithm::Generators.begin(); 
			it != VAlgorithm::Generators.end(); ++it )
	{
		(*it)->outputSamples(length, out, nchans);
	}

	globs.SampleCount += length;
	return 1;
}

// Server calls this in response to a "Ping" request from client, to reply.
void PingServer(struct sockaddr_in *cl_addr)
{
	mm mmT;
	mmT.fRetval = 0;
	char *addr = inet_ntoa(cl_addr->sin_addr);
	fprintf(stderr, "vss remark: ping from %s\n",
		strcmp(addr, "127.0.0.1") ? addr : "local client");
	sprintf(mmT.rgch, "AckNoteMsg %f", hNil);
	MsgsendObj(udpDescObj, cl_addr, &mmT);
}

void FlushServer(void)
{
	VActor::WantToFlush();
}

void Srv_UpdateMasterVolume(float newGain)
{
	VSS_SetGlobalAmplitude(ScalarFromdB(newGain));
}

void AddToCreatedlist(const char*) {}

// Irix 5.3 and 6.2 needs sproc, everything else can use pthreads.
#if defined(VSS_IRIX_53) || defined (VSS_IRIX_62)
//|| defined (VSS_IRIX_63)

#include <sys/types.h>
#include <sys/prctl.h>

static void SynthThread(void *)
{
	schedulerMain(globs, &sample);
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
		fprintf(stderr, "vss remark: set priority to -10\n");
	schedulerMain(globs, &sample);
	return NULL;
}
#endif

#ifdef VSS_IRIX
#include <sys/utsname.h>
#endif

extern "C" int VSS_main(int argc,char *argv[])
{
#ifdef VSS_LINUX
	// check that alsa or studi/o or whatever is installed (uname, /proc/pci, lsmod).
#endif

#ifdef VSS_IRIX
	// Run on only the version of IRIX this was compiled for.
	{
	struct utsname name;
	(void)uname(&name);
	char* sz = name.release;
#if defined(VSS_IRIX_65)
	if (!(sz[0]=='6' && sz[1]=='.' &&sz[2] >= '5'))
		{
		fprintf(stderr, "This copy of VSS requires IRIX 6.5 or greater.\n");
		return -1;
		}
#endif
#if defined(VSS_IRIX_63)
	if (!(sz[0]=='6' && sz[1]=='.' &&sz[2] >= '3'))
		{
		fprintf(stderr, "This copy of VSS requires IRIX 6.3 or greater.\n");
		return -1;
		}
#endif
#ifdef VSS_IRIX_62
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
	if ((udpDescObj = BgnMsgsend(globs.hostname, globs.udp_port)) == 0)
		return -1;

	fprintf(stderr, 
		"%s, built %s\n  (C) 2021 U. of Illinois Board of Trustees\n  github.com/camilleg/vss\n",
		GetVssLibVersion(),
		GetVssLibDate());

#if defined(VSS_IRIX_53) || defined (VSS_IRIX_62)
//|| defined (VSS_IRIX_63)
	prctl(PR_SETEXITSIG, SIGINT);
		SynthThread(NULL);
#elif defined(VSS_WINDOWS)
	schedulerMain(globs, &sample);
#else
	// VSS_IRIX_63, VSS_LINUX, etc.
		SynthThread(NULL);
#endif

	return 0;
}

extern void DumpServerStats(void)
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


//	If 1, print out all messages as they are received.
int printCommands = 0;

//===========================================================================
//		returning handles to clients
//

//	Handle to return to client. Initialize to hNil.
static float vzReturnToClient = hNil;
extern float ClientReturnVal(void)
{
	return vzReturnToClient;
}

//	Data to return to client.
static char vszReturnToClient[cchmm] = {0};
extern const char* ClientReturnValString(void)
{
	if ( vszReturnToClient[0] != '\0' )
		return vszReturnToClient;
	else {
		//	if there's no other string, 
		//	write the return float on a string
		//	and return that.
		//	-kel 12 Oct 99
		static char s[64];
		sprintf(s, "%g", vzReturnToClient);
		return s;
	}
}

static struct sockaddr_in *vcl_addr; // set in actorMessage()

// vcl_addr is set maybe quite a while before somebody like
// TRACKAMPLRMS_Actor::receiveMessage uses it by calling this function.
// It shouldn't be clobbered in the meantime, though.

extern "C" void ReturnFloatMsgToClient(float z, const char* msg)
{
	mm mmT;
	mmT.fRetval = 0; // Be hygienic, don't leave this uninitialized

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
		fprintf(stderr, "vss error: bogus argument to ReturnFloatMsgToClient: %s\n", msg);
}

extern "C" void ReturnSzMsgToClient(const char* sz, const char* msg)
{
	mm mmT;
	mmT.fRetval = 0; // Be hygienic, don't leave this uninitialized
	sprintf(mmT.rgch, "%s %s", msg, sz);
	MsgsendObj(udpDescObj, vcl_addr, &mmT);
}

//===========================================================================
//		doActors
//
//	Called by scheduler. Very exciting.
//
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
//

extern "C" void ReturnStringToClient(const char* sz)
{
	//printf("ReturnStringToClient: <%s> (%d)\n", sz, strlen(sz));;
	strncpy(vszReturnToClient, sz, cchmm-1);
	vszReturnToClient[cchmm-1] = '\0';
	//printf("ReturnStringToClient: %d chars\n", strlen(vszReturnToClient));;
	ReturnSzMsgToClient(vszReturnToClient, "DataReply");
}

extern "C" void ReturnFloatToClient(float aFloat)
{
	//	if it is a float to be returned, make sure the
	//	string isn't hanging around.
	//	-kel 12 Oct 99
	vszReturnToClient[0] = '\0';
	vzReturnToClient = aFloat; // in case we need to return it to client.
}

float vzMessageGroupRecentHandle = hNil;

extern "C" float* PvzMessageGroupRecentHandle(void)
{
	return &vzMessageGroupRecentHandle;
}

static int vfAlreadyLogged = 0;

//;; Is this reentrant?  (is it ok if pmm is on the stack or otherwise dynamic?)
extern "C" int actorMessageMM(const void* pv, struct sockaddr_in *cl_addr)
{
	mm* pmm = (mm*)pv;
	char* message = pmm->rgch;
	vcl_addr = cl_addr;
	int vfReturnToClient = pmm->fRetval ? 1 : 0;

	// First whitespace-delimited token in message is the command

	if (printCommands >= 1)
		{
#ifndef VSS_WINDOWS
		fprintf(stderr, "\033[32m\033[3m%s\033[0m\n", message);
#else
		fprintf(stderr, "%s\n", message);
		fflush(stderr);
#endif
		}

	vfAlreadyLogged = 1;
	int liveOn = actorMessageHandler(message);
	vfAlreadyLogged = 0;
	if (vfReturnToClient)
		{
		if (vzReturnToClient == hNil)
			{
			// Maybe this is an AUDupdate() "SendData mgFoo [...]"
			// which wants to get "*?" back.
			vzReturnToClient = vzMessageGroupRecentHandle;
			}
		ReturnFloatMsgToClient(vzReturnToClient, "AckNoteMsg");
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
				fprintf(stderr, "vss error: failed to create a \"%s\" actor.\n", szName);
			return p;
		}
	}
	fprintf(stderr, "vss error: can't create unrecognized \"%s\" actor.\n", szName);
	return NULL;
}

static void InternalCreateActor(const char* s)
{
	VActor* anActor = newActor(s);
	if (!anActor)
		{
		ReturnFloatToClient(hNil);
		return;
		}
	float aHandle = anActor->handle();
	if (aHandle == hNil)
		{
		VSS_BeginCriticalError();
		fprintf(stderr, "vss error: got a nil handle to a new \"%s\" actor.\n", s);
		VSS_EndCriticalError();
		}
	ReturnFloatToClient(aHandle);
}

static void InternalEnableOfile(int d, const char* s, int cbBuf=0)
{
	if (d && !*s)
		{
		fprintf(stderr, "vss error: need a filename with EnableOfile.\n");
		return;
		}

	globs.ofile_enabled = d;
	fprintf(stderr, "vss remark: file output %s.\n",
		globs.ofile_enabled ? "enabled" : "disabled");

	if (globs.ofile_enabled)
		OpenOfile(s, cbBuf);
	else
		CloseOfile(s);
}

static void InternalSetPrintCommands(int d)
{
	printCommands = d;
//	cerr << "printCommands = " << printCommands << endl;
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
//ulong vssRandSeed;			// not currently used anywhere

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

//	if (CommandIs("SeedRandom"))
//		{
//		ifD(d, vssRandSeed = (ulong)d );
//		return Uncatch();
//		}

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

//===========================================================================
//		actorMessageHandler
//
//	vzReturnToClient is a float that can be returned to the client, if the
//	client requests it. It should be set to hNil before the message is 
//	processed, rather than just leaving whatever whatever was left over from 
//	the previous message, so that there will be no confusion about which message 
//	produced the return value. 
//
//	actorMessageHandler() returns 1 almost always, zero means that we received a
//	message that should cause vss to exit.
//
extern "C" int actorMessageHandler(const char* Message)
{
	vzReturnToClient = hNil;
	fKeepRunning = 1;
	int caught = actorMessageHandlerCore(Message);
	if (!fKeepRunning)
		return 0;

	if (printCommands >= 2 && !vfAlreadyLogged)
		{
#ifndef VSS_WINDOWS
		// 30-37  Set the text color to black, red, green, yellow, blue,
		// magenta, cyan or white, respectively (ISO 6429).
		// 40-47  Set the page color to black, red, green, yellow, blue,
		// magenta, cyan or white, respectively (ISO 6429).
		fprintf(stderr, "\033[34m\033[47m%s\033[0m\n", Message);
#else
		fprintf(stderr, "\t(internal)  %s\n", Message);
#endif
		}
	vfAlreadyLogged = 0; // This was the 2nd time.  Allow later msgs to be printf'ed now.

	switch(caught)
		{
	case 0:
		fprintf(stderr, "vss error: built-in message had garbled arguments: <%s>\n", Message);
		return 1;
	case 1:
		return 1;
	case 2:
		break;
	default:
		printf("vss error: internal error #1 in actorMessageHandler()\n");
		return 1;
		}

	if (!*Message)
		{
		// We printed an error message already in CommandFromMessage().
		return 1;
		}

	// Now deal with any messages sent to an individual actor instance
	float aHandle;
	if (1 != sscanf(Message, "%*s %f", &aHandle))
		{
		cerr << "vss error: garbled message \"" << Message << "\": missing actor handle?" << endl;
		return 1;
		}

	VActor* anActor = VActor::getByHandle(aHandle);
	if(!anActor)
		{
		cerr << "vss error: Unknown actor handle " << aHandle << " in message \"" << Message << "\"\n";
		return 1;
		}

	if (!anActor->receiveMessage(Message))
		{
		cerr << "vss error: " << anActor->typeName() << ": Unknown command \"" << Message << "\"\n";
		return 1;
		}
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
	fprintf(stderr, "vss remark: writing to %s.\n", fileName);
	strcpy(globs.ofile, fileName);

	if (cbBuf > 0)
		fprintf(stderr, "vss remark: Buffering %d seconds (%d MB) of output.\n",
			(int)(cbBuf / globs.SampleRate / globs.nchansVSS),
			cbBuf / 1000000);
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
		fprintf(stderr, "vss error: cannot close output file %s. Current output file is %s.\n", 
				fileName, globs.ofile);
		return;
		}

	if (globs.fdOfile >= 0)
		{
		fprintf(stderr, "vss remark: wrote to %s.\n", globs.ofile);
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
			fprintf(stderr, "vss remark: output is in %s.aiff.\n", globs.ofile);
			}
#endif

		*globs.ofile= '\0';
		}
}
