#pragma once

#ifdef VSS_WINDOWS
#include <windows.h> // This has to be included before most other stuff.
#endif

#include <dlfcn.h>
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
extern "C" double drand48(void);
extern "C" void srand48(long seedval);
#endif

#ifndef VSS_IRIX
#define flog10(_) log10(_)
#endif

#include "vssglobals.h"

extern void OpenOfile(const char*, int cbBuf);
extern void CloseOfile(const char*);

extern void VSS_BeginCriticalError(void);
extern void VSS_EndCriticalError(void);

extern void  VSS_SetGlobalAmplitude(float ampl);
extern float VSS_GetGlobalAmplitude(void);

extern void VSS_SetGear(int iGear);

void doActors(void);
void doActorsCleanup(void);
void deleteActors(void);

extern "C" int actorMessageMM(const void *pmm, struct sockaddr_in *cl_addr);

int Initsynth(int udp_port, float srate, int nchans,
			  int nchansIn, int liveaudio, int latency, int hwm);
int Synth(register int (*sfunc)(int n, float* outvecp, int nchans),
		  int n, int nchans);
void Closesynth(void);

#ifdef VSS_IRIX
int mdClosePortInput(MDport port);
int mdClosePortOutput(MDport port);
#endif

extern "C" const float* VssInputBuffer(void);

typedef void *OBJ;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#ifndef PI
#define PI (3.14159265358979323)
#endif

void Line_seg(OBJ *o, int n, float *out, int stride);
OBJ Line_segNew(float srate,float initial);
void Line_seg_start(OBJ *oo, int n, float *v, int stride);
void Line_seg_jam(OBJ *oo, float *v);
OBJ RandLine_segNew(float srate,float initialPeriod);
void Line_seg_time(OBJ *oo, float *v);

void ParseArgs(int argc,char *argv[],int *udp_port, int *liveaudio,
	float *sample_rate, int *nchansVSS, int *nchansIn, int *nchansOut, int *hog, int *lwm, int *hwm, char* ofile);

void schedulerMain(VSSglobals& vv, int (*sfunc)(int n, float* outvecp, int nchans));
extern int Scount(void);
extern void SetMidiFunction(int (*vpfnMidiArg)(int));
OBJ GraphNew(float srate, char *title, float from, float to, int width);
void Graph(OBJ op, int n, float *d, int stride);
void GraphShort(OBJ op, int n, short *d, int stride);
void cGraph(OBJ op, int n, float *d, int stride, int colorindex);
void GraphSpect(OBJ op, float *pzwindow, int npoints);
void GraphTimemode(OBJ op,  int time);
void GraphBang(OBJ op, float *count);
float dbtolin(float);
void assignvec(int n, float *out, int stride,
	 float *a, int astride);
void addvec2(int n, float *out, int stride,
	 float *a, int astride, float *b, int bstride);
void addvec3(int n, float *out, int stride,
	 float *a, int astride,
	 float *b, int bstride,
	 float *c, int cstride);
void addvec4(int n, float *out, int stride,
	 float *a, int astride,
	 float *b, int bstride,
	 float *c, int cstride,
	 float *d, int dstride);
void noisevec(register int n, float *out, int stride);
void gaussian_noisevec(register int n, float *out, int stride);
void smulvec(float s, int n, float *out, int stride, float *in, int istride);
void mulvec(int n, float *out, int stride, float *a, int astride, float *b, int bstride);
void kvec(float k, int n, float *out, int stride);
float productvec( int n,
	 float *a, int astride, float *b, int bstride);
void hanning(int n, float *sig, int stride);
void RvecBhwind(int n, float *wind,int stride, int k);
void RvecBhwind3(int n, float *wind,int stride);
void RvecRecwind(int n, float *sig, int stride);
void RvecHammingwind(int n, float *sig, int stride);

void *platform_dlopen(const char *pathname);
int   platform_dlclose(void *handle);
void *platform_dlsym(void *handle, const char *name);
char *platform_dlerror(void);

OBJ BgnMsgsend(const char *szHostname, int channel);
void MsgsendObj(OBJ obj, struct sockaddr_in *paddr, void* pv);

typedef struct
{
	struct sockaddr_in addr;
	int len;
	int sockfd;
	int channel;
} desc;
