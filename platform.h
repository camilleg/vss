#pragma once

#ifdef VSS_WINDOWS
#include <windows.h> // This has to be included before most other stuff.
extern double drand48();
extern void srand48(long seedval);
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
static inline float fsqrt(float x) { return sqrt(x); }
#define flog10(_) log10(_)

#endif

#include "vssglobals.h"

void OpenOfile(const char*, int cbBuf);
void CloseOfile(const char*);

void  VSS_SetGlobalAmplitude(float ampl);
float VSS_GetGlobalAmplitude();

void VSS_SetGear(int iGear);

void doActors();
void doActorsCleanup();
void deleteActors();

int Synth(int);
void Closesynth();

const float* VssInputBuffer();

void ParseArgs(int argc,char *argv[],int *udp_port, int *liveaudio,
	float *sample_rate, int *nchansVSS, int *nchansIn, int *nchansOut, int *hog, int *lwm, int *hwm, char* ofile);

void schedulerMain();
int Scount();
void SetMidiFunction(int (*vpfnMidiArg)(int));

void BgnMsgsend(const char* host, int port);
void Msgsend(struct sockaddr_in*, mm*);
