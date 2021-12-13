//	Globals defined in vssSrv.c++.

#pragma once

// MaxNumChannels may be any positive number, but it's typically 2, 4, or 8.
// MaxSampsPerBuffer must be a power of two, typically 128 to 4096.
// For both, smaller values conserve RAM and run faster.
constexpr auto MaxNumChannels = 8;
constexpr auto MaxSampsPerBuffer =
#if defined VSS_IRIX || defined VSS_LINUX || defined VSS_MAC
  128;
#elif defined VSS_WINDOWS
  /*8192*/4096;
#endif

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
	int		dacfd;		// Was audio input.  Deprecated because it practically duplicates liveaudio.
	int		fdOfile;
	int		vcbBufOfile;
	int		vibBufOfile;
	char*	rgbBufOfile;
	char    ofile[256];

	VSSglobals();
	~VSSglobals();
	VSSglobals(const VSSglobals&) = delete;
	VSSglobals& operator=(const VSSglobals&) = delete;
};

//;; These public members should be hidden behind set/get function calls,
//;; so we can do things like enforce consistency between
//;; globalOneOverSR and globalSampleRate.

extern VSSglobals globs;

// conflict with vssclient.h's definitions of cchmm and struct mm?
const int cchmm = 5 * 1024;
typedef struct mm
{
	char fRetval; // Nonzero iff server should respond with a float.
	char rgch[cchmm]; // Message.
} mm;

inline int Nchans()
	{ return globs.nchansVSS; }
inline int NchansIn()
	{ return globs.nchansIn; }
inline int NchansOut()
	{ return globs.nchansOut; }
inline unsigned long SamplesToDate()
	{ return globs.SampleCount; }

#ifdef VSS_LINUX
#include <sys/time.h>
#endif
inline float currentTime() // In seconds.
	{
#ifdef VSS_LINUX
	if (globs.liveaudio)
		return globs.SampleCount * globs.OneOverSR;
	struct timeval tNow;
	gettimeofday(&tNow, 0);
	tNow.tv_sec -= 86400*365*36; // seconds since 1/1/2006 approx.
	return tNow.tv_sec + tNow.tv_usec / 1000000.0;
#else
	return globs.SampleCount * globs.OneOverSR;
#endif
	}

extern int VSS_main(int argc, char *argv[]);
