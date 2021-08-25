// platform-specific code

#ifdef VSS_WINDOWS

#include "windows.h"
#include "winplatform.h"

#endif

#include <iostream>

#include <cerrno>
#include <climits>
#include <cmath>

#include <netdb.h>
#include <netinet/in.h>

#ifdef VSS_WINDOWS
#include <sys/select.h>
#else
#include <poll.h>
#include <rpc/rpc.h>
#endif

#include <pwd.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <arpa/inet.h>
#include <fcntl.h>
#include <grp.h>
#include <netdb.h>
#include <netinet/in.h>
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

//	IRIX media headers:
#ifdef VSS_IRIX
extern "C" {
#include <dmedia/audio.h>
}
#include <stropts.h>
#include <sys/lock.h>
#include <sys/prctl.h>
#include <sys/schedctl.h>

#if defined(VSS_IRIX_62) || defined(VSS_IRIX_53)
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

#include "platform.h"
#include "vssglobals.h"
#include "VAlgorithm.h" // only for dBFromScalar()

// VSS_LINUX_21ALSA is for ALSA 0.5.3+.

#ifdef VSS_LINUX
#if defined(VSS_LINUX_UBUNTU)
  #define VSS_LINUX_2012
#elif defined(VSS_LINUX_20ALSA) || defined(VSS_LINUX_21ALSA)
  #define VSS_LINUX_ALSA
#else
  #define VSS_LINUX_OSS
#endif
#endif

#ifdef VSS_LINUX_UBUNTU
#include <alsa/asoundlib.h> // apt-get install libasound2-dev
snd_pcm_t *pcm_handle_read = NULL;
snd_pcm_t *pcm_handle_write = NULL;
snd_pcm_format_t pformat, rformat;
const snd_pcm_format_t format = SND_PCM_FORMAT_S16; /* sample format */
unsigned int buffer_time = 30000; /* ring buffer length in us */
unsigned int period_time = 5000; /* period time in us (as low as 500 might work fine) */
const int resample = 1; /* enable alsa-lib resampling */
snd_pcm_sframes_t buffer_size;
snd_pcm_sframes_t period_size;
snd_output_t *alsa_output = NULL;

#if 0
const double freq = 440; /* sinusoidal wave frequency in Hz */
void generate_sine(const snd_pcm_channel_area_t *areas, snd_pcm_uframes_t offset, int count, double *_phase, const unsigned int channels)
{
    const static double max_phase = 2. * M_PI;
    double phase = *_phase;
    const unsigned int rate = globs.SampleRate;
    static double step = 0.03; // max_phase*freq/(double)rate;
    unsigned char *samples[channels];
    int steps[channels];
    int format_bits = snd_pcm_format_width(format);
    const unsigned int maxval = (1 << (format_bits - 1)) - 1;
    const int bps = format_bits / 8; /* bytes per sample */
    const int big_endian = snd_pcm_format_big_endian(format) == 1;
    const int to_unsigned = snd_pcm_format_unsigned(format) == 1;
    /* verify and prepare the contents of areas */
    unsigned int chn;
    for (chn = 0; chn < channels; chn++) {
        if ((areas[chn].first % 8) != 0) {
            printf("areas[%i].first == %i, aborting...\n", chn, areas[chn].first);
            exit(EXIT_FAILURE);
        }
        samples[chn] = /*(signed short *)*/(((unsigned char *)areas[chn].addr) + (areas[chn].first / 8));
        if ((areas[chn].step % 16) != 0) {
            printf("areas[%i].step == %i, aborting...\n", chn, areas[chn].step);
            exit(EXIT_FAILURE);
        }
        steps[chn] = areas[chn].step / 8;
        samples[chn] += offset * steps[chn];
    }
    /* fill the channel areas */
    while (count-- > 0) {
	const short res = sin(phase) * maxval;
	assert(!to_unsigned && !big_endian && bps == 2); // signed 16-bit
        for (chn = 0; chn < channels; chn++) {
	    *(short *)samples[chn] = res;
            samples[chn] += steps[chn];
        }
        phase += step;
	step *= 1.000001;
        if (phase >= max_phase)
            phase -= max_phase;
    }
    *_phase = phase;
}
#endif
static int xrun_recovery(snd_pcm_t *handle, int err)
{
    if (err == -EPIPE) { /* under-run */
        err = snd_pcm_prepare(handle);
        if (err < 0)
            printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
        return 0;
    }
    if (err == -ESTRPIPE) {
        while ((err = snd_pcm_resume(handle)) == -EAGAIN)
            sleep(1); /* wait until the suspend flag is released */
        if (err < 0) {
            err = snd_pcm_prepare(handle);
            if (err < 0)
                printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
        }
        return 0;
    }
    return err;
}
#if 0
static int write_loop(snd_pcm_t *handle, signed short *samples, snd_pcm_channel_area_t *areas)
{
    double phase = 0;
    while (true) {
        generate_sine(areas, 0, period_size, &phase, NchansOut());
	signed short *ptr = samples;
        int cptr = period_size;
        while (cptr > 0) {
            const int err = snd_pcm_writei(handle, ptr, cptr);
            if (err == -EAGAIN)
                continue;
            if (err < 0) {
                if (xrun_recovery(handle, err) < 0) {
                    printf("Write error: %s\n", snd_strerror(err));
                    exit(EXIT_FAILURE);
                }
                break; /* skip one period */
            }
            ptr += err * NchansOut();
            cptr -= err;
        }
    }
}
#endif
static int set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_access_t access)
{
    /* choose all parameters */
    int err = snd_pcm_hw_params_any(handle, params);
    if (err < 0) {
        printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
        return err;
    }
    /* set hardware resampling */
    err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
    if (err < 0) {
        printf("Resampling setup failed for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* set the interleaved read/write format */
    err = snd_pcm_hw_params_set_access(handle, params, access);
    if (err < 0) {
        printf("Access type not available for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* set the sample format */
    err = snd_pcm_hw_params_set_format(handle, params, format);
    if (err < 0) {
        printf("Sample format not available for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* set the count of channels */
    err = snd_pcm_hw_params_set_channels(handle, params, NchansOut());
    if (err < 0) {
        printf("Channels count (%i) not available for playbacks: %s\n", NchansOut(), snd_strerror(err));
        return err;
    }
    /* set the stream rate */
    const unsigned int rate = globs.SampleRate;
    unsigned int rrate = rate;
    err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
    if (err < 0) {
        printf("Rate %iHz not available for playback: %s\n", rate, snd_strerror(err));
        return err;
    }
    if (rrate != rate) {
        printf("Rate doesn't match (requested %iHz, get %iHz)\n", rate, rrate);
        return -EINVAL;
    }
    /* set the buffer time */
    int dir = 1;
    err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, &dir);
    if (err < 0) {
        printf("Unable to set buffer time %i for playback: %s\n", buffer_time, snd_strerror(err));
        return err;
    }
    snd_pcm_uframes_t size;
    err = snd_pcm_hw_params_get_buffer_size(params, &size);
    if (err < 0) {
        printf("Unable to get buffer size for playback: %s\n", snd_strerror(err));
        return err;
    }
    buffer_size = size;
    /* set the period time */
    err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, &dir);
    if (err < 0) {
        printf("Unable to set period time %i for playback: %s\n", period_time, snd_strerror(err));
        return err;
    }
    err = snd_pcm_hw_params_get_period_size(params, &size, &dir);
    if (err < 0) {
        printf("Unable to get period size for playback: %s\n", snd_strerror(err));
        return err;
    }
    period_size = size;
    // printf("\n\nfyi SR %d, %ld samples per 'period' (buf) = %f msec.\n\n", rate, period_size, float(period_size)/float(rate) * 1000.0);
    /* write the parameters to device */
    err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
        return err;
    }
    return 0;
}
static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
    int err = snd_pcm_sw_params_current(handle, swparams);
    if (err < 0) {
        printf("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
        return err;
    }
    /* start the transfer when the buffer is almost full: */
    /* (buffer_size / avail_min) * avail_min */
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);
    if (err < 0) {
        printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
        return err;
    }

    /* allow the transfer when at least period_size samples can be processed */
    err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size);
    if (err < 0) {
        printf("Unable to set avail min for playback: %s\n", snd_strerror(err));
        return err;
    }
    err = snd_pcm_sw_params(handle, swparams);
    if (err < 0) {
        printf("Unable to set sw params for playback: %s\n", snd_strerror(err));
        return err;
    }
    return 0;
}
#endif // UBUNTU

#ifdef VSS_LINUX_ALSA
#ifdef VSS_LINUX_20ALSA
  #include <sys/soundlib.h>
  #include "/home/camilleg/audio_src/ALSA_0.1.4/ALSA/alsa-utils/include/config.h"
  #include "/home/camilleg/audio_src/ALSA_0.1.4/ALSA/alsa-utils/aplay/formats.h"
  void *pcm_handle_read = NULL;
  void *pcm_handle_write = NULL;
  struct snd_pcm_playback_info pinfo;
  struct snd_pcm_record_info rinfo;
  #define SND_PCM_OPEN_CAPTURE O_RDONLY
#endif
#ifdef VSS_LINUX_21ALSA
  #include <alsa/asoundlib.h>		/* BS: changed 04/24/2006 -- although I didn't get the ALSA version working */
  snd_pcm_t *pcm_handle_read = NULL;
  snd_pcm_t *pcm_handle_write = NULL;
  #define snd_pcm_playback_info(a,b) snd_pcm_info(a,b)
  #define snd_pcm_record_info(a,b) snd_pcm_info(a,b)
  snd_pcm_info_t pinfo, rinfo;
#endif
snd_pcm_format_t pformat, rformat;
#endif

//#ifdef DEBUG
//extern "C" void assert(int x)
//{
	//if (x) return;
//
	//VSS_BeginCriticalError();
	//fprintf(stderr, "vss internal error: assertion failure.\n");
	//VSS_EndCriticalError();
//}
//#endif

#if defined(VSS_LINUX)
#include <linux/soundcard.h>
#include <csignal>

int fdDAC = -1; // hardware output (and input, actually)
#	ifdef VSS_LINUX_SONORUS
	int fdADC = -1; // hardware input. /dev/dsp# is NOT duplex here
#	endif

//	yucky globals, IRIX-specific:
#elif defined(VSS_IRIX)
static ALport alp;
static ALport alpin;
static ALconfig alc;
static unsigned long qsize;
static int latency;
#endif

#ifdef VSS_WINDOWS
#include "fmod.h"
#include "fmod_errors.h"
static int (*pfn)() = NULL;
#define TESTIT \
	if (!pfn) \
		{ \
		printf("failed to find function in fmod.dll\n"); \
		FreeLibrary(fmod_dll); \
		exit(1); \
		}
static int (*_FSOUND_Stream_GetTime_hack)(FSOUND_STREAM*) = NULL;
static FSOUND_STREAM *fsound_stream = NULL;
static HMODULE fmod_dll = NULL;
static int vfCalledback = 0;
static int vfLiveTickPaused = 1; // first streamcallback happens before first LiveTick.
static int vfLiveTickShouldReturnZero = 0;
extern short* vrgsCallback;
short* vrgsCallback = NULL;
int vcbCallback = 0;

extern int vfMMIO;
int vfMMIO = 0;
#endif

extern int liveaudio;
extern int vfDie; // set to 1 when app is dying
int vfDie = 0;

int vfSoftClip = FALSE;
int vfLimitClip = FALSE;
int vfGraphSpectrum = FALSE;
int vfGraphOutput = FALSE;
int vwAntidropout = 1; // # of extra 128-byte buffers of resistance to dropouts
static const int vwAntidropoutMax = 64; // at 44kHz, that's 185 msec latency.

#define NSAMPS (MaxSampsPerBuffer * MaxNumChannels) /* or even more! */
short sampbuff[NSAMPS] = {0};

#define wSoftclipLim 50000	/* start clipping at +-25000 (about -3 dB) */
static int rgwSoftclip[wSoftclipLim + 1] = {0};

static short *ssp; /* sample pointer */

static float outvecp[NSAMPS] = {0};
static float inpvecp[NSAMPS] = {0};
static short ibuf   [NSAMPS] = {0};

static int nchansIn;
static int fSoundIn = 0;
extern void SetSoundIn(int fSoundInArg)
	{ fSoundIn = fSoundInArg; }
extern "C" const float* VssInputBuffer(void)
	{ return fSoundIn ? inpvecp : NULL; }
extern int vfWaitForReinit;
int vfWaitForReinit = 0;

// static int vf24bit = 0; // might need this later.

int Initsynth(int /*udp_port*/, float srate, int nchans,
	int nchansInArg, int liveaudio, int lat, int hwm)
{
	vfWaitForReinit = 1;
	usleep(250000); // make sure that LiveTick() noticed that we set vfWaitForReinit, and is now waiting.

	//;; test and report if nchans or nchansInArg is other than 1, 2, or 4 or 8.
	if(nchans<1)
		nchans=1;
	else if(nchans>MaxNumChannels)
		nchans=MaxNumChannels;
	if (nchansInArg < 1)
		nchansInArg = 1;
	else if (nchansInArg > MaxNumChannels)
		nchansInArg = MaxNumChannels;
	ssp = sampbuff;
	globs.fdOfile = -1;
	globs.vcbBufOfile = 0;
	globs.vibBufOfile = 0;
	globs.rgbBufOfile = NULL;

//	liveaudio is a badly-named flag indicating that
//	samples are being scheduled and sent to a CODEC
//	in real time, rather than being dumped to a file.
	if (liveaudio)
		{
#ifdef VSS_IRIX
		alc = alNewConfig();
		alSetWidth(alc, AL_SAMPLE_16);
		latency = lat;
		qsize = hwm;
		alSetQueueSize (alc, (int)qsize);
		if (alSetChannels(alc, (long)nchans) < 0)
			{
			fprintf(stderr, "vss warning: couldn't output %d channels, using 1 instead.\n",
				nchans);
			nchans = 1;
			}
		alp = alOpenPort("obuf", "w", alc);
		if (!alp)
			{
#ifdef VSS_IRIX_63PLUS
			cerr <<"vss error: failed to output audio: "
				 <<alGetErrorString(oserror());
#endif
			return -1;  /* no audio hardware */
			}

		if (fSoundIn)
			{
			if (alSetChannels(alc, (long)nchansInArg) < 0)
				{
				cerr <<"vss warning: couldn't input "
					<<nchansInArg <<" channels, using 1 instead.\n";
				nchansInArg = 1;
				alSetChannels(alc, (long)nchansInArg);
				}
			alpin = alOpenPort("ibuf", "r", alc);
			if (!alpin)
				{
#ifdef VSS_IRIX_63PLUS
				cerr <<"vss error: failed to input audio: "
					 <<alGetErrorString(oserror());
#endif
				nchansInArg = 0;
				}
			globs.nchansIn = nchansIn = nchansInArg;
			}

#if defined(VSS_IRIX_62) || defined(VSS_IRIX_53)
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
		if (nchans == 4)
			{
			pvbuf[pvlen] = AL_CHANNEL_MODE;
			pvbuf[pvlen+1] = 4;
			pvlen += 2;
			}
		ALsetparams(AL_DEFAULT_DEVICE, pvbuf, pvlen);
#else

		ALpv pvbuf[3];
		long npvs = 0;
		pvbuf[npvs].param = AL_RATE;
		pvbuf[npvs].value.ll = alDoubleToFixed(srate);
		++npvs;
		if (nchans == 4)
			{
			pvbuf[npvs].param = AL_CHANNEL_MODE;
			pvbuf[npvs].value.i = 4;
			++npvs;
			}

		if (alSetParams(AL_DEFAULT_OUTPUT, pvbuf, npvs) < 0)
			{
			cerr << "vss error: alSetParams failed: " << alGetErrorString(oserror()) <<"\n";
			if (pvbuf[1].sizeOut < 0)
				cerr << "vss error: output sample rate " <<srate <<" was invalid.\n";
			}

		npvs = 0;
		if (fSoundIn)
			{
			pvbuf[npvs].param = AL_RATE;
			pvbuf[npvs].value = pvbuf[0].value;
			++npvs;

			if (alSetParams(AL_DEFAULT_INPUT, pvbuf, npvs) < 0)
				{
				cerr << "vss error: alSetParams failed: " << alGetErrorString(oserror()) <<"\n";
				if (pvbuf[1].sizeOut < 0)
					cerr << "vss error: input sample rate " <<srate <<" was invalid.\n";
				}
			}

#endif
#endif
#ifdef VSS_LINUX_OSS
#ifdef VSS_LINUX_SONORUS
		/*
		  This is only for studio_mode 1 (ADAT+SPDIF),
		  using out port A(ADAT) and in port B(SPDIF).
		  Future task: make all four combinations of I/O channel
		  8-8, 8-2, 2-8, 2-2, to work.

		  Device numbers are:
		    out port A0-7: dsp1-8
		    out port B0-7: dsp9-16
		    in port A0-1: dsp17-18
		    in port B0-1: dsp19-20

		  Opening ADC before DAC causes problem. Don't know why.
		*/
		int chans, frag, audioformat, SR;

		if (fdDAC < 0)
		  {
		    if ( (fdDAC = open("/dev/dsp1", O_WRONLY)) < 0 )
		      {
			fprintf(stderr, "vss warning: can't open Sonorus output port A0-A7.\n");
			liveaudio = 0;
			goto LContinue;
		      }
		  }
		chans = 8;
		if (ioctl(fdDAC, SNDCTL_DSP_CHANNELS, &chans) || chans != 8)
		  {
		    fprintf(stderr, "vss warning: can't set output to 8 chans but %d.\n", chans);
		    liveaudio = 0;
		    goto LContinue;
		  }
		//printf("Output chans %d %d, ", chans, globs.nchansOut);

		SR = 44100;
		if (ioctl(fdDAC, SNDCTL_DSP_SPEED, &SR) || SR != 44100)
		  {
		    fprintf(stderr, "vss warning: can't set output sr to 44100 but %d.\n", SR);
//  		    liveaudio = 0;
//  		    goto LContinue;
		  }
		//printf("SR %d %f\n", SR, globs.SampleRate);

		frag = 0x00030000 + 8 * 8; // output 8 chan
		if (ioctl(fdDAC, SNDCTL_DSP_SETFRAGMENT, &frag))
			perror("vss warning: ioctl failed");

		audioformat = AFMT_S16_LE;
		if (ioctl(fdDAC, SNDCTL_DSP_SETFMT, &audioformat) ||
			audioformat != AFMT_S16_LE)
			{
			fprintf(stderr, "vss error: can't set output format to signed 16-bit little-endian.\n");
			liveaudio = 0;
			goto LContinue;
			}

		if ( fSoundIn )
		  if (fdADC < 0)
		    if ( (fdADC = open("/dev/dsp19", O_RDONLY)) < 0 )
		      {
			fprintf(stderr, "vss warning: can't open Sonorus input port B0-B1.\n");
			fSoundIn = 0;
		      }

		chans = nchansInArg;
		if ( fSoundIn )
		  if (ioctl(fdADC, SNDCTL_DSP_CHANNELS, &chans) || 
		      chans != nchansInArg)
		    {
		      fprintf(stderr, "vss warning: can't set input to %d chans but %d.\n", nchansInArg, chans);
		      fSoundIn = 0;
		    }
		nchansIn = chans;
//  		printf("Input chans %d %d %d %d\n", 
//  		       chans, nchansInArg, nchansIn, globs.nchansIn);

		SR = 44100;
		if ( fSoundIn )
		  if (ioctl(fdADC, SNDCTL_DSP_SPEED, &SR) || SR != 44100)
		    {
		      fprintf(stderr, "vss warning: can't set input sr to 44100 but %d.\n", SR);
		      fSoundIn = 0;
		    }
		//printf("SR %d %f\n", SR, globs.SampleRate);

#define CFRAGMENT 3 /*6*/
		if ( fSoundIn )
		  {
		    frag = 0x00030000 + 8 * 2; // input 2 chan
		    if (ioctl(fdADC, SNDCTL_DSP_SETFRAGMENT, &frag))
				perror("vss warning: ioctl failed");
		    audioformat = AFMT_S16_LE;
		    if (ioctl(fdADC, SNDCTL_DSP_SETFMT, &audioformat) ||
			audioformat != AFMT_S16_LE)
		      {
			fprintf(stderr, "vss error: can't set input format to signed 16-bit little-endian.\n");
			fSoundIn = 0;
		      }
		  }

#else // non-Sonorus OSS

		// If fdDAC >= 0, we already opened it -- this is a reset button or something.
		if (fdDAC < 0)
			fdDAC = open("/dev/dsp", fSoundIn ? O_RDWR : O_WRONLY);
		if (fdDAC < 0)
			{
			if (fSoundIn)
				{
				// Fall back to only audio output.
				fSoundIn = 0;
				fprintf(stderr, "vss warning: can't open audio input port.\n");
				if (errno == EACCES)
					(void)system("/bin/ls -l /dev/dsp");
				fdDAC = open("/dev/dsp", O_WRONLY);
				}
			if (fdDAC < 0)
				{
				perror("vss error: can't open audio output port");
				if (errno == EACCES)
					{
					cerr << "\n";
					(void)system("/bin/ls -l /dev/dsp*");
					cerr << "\n";
					return -1;
					}
				liveaudio = 0;
				}
			}
		if (ioctl(fdDAC, SNDCTL_DSP_RESET, 0))
			perror("vss warning: ioctl failed");
		if (fSoundIn)
			{
			int caps = 0;
			if (ioctl(fdDAC, SNDCTL_DSP_SETDUPLEX, 0))
				perror("vss warning: ioctl failed");
			if (ioctl(fdDAC, SNDCTL_DSP_GETCAPS, &caps))
				perror("vss warning: ioctl failed");
			// caps == 0x3101 on edison, 3/19/99.
			if (!(caps & DSP_CAP_DUPLEX))
				{
				fprintf(stderr, "vss error: sound card can't do full duplex.\n");
				// Fall back to only audio output.
				fSoundIn = 0;
				}
			else
				globs.nchansIn = nchansIn = nchansInArg;
			}

		// Compute vwAntidropout from globs.msecAntidropout and globs.SampleRate
		// vwAntidropout==1 iff the dropout length in msec is 128000/SR,
		// where 128000 = MaxSampsPerBuffer * 1000
		if (globs.msecAntidropout == 0.)
			vwAntidropout = 1;
		else
			{
			vwAntidropout = (int)(globs.msecAntidropout / (MaxSampsPerBuffer * 1000. / globs.SampleRate));
			if (vwAntidropout <= 1)
				{
				fprintf(stderr, "vss warning: -antidropout duration too short, ignored (%.1f msec instead).\n",
					MaxSampsPerBuffer * 1000. / globs.SampleRate);
				vwAntidropout = 1;
				}
			if (vwAntidropout > vwAntidropoutMax)
				{
				fprintf(stderr, "vss warning: -antidropout duration too long, limiting to %d msec.\n",
					(int)(vwAntidropoutMax * (MaxSampsPerBuffer * 1000. / globs.SampleRate)));
				vwAntidropout = vwAntidropoutMax;
				}
			}

#define CFRAGMENT (3*vwAntidropout)
		// CFRAGMENT fragments of 256*nchans bytes (128 samples) each
		int frag = (CFRAGMENT << 16) + (8 * nchans);
		//printf("\t\t(%d fragments of %d bytes each).\n", (frag>>16), 1<<(frag&0xffff));;
		if (ioctl(fdDAC, SNDCTL_DSP_SETFRAGMENT, &frag))
			perror("vss warning: ioctl failed");

		int audioformat = AFMT_S16_LE;
		if (ioctl(fdDAC, SNDCTL_DSP_SETFMT, &audioformat) ||
			audioformat != AFMT_S16_LE)
			{
			fprintf(stderr, "vss error: can't set output format to signed 16-bit little-endian.\n");
			liveaudio = 0;
			goto LContinue;
			}

		int stereo = nchans==1 ? 0 : 1;
		if (ioctl(fdDAC, SNDCTL_DSP_STEREO, &stereo) ||
			stereo != (nchans==1 ? 0 : 1))
			fprintf(stderr, "vss warning: can't set output mono/stereo.\n");

		int SR = (int)globs.SampleRate;
		if (ioctl(fdDAC, SNDCTL_DSP_SPEED, &SR) ||
			SR != (int)globs.SampleRate)
			{
			fprintf(stderr, "vss warning: set output sampling rate to %d Hz instead of %d Hz.\n",
				SR, (int)globs.SampleRate);
			globs.SampleRate = SR;
			globs.OneOverSR = 1.0 / globs.SampleRate;
			}
#endif // end #ifdef VSS_LINUX_SONORUS
#endif
#ifdef VSS_LINUX_UBUNTU
		snd_pcm_hw_params_t *hwparams; snd_pcm_hw_params_alloca(&hwparams);
		snd_pcm_sw_params_t *swparams; snd_pcm_sw_params_alloca(&swparams);
		int err = snd_output_stdio_attach(&alsa_output, stdout, 0);
		if (err < 0)
		      {
		      fprintf(stderr, "vss error: alsa output failed: %s\n", snd_strerror(err));
		      liveaudio = 0;
		      goto LContinue;
		      }
		if ((err = snd_pcm_open(&pcm_handle_write, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0)
		      {
		      fprintf(stderr, "vss error: can't open audio output port: %s\n", snd_strerror(err));
		      liveaudio = 0;
		      goto LContinue;
		      }
		if ((err = set_hwparams(pcm_handle_write, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
		      {
		      fprintf(stderr, "vss error: failed to set hwparams: %s\n", snd_strerror(err));
		      liveaudio = 0;
		      goto LContinue;
		      }
		if ((err = set_swparams(pcm_handle_write, swparams)) < 0)
		      {
		      fprintf(stderr, "vss error: failed to set swparams: %s\n", snd_strerror(err));
		      liveaudio = 0;
		      goto LContinue;
		      }
#if 0
		const unsigned int channels = NchansOut();
		signed short *samples = (short int*)malloc((period_size * channels * snd_pcm_format_physical_width(format)) / 8);
		if (!samples)
		      {
		      fprintf(stderr, "vss error: OOM\n");
		      liveaudio = 0;
		      goto LContinue;
		      }
		snd_pcm_channel_area_t *areas = (snd_pcm_channel_area_t*)calloc(channels, sizeof(snd_pcm_channel_area_t));
		if (!areas)
		      {
		      fprintf(stderr, "vss error: OOM\n");
		      liveaudio = 0;
		      goto LContinue;
		      }
		unsigned int chn;
		for (chn = 0; chn < channels; chn++) {
		  areas[chn].addr = samples;
		  areas[chn].first = chn * snd_pcm_format_physical_width(format);
		  areas[chn].step = channels * snd_pcm_format_physical_width(format);
		}

		err = write_loop(pcm_handle_write, samples, areas);
		free(areas);		//;;;; now this is unused
		free(samples);
		snd_pcm_close(pcm_handle_write);
		snd_pcm_sw_params_free(swparams);
		snd_pcm_hw_params_free(hwparams);
		printf("terminating early while writing this code in 2012\n");;;;
		exit(0);
#endif
#endif
#ifdef VSS_LINUX_ALSA
		int err;

		// O_RDWR O_WRONLY are actually SND_PCM_OPEN_DUPLEX SND_PCM_OPEN_PLAYBACK
		if ((err = snd_pcm_open( &pcm_handle_write, 0/*card*/, 0/*dev*/, SND_PCM_OPEN_PLAYBACK)) < 0)
			{
			fprintf(stderr, "vss error: can't open audio output port: %s\n", snd_strerror(err));
			liveaudio = 0;
			goto LContinue;
			}

		{
		snd_pcm_channel_info_t info;
		memset(&info, 0, sizeof(info));
		if ((err = snd_pcm_channel_info(pcm_handle_write, &info)) < 0)
			{
			fprintf(stderr, "ALSA->open(): %s\n", snd_strerror(err));
			liveaudio = 0;
			goto LContinue;
			}
		}


			{
			if (fSoundIn)
				{
				if ((err = snd_pcm_open( &pcm_handle_read, 0/*card*/, 0/*dev*/, SND_PCM_OPEN_CAPTURE)) < 0)
					{
					fSoundIn = 0;
					fprintf(stderr, "vss warning: can't open audio input port.\n");
					}
				}
			}

		// can we play sounds ok?
		if ( (err = snd_pcm_playback_info(pcm_handle_write, &pinfo )) < 0 )
			{
			fprintf(stderr, "vss error: playback info error: %s\n",
				snd_strerror(err));
			liveaudio = 0;
			goto LContinue;
			}
		if (fSoundIn)
			{
			if ( (err = snd_pcm_record_info(pcm_handle_read, &rinfo )) < 0 )
				{
				fprintf(stderr, "vss error: record info error: %s\n",
					snd_strerror(err));
				}

	/*
		printf("\n\nplayback info flags %x\n", pinfo.flags);
		printf("\n\nrecord   info flags %x\n", rinfo.flags);
	*/
	/*
		printf("\n\nplayback frag bounds: %d %d, align %d\n\n",
			pinfo.min_fragment_size,
			pinfo.max_fragment_size,
			pinfo.fragment_align);;

		printf("\n\nrecord frag bounds: %d %d, align %d\n\n",
			rinfo.min_fragment_size,
			rinfo.max_fragment_size,
			rinfo.fragment_align);;
	*/

			}

		// Set up playback format.
#ifdef VSS_LINUX_20ALSA
		memset( &pformat, 0, sizeof( pformat ) );
		pformat.format = SND_PCM_SFMT_S16_LE;
		pformat.rate = (unsigned int)srate;
		pformat.channels = nchans;
	//	printf("playing %i Hz, %i channels\n", pformat.rate, pformat.channels);
		if (snd_pcm_playback_format(pcm_handle_write, &pformat) < 0)
			{
			fprintf(stderr, "vss error: unable to set playback format %iHz, %i channels\n",
				pformat.rate, pformat.channels);
			}
		struct snd_pcm_playback_params pparams;
		memset( &pparams, 0, sizeof( pparams ) );
		pparams.fragment_size = 256; // 16-bit, mono.
		pparams.fragments_max = 8; //;; was 1, then was 3
		pparams.fragments_room = 8; //;; was 1, then was 3
		// we need 4 not 1 (well, maybe 3) to accommodate HidimMapper.
		if ( snd_pcm_playback_params(pcm_handle_write, &pparams ) < 0 )
			{
			fprintf(stderr, "vss error: unable to set playback params\n");
			liveaudio = 0;
			goto LContinue;
			}
#else
		snd_pcm_channel_params_t params;
		memset(&params, 0, sizeof(params));
		params.channel = SND_PCM_CHANNEL_PLAYBACK;
		params.buf.block.frag_size = 256; // 16-bit, mono.
		params.buf.block.frags_max = 3;
		params.buf.block.frags_min = 3;
		memset( &pformat, 0, sizeof( pformat ) );
		pformat.format = SND_PCM_SFMT_S16_LE;
		pformat.rate = (int)srate;
		pformat.voices = nchans;
		pformat.interleave = 1;
		memcpy(&params.format, &pformat, sizeof(pformat));

		if ((err = snd_pcm_channel_params(pcm_handle_write, &params) < 0))
			{
			fprintf(stderr, "vss error: unable to set playback format %iHz, %i channels\n\t\"%s\"\n",
				pformat.rate, pformat.voices, snd_strerror(err));
			liveaudio = 0;
			goto LContinue;
			}
		printf("playing %i Hz, %i channels\n", pformat.rate, pformat.voices);
#endif

		if (fSoundIn)
			{
			globs.nchansIn = nchansIn = nchansInArg;
#ifdef VSS_LINUX_20ALSA
			memset( &rformat, 0, sizeof( rformat ) );
			rformat.format = SND_PCM_SFMT_S16_LE;
			rformat.rate = (unsigned int)srate;
			rformat.channels = nchansIn;
			if (snd_pcm_record_format(pcm_handle_read, &rformat ) < 0 )
				{
				fprintf(stderr, "vss error: unable to set record format %i Hz, %i channels\n",
					rformat.rate, rformat.channels);
				liveaudio = 0;
				goto LContinue;
				}
		//	printf("recording %i Hz, %i channels\n", rformat.rate, rformat.channels);
			struct snd_pcm_record_params rparams = {0};
			rparams.fragment_size = 256; // 16-bit, mono.
			rparams.fragments_min = 8; //;; was 1, then was 2.
			if (snd_pcm_record_params(pcm_handle_read, &rparams ) < 0 )
				fprintf(stderr, "vss error: unable to set record params\n");
#else
			snd_pcm_channel_params_t params;
			memset(&params, 0, sizeof(params));
			params.channel = SND_PCM_CHANNEL_CAPTURE;
			params.buf.block.frag_size = 256; // 16-bit, mono.
			params.buf.block.frags_max = 3;
			params.buf.block.frags_min = 3;
			memset( &rformat, 0, sizeof( rformat ) );
			rformat.format = SND_PCM_SFMT_S16_LE;
			rformat.rate = (int)srate;
			rformat.voices = nchansIn;
			rformat.interleave = 1;
			memcpy(&params.format, &rformat, sizeof(rformat));

			if ((err = snd_pcm_channel_params(pcm_handle_read, &params) < 0))
				{
				fprintf(stderr, "vss error: unable to set record format %i Hz, %i channels\n",      
					rformat.rate, rformat.voices);
				liveaudio = 0;
				goto LContinue;
				}
			printf("recording %i Hz, %i channels\n", rformat.rate, rformat.voices);       

#endif
			}
#ifdef VSS_LINUX_20ALSA
		fdDAC = snd_pcm_file_descriptor(pcm_handle_write);
#else
		fdDAC = snd_pcm_file_descriptor(pcm_handle_write, SND_PCM_CHANNEL_PLAYBACK);
#endif
#endif

#ifdef VSS_WINDOWS
		if (vfMMIO)
			{
			if (!areal_internal_FInitAudio(nchans, nchansIn, fSoundIn, nchans*MaxSampsPerBuffer))
				goto LFailed;

			// Using MMIO audio i/o.
			cerr <<"vss remark: MMIO initialized.\n";
			goto LContinue;
			}

		// Using FMOD DirectSound audio i/o.

		if (!fmod_dll)
			fmod_dll = LoadLibrary("fmod.dll");
		if (!fmod_dll)
			{
			printf("vss error: failed to load FMOD.DLL.\n");
			goto LFailed;
			}

		if (nchans > 2)
			{
			printf("vss warning: DirectSound outputs at most 2 channels.  Using 2 instead of %d.\n",
				nchans);
			nchans = 2;
			}
		{
		pfn = (int (*)())GetProcAddress(fmod_dll, "_FSOUND_GetVersion@0"); TESTIT
		float (*_FSOUND_GetVersion)() = (float(*)())pfn;

		float version = _FSOUND_GetVersion();
		if (version != FMOD_VERSION)
			{
			printf("vss error: FMOD.DLL is version %.02f, but should be %.02f.\n", version, FMOD_VERSION);
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
		//cerr <<"vss remark: default sound card selected.\n";;;;
		_FSOUND_SetBufferSize(100 /* msec */ );
		if (!_FSOUND_Init((int)srate, 2/*RTFM*/, 0))
			goto LFailed2;

		extern void streamcallback(FSOUND_STREAM*, void *buff, int len, int param);
		fsound_stream = _FSOUND_Stream_Create(streamcallback, nchans*MaxSampsPerBuffer*2, FSOUND_LOOP_OFF | FSOUND_16BITS | \
			(nchans==2 ? FSOUND_STEREO : FSOUND_MONO) \
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
		cerr <<"vss remark: DirectSound initialized.\n";
		goto LContinue;
		}

LFailed:
		if (vfMMIO)
			cerr <<"vss error: failed to initialize MMIO audio i/o.  Can't output audio.\n";
		else
			cerr <<"vss error: failed to initialize FMOD/DirectSound.  Can't output audio.\n";
		liveaudio = 0;
#endif

		}
	else
		{
		if (fSoundIn || nchansInArg > 1)
			cerr <<"vss warning: audio input doesn't work with -silent.\n";
		globs.nchansIn = nchansIn = nchansInArg = 0;
		SetSoundIn(0);
		}

LContinue:
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

	vfWaitForReinit = 0; // enable LiveTick()
#ifdef VSS_LINUX
#ifdef VSS_LINUX_UBUNTU
	return liveaudio ? 0/*unused globs.dacfd*/ : -1;
#else
	if (!liveaudio)
		{
		if (fdDAC >= 0)
			close(fdDAC);
		fdDAC = -1;
		}
	return fdDAC;
#endif
#endif
#ifdef VSS_IRIX
	return liveaudio ? alGetFD(alp) : -1;
#endif
#ifdef VSS_HAMMERFALL
	printf("hammerfall init NYI\n");
	return -1;
#endif
#ifdef VSS_WINDOWS
		return liveaudio ? 0/*unused globs.dacfd in VSS_WINDOWS*/ : -1;
#endif
}

extern void VSS_ResyncHardware(void)
{
#ifdef VSS_LINUX_UBUNTU
	if (pcm_handle_read)
		snd_pcm_drain(pcm_handle_read);
	if (pcm_handle_write)
		snd_pcm_drain(pcm_handle_write);
#endif
#ifdef VSS_LINUX_ALSA
#ifdef VSS_LINUX_20ALSA
	if (pcm_handle_read)
		snd_pcm_flush_record(pcm_handle_read);
	if (pcm_handle_write)
		snd_pcm_drain_playback(pcm_handle_write);
#else
	if (pcm_handle_read)
		snd_pcm_capture_flush(pcm_handle_read);
	if (pcm_handle_write)
		snd_pcm_playback_drain(pcm_handle_write);
#endif
#endif
}

// How many samples can we compute without getting too far ahead?
int Scount(void)
{
#ifdef VSS_IRIX
	return alGetFilled(alp);
#endif
#if defined(VSS_LINUX_ALSA)
	return MaxSampsPerBuffer; // lie, always return MaxSampsPerBuffer, just to get it running!
#endif
#if defined(VSS_LINUX_UBUNTU)
	// get # frames ready to play (fill level, how far from underrun): snd_pcm_avail(), or cheap approximate snd_pcm_avail_update().
	return pcm_handle_write ? snd_pcm_avail_update(pcm_handle_write) : 0;
#endif
#ifdef VSS_LINUX_OSS
	audio_buf_info x;
	if (ioctl(fdDAC, SNDCTL_DSP_GETOSPACE, &x))
		perror("vss warning: ioctl failed");

	// x.bytes is how many bytes we can write without blocking.
	// So, to emulate alGetFilled, which is the # of sample frames pending,
	// we compute numfragments * bytes_per_frag / samples_per_byte,
	// the total # of samples.
	// From this we subtract x.bytes / samples_per_byte,
	// the number of samples we can write without blocking.

	return CFRAGMENT * 256 / sizeof(short) - x.bytes / sizeof(short);
#endif
#ifdef VSS_WINDOWS
	return MaxSampsPerBuffer; // wild guess
#endif
	}

static float global_ampl = 1.0;

extern void VSS_SetGlobalAmplitude(float ampl)
{
	global_ampl = ampl;
}

extern float VSS_GetGlobalAmplitude(void)
{
	return global_ampl;
}
static int fWantToResetsynth = 0;

void Closesynth()
{	
	if (liveaudio)
		{
#ifdef VSS_IRIX
		alClosePort(alp);
#endif
#ifdef VSS_LINUX_ALSA
		if (pcm_handle_write)
			snd_pcm_close(pcm_handle_write);
		if (pcm_handle_read)
			snd_pcm_close(pcm_handle_read);
#endif
#ifdef VSS_LINUX_OSS
		close(fdDAC);
		fdDAC = -1;
#  ifdef VSS_LINUX_SONORUS
		if ( fSoundIn )
		  {
		    close(fdADC);
		    fdADC = -1;
		  }
#  endif
#endif
#ifdef VSS_WINDOWS
		if (vfCalledback)
			{
			//printf("Closesynth called during callback. dehr?!  It's waiting.\n");
			vfLiveTickPaused = 1; // Ack that FMOD callback needs to run.
			//;; vfDie = 1; // give callback a hint to go away already, eh.
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
#endif
		}
	if (globs.fdOfile >= 0)
		CloseOfile(globs.ofile);
	usleep(100000);
}

void WantToResetsynth()
{
	fWantToResetsynth = 1;
}

static inline void MaybeResetsynth()
{
	if (!fWantToResetsynth)
		return;
	//printf("MaybeResetsynth calling Closesynth!\n");;;;
    Closesynth();
	fWantToResetsynth = 0;
	//printf("MaybeResetsynth calling Initsynth!\n");;;;
	globs.dacfd = Initsynth(globs.udp_port,
		globs.SampleRate, globs.nchansVSS, globs.nchansIn,
		globs.liveaudio, globs.lwm, globs.hwm);
	fWantToResetsynth = 0;
}

static int (*vsfunc)(int n, float* outvecp, int nchans) = NULL;

int Synth(int (*sfunc)(int n, float* outvecp, int nchans),
              int n, int nchans)
{
	// printf("\t\t\tSynth() b6: %d samps, %d chans\n", n, nchans);;;;
#ifndef VSS_WINDOWS
	MaybeResetsynth();
#endif

	float k = global_ampl;
	int i,j;

	/* Zero the output buffer, just in case. */
	memset((char*)outvecp, 0, n * nchans * sizeof(float));

#undef PARANOID
#ifdef PARANOID
	// Zero the input buffer too.
	memset(ibuf, 0, n * nchansIn * sizeof(short));
#endif

	if(liveaudio && fSoundIn)
		{
#ifdef VSS_LINUX_ALSA
		(void)snd_pcm_read(pcm_handle_read, ibuf, n * nchansIn * sizeof(short));
#endif
#ifdef VSS_LINUX_OSS
#ifdef VSS_LINUX_SONORUS
		read(fdADC, ibuf, n * nchansIn * sizeof(short));
#else
		// EFFECTIVELY nchansIn == nchans in ALSA and OSS,
		// even if nchansIn is actually 1.
		// So if nchans==2, fake nchansIn to be 1 by summing the two
		// channels actually read in.
		if (nchans==1)
			(void)read(fdDAC, ibuf, n * nchansIn * sizeof(short));
		else if (nchans==2)
			{
			//printf(";; don't do stereo input!\n");;;;
			// This does the right thing, but we get ~1/3 second lag!
			(void)read(fdDAC, ibuf, n * 2 * sizeof(short));
			for (i=0; i<n; ++i)
				//ibuf[i] = ibuf[i*2];
				ibuf[i] = (ibuf[i*2] + ibuf[i*2+1]) / 2;
			}
		else
			fprintf(stderr, "input for > 2 channels NYI.\n");
		// read(fdDAC, ibuf, n * nchansIn * sizeof(short));
		// identical. crashed with alsa014, didn't try alsa030.
#endif
#endif
#ifdef VSS_IRIX
		alReadFrames(alpin, ibuf, n);
#endif

#define REMOVE_DC
#ifdef REMOVE_DC
// Remove DC offset from input, assuming that input was silent (on average,
// at least!) when VSS started.
		static int wDCOffset = 1<<20;
		if (wDCOffset == 1<<20)
			{
			wDCOffset = 0;
			for (i=0; i<n; ++i)
				for (j=0; j<nchansIn; ++j)
					wDCOffset += ibuf[i*nchansIn + j];
			wDCOffset /= -n*nchansIn;
			}
#else
#define wDCOffset 0
#endif

		for (i=0; i<n; ++i)
			for (j=0; j<nchansIn; ++j)
				inpvecp[i*MaxNumChannels + j] =
					(ibuf[i*nchansIn + j] + wDCOffset) / 32768.0f;
				/* 32768 not 32767, to stay >= -1. */
				// So input to vss is [-1,1].  But output is [-32k,32k].
				// That's ok, synthesis classes only see [-1,1].
		}

	if (!sfunc)
		{
#if 0
		printf("vss internal error: null sfunc passed to Synth() (%x).\n", (int)vsfunc);
#else /* BS: fixed 04/24/2006 */
		printf("vss internal error: null sfunc passed to Synth() (%p).\n", vsfunc);
#endif
		return FALSE;
		}
	if (!(*sfunc)(n, outvecp, nchans))
		return FALSE;

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
#if 0
		very fast gain reduction and a very slow gain increase.
		if level gets within -2dB of clip, reduce gain "immediately"
			(within 128 samples, = 3 msec at 44kHz) by 5dB.
		else if >-5dB, reduce gain by .5dB per second
		else if <-30 dB, increase gain by 0.1dB per second.
		else if <-15dB, increase gain by .1dB per second
		Boost is no more than 60dB, though (600 seconds).
		Cut is no more than 200dB (120 msec).
		slower pumping to keep it between -15dB and -5dB.
#endif

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
			fprintf(stderr, "vss warning: avoiding hard clipping on output.\n");

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
				fprintf(stderr, "vss warning: limiting output by %.1f dB\n",
					-zLimitdB);
			if (zLimitdB > 3.)
				fprintf(stderr, "vss remark: boosting output by %.1f dB\n",
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
					fprintf(stderr, "vss remark: soft clipping on output.\n");
					// report this not more than once every 10 seconds
					fShoutedSoft = (int)(10.0 * globs.SampleRate/MaxSampsPerBuffer);
					}
				if (wAmpl > wSoftclipLim)
					{
					wAmpl = wSoftclipLim;
					if (!fShouted)
						{
						fprintf(stderr, "vss warning: hard clipping on output (%.2f\n", fabs(outvecp[i]) * k / 32768);
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
		static int fFirstShout = 1;
		static int fShouted = 0;
		if (fShouted>0)
			--fShouted;
		for (i=0; i < n*nchans; ++i)
			{
			float wAmpl = outvecp[i] * k;
			if (!fShouted && (wAmpl > 0.9995*32768 || wAmpl < -0.9995*32768))
				{
				if (fFirstShout)
					// Sometimes the first occurrence is a bogus NaN value.
					fFirstShout = 0;
				else
					fprintf(stderr, "vss warning: hard clipping on output (%.2f)\n",
						fabs(wAmpl)/32768.);
				// report this not more than once every 2 seconds
				fShouted = (int)(2.0 * globs.SampleRate/MaxSampsPerBuffer);
				}
			*ssp++ = (short)wAmpl;
			}
		}

		{
		int samps = ssp - sampbuff;
		if (liveaudio)
			{
#if defined(VSS_LINUX_ALSA)
			(void)snd_pcm_write(pcm_handle_write, sampbuff, samps*sizeof(short));
#elif defined(VSS_LINUX_OSS)
			write(fdDAC, sampbuff, samps*sizeof(short));
#elif defined(VSS_LINUX_OSS)
			const int cb = samps*sizeof(short);
			// assert(vwAntidropout >= 1)
			// Simon Bates <Simon.Bates@cl.cam.ac.uk>'s idea led to this.
			if (vwAntidropout == 1)
				write(fdDAC, sampbuff, cb);
			else
				{
				const int sampsMax = MaxSampsPerBuffer*MaxNumChannels;
				static int i=0;
				static char rgb[sampsMax*sizeof(short) * vwAntidropoutMax];
				const int cBuffer = vwAntidropout; // 16: 44kHz delays 46 msec not 3 msec.

				// Pre-delay, an intentional "dropout".
				static int fFirst = 1;
				if (fFirst)
					{
					fFirst = 0;
					memset(rgb, 0, cBuffer * cb);
					write(fdDAC, rgb, cBuffer * cb);
					}

				// Append to the buffer.
				memcpy(rgb + i*cb, (const char*)sampbuff, cb);

				if (++i >= cBuffer)
					{
					i = 0;
					// Flush the buffer.
					write(fdDAC, rgb, cBuffer * cb);
					}
				}
#elif defined(VSS_IRIX)
			alWriteFrames(alp, sampbuff, samps/nchans);
			alSetFillPoint(alp, (long)(qsize-latency));
#elif defined(VSS_WINDOWS)
			// Tell streamcallback() where to find the samples.
			vrgsCallback = sampbuff;
			vcbCallback = samps*sizeof(short);
#elif defined(VSS_LINUX_UBUNTU)
			// to sync: snd_pcm_delay() returns how much time will pass before a sample written now gets played.
			{
			    signed short *ptr = sampbuff; // sampbuff is interleaved.
			    int cptr = samps / NchansOut();
			    while (cptr > 0) {
				const int err = snd_pcm_writei(pcm_handle_write, ptr, cptr);
				if (err == -EAGAIN)
				    continue;
				if (err < 0) {
				    if (xrun_recovery(pcm_handle_write, err) < 0) {
					printf("Write error: %s\n", snd_strerror(err));
					exit(EXIT_FAILURE);
				    }
				    break; /* skip one period */
				}
				ptr += err * NchansOut();
				cptr -= err;
			    }
			}
#else
			#error No source code written for audio output.
#endif
			}
		if (globs.fdOfile >= 0 && globs.ofile_enabled)
			{
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

	return TRUE;
}

extern void VSS_BeginCriticalError(void)
{
#ifdef VSS_IRIX
	cerr << "\n\033[30m\033[41mVSS CRITICAL ERROR:\n";
#else
	// security hole -- use tgetent()/tgetstr() instead?
	// portability hole -- requires tcsh.
	// system("echotc so");
#endif
}

extern void VSS_EndCriticalError(void)
{
#ifdef VSS_IRIX
	cerr << "\033[0m\n\n";

	// We should really do this with Xforms.
	unlink("/tmp/...vss...");
	system("/usr/bin/X11/xconfirm -b 'I read it.' -B 'Quit VSS' -c -icon critical -t 'VSS critical error' -header VSS | wc -c > /tmp/...vss...");

	// sleep(2); // wait for two seconds, so user notices this critical error!

	FILE* pf = fopen("/tmp/...vss...", "r");
	if (!pf)
		return; // don't bother reporting errors for this.
	int cch = -1;
	fscanf(pf, "%d", &cch);
	fclose(pf);
	unlink("/tmp/...vss...");
	if (cch == 9)
		kill(getpid(), SIGINT); // user clicked "Quit VSS" not "I read it."
#else
	// system("echotc se");
#endif
}

#ifndef VSS_WINDOWS
int main(int argc,char *argv[])
{
	return VSS_main(argc, argv);
}
#endif

#ifdef VSS_IRIX
int mdClosePortInput(MDport port)
{
	return mdClosePort(port);
}
int mdClosePortOutput(MDport port)
{
	return mdClosePort(port);
}
#endif


// If caller passes in 0, *vpfnMidi should return an int filehandle
// corresponding to the midi port to read from.
// Otherwise, *vpfnMidi should read all pending incoming midi messages
// on this port, returning zero iff an error occured.
static int (*vpfnMidi)(int) = NULL;
void SetMidiFunction(int (*vpfnMidiArg)(int))
{
	vpfnMidi = vpfnMidiArg;
}

static inline void mdSchedule(int hog)
{
#ifdef VSS_IRIX
	if(hog>0)
		{
		if(hog>1)
			if (schedctl (NDPRI,getpid(), NDPHIMIN) < 0)
				perror ("schedctl NDPNORMMIN");
		plock(PROCLOCK);
		setuid(getuid());
		}
#endif
}

int liveaudio = 0;
static inline int initudp(int chan)
{
	struct sockaddr_in serv_addr;
	int sockfd;

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
			return sockfd;
	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(chan);

	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		return -1;
	}
	fcntl(sockfd, F_SETFL, FNDELAY);
#ifdef DEBUG
	printf("socket fd %d on channel %d\n", sockfd, chan);
#endif
	return sockfd;
}
static inline void closeudp(int sockfd)
{
		close(sockfd);
}

#define MAXMESG 16384
char mbuf[MAXMESG];
int caught_sigint = 0;

#if defined(VSS_IRIX_63_MIPS3) || defined(VSS_LINUX) || defined(VSS_REDHAT7) || defined(VSS_WINDOWS)
#define SignalHandlerType int
#else
#define SignalHandlerType ...
#endif

void catch_sigint(SignalHandlerType)
{
   caught_sigint = 1;
   // cerr << "caught_sigint!\n";
}

#ifdef VSS_IRIX
#include <sys/types.h>
#include <dmedia/midi.h>

extern void CloseOfile(char*);

extern void VSS_BeginCriticalError(void);
extern void VSS_EndCriticalError(void);

extern void VSS_SetGlobalAmplitude(float ampl);
extern float VSS_GetGlobalAmplitude(void);

extern void VSS_SetGear(int iGear);

void doActors(void);
void doActorsCleanup(void);
void deleteActors(void);

extern "C" int actorMessageMM(void *pmm, struct sockaddr_in *cl_addr);

int Initsynth(int udp_port, float srate, int nchans,
			  int nchansIn, int liveaudio, int latency, int hwm);
void Closesynth(void);

int mdClosePortInput(MDport port);
int mdClosePortOutput(MDport port);

extern "C" const float* VssInputBuffer(void);
#endif

static int viGear = 1;
enum { prndl_parked=0, prndl_low, prndl_drive }; // for viGear

inline int FParked() { return viGear == prndl_parked; }
inline int FDrive()  { return viGear == prndl_drive; }

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
struct pollfd pfds[3];
#endif
int nfds = -1;

// Compute (c*MaxSampsPerBuffer) samples into the output buffer.
//
//	wCatchUp seems like a very bad idea. It allows sample buffers
//	to be computed far in advance of what can be written, and then
//	the write() (or whatever call) has to block! Dehr? 
static inline int doSynth(VSSglobals& vv, int r, int fForce=0, int wCatchUp=0)
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
	if (vfDie)
		return 0;

	// Do nothing if we're parked,
	// or if we hit an interruption (r==0) and we're not forcing it anyways.
	if (FParked() || (r <= 0 && !fForce))
		{
		return 1;
		}

//	compute a number of sample buffers that will keep the 
//	latency within the bounds set by the high and low
//	water marks:
#if defined(VSS_IRIX) 
	const int c = (wCatchUp!=0) ? wCatchUp :
					(vv.hwm-r) / (vv.nchansOut * MaxSampsPerBuffer);
#else
	const int c = 1; // much better for linux, I observe.
#endif
	for (int i=0; i<c; i++)
		{
		(void)Synth(vsfunc, MaxSampsPerBuffer, vv.nchansVSS);
		}
	return 1;
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

	if (!doSynth(globs/*vv*/, Scount()))
		vfLiveTickShouldReturnZero = 1;
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

	for (int i=0; i<foo/128; i++)
		{
		if (!doSynth(globs/*vv*/, Scount()))
			{
			vfLiveTickShouldReturnZero = 1;
			break;
			}
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
#endif	//	def VSS_WINDOWS


// Usec() returns how many microseconds have elapsed since the
// previous time it was called.
//
#if defined(VSS_IRIX)
#include <dmedia/dmedia.h>
static unsigned long long tPrev;
static inline int Usec()
{
	unsigned long long t;
	dmGetUST(&t);
	unsigned long long dt = t - tPrev;
	tPrev = t;
	return (int)(dt / 1000);
}
#elif defined(VSS_WINDOWS)
static inline const int Usec()
{
	return 10000;
}
#else
//	note: no account is taken of the fact that clock() wraps around
//	every 36 minutes or so (according to the man pages).
//
// #include <time.h>	using <ctime>
static clock_t t0;
static clock_t tPrev;
static inline const int Usec()
{
	clock_t t = clock() - t0;
	clock_t dt = t - tPrev;
	tPrev = t;
	return (int)(dt / (CLOCKS_PER_SEC / 1000000));
}
#endif


//	tick function when liveaudio is set.
//
//	The basic structure (for Unices) is:
//		generate samples
//		do actors
//		generate samples
//		poll for pending messages
//		generate samples
//		handle messages
//
static int LiveTick(VSSglobals& vv, int sockfd)
{
	while (vfWaitForReinit)
		usleep(50000);

#ifdef VSS_LINUX
	struct sockaddr cl_addr;
	unsigned int clilen;
#endif
#ifndef VSS_WINDOWS
	{
	int r = Scount();
//	if (r<globs.lwm || r>globs.hwm) printf("\t\t\t\tr == %d, water %d/%d\n", r, globs.lwm, globs.hwm);
	if (!doSynth(vv, r, 1))
		return 0;
	}
#endif

	doActors();
	doActorsCleanup(); // after doActors(), in case handlers got deleted.
#ifndef VSS_WINDOWS
	if (!doSynth(vv, Scount()))
		return 0;
#endif

#ifdef VSS_WINDOWS
#else
	nfds = (vpfnMidi && ((pfds[2].fd = (*vpfnMidi)(0)) >= 0)) ? 3 : 2;
	// 3 if midi is running, otherwise 2.

	//	pfds is an array of file descriptors, initialized
	//	in schedulerMain(). The first one is the audio device,	
	//	and doesn't need to be polled. The second one is 
	//	where we listen for client messages. The third is 
	//	 the midi port if midi is enabled.
	//	(why poll the audio hardware under IRIX?)
	if (poll(pfds+1, nfds-1, 0) < 0) // not using pfds[0]
		{
#ifndef VSS_LINUX
		// This happens in linux whenever we ^C to exit vss:
		// "interrupted system call".
		// So don't bother reporting it there.
		perror("vss internal error: problem in poll");
#endif
		return 1;
		}
#endif

#ifndef VSS_WINDOWS
	//	this call using the catchup argument (2)
	//	is highly suspect! _Must_ screw up our 
	//	latency control, no?
	//	That arg is ignored on platforms other than
	//	IRIX.
	int r = Scount();
	if (!doSynth(vv, r, 0, 2))		// ;;;; what is this "2" wCatchUp for?
		return 0;

	// Is midi input pending?  (And do we have time to deal with it now?)
	if (r>0 && nfds==3 && (pfds[2].revents & POLLIN))
		{
		if (!(*vpfnMidi)(1))
			fprintf(stderr, "vss: error reading midi input\n");
		if (r < vv.lwm)
			return 1;
		}

	// Is client input pending?...
	//	If not, return.
	if (!(pfds[1].revents & POLLIN))
	{
#ifdef EXPERIMENT
		// don't soak the CPU if we're idle: take at least 
		// MinStep microseconds per timestep.
		const int MinStep = 300;
		int usec = Usec();
		if (usec < MinStep)
			usleep(MinStep - usec);
#endif
		return 1;
	}

#endif

	int n;

//... Yes. Client input pending.
#if defined(VSS_WINDOWS)
	if
	// "while" throws an exception, oddly.
	// Even if we ioctl FIONBIO and fcntl FNDELAY.
#else
	while
#endif
		(clilen = sizeof(cl_addr),
		(n = recvfrom(sockfd, mbuf, MAXMESG, 0, &cl_addr, &clilen)) >0)

		{
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
#endif
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
		if (r > 300 && Scount() < vv.lwm)
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

static inline int BatchTick(VSSglobals& vv, int sockfd)
{
#if defined(VSS_IRIX)
	struct sockaddr_in cl_addr;
	int clilen;
#endif
#if defined(VSS_LINUX)
	struct sockaddr cl_addr;
	unsigned int clilen;
#endif
#if defined(VSS_WINDOWS)
	struct sockaddr cl_addr;
	int clilen;
#endif

	while (clilen = sizeof(cl_addr),
		recvfrom(sockfd, mbuf, MAXMESG, 0, &cl_addr, &clilen) > 0)
		{
		if (!actorMessageMM(mbuf, (SOCK*)&cl_addr))
			return 0;
		}

	if (!FParked())
		if (!Synth(vsfunc, MaxSampsPerBuffer, vv.nchansVSS))
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

#ifdef VSS_IRIX
extern "C" void flushme_(void);
#endif

void schedulerMain(VSSglobals& vv,
				   int (*sfunc)(int n, float* outvecp, int nchans))
{
	if (!sfunc)
		{
		cerr << "vss internal error: schedulerMain() got null sfunc.\n";
		return;
		}
	vsfunc = sfunc;
	int sockfd;
	liveaudio = vv.liveaudio;

	mdSchedule(vv.hog);
	/* xxx control tic interval */

#ifdef VSS_WINDOWS
LAgain:
#endif
	if ((sockfd = initudp(vv.udp_port)) <= 0)
		{
		perror("Another copy of vss may be running on this machine");
		fprintf(stderr, " (port %d)\n", vv.udp_port);
#ifdef VSS_WINDOWS
		fprintf(stderr, "HACK: trying next lower port.\n");
		vv.udp_port--;
		goto LAgain;
#endif
#ifndef VSS_WINDOWS
		fprintf(stderr, "\nIf so, type \"vsskill\" or \"kill -9 <processid>\" to kill it.\n");
#endif
		return;
		}

	vv.dacfd = Initsynth(vv.udp_port, vv.SampleRate, vv.nchansVSS,
		vv.nchansIn, liveaudio, vv.lwm, vv.hwm);

	if (liveaudio && (vv.dacfd < 0))
		{
		// This was already reported.
		// fprintf(stderr, "vss error: couldn't open hardware audio output port.\n");
		goto LDie;
		}

#ifndef VSS_WINDOWS
	if (liveaudio)
		{
		//;; probably we don't need to poll this first one, vv.dacfd.
		pfds[0].fd = vv.dacfd;				// audio to audio-output port
		pfds[0].events = POLLOUT;
		pfds[0].revents = 0;

		pfds[1].fd = sockfd;					// messages from clients
		pfds[1].events = POLLIN /* | POLLOUT */;
		pfds[1].revents = 0;
		pfds[2].fd = -1;						// midi in (if enabled)
		pfds[2].events = POLLIN;
		pfds[2].revents = 0;
		}
#endif

#if defined(VSS_IRIX)
	flushme_(); // set "flush zero" bit to avoid underflow exceptions
#elif !defined(VSS_WINDOWS)
	t0 = clock();
#endif
	
	caught_sigint = 0;
	signal(SIGINT, catch_sigint);
	while ((!caught_sigint && !vfDie) &&
		(liveaudio ? LiveTick(vv, sockfd) : BatchTick(vv, sockfd)))
		;

LDie:
	// we got a ^C, or the control panel's Quit button was pushed.
	vfDie = 1; // Tell the control panel to die.
	deleteActors();
	if (fWantToResetsynth)
		cerr <<"vss internal error: confused between quit and reset.\n";
	Closesynth();
	closeudp(sockfd);
}
