#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#include "platform.h"
using std::cerr;

// Create a socket for sending msgs back to clients.

using desc = struct {
	struct sockaddr_in addr;
	int len;
	int sockfd;
	int port;
};
static desc udpDesc = {};
// Set by BgnMsgsend().  Used by Msgsend().

void BgnMsgsend(const char *hostname, int port) {
	udpDesc.port = port;
	memset(&udpDesc.addr, 0, sizeof udpDesc.addr);
	udpDesc.addr.sin_family = AF_INET;
	udpDesc.addr.sin_addr.s_addr = inet_addr(hostname);
	udpDesc.addr.sin_port = htons(port);
	udpDesc.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpDesc.sockfd < 0) {
		cerr << "failed to make socket\n";
		return;
	}

	struct sockaddr_in cl_addr = {};
	cl_addr.sin_family = AF_INET;
	cl_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	cl_addr.sin_port = htons(0);
	if (bind(udpDesc.sockfd, (struct sockaddr *)&cl_addr, sizeof cl_addr) < 0) {
		perror("failed to bind");
		close(udpDesc.sockfd);
		udpDesc.sockfd = -1;
		return;
	}
	udpDesc.len = sizeof udpDesc.addr;
	fcntl(udpDesc.sockfd, F_SETFL, FNDELAY); // Non-blocking.
}

#if defined VSS_LINUX || defined VSS_CYGWIN32_NT40 || defined VSS_MAC
#define SPOOGE (const struct sockaddr*)
#else
#define SPOOGE
#endif

static bool sendudp(const struct sockaddr_in* sp, int sockfd, long count, mm* pmm)
{
	if (sendto(sockfd, pmm, count, 0, SPOOGE sp, sizeof *sp) != count)
		return false;
#ifdef NOISY
	printf("sendto %x:%d \"%s\", fd=%d, cb=%ld.",
		sp->sin_addr.s_addr, sp->sin_port,
		pmm->rgch, sockfd, count);
#endif
	return true;
}
#undef SPOOGE

void Msgsend(struct sockaddr_in* addr /*always vcl_addr*/, mm* pmm) {
	if (udpDesc.sockfd < 0)
		return;
	if (!addr)
		addr = &udpDesc.addr; // Why would this be useful?
	if(!sendudp(addr, udpDesc.sockfd, strlen(pmm->rgch)+1+1, pmm))
		// extra +1 for fRetval field.
		printf("send failed\n");
}

#ifdef VSS_WINDOWS
extern int vfMMIO;
#endif
extern int vfSoftClip;
extern int vfLimitClip;
extern void SetSoundIn(int fSoundIn);

// For parsing numbers from argv[] in ParseArgs(),
// more robustly than atoi() and atof().
// On error, simply exit(1).
int parseInt(const char* s)
{
	const auto saved = errno;
	errno = 0;
	char* tmp;
	const auto val = strtol(s, &tmp, 0);
	if (tmp == s || *tmp != '\0' || errno == ERANGE || errno == EINVAL) {
		std::cerr << "Failed to parse int from '" << s << "'.\n";
		exit(1);
	}
	if (errno == 0)
		errno = saved;
	return int(val);
}

double parseFloat(const char* s)
{
	const auto saved = errno;
	errno = 0;
	char* tmp;
	const auto val = strtod(s, &tmp);
	if (tmp == s || *tmp != '\0' || errno == ERANGE || errno == EINVAL) {
		std::cerr << "Failed to parse float from '" << s << "'.\n";
		exit(1);
	}
	if (errno == 0)
		errno = saved;
	return val;
}

void ParseArgs(int argc,char *argv[],int * /*udp_port*/, int *liveaudio,
	float *sample_rate, int *nchansVSS, int *nchansIn, int *nchansOut, int *hog, int * /*lwm*/, int * /*hwm*/, char* ofile)
{
#ifdef VSS_WINDOWS
	int vfNeededMMIO = 0;
#endif
    *nchansIn = 0;
	int nargs = 0;
    argc--; argv++;++nargs;
    while(argc>0)
    {
    	if(strcmp(*argv, "-srate")==0)
    	{
    		argc--; argv++;++nargs;
			if (argc <= 0)
				cerr << "vss: ignoring -srate with missing value.\n";
			else {
				*sample_rate = parseFloat(*argv);
				if (*sample_rate < 8000.)
				{
					cerr << "vss: boosting sampling rate to 8000 from " << *sample_rate << "\n";
    				*sample_rate = 8000.;
				}
				if (*sample_rate > 48000.)
				{
					cerr << "vss: limiting sampling rate to 48000 from " << *sample_rate << "\n";
    				*sample_rate = 48000.;
				}
 	     		argc--; argv++;++nargs;
  			}
    	}
	    else	if(strcmp(*argv, "-hog")==0)
    	{
    		argc--; argv++;++nargs;
			if (argc <= 0)
				cerr << "vss: ignoring -hog with missing value.\n";
			else {
				*hog = parseInt(*argv);
 	     		argc--; argv++;++nargs;
  			}
    	}
	    else	if(strcmp(*argv, "-limit")==0)
    	{
    		argc--; argv++;++nargs;
			vfLimitClip = true;
    	}
	    else	if(strcmp(*argv,"-nopanel")==0)
		{
			argc--; argv++;++nargs;
			// Silently ignore, for backwards compatibility.
		}
	    else	if(strcmp(*argv, "-silent")==0)
    	{
    		argc--; argv++;++nargs;
			*liveaudio = false;
    	}
#ifdef VSS_WINDOWS
	    else	if(strcmp(*argv, "-mmio")==0)
    	{
    		argc--; argv++;++nargs;
			vfMMIO = true;
			if (vfNeededMMIO)
				cerr << "vss remark: try using -mmio before other flags.\n";
    	}
#endif
	    else	if(strcmp(*argv, "-port")==0)
    	{
    		argc--; argv++;++nargs;
			if (argc <= 0)
				cerr << "vss: ignoring -port with missing value.\n";
			else {
				globs.udp_port = parseInt(*argv);
				if (globs.udp_port < 1000 || globs.udp_port > 65535)
					{
					cerr << "vss: port number " << globs.udp_port << " out of range [1000, 65535].  Using default 7999.\n";
					globs.udp_port = 7999;
					}
				argc--; argv++;++nargs;
				}
    	}
	    else	if(strcmp(*argv, "-soft")==0)
    	{
    		argc--; argv++;++nargs;
			vfSoftClip = true;
    	}
	    else	if(strcmp(*argv, "-lowlatency")==0)
    	{
    		argc--; argv++;++nargs;
    		globs.lwm = 128;
			globs.hwm = 384;
    	}
	    else	if(strcmp(*argv, "-latency")==0)
		{
    		argc--; argv++;++nargs;
			if(argc>1)
				{
				const auto lwm = parseInt(*argv);
				argc--; argv++;++nargs;
				const auto hwm = parseInt(*argv);
				argc--; argv++;++nargs;
				if (0 < lwm && lwm < hwm)
					{
					globs.lwm = lwm;
					globs.hwm = hwm;
					}
				else
					cerr << "vss: ignoring -latency with invalid args " << argv[-2] << " and " << argv[-1] << ".\n";
				}
			else
				{
				cerr << "vss: ignoring -latency with missing values.\n";
				if (argc>0)
					{
					argc--; argv++;++nargs;
					}
				}
    	}
	    else	if(strcmp(*argv, "-antidropout")==0)
		{
    		argc--; argv++;++nargs;
			if (argc <= 0)
				cerr << "vss: ignoring -antidropout with missing value.\n";
			else {
				const auto msec = parseFloat(*argv);
				if (msec > 0.)
					globs.msecAntidropout = msec;
				else
					cerr << "vss: ignoring -antidropout with nonpositive duration.\n";
 	     		argc--; argv++;++nargs;
  			}
    	}
	    else	if(strcmp(*argv, "-input")==0)
    	{
    		argc--; argv++;++nargs;
			SetSoundIn(true);
			if (argc <= 0)
				cerr << "vss: ignoring -input with missing value.\n";
			else {
				const auto w = parseInt(*argv);
				if (w > 0)
					{
					// it really was "-input 4", not "-input -graphoutput" (for example).
					*nchansIn = w;
					argc--; argv++;++nargs;
					}
				}
    	}
	    else	if(strcmp(*argv, "-ofile")==0)
    	{
    		argc--; argv++;++nargs;
			if (argc <= 0)
				cerr << "vss: ignoring -ofile with missing value.\n";
			else {
		  strcpy(ofile, *argv);
 	     		argc--; argv++;++nargs;
		}
    	}
	    else	if(strcmp(*argv, "-chans")==0)
    	{
    		argc--; argv++;++nargs;
			if (argc <= 0)
				cerr << "vss: ignoring -chans with missing value.\n";
			else {
				if (strstr(*argv, "into"))
					{
					// Fancy "-chans 8into4:0,1,2,3" parsing.
					int cch = 0;
					int i;
					char* pch = *argv;
					if (2 != sscanf(pch, "%dinto%d:%n", nchansVSS, nchansOut, &cch))
						goto LChansError;
					pch += cch;
					for (i=0; i<*nchansOut; i++)
						{
						int w;
						if (1 != sscanf(pch, "%d%n", &w, &cch))
							{
LChansError:
							cerr << "vss: unexpected \"-chans " << argv[0] << "\": playing 1 channel.\n";
							*nchansVSS = *nchansOut = 1;
							goto LDoneChans;
							}
						if (w<0 || w>= *nchansVSS)
							{
							cerr << "vss: channel " << w << "out of range [0, " << *nchansVSS - 1 << "].  Using 0.\n";
							w = 0;
							}
						globs.rgwRemappedOutput[i] = w;
						pch += cch+1; // just skip the comma, don't parse it.
						}
					if (pch[-1] == ',')
						cerr << "vss: ignoring extra channel numbers \"" << pch-1 << "\"\n";
					globs.fRemappedOutput = 1;
					printf("vss: outputing channels ( ");
					for (i=0; i<*nchansOut; i++)
						printf("%d ", globs.rgwRemappedOutput[i]);
					printf(") of %d into %d channels.\n", *nchansVSS, *nchansOut);
LDoneChans:;
					}
				else
					{
					// Simple case "-chans x".
					*nchansVSS = *nchansOut = parseInt(*argv);
					globs.fRemappedOutput = 0;
					}
 	     		argc--; argv++;++nargs;
				}

			// globs.fdOfile via AIFF supports 1 to 8 channels, even non-power-of-two.
    		if (*nchansVSS < 1)
				{
				cerr << "vss: outputing to 1 channel instead of " << *nchansVSS << "\n";
    			*nchansVSS = *nchansOut = 1;
				globs.fRemappedOutput = 0;
				}
#ifdef VSS_WINDOWS
    		else if (*nchansVSS > 2 && !vfMMIO)
				{
				vfNeededMMIO = 1;
				cerr << "vss warning: outputing to 2 channels (DirectSound's limit) instead of " << *nchansVSS << ".\n";
    			*nchansVSS = *nchansOut = 2;
				globs.fRemappedOutput = 0;
				}
#endif	

			if (*nchansVSS > MaxNumChannels)
				{
				cerr << "vss: playing only " << MaxNumChannels << " channels instead of " << *nchansVSS << ".\n";
				*nchansVSS = *nchansOut = MaxNumChannels;
				globs.fRemappedOutput = 0;
				// Otherwise Synth() causes a buffer overflow.
				}
    	}
    	else
    	{
			if (*argv)
				cerr << "vss: ignoring unexpected flag \"" << *argv << "\"\n";
    		cerr <<"Usage: vss \
\n\t-chans n \n\t-srate n \n\t-silent \n\t-ofile filename\n\t-input \n\t-port n \
\n\n\t-hog [012] \n\t-lowlatency \n\t-latency lwm hwm \n\t-antidropout msec \
\n\n\t-limit \n\t-soft\n";
#ifdef VSS_WINDOWS
			cerr <<"\n\n\t-mmio";
#endif
		exit(-1);
    	}
    }

	if (VssInputBuffer() && *liveaudio == 0)
		{
		cerr << "vss: ignoring audio input because -silent.\n";
		SetSoundIn(0);
		}
}
