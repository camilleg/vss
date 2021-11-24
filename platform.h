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
extern double drand48();
extern void srand48(long seedval);
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
int Synth(int n, int nchans);
void Closesynth();

#ifdef VSS_IRIX
int mdClosePortInput(MDport port);
int mdClosePortOutput(MDport port);
#endif

extern const float* VssInputBuffer();

void ParseArgs(int argc,char *argv[],int *udp_port, int *liveaudio,
	float *sample_rate, int *nchansVSS, int *nchansIn, int *nchansOut, int *hog, int *lwm, int *hwm, char* ofile);

void schedulerMain();
extern int Scount();
extern void SetMidiFunction(int (*vpfnMidiArg)(int));

void BgnMsgsend(const char* host, int port);
void Msgsend(struct sockaddr_in*, mm*);
