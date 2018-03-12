//	Globals defined in vssSrv.c++ that we need to access elsewhere.

#ifndef __VSS_GLOBALS_H__
#define __VSS_GLOBALS_H__


////////////////////////////////////////////////////////////////////////////
//
// constants

// MaxSampsPerBuffer must be a power of two.

#ifdef VSS_IRIX
#define MaxNumChannels          ((int) 8)
#define MaxSampsPerBuffer       ((int) 128)
#endif
#ifdef VSS_LINUX
#define MaxNumChannels          ((int) 8)
#define MaxSampsPerBuffer       ((int) 128)
#endif
#ifdef VSS_WINDOWS
#define MaxNumChannels          ((int) 8)
#define MaxSampsPerBuffer       ((int) /*8192*/4096)
#endif
#ifdef VSS_FreeBSD
#define MaxNumChannels          ((int) 2)
#define MaxSampsPerBuffer       ((int) 128)
#endif
#ifdef VSS_SOLARIS
#define MaxNumChannels          ((int) 2)
#define MaxSampsPerBuffer       ((int) 128)
#endif


////////////////////////////////////////////////////////////////////////////
//
// types and classes

class VSSglobals
{
public:
	float   SampleRate;
	float   OneOverSR;
	int     liveaudio;	// non-zero means play samples in real-time
	int     ofile_enabled;
	unsigned long smax;
	long   	SampleCount;
	int     nchansVSS;		// number of channels of output that VSS computes
	int     nchansIn;		// number of channels of  input that gets absorbed
	int     nchansOut;		// number of channels of output that gets emitted
	int		fRemappedOutput; // true iff nchansVSS-to-nchansOut isn't the identity map.
	int		rgwRemappedOutput[MaxNumChannels];
	int     hog;			// CPU hog: 1=lock memory, 2=nondegrading pri.
	int     lwm;			// low water mark in samples
	int     hwm;			// high water mark in samples
	float   msecAntidropout;// longest duration of cpu-starvation to endure
	const char *  hostname;
	int     udp_port;	// port to receive client msgs from
	int		dacfd;
	int		fdOfile;
	int		vcbBufOfile;
	int		vibBufOfile;
	char*	rgbBufOfile;
	char    ofile[256];

	VSSglobals(void);
	~VSSglobals(void) {}
};

//;; These public members should be hidden behind set/get function calls,
//;; so we can do things like enforce consistency between
//;; globalOneOverSR and globalSampleRate.

extern VSSglobals globs;

// conflict with vssclient.h's definitions of cchmm and struct mm?
const int cchmm = 5 * 1024;
typedef struct mm
{
	char fRetval;		// Nonzero iff server should return a float to the
						// client in response to this message.
	char rgch[cchmm];	// ASCII string containing the message.
} mm;

////////////////////////////////////////////////////////////////////////////
//
// Accessors for VSSglobals

#if defined(VSS_LINUX)
#include <unistd.h>
#include <sys/time.h> // for struct timeval, and gettimeofday()
#else
#include <ctime>
#endif

inline int Nchans(void) 
	{ return globs.nchansVSS; }
inline int NchansIn(void) 
	{ return globs.nchansIn; }
inline int NchansOut(void) 
	{ return globs.nchansOut; }
inline unsigned long SamplesToDate(void) 
	{ return globs.SampleCount; }
inline float currentTime(void) 
#ifdef VSS_LINUX
	{
	if (globs.liveaudio)
		return (float)globs.SampleCount * globs.OneOverSR;
	struct timeval tNow;
	gettimeofday(&tNow, 0);
	tNow.tv_sec -= 86400*365*36; // seconds since 1/1/2006 approx.
	return tNow.tv_sec + (float)tNow.tv_usec / 1e6;
	}
#else
	{ return (float)globs.SampleCount * globs.OneOverSR; }
#endif

extern "C" int VSS_main(int argc, char *argv[]);

#endif
