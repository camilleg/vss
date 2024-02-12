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

class VSSglobals {
	unsigned long SampleCount;
	int nchansVSS;	// how many channels VSS computes
	int nchansIn;	// how many channels VSS reads
	int nchansOut;	// how many channels VSS writes
public:
	float SampleRate;
	float OneOverSR;
	int liveaudio;	// non-zero means play samples in real-time
	bool ofile_enabled;
	bool fRemappedOutput; // nchansVSS-to-nchansOut isn't the identity map.
	int rgwRemappedOutput[MaxNumChannels];
	int hog; // CPU hog: 1=lock memory, 2=nondegrading pri.
	int lwm; // low water mark in samples
	int hwm; // high water mark in samples
	float msecAntidropout;// longest duration of cpu-starvation to endure
	const char* hostname;
	int udp_port; // listen to clients
	int fdOfile;
	int vcbBufOfile;
	int vibBufOfile;
	char* rgbBufOfile;
	char ofile[256];

	VSSglobals();
	~VSSglobals();
	VSSglobals(const VSSglobals&) = delete;
	VSSglobals& operator=(const VSSglobals&) = delete;

	int Initsynth();
	void dump();
	friend int Nchans();
	friend int NchansIn();
	friend int NchansOut();
	friend unsigned long SamplesToDate();
	friend int VSS_main(int, char *[]);
	friend int Synth(int);
};

extern VSSglobals globs;

inline int Nchans() { return globs.nchansVSS; }
inline int NchansIn() { return globs.nchansIn; }
inline int NchansOut() { return globs.nchansOut; }
inline unsigned long SamplesToDate() { return globs.SampleCount; }

#ifdef VSS_LINUX
#include <sys/time.h>
#endif
inline float currentTime() // In seconds.
	{
#ifdef VSS_LINUX
	if (globs.liveaudio)
		return SamplesToDate() * globs.OneOverSR;
	struct timeval tNow;
	gettimeofday(&tNow, 0);
	tNow.tv_sec -= 86400*365*36; // seconds since 1/1/2006 approx.
	return tNow.tv_sec + tNow.tv_usec / 1000000.0;
#else
	return SamplesToDate() * globs.OneOverSR;
#endif
	}

extern int VSS_main(int argc, char *argv[]);
