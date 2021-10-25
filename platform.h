#pragma once

#ifdef VSS_WINDOWS
#include <windows.h> // This has to be included before most other stuff.
#endif

#include <arpa/inet.h>

#ifdef VSS_IRIX

#include <sys/types.h>
#include <dmedia/midi.h>
#ifdef VSS_IRIX_53
#	include <unistd.h> // for sginap() or usleep()
#	define usleep(_) sginap((long)(_/10000))
#endif

#else

#include <cmath>
static inline float fhypot(float x, float y) { return (float)hypot(x, y); }
static inline float fsqrt(float x) { return (float)sqrt(x); }

#endif

#ifdef VSS_WINDOWS
extern "C" double drand48();
extern "C" void srand48(long seedval);
#endif

#ifndef VSS_IRIX
#define flog10(_) log10(_)
#endif

#include "vssglobals.h"

extern void OpenOfile(const char*, int cbBuf);
extern void CloseOfile(const char*);

extern void  VSS_SetGlobalAmplitude(float ampl);
extern float VSS_GetGlobalAmplitude();

extern void VSS_SetGear(int iGear);

void doActors();
void doActorsCleanup();
void deleteActors();

int Initsynth(int udp_port, float srate, int nchans,
			  int nchansIn, int liveaudio, int latency, int hwm);
int Synth(register int (*sfunc)(int n, float* outvecp, int nchans),
		  int n, int nchans);
void Closesynth();

#ifdef VSS_IRIX
int mdClosePortInput(MDport port);
int mdClosePortOutput(MDport port);
#endif

extern "C" const float* VssInputBuffer();

void ParseArgs(int argc,char *argv[],int *udp_port, int *liveaudio,
	float *sample_rate, int *nchansVSS, int *nchansIn, int *nchansOut, int *hog, int *lwm, int *hwm, char* ofile);

void schedulerMain(VSSglobals& vv, int (*sfunc)(int n, float* outvecp, int nchans));
extern int Scount();
extern void SetMidiFunction(int (*vpfnMidiArg)(int));

typedef void* OBJ;
OBJ BgnMsgsend(const char *szHostname, int channel);
void MsgsendObj(OBJ obj, struct sockaddr_in *paddr, void* pv);

typedef struct
{
	struct sockaddr_in addr;
	int len;
	int sockfd;
	int channel;
} desc;
