// platform-specific code

#ifdef VSS_WINDOWS
#include "windows.h"
#include "winplatform.h"
#include <sys/select.h>
#endif

#ifdef VSS_IRIX
#include <dmedia/audio.h>
#include <stropts.h>
#include <sys/lock.h>
#include <sys/prctl.h>
#include <sys/schedctl.h>
#if defined VSS_IRIX_62 || defined VSS_IRIX_53
#define alGetFD(_) ALgetfd(_)
#define alNewConfig() ALnewconfig()
#define alSetWidth(_,__) ALsetwidth(_,__)
#define alSetQueueSize(_,__) ALsetqueuesize(_,__)
#define alSetChannels(_,__) ALsetchannels(_,__)
#define alOpenPort(_,__,___) ALopenport(_,__,___)
#define alSetParams(_,__,___) ALsetparams(_,__,___) /* not quite right! */
#define alGetFD(_) ALgetfd(_)
#define alGetFilled(_) ALgetfilled(_)
#define alClosePort(_) ALcloseport(_)
#define alReadFrames(_,__,___) ALreadsamps(_,__,___*nchansIn) // in Synth()
#define alWriteFrames(_,__,___) ALwritesamps(_,__,___*nchans) // in Synth()
#define alSetFillPoint(_,__) ALsetfillpoint(_,__*nchans) // in Synth()
#endif
#endif

#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <climits>
#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <grp.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h> // struct pollfd
#include <pwd.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <unistd.h>

#include "platform.h"
#include "vssglobals.h"
#include "VAlgorithm.h" // only for dBFromScalar()

using std::cerr;

#ifdef VSS_LINUX_UBUNTU
#include <alsa/asoundlib.h> // apt install libasound2-dev
snd_pcm_t* pcm_handle_read = nullptr;
snd_pcm_t* pcm_handle_write = nullptr;

// Set by set_hwparams(). Used by set_swparams().
snd_pcm_sframes_t buffer_size;
snd_pcm_sframes_t period_size;

int xrun_recovery(snd_pcm_t* handle, int err)
{
    if (err == -EPIPE) {
LPrepare:
        err = snd_pcm_prepare(handle);
        if (err < 0)
            printf("Failed to recover from underrun: %s\n", snd_strerror(err));
        return 0;
    }
    if (err == -ESTRPIPE) {
		// Busywait until the suspend flag is released.
        while ((err = snd_pcm_resume(handle)) == -EAGAIN)
            usleep(100000);
        if (err < 0)
            goto LPrepare;
        return 0;
    }
    return err;
}

int set_hwparams(snd_pcm_t* handle, snd_pcm_hw_params_t* params, int nchans)
{
    int rc;
    if ((rc = snd_pcm_hw_params_any(handle, params)) < 0) {
        cerr << "No configurations for playback: " << snd_strerror(rc) << "\n";
        return rc;
    }
    if ((rc = snd_pcm_hw_params_set_rate_resample(handle, params, 1)) < 0) {
        cerr << "Hardware resampling failed for playback: " << snd_strerror(rc) << "\n";
        return rc;
    }
    // set the interleaved read/write format
    if ((rc = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        cerr << "Access type unavailable for playback: " << snd_strerror(rc) << "\n";
        return rc;
    }
    if ((rc = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE)) < 0) {
        cerr << "Sample format not available for playback: " << snd_strerror(rc) << "\n";
        return rc;
    }
    if ((rc = snd_pcm_hw_params_set_channels(handle, params, nchans)) < 0) {
        cerr << "Failed to play " << nchans << " channels: " << snd_strerror(rc) << "\n";
        return rc;
    }
    const unsigned int rate = globs.SampleRate;
    unsigned int rrate = rate;
    if ((rc = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0)) < 0) {
        cerr << "Failed to play at " << rate << " Hz: " << snd_strerror(rc) << "\n";
        return rc;
    }
    if (rrate != rate) {
        cerr << "Requested rate " << rate << " Hz != actual rate " << rrate << " Hz\n";
        return -EINVAL;
    }
    auto dir = 1;
	auto usec_buffer = 10000u;
    if ((rc = snd_pcm_hw_params_set_buffer_time_near(handle, params, &usec_buffer, &dir)) < 0) {
        cerr << "Failed to play with ring buffer of " << usec_buffer << " usec: " << snd_strerror(rc) << "\n";
        return rc;
    }
    snd_pcm_uframes_t size;
    if ((rc = snd_pcm_hw_params_get_buffer_size(params, &size)) < 0) {
        cerr << "Failed to get playback buffer size: " << snd_strerror(rc) << "\n";
        return rc;
    }
    buffer_size = size;
	auto usec_period = 3000u; // As low as 500 might work, or even 32.
    if ((rc = snd_pcm_hw_params_set_period_time_near(handle, params, &usec_period, &dir)) < 0) {
        cerr << "Failed to play at period of " << usec_period << " usec: " << snd_strerror(rc) << "\n";
        return rc;
    }
    if ((rc = snd_pcm_hw_params_get_period_size(params, &size, &dir)) < 0) {
        cerr << "Failed to get playback period size: " << snd_strerror(rc) << "\n";
        return rc;
    }
    period_size = size;
    if ((rc = snd_pcm_hw_params(handle, params)) < 0) {
        cerr << "Failed to set playback hw params: " << snd_strerror(rc) << "\n";
        return rc;
    }
    //printf("\nSR = %d Hz.\n%d channels.\nLatency = %.1f ms.\n\n", rate, nchans, float(period_size)/rate * 1000.0);
    return 0;
}

int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
    int rc;
    if ((rc = snd_pcm_sw_params_current(handle, swparams)) < 0) {
        cerr << "Failed to get playback swparams: " << snd_strerror(rc) << "\n";
        return rc;
    }
    /* start the transfer when the buffer is almost full: */
    /* (buffer_size / avail_min) * avail_min */
    if ((rc = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size)) < 0) {
        cerr << "Failed to set playback start threshold: " << snd_strerror(rc) << "\n";
        return rc;
    }
    /* allow the transfer when at least period_size samples can be processed */
    if ((rc = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size)) < 0) {
        cerr << "Failed to set playback available min: " << snd_strerror(rc) << "\n";
        return rc;
    }
    if ((rc = snd_pcm_sw_params(handle, swparams)) < 0) {
        cerr << "Failed to set playback sw params: " << snd_strerror(rc) << "\n";
        return rc;
    }
    return 0;
}
#endif // VSS_LINUX_UBUNTU

#ifdef VSS_LINUX

#include <linux/soundcard.h>
#include <csignal>
#ifndef VSS_LINUX_UBUNTU
	int fdDAC = -1; // hardware output (and input, actually)
#endif

#elif defined VSS_IRIX

	// Yucky globals.
	static ALport alp;
	static ALport alpin;
	static ALconfig alc;
	static unsigned long qsize;
	static int latency;

#elif defined VSS_WINDOWS

	#include "fmod.h"
	#include "fmod_errors.h"
	static int (*pfn)() = NULL;
	#define TESTIT \
		if (!pfn) { \
			printf("failed to find function in fmod.dll\n"); \
			FreeLibrary(fmod_dll); \
			exit(1); }
	static int (*_FSOUND_Stream_GetTime_hack)(FSOUND_STREAM*) = NULL;
	static FSOUND_STREAM *fsound_stream = NULL;
	static HMODULE fmod_dll = NULL;
	static int vfCalledback = 0;
	static int vfLiveTickPaused = 1; // first streamcallback happens before first LiveTick.
	extern short* vrgsCallback;
	short* vrgsCallback = NULL;
	int vcbCallback = 0;
	extern int vfMMIO;
	int vfMMIO = 0;

#endif // VSS_WINDOWS

static auto vfDie = false; // Set to true when vss is dying.

int vfSoftClip = false;
int vfLimitClip = false;
constexpr auto wSoftclipLim = 50000; // Start clipping at +-25000, about -3 dB.
static int rgwSoftclip[wSoftclipLim + 1] = {0};

constexpr auto NSAMPS = MaxSampsPerBuffer * MaxNumChannels; /* or even more! */
static short sampbuff[NSAMPS] = {0};
static short* ssp; // into sampbuff

static float outvecp[NSAMPS] = {0};
static float inpvecp[NSAMPS] = {0};
static short ibuf   [NSAMPS] = {0};

static bool fSoundIn = false;
extern void SetSoundIn(int f) { fSoundIn = f; } // Only misc.c++.
const float* VssInputBuffer() { return fSoundIn ? inpvecp : nullptr; }
static bool vfWaitForReinit = false;

int VSSglobals::Initsynth() {
	vfWaitForReinit = true;
	usleep(250000); // make sure that LiveTick() noticed that we set vfWaitForReinit, and is now waiting.

	nchansOut = std::clamp(nchansOut, 1, MaxNumChannels);
	nchansIn  = std::clamp(nchansIn,  1, MaxNumChannels);
	ssp = sampbuff;
	fdOfile = -1;
	vcbBufOfile = 0;
	vibBufOfile = 0;
	rgbBufOfile = nullptr;

//	liveaudio is a badly-named flag indicating that
//	samples are being scheduled and sent to a CODEC
//	in real time, rather than being dumped to a file.
	if (!liveaudio)
		{
		// ParseArgs() already checked this.
		nchansIn = 0;
		fSoundIn = false;
		}
	else
		{
#ifdef VSS_IRIX
		alc = alNewConfig();
		alSetWidth(alc, AL_SAMPLE_16);
		latency = lat;
		qsize = hwm;
		alSetQueueSize (alc, (int)qsize);
		if (alSetChannels(alc, (long)nchansOut) < 0)
			{
			cerr << "vss: couldn't play " << nchansOut << " channels, using 1 instead.\n";
			nchansOut = 1;
			}
		alp = alOpenPort("obuf", "w", alc);
		if (!alp)
			{
#ifdef VSS_IRIX_63PLUS
			cerr << "vss: failed to output audio: " << alGetErrorString(oserror());
#endif
			return -1;  /* no audio hardware */
			}

		if (fSoundIn)
			{
			if (alSetChannels(alc, nchansIn) < 0)
				{
				cerr << "vss: couldn't input " << nchansIn << " channels, using 1 instead.\n";
				nchansIn = 1;
				alSetChannels(alc, nchansIn);
				}
			alpin = alOpenPort("ibuf", "r", alc);
			if (!alpin)
				{
#ifdef VSS_IRIX_63PLUS
				cerr << "vss: failed to input audio: " << alGetErrorString(oserror());
#endif
				nchansIn = 0;
				}
			}

#if defined VSS_IRIX_62 || defined VSS_IRIX_53
		long pvbuf[6];
		long pvlen;
		pvbuf[0] = AL_OUTPUT_RATE;
		pvbuf[1] = AL_RATE_44100; 
		pvlen = 2;
		if(srate== 8000) pvbuf[1] = AL_RATE_8000;
		else if(srate== 11025) pvbuf[1] = AL_RATE_11025;
		else if(srate== 16000) pvbuf[1] = AL_RATE_16000; 
		else if(srate== 22050) pvbuf[1] = AL_RATE_22050; 
		else if(srate== 32000) pvbuf[1] = AL_RATE_32000; 
		else if(srate== 44100) pvbuf[1] = AL_RATE_44100; 
		else if(srate== 48000) pvbuf[1] = AL_RATE_48000;
		else
			fprintf(stderr, "unsupported sample rate %f, using %d instead\n",srate,44100);
		if (fSoundIn)
			{
			pvbuf[pvlen] = AL_INPUT_RATE;
			pvbuf[pvlen+1] = pvbuf[1];
			ALsetparams(AL_DEFAULT_DEVICE, pvbuf, pvlen);
			pvlen += 2;
			}
		if (nchansOut == 4)
			{
			pvbuf[pvlen] = AL_CHANNEL_MODE;
			pvbuf[pvlen+1] = 4;
			pvlen += 2;
			}
		ALsetparams(AL_DEFAULT_DEVICE, pvbuf, pvlen);
#else // VSS_IRIX_63 or VSS_IRIX_65
		ALpv pvbuf[3];
		long npvs = 0;
		pvbuf[npvs].param = AL_RATE;
		pvbuf[npvs].value.ll = alDoubleToFixed(srate);
		++npvs;
		if (nchansOut == 4)
			{
			pvbuf[npvs].param = AL_CHANNEL_MODE;
			pvbuf[npvs].value.i = 4;
			++npvs;
			}

		if (alSetParams(AL_DEFAULT_OUTPUT, pvbuf, npvs) < 0)
			{
			cerr << "vss: alSetParams failed: " << alGetErrorString(oserror()) << "\n";
			if (pvbuf[1].sizeOut < 0)
				cerr << "vss: invalid output sample rate " << srate << ".\n";
			}

		npvs = 0;
		if (fSoundIn)
			{
			pvbuf[npvs].param = AL_RATE;
			pvbuf[npvs].value = pvbuf[0].value;
			++npvs;

			if (alSetParams(AL_DEFAULT_INPUT, pvbuf, npvs) < 0)
				{
				cerr << "vss: alSetParams failed: " << alGetErrorString(oserror()) << "\n";
				if (pvbuf[1].sizeOut < 0)
					cerr << "vss: invalid input sample rate " << srate << ".\n";
				}
			}

#endif // VSS_IRIX_63 or VSS_IRIX_65
#endif // VSS_IRIX

#ifdef VSS_LINUX_UBUNTU
		int rc;
		if ((rc = snd_pcm_open(&pcm_handle_write, "default" /* or e.g. "hw:0,0" */, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
			// ALSA prints a dozen errors first, even without snd_output_stdio_attach().
			cerr << "vss: no audio out: " << snd_strerror(rc) << "\n";
			liveaudio = 0;
			goto LContinue;
		}
		snd_pcm_hw_params_t* hwparams;
		snd_pcm_hw_params_alloca(&hwparams);
		if ((rc = set_hwparams(pcm_handle_write, hwparams, NchansOut())) < 0) {
			// set_hwparams already complained.
			liveaudio = 0;
			goto LContinue;
		}
		snd_pcm_sw_params_t* swparams;
		snd_pcm_sw_params_alloca(&swparams);
		if ((rc = set_swparams(pcm_handle_write, swparams)) < 0) {
			cerr << "vss failed to set swparams: " << snd_strerror(rc) << "\n";
			liveaudio = 0;
			goto LContinue;
		}
		if (fSoundIn) {
			if ((rc = snd_pcm_open(&pcm_handle_read, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
				cerr << "vss: no audio in: " << snd_strerror(rc) << "\n";
				liveaudio = 0;
				goto LContinue;
			}
			if ((rc = set_hwparams(pcm_handle_read, hwparams, nchansIn)) < 0) {
				// set_hwparams already complained.
				fSoundIn = false;
		    }
			if ((rc = set_swparams(pcm_handle_read, swparams)) < 0) {
				cerr << "vss failed to set input swparams: " << snd_strerror(rc) << "\n";
				fSoundIn = false;
		    }
		}
#if 0
		// These segfault.  Maybe only after snd_pcm_close(), in CloseSynth()?
		snd_pcm_hw_params_free(hwparams);
		snd_pcm_sw_params_free(swparams);
#endif
#endif // VSS_LINUX_UBUNTU

#ifdef VSS_WINDOWS
		if (vfMMIO)
			{
			if (!areal_internal_FInitAudio(nchansOut, nchansIn, fSoundIn, nchansOut*MaxSampsPerBuffer))
				goto LFailed;

			// Using MMIO audio i/o.
			goto LContinue;
			}

		// Using FMOD DirectSound audio i/o.

		if (!fmod_dll)
			fmod_dll = LoadLibrary("fmod.dll");
		if (!fmod_dll)
			{
			printf("vss: failed to load FMOD.DLL.\n");
			goto LFailed;
			}

		if (nchansOut > 2)
			{
			printf("vss: DirectSound outputs at most 2 channels.  Using 2 instead of %d.\n", nchansOut);
			nchansOut = 2;
			}
		{
		pfn = (int (*)())GetProcAddress(fmod_dll, "_FSOUND_GetVersion@0"); TESTIT
		float (*_FSOUND_GetVersion)() = (float(*)())pfn;

		float version = _FSOUND_GetVersion();
		if (version != FMOD_VERSION)
			{
			printf("vss: FMOD.DLL is version %.02f, but should be %.02f.\n", version, FMOD_VERSION);
			FreeLibrary(fmod_dll);
			goto LFailed;
			}

		pfn = (int (*)())GetProcAddress(fmod_dll, "_FSOUND_Stream_GetTime@4"); TESTIT
		_FSOUND_Stream_GetTime_hack = (int(*)(FSOUND_STREAM*))pfn;

		pfn = (int (*)())GetProcAddress(fmod_dll, "_FSOUND_SetOutput@4"); TESTIT
		void (*_FSOUND_SetOutput)(int) = (void(*)(int))pfn;

		pfn = (int (*)())GetProcAddress(fmod_dll, "_FSOUND_SetBufferSize@4"); TESTIT
		void (*_FSOUND_SetBufferSize)(int) = (void(*)(int))pfn;

		pfn = (int (*)())GetProcAddress(fmod_dll, "_FSOUND_SetDriver@4"); TESTIT
		void (*_FSOUND_SetDriver)(int) = (void(*)(int))pfn;

		pfn = (int (*)())GetProcAddress(fmod_dll, "_FSOUND_Init@12"); TESTIT
		int (*_FSOUND_Init)(int, int, int) = (int(*)(int, int, int))pfn;

		pfn = (int (*)())GetProcAddress(fmod_dll, "_FSOUND_GetError@0"); TESTIT
		int (*_FSOUND_GetError)() = (int(*)())pfn;

		pfn = (int (*)())GetProcAddress(fmod_dll, "_FSOUND_Stream_Create@20"); TESTIT
		FSOUND_STREAM* (*_FSOUND_Stream_Create)(FSOUND_STREAMCALLBACK, int, unsigned int, int, int) = (FSOUND_STREAM*(*)(FSOUND_STREAMCALLBACK, int, unsigned int, int, int))pfn;

		pfn = (int (*)())GetProcAddress(fmod_dll, "_FSOUND_Stream_Play@8"); TESTIT
		int (*_FSOUND_Stream_Play)(int, FSOUND_STREAM*) = (int(*)(int, FSOUND_STREAM*))pfn;

		_FSOUND_SetOutput(FSOUND_OUTPUT_DSOUND);
		_FSOUND_SetDriver(0);  // Select sound card (0 = default)
		_FSOUND_SetBufferSize(100 /* msec */ );
		if (!_FSOUND_Init((int)srate, 2/*RTFM*/, 0))
			goto LFailed2;

		extern void streamcallback(FSOUND_STREAM*, void *buff, int len, int param);
		fsound_stream = _FSOUND_Stream_Create(streamcallback, nchansOut*MaxSampsPerBuffer*2, FSOUND_LOOP_OFF | FSOUND_16BITS | \
			(nchansOut==2 ? FSOUND_STEREO : FSOUND_MONO) \
			| FSOUND_STREAMABLE | FSOUND_2D, (int)srate, 12345);
		if (!fsound_stream)
			goto LFailed2;

		if (_FSOUND_Stream_Play(FSOUND_FREE, fsound_stream) == -1)
			{
LFailed2:
			printf("vss FMOD error: %s\n", FMOD_ErrorString(_FSOUND_GetError()));
			FreeLibrary(fmod_dll);
			goto LFailed;
			}
		goto LContinue;
		}

LFailed:
		if (vfMMIO)
			cerr <<"vss failed to initialize MMIO.  Can't output audio.\n";
		else
			cerr <<"vss failed to initialize FMOD/DirectSound.  Can't output audio.\n";
		liveaudio = 0;
#endif // VSS_WINDOWS
		}
#ifndef VSS_MAC
LContinue:
#endif
	{
	const double Cdelt = 2./(double)wSoftclipLim;
	const double Cdelt2 = Cdelt*Cdelt;
	const double Cdelt3 = Cdelt*Cdelt2;
	const double Aval  =  1. - 2.*(1.-(wSoftclipLim/2.))/(wSoftclipLim/2.);
	const double Bval  = -2. + 3.*(1.-(wSoftclipLim/2.))/(wSoftclipLim/2.);
	const double Cval  = 1.;
		  double Cy    = wSoftclipLim/2.;
		  double Cyi   =    Aval*Cdelt3 +    Bval*Cdelt2 + Cval*Cdelt;
		  double Cyii  = 6.*Aval*Cdelt3 + 2.*Bval*Cdelt2;
	const double Cyiii = 6.*Aval*Cdelt3;

	for (int m = 0; m < wSoftclipLim/2; ++m)
		{
		rgwSoftclip[m] = m;
		rgwSoftclip[m + wSoftclipLim/2] = (int)Cy;
		Cy += Cyi * wSoftclipLim / 2.;
		Cyi += Cyii;
		Cyii += Cyiii;
		}
	rgwSoftclip[wSoftclipLim] = 1;
	}

	vfWaitForReinit = false; // enable LiveTick()

#ifdef VSS_LINUX
#ifdef VSS_LINUX_UBUNTU
	return liveaudio ? 0 : -1;
#else
	if (!liveaudio)
		{
		if (fdDAC >= 0)
			close(fdDAC);
		fdDAC = -1;
		}
	return fdDAC;
#endif
#elif defined VSS_IRIX
	return liveaudio ? alGetFD(alp) : -1;
#elif defined VSS_WINDOWS || defined VSS_MAC
	return liveaudio ? 0 : -1;
#endif
}

void VSS_ResyncHardware()
{
#ifdef VSS_LINUX_UBUNTU
	if (pcm_handle_read)
		snd_pcm_drain(pcm_handle_read);
	if (pcm_handle_write)
		snd_pcm_drain(pcm_handle_write);
#endif
}

// How many samples can we compute without getting too far ahead?
int Scount()
{
#ifdef VSS_IRIX
	return alGetFilled(alp);
#elif defined VSS_LINUX_UBUNTU
	// # frames ready to capture or play (how far from xrun): snd_pcm_avail(), or cheap approximate snd_pcm_avail_update().
	return pcm_handle_write ? snd_pcm_avail_update(pcm_handle_write) : 0;
#elif defined VSS_WINDOWS
	return MaxSampsPerBuffer; // wild guess
#elif defined VSS_MAC
	return MaxSampsPerBuffer; // wild guess
#endif
}

static float global_ampl = 1.0;
extern void VSS_SetGlobalAmplitude(float ampl) { global_ampl = ampl; }
extern float VSS_GetGlobalAmplitude() { return global_ampl; }
static bool fWantToResetsynth = false;

void Closesynth()
{	
	if (globs.liveaudio)
		{
#ifdef VSS_IRIX
		alClosePort(alp);
#elif defined VSS_LINUX_UBUNTU
		if (pcm_handle_write) {
			snd_pcm_drain(pcm_handle_write);
			snd_pcm_close(pcm_handle_write);
		}
		if (pcm_handle_read) {
			snd_pcm_drain(pcm_handle_read);
			snd_pcm_close(pcm_handle_read);
		}
#elif defined VSS_WINDOWS
		if (vfCalledback)
			{
			//printf("Closesynth called during callback. dehr?!  It's waiting.\n");
			vfLiveTickPaused = 1; // Ack that FMOD callback needs to run.
			//;; vfDie = true; // give callback a hint to go away already, eh.
			while (vfCalledback) // wait for callback to go away
				usleep(2000);
			printf("Closesynth finished waiting.\n");;;;
			}

		if (vfMMIO)
			{
			areal_internal_TermAudio();
			}
		else
			{
			printf("in Closesynth, fWantToResetsynth==%d\n", fWantToResetsynth);;;;
			if (fsound_stream)
				{
				if (vfCalledback)
					printf("\n\n\nmaybe internal error!!! asdf\n");;;;

				pfn = (int (*)())GetProcAddress(fmod_dll, "_FSOUND_Stream_Stop@4"); TESTIT
				signed char (*_FSOUND_Stream_Stop)(FSOUND_STREAM*) = (signed char(*)(FSOUND_STREAM*))pfn;
				_FSOUND_Stream_Stop(fsound_stream);

				pfn = (int (*)())GetProcAddress(fmod_dll, "_FSOUND_Stream_Close@4"); TESTIT
				signed char (*_FSOUND_Stream_Close)(FSOUND_STREAM*) = (signed char(*)(FSOUND_STREAM*))pfn;
				_FSOUND_Stream_Close(fsound_stream);

				fsound_stream = NULL;

				pfn = (int (*)())GetProcAddress(fmod_dll, "_FSOUND_Close@0"); TESTIT
				void (*_FSOUND_Close)() = (void(*)())pfn;
				_FSOUND_Close();

				// don't bother... just messier for reset button.
				// FreeLibrary(fmod_dll);
				}
			else
				cerr << "vss internal error: Closesynth() called twice.\n";
			}
#endif // VSS_WINDOWS
		}
	if (globs.fdOfile >= 0)
		CloseOfile(globs.ofile);
	usleep(100000);
}

void WantToResetsynth()
{
	fWantToResetsynth = true;
}

static void MaybeResetsynth()
{
	if (!fWantToResetsynth)
		return;
    Closesynth();
	fWantToResetsynth = false;
	(void)globs.Initsynth();
	fWantToResetsynth = false;
}

int Synth(int n) {
	const int nchans = Nchans();
#ifndef VSS_WINDOWS
	MaybeResetsynth();
#endif
	float k = global_ampl;
	int i,j;
	if (globs.liveaudio && fSoundIn)
		{
#ifdef VSS_LINUX_UBUNTU
		const int rc = snd_pcm_readi(pcm_handle_read, ibuf, n);
		if (rc == -EPIPE) {
			fprintf(stderr, "vss: input overrun.\n");
			snd_pcm_prepare(pcm_handle_read);
		} else if (rc < 0) {
			fprintf(stderr, "vss: input error: %s\n", snd_strerror(rc));
		} else if (rc != n) {
			fprintf(stderr, "vss: input read only %d frames, not %d\n", rc, n);
		}
#if 0
		// Cheap VU meter for input.
		auto p = 0; for (i=0; i<n*nchansIn; ++i) p = std::max(p, abs(ibuf[i]));
		if (p != 0) printf("peak = %4d\n", p);
#endif
#elif defined VSS_IRIX
		alReadFrames(alpin, ibuf, n);
#endif

#define REMOVE_DC
#ifndef REMOVE_DC
		constexpr auto wDCOffset = 0;
#else
		// Remove DC offset from input, assuming that input
		// was roughly silent when VSS started.
		static auto wDCOffset = 1<<20;
		if (wDCOffset == 1<<20) {
			wDCOffset = 0;
			if (NchansIn() > 0) {
				for (i=0; i<n*NchansIn(); ++i)
					wDCOffset += ibuf[i];
				wDCOffset /= -n*NchansIn();
			} // Else, input failed to init.
		}
#endif

		for (i=0; i<n; ++i)
			for (j=0; j<NchansIn(); ++j)
				inpvecp[i*MaxNumChannels + j] =
					(ibuf[i*NchansIn() + j] + wDCOffset) / 32768.0f;
				/* 32768 not 32767, to stay >= -1. */
				// So input to vss is [-1,1].  But output is [-32k,32k].
				// That's ok, synthesis classes only see [-1,1].
		}

	// Stuff the output buffer.
	ZeroFloats(outvecp, n * nchans);
    for (auto alg: VAlgorithm::Generators)
        alg->outputSamples(n, outvecp, nchans);
    globs.SampleCount += n;

	if (globs.fRemappedOutput)
		{
		static float temp[MaxSampsPerBuffer][MaxNumChannels];
		for (i=0; i<n; i++)
			for (j=0; j<globs.nchansOut; j++)
				temp[i][j] = outvecp[i*nchans + globs.rgwRemappedOutput[j]];
		for (i=0; i<n; i++)
			for (j=0; j<globs.nchansOut; j++)
				outvecp[i*globs.nchansOut + j] = temp[i][j];
		}

	ssp = sampbuff;

	if (vfLimitClip)
		{
/*
		very fast gain reduction and a very slow gain increase.
		if level gets within -2dB of clip, reduce gain "immediately"
			(within 128 samples, = 3 msec at 44kHz) by 5dB.
		else if >-5dB, reduce gain by .5dB per second
		else if <-30 dB, increase gain by 0.1dB per second.
		else if <-15dB, increase gain by .1dB per second
		Boost is no more than 60dB, though (600 seconds).
		Cut is no more than 200dB (120 msec).
		slower pumping to keep it between -15dB and -5dB.
*/

		static float zLimitdB = 0.;

		static int fShouted = 0;
		if (fShouted>0)
			--fShouted;

		float ampMax = 0.;
		for (i=0; i < n*nchans; ++i)
			{
			float amp = outvecp[i];
			if (amp > ampMax)
				ampMax = amp;
			}
		float dBMax = dBFromScalar(ampMax * k * ScalarFromdB(zLimitdB) / 32768.);
	//	printf("ampMax = %.2f   dBMax = %.2f\n", ampMax, dBMax);;
		if (dBMax > -2.)
			fprintf(stderr, "vss: avoiding hard clipping on output.\n");

		float dBNew = 0.;
		if (dBMax > 0.)
			dBNew = -dBMax - 4.;
		else if (dBMax > -2.)
			dBNew = -5.;
		else if (dBMax > -5.)
			dBNew = -0.5 * globs.OneOverSR * MaxSampsPerBuffer;
			// globs.OneOverSR * MaxSampsPerBuffer is seconds per buffer
		else if (dBMax < -30.)
			dBNew = 0.4 * globs.OneOverSR * MaxSampsPerBuffer;
		else if (dBMax < -15.)
			dBNew = 0.2 * globs.OneOverSR * MaxSampsPerBuffer;

		zLimitdB += dBNew;
		k *= ScalarFromdB(zLimitdB);

		if (!fShouted)
			{
			if (zLimitdB < -3.)
				fprintf(stderr, "vss: limiting output by %.1f dB\n",
					-zLimitdB);
			if (zLimitdB > 3.)
				fprintf(stderr, "vss: boosting output by %.1f dB\n",
					zLimitdB);
			// report this not more than once every 10 seconds
			fShouted = (int)(10.0 * globs.SampleRate/MaxSampsPerBuffer);
			}

		for (i=0; i < n*nchans; ++i)
			{
			float wAmpl = outvecp[i] * k;
			*ssp++ = (short)wAmpl;
			}
		}
	else if (vfSoftClip)
		{
		static int fShoutedSoft = 0;
		if (fShoutedSoft>0)
			--fShoutedSoft;
		static int fShouted = 0;
		if (fShouted>0)
			--fShouted;
		for (i=0; i<n*nchans; ++i)
			{
			int wT = (int)(outvecp[i] * k);
			int wAmpl = abs(wT);
			if (wAmpl > wSoftclipLim/2)
				{
				if (!fShoutedSoft && !fShouted)
					{
					fprintf(stderr, "vss: soft clipping.\n");
					// report this not more than once every 10 seconds
					fShoutedSoft = (int)(10.0 * globs.SampleRate/MaxSampsPerBuffer);
					}
				if (wAmpl > wSoftclipLim)
					{
					wAmpl = wSoftclipLim;
					if (!fShouted)
						{
						fprintf(stderr, "vss: hard clipping (%.2f\n", fabs(outvecp[i]) * k / 32768);
						// report this not more than once every 2 seconds
						fShouted = (int)(2.0 * globs.SampleRate/MaxSampsPerBuffer);
						fShoutedSoft = (int)(10.0 * globs.SampleRate/MaxSampsPerBuffer);
						}
					}
				wT = (wT >= 0) ?
					 rgwSoftclip[wAmpl] :
					-rgwSoftclip[wAmpl];
				}
			*ssp++ = wT;
			}
		}
	else
		{
		static bool fFirstShout = true; // The first one might be a bogus NaN.
		static int fShouted = 0;
		if (fShouted>0)
			--fShouted;
		for (i=0; i < n*nchans; ++i) {
			double wAmpl = outvecp[i] * k;
			const auto pos = abs(wAmpl);
			if (pos > 0.9995*32768) {
				if (fShouted <= 0) {
					if (fFirstShout) {
						fFirstShout = false;
					} else {
						fprintf(stderr, "vss: hard clipping (%.2f)\n", pos/32768.);
						// report this not more than once every 2 seconds
						fShouted = 2.0 * globs.SampleRate/MaxSampsPerBuffer;
					}
				}
				// Actually clip.  Don't just wrap around (2021).
				wAmpl = wAmpl > 0.0 ? 32767 : -32767;
			}
			*ssp++ = wAmpl;
		}
		}

		{
		int samps = ssp - sampbuff;
		if (globs.liveaudio)
			{
#ifdef VSS_IRIX
			alWriteFrames(alp, sampbuff, samps/nchans);
			alSetFillPoint(alp, (long)(qsize-latency));
#elif defined VSS_WINDOWS
			// Tell streamcallback() where to find the samples.
			vrgsCallback = sampbuff;
			vcbCallback = samps*sizeof(short);
#elif defined VSS_LINUX_UBUNTU
			// to sync: snd_pcm_delay() returns how much time will pass before a sample written now gets played.
			{
			    signed short *ptr = sampbuff; // sampbuff is interleaved.
			    int cptr = samps / NchansOut();
				while (cptr > 0) {
					const auto err = snd_pcm_writei(pcm_handle_write, ptr, cptr);
					if (err == -EAGAIN)
						continue;
					if (err < 0) {
						if (xrun_recovery(pcm_handle_write, err) < 0) {
							printf("vss failed to play: %s\n", snd_strerror(err));
							return false;
						}
						break;
					}
					ptr += err * NchansOut(); // err is also # of frames written.
					cptr -= err;
				}
			}
#else
			#warning "Audio output unimplemented for this platform."
#endif // many platforms
			}
		if (globs.fdOfile >= 0 && globs.ofile_enabled)
			{
			// ALSA does this with the "tee device" tee:hw.
			const int cb = samps*sizeof(short);
			if (globs.vcbBufOfile)
				{
				if (globs.vibBufOfile + cb >= globs.vcbBufOfile)
					{
					// flush memory buffer
					(void)!write(globs.fdOfile, globs.rgbBufOfile, globs.vibBufOfile);
					globs.vibBufOfile=0;
					}
				// append to memory buffer
				memcpy(globs.rgbBufOfile+globs.vibBufOfile, (const char*)sampbuff, cb);
				globs.vibBufOfile += cb;
				}
			else
				(void)!write(globs.fdOfile, sampbuff, cb);
			}
		}

	return true;
}

#ifndef VSS_WINDOWS
int main(int argc,char *argv[])
{
	return VSS_main(argc, argv);
}
#endif

static int initudp(int chan)
{
	const auto sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		return -1;
	struct sockaddr_in serv_addr = {0};
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(chan);
	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof serv_addr) < 0)
		return -1;
	fcntl(sockfd, F_SETFL, FNDELAY);
	return sockfd;
}
static void closeudp(int sockfd)
{
	close(sockfd);
}

const size_t MAXMESG = 500; // Small enough to avoid fragmentation when MTU is typically 1500 bytes.
char mbuf[MAXMESG];
int caught_sigint = 0;

#if defined VSS_IRIX_63_MIPS3 || defined VSS_LINUX || defined VSS_WINDOWS || defined VSS_MAC
	#define SignalHandlerType int
#else
	#define SignalHandlerType ...
#endif

void catch_sigint(SignalHandlerType)
{
   caught_sigint = 1;
}

#ifdef VSS_IRIX
void doActors();
void doActorsCleanup();
void deleteActors();
#endif

static int viGear = 1;
enum { prndl_parked=0, prndl_low, prndl_drive }; // for viGear
bool FParked() { return viGear == prndl_parked; }
bool FDrive()  { return viGear == prndl_drive; }
void VSS_SetGear(int iGear)
{
	if (iGear != prndl_parked && iGear != prndl_low && iGear != prndl_drive)
		return;
	if (iGear != prndl_parked && viGear == prndl_parked)
		{
		// switching out of park:  resynch (input and) output
		VSS_ResyncHardware();
		}
	viGear = iGear;
}

#ifndef VSS_WINDOWS
struct pollfd vpfd; // vpfd.fd replaces the redundant udpDesc.sockfd
#endif

// Compute (c*MaxSampsPerBuffer) samples into the output buffer.
//
//	wCatchUp seems like a very bad idea. It allows sample buffers
//	to be computed far in advance of what can be written, and then
//	the write() (or whatever call) has to block! Dehr? 
static void doSynth(int r, int fForce=0, int wCatchUp=0)
{
	//;;;; get rid of wCatchUp, try this in irix, it may be jumping ahead and
	//;;;; then waiting.
	/*
	but irix has alSetQueueSize().
	even _with_ it, Kelly's not sure that we want to be blocking on
	writes() as a way of governing our cpu usage. On the other hand, I am
	finding that its a real balancing act, between latency,
	interruptability, and cpu usage.
	*/

	// Do nothing if we're parked,
	// or if we hit an interruption (r==0) and we're not forcing it anyways.
	if (FParked() || (r <= 0 && !fForce))
		return;

//	compute a number of sample buffers that will keep the 
//	latency within the bounds set by the high and low
//	water marks:
#ifdef VSS_IRIX
	const int c = (wCatchUp!=0) ? wCatchUp :
					(globs.hwm-r) / (NchansOut() * MaxSampsPerBuffer);
#else
	const int c = 1; // much better for linux, I observe.
#endif
	for (int i=0; i<c; i++)
		(void)Synth(MaxSampsPerBuffer);
}

#ifdef VSS_IRIX
	#define SOCK struct sockaddr_in
#else
	#define SOCK sockaddr_in
#endif

#ifdef VSS_WINDOWS
void streamcallback(FSOUND_STREAM*, void *buff, int len, int)
{
	if (vfDie)
		{
LAbort:
		printf("streamcallback aborting: vfDie!\n");;;;
		vfCalledback = 0;
		vfLiveTickPaused = 0;
		return; // Don't do anything.
		}

	if (fWantToResetsynth)
		{
LReset:
		printf("streamcallback aborting: fWantToResetsynth!\n");;;;
		vfCalledback = 0;
		return; // Don't do anything.
		}

	// MMIO is synchronous, a single thread.  Don't need the
	// fancy handshaking, therefore.
	if (!vfMMIO)
		{
		if (vfWaitForReinit)
			{
			// LiveTick() is busywaiting, so we can go ahead and
			// skip the usual handshaking.  This probably means that
			// CloseSynth() got called, though.
			vfLiveTickPaused = 1;
			vfCalledback = 0;
			//printf("streamcallback %x abending!\n", (int)sss);;;;
			return;
			}

		// set a flag so livetick stops processing input requests
		vfCalledback = 1;

		// wait for livetick to acknowledge that fact (by setting another flag)
		while (!vfLiveTickPaused)
			{
			usleep(1000);
			if (vfDie)
				{
				vfCalledback = 0;
				goto LAbort;
				}
			if (fWantToResetsynth)
				{
				vfCalledback = 0;
				printf("\tstreamcallback aborted waiting, gonna reset.\n");;;;
				goto LReset;
				}
			}
		//printf("\tstreamcallback %x done waiting!\n", (int)sss);;;;
		}

#define without_smoother_every_128
#ifdef without_smoother_every_128

	if (vfDie)
		return;
	doSynth(Scount());
	doActors();
	doActorsCleanup(); // after doActors(), in case handlers got deleted.

	if (vrgsCallback)
		{
		if (len != vcbCallback)
			fprintf(stderr, "vss internal error: streamcallback %d != %d\n",
				len, vcbCallback);
		memcpy(buff, vrgsCallback, vcbCallback);
		}

#else

	if (!vrgsCallback)
		goto LDone;
	if (len != vcbCallback)
		{
		fprintf(stderr, "vss internal error: streamcallback %d != %d\n",
			len, vcbCallback);
		goto LDone;
		}

	for (int i=0; i<foo/128 && !vfDie; i++)
		{
		doSynth(Scount());
		doActors();
		doActorsCleanup(); // after doActors(), in case handlers got deleted.
		memcpy(buff + foo, vrgsCallback, vcbCallback);
		}

LDone:
#endif

	// reset flag, so livetick is enabled again.
	vfCalledback = 0;
	vfLiveTickPaused = 0;
}
#endif // VSS_WINDOWS

// Usec() returns how many microseconds have elapsed since the
// previous time it was called.
#ifdef VSS_IRIX
#include <dmedia/dmedia.h>
static unsigned long long tPrev;
static int Usec()
{
	unsigned long long t;
	dmGetUST(&t);
	unsigned long long dt = t - tPrev;
	tPrev = t;
	return (int)(dt / 1000);
}
#elif defined VSS_WINDOWS
static int Usec()
{
	return 10000;
}
#else // VSS_LINUX
//	note: no account is taken of the fact that clock() wraps around
//	every 36 minutes or so (according to the man pages).
static clock_t t0;
static clock_t tPrev;
static int Usec()
{
	const auto t = clock() - t0;
	const auto dt = t - tPrev;
	tPrev = t;
	return dt / (CLOCKS_PER_SEC / 1000000);
}
#endif // VSS_LINUX

//	The basic structure (for Unices) is:
//		generate samples
//		do actors
//		generate samples
//		poll for pending messages
//		generate samples
//		handle messages
//
static int LiveTick(int sockfd)
{
	while (vfWaitForReinit)
		usleep(50000);
	if (vfDie)
		return 0;

#ifndef VSS_WINDOWS
	doSynth(Scount(), 1);
#endif

	doActors();
	doActorsCleanup(); // after doActors(), in case handlers got deleted.

#ifndef VSS_WINDOWS
	if (vfDie)
		return 0;
	doSynth(Scount());
	if (poll(&vpfd, 1, 0) < 0)
		return 1;

	//	this call using the catchup argument (2)
	//	is highly suspect! _Must_ screw up our 
	//	latency control, no?
	//	That arg is ignored on platforms other than
	//	IRIX.
	if (vfDie)
		return 0;
	const int r = Scount();
	doSynth(r, 0, 2);
	if (!(vpfd.revents & POLLIN))
	{
#ifdef EXPERIMENT
		// don't soak the CPU if we're idle: take at least 
		// MinStep microseconds per timestep.
		const int MinStep = 300;
		int usec = Usec();
		if (usec < MinStep)
			usleep(MinStep - usec);
#endif
		// No pending input from a client.
		return 1;
	}
#endif // !VSS_WINDOWS

	int n;

	// Input is pending from a client.
#ifdef VSS_IRIX
	struct sockaddr_in cl_addr;
	int clilen;
#elif defined VSS_LINUX || defined VSS_MAC
	struct sockaddr cl_addr;
	unsigned int clilen;
#elif defined VSS_WINDOWS
	struct sockaddr cl_addr;
	int clilen;
#endif

#ifdef VSS_WINDOWS
	if
	// "while" throws an exception, oddly.
	// Even if we ioctl FIONBIO and fcntl FNDELAY.
#else
	while
#endif
		(clilen = sizeof(cl_addr),
		(n = recvfrom(sockfd, mbuf, MAXMESG, 0, &cl_addr, &clilen)) >0)

		{
		mbuf[n] = '\0'; // Terminate the ascii string.
#ifdef UNDER_CONSTRUCTION
		if (FDrive())
			{
			if (!actorMessageMM(mbuf, (SOCK*)&cl_addr))
				return 0;
			/*
			if (mbuf is a SendData message)
				{
			LAgain:
				if (there's another message pending)
					{
					recvfrom() that message too, into mbuf2.
					if (mbuf2 is also a SendData message, to the same actor)
						{
						// Skip the previous one.

						copy mbuf2 into mbuf
						goto LAgain;
						}
					else
						{
						// Dispatch the previous message.
						if (!actorMessageMM(mbuf, (SOCK*)&cl_addr))
							return 0;
						// Dispatch this message.
						if (!actorMessageMM(mbuf2, (SOCK*)&cl_addr))
							return 0;
						}
					}
				}
			*/

			}
		else
#endif // UNDER_CONSTRUCTION
			{
			if (!actorMessageMM(mbuf, (SOCK*)&cl_addr))
				return 0;
			}

// I wish there was a way to test for the # of messages pending,
// so we could flush that queue and force an audio interrupt
// in order to not fall behind too far.

// Absorb *all* the messages if audio actually got interrupted,
// or nearly so interrupted,
// so it gets interrupted just once instead of dragged on.

// If r<=300, the early-break-from-dispatching-messages exit is disabled.
// If r<=300, the message queue is flushed and we'll probably get an
// audio interrupt.
#ifndef VSS_WINDOWS
		if (r > 300 && Scount() < globs.lwm)
			{
		//	printf("message backlog (%d)\n", Scount());;
			break;
			}
#endif
		}
	if (n < 0 && errno != EWOULDBLOCK)
		perror("vss internal error: possible problem reading input messages");

	doActorsCleanup(); // after actorMessageMM()'s, in case handlers got deleted.
	return 1;
}

static int BatchTick(int sockfd)
{
#ifdef VSS_IRIX
	struct sockaddr_in cl_addr;
	int clilen;
#elif defined VSS_LINUX || defined VSS_MAC
	struct sockaddr cl_addr;
	unsigned int clilen;
#elif defined VSS_WINDOWS
	struct sockaddr cl_addr;
	int clilen;
#endif
	int n;
	while (clilen = sizeof(cl_addr),
		(n = recvfrom(sockfd, mbuf, MAXMESG, 0, &cl_addr, &clilen)) > 0)
		{
		mbuf[n] = '\0'; // Terminate the ascii string.
		if (!actorMessageMM(mbuf, (SOCK*)&cl_addr))
			return 0;
		}

	if (!FParked())
		if (!Synth(MaxSampsPerBuffer))
			return 0;

	doActors();
	doActorsCleanup(); // after doActors(), in case handlers got deleted.

	// don't soak the CPU if we're idle: take at least 3 msec per timestep.
	// (Maybe we're running in real time, but another process like jMax
	// needs the audio hardware open instead of us.)
	int usec = Usec();
	if (usec < 3000)
		usleep(3000 - usec);
	return 1;
}

void schedulerMain()
{
#ifdef VSS_IRIX
	if (globs.hog > 0) {
		if (globs.hog > 1 && schedctl(NDPRI,getpid(), NDPHIMIN) < 0)
			perror("schedctl NDPNORMMIN");
		plock(PROCLOCK);
		setuid(getuid());
	}
#endif
	const auto sockfd = initudp(globs.udp_port);
	if (sockfd <= 0) {
		perror("Another copy of vss may be running on this machine");
		fprintf(stderr, " (port %d)\n", globs.udp_port);
		fprintf(stderr, "\nIf so, type \"vsskill\" or \"kill -9 <processid>\" to kill it.\n");
		return;
	}

	if (globs.liveaudio && globs.Initsynth() < 0)
		goto LDie;

#ifndef VSS_WINDOWS
	if (globs.liveaudio)
		{
		vpfd.fd = sockfd; // messages from clients
		vpfd.events = POLLIN;
		vpfd.revents = 0;
		}
#endif

#ifdef VSS_IRIX
	void flushme_();
	flushme_(); // underflow.c
#elif defined VSS_LINUX
	t0 = clock();
#endif
	
	caught_sigint = 0;
	signal(SIGINT, catch_sigint);
	while ((!caught_sigint && !vfDie) &&
		(globs.liveaudio ? LiveTick(sockfd) : BatchTick(sockfd)))
		;

LDie:
	// We got a ctrl+C.
	vfDie = true;
	deleteActors();
	if (fWantToResetsynth)
		cerr <<"vss internal error: confused between quit and reset.\n";
	Closesynth();
	closeudp(sockfd);
}
