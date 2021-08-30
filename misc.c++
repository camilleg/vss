#include <climits> // for LONG_MAX

#ifndef VSS_WINDOWS
#include <malloc.h>
#endif

#include <cstring> // for strstr()
#include "platform.h"

#include <iostream>
using namespace std;
#include <cmath>
#include <cstdlib>
#include <cstdio>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// Create a socket for sending msgs back to clients.

extern OBJ BgnMsgsend(const char *szHostname, int channel)
{
	struct sockaddr_in  cl_addr;
	int  sockfd;
	desc *o = (desc *)malloc(sizeof(desc));

	if (!o)
		return (OBJ)0;

	o->channel = channel;
	memset((char *)&o->addr, 0, sizeof(o->addr));
	o->addr.sin_family = AF_INET;
	o->addr.sin_addr.s_addr = inet_addr(szHostname);
	o->addr.sin_port = htons(channel);
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
		{
		memset((char *)&cl_addr, 0, sizeof(cl_addr));
		cl_addr.sin_family = AF_INET;
		cl_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		cl_addr.sin_port = htons(0);
		if (bind(sockfd, (struct sockaddr *)&cl_addr,
			sizeof(cl_addr)) < 0)
			{
			perror("can't bind");
			close(sockfd);
			sockfd = -1;
			}
		}
	else
		printf("unable to make socket\n");
	
	o->sockfd = sockfd;
	o->len = sizeof(o->addr);
	fcntl(sockfd, F_SETFL, FNDELAY); /* Non-blocking I/O (man 5 fcntl) */
#if defined(SET_SOCK_SND_BUF_SIZE)
	setsockopt(sockfd, F_SETFL, FNDELAY);
#endif
#ifdef NOISY
	printf("opened send socket fd %d on channel %d\n",
		o->sockfd, o->channel);
#endif
	return (OBJ)o;
}

/***************************************/
#if defined(VSS_LINUX) || defined(VSS_CYGWIN32_NT40)
#define SPOOGE (const struct sockaddr*)
#else
#define SPOOGE
#endif

static inline int sendudp(const struct sockaddr_in *sp, int sockfd, long count, void *b)
{
	if(sendto(sockfd, b, int(count), 0, SPOOGE sp, sizeof(*sp)) != count)
		return 0;

#ifdef NOISY
	printf("sendto \"%s\", fd=%d, cb=%d,\t\t",
		((mm*)b)->rgch, sockfd, count);
	/* sockaddr_in defined in /usr/include/netinet/in.h */
	printf("port=%d, ipaddr=%x\n",
		(int)sp->sin_port,
		(int)sp->sin_addr.s_addr);
#endif

	return 1;
}

#undef SPOOGE

/***************************************/

extern void MsgsendObj(OBJ obj, struct sockaddr_in *paddr, void* pv)
{
	if (!obj)
		return;

	mm* pmm = (mm *)pv;
	desc* o = (desc *)obj;
	if (paddr == NULL)
		paddr = &o->addr;
	if(!sendudp(paddr, o->sockfd, (long)strlen(pmm->rgch)+1+1, pmm))
		// extra +1 for fRetval field.
		printf("send failed\n");
}

/*	parse argument list for standard parameters */

#ifdef VSS_WINDOWS
extern int vfMMIO;
#endif
extern int vfSoftClip;
extern int vfLimitClip;
extern int vfGraphSpectrum;
extern int vfGraphOutput;
extern void SetSoundIn(int fSoundIn);

void ParseArgs(int argc,char *argv[],int * /*udp_port*/, int *liveaudio,
	float *sample_rate, int *nchansVSS, int *nchansIn, int *nchansOut, int *hog, int * /*lwm*/, int * /*hwm*/, char* ofile)
{
#ifdef VSS_WINDOWS
	int vfNeededMMIO = 0;
#endif
    *nchansIn = 0;
	int nargs = 0;
    argc--;
    argv++;++nargs;
    while(argc>0)
    {
    	if(strcmp(*argv, "-srate")==0)
    	{
    		argc--; argv++;++nargs;
    		if(argc>0)
    		{
    			*sample_rate = atof(*argv);
				if (*sample_rate < 8000.)
				{
					cerr << "vss warning: limiting sampling rate to 8000 from "
						<<*sample_rate
						<<"\n\n";
    				*sample_rate = 8000.;
				}
				if (*sample_rate > 48000.)
				{
					cerr << "vss warning: limiting sampling rate to 48000 from "
						<<*sample_rate
						<<"\n\n";
    				*sample_rate = 48000.;
				}
 	     		argc--; argv++;++nargs;
  			}
    	}
	    else	if(strcmp(*argv, "-hog")==0)
    	{
    		argc--; argv++;++nargs;
    		if(argc>0)
    		{
    			*hog = atoi(*argv);
 	     		argc--; argv++;++nargs;
  			}
    	}
	    else	if(strcmp(*argv, "-limit")==0)
    	{
    		argc--; argv++;++nargs;
    		vfLimitClip = TRUE;
    	}
	    else	if(strcmp(*argv,"-nopanel")==0)
		{
			argc--; argv++;++nargs;
			// Silently ignore, for backwards compatibility.
		}
	    else	if(strcmp(*argv, "-silent")==0)
    	{
    		argc--; argv++;++nargs;
    		*liveaudio = FALSE;
    	}
#ifdef VSS_WINDOWS
	    else	if(strcmp(*argv, "-mmio")==0)
    	{
    		argc--; argv++;++nargs;
    		vfMMIO = TRUE;
			if (vfNeededMMIO)
				cerr << "vss remark: try using -mmio before other flags.\n";
    	}
#endif
	    else	if(strcmp(*argv, "-port")==0)
    	{
    		argc--; argv++;++nargs;
			if(argc>0)
				{
				globs.udp_port = atoi(*argv);
				if (globs.udp_port < 1000)
					{
					cerr <<"vss error: port number must be at least 1000.  Using default 7999 instead.\n";
					globs.udp_port = 7999;
					}
				if (globs.udp_port > 65535)
					{
					cerr <<"vss error: port number must be at most 65535.  Using default 7999 instead.\n";
					globs.udp_port = 7999;
					}
				argc--; argv++;++nargs;
				}
			else
				cerr << "vss warning: ignoring missing args for -port flag.\n";
    	}
	    else	if(strcmp(*argv, "-soft")==0)
    	{
    		argc--; argv++;++nargs;
    		vfSoftClip = TRUE;
    	}
	    else	if(strcmp(*argv, "-graphoutput")==0)
    	{
    		argc--; argv++;++nargs;
    		vfGraphOutput = TRUE;
    	}
	    else	if(strcmp(*argv, "-graphspectrum")==0)
    	{
    		argc--; argv++;++nargs;
    		vfGraphSpectrum = TRUE;
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
				int lwm = atoi(*argv);
				argc--; argv++;++nargs;
				int hwm = atoi(*argv);
				argc--; argv++;++nargs;
				if (0 < lwm && lwm < hwm)
					{
					globs.lwm = lwm;
					globs.hwm = hwm;
					}
				else
					cerr << "vss warning: ignoring invalid args "
						<< argv[-2] << " and "
						<< argv[-1] << " for -latency [lwm] [hwm] flag.\n";
				}
			else
				{
				cerr << "vss warning: ignoring missing args for -latency [lwm] [hwm] flag.\n";
				if (argc>0)
					{
					argc--; argv++;++nargs;
					}
				}
    	}
	    else	if(strcmp(*argv, "-antidropout")==0)
		{
    		argc--; argv++;++nargs;
    		if(argc>0)
    		{
    			float msec = atof(*argv);
				if (msec > 0.)
					globs.msecAntidropout = msec;
				else
					cerr << "vss warning: ignoring nonpositive duration arg for -antidropout.\n";
 	     		argc--; argv++;++nargs;
  			}
    	}
	    else	if(strcmp(*argv, "-input")==0)
    	{
    		argc--; argv++;++nargs;
    		SetSoundIn(TRUE);
			if (argc>0)
				{
				int w = atoi(*argv);
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
    		if(argc>0)
    		{
		  strcpy(ofile, *argv);
 	     		argc--; argv++;++nargs;
		}
    	}
	    else	if(strcmp(*argv, "-chans")==0)
    	{
    		argc--; argv++;++nargs;
    		if(argc>0)
				{
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
							cerr <<"vss warning: syntax error \"-chans "
								 <<argv[0]
								 <<"\": reverting to 1-channel output\n";
							*nchansVSS = *nchansOut = 1;
							goto LDoneChans;
							}
						if (w<0 || w>= *nchansVSS)
							{
							cerr <<"vss warning: channel number "
								 <<w
								 <<"out of range (0 to "
								 <<*nchansVSS - 1
								 <<"), using 0 instead.\n";
							w = 0;
							}
						globs.rgwRemappedOutput[i] = w;
						pch += cch+1; // just skip the comma, don't parse it.
						}
					if (pch[-1] == ',')
						cerr <<"vss warning: ignoring extra channel numbers \""
							 <<pch-1
							 <<"\"\n";
					globs.fRemappedOutput = 1;
					printf("vss remark: outputing channels ( ");
					for (i=0; i<*nchansOut; i++)
						printf("%d ", globs.rgwRemappedOutput[i]);
					printf(") of %d into %d channels.\n", *nchansVSS, *nchansOut);
LDoneChans:;
					}
				else
					{
					// Simple case "-chans x".
					*nchansVSS = *nchansOut = atoi(*argv);
					globs.fRemappedOutput = 0;
					}
 	     		argc--; argv++;++nargs;
				}

			// globs.fdOfile via AIFF supports 1 to 8 channels, even non-power-of-two.
    		if (*nchansVSS < 1)
				{
    			cerr <<"vss warning: outputing to 1 channel instead of "
					 <<*nchansVSS
					 <<"\n\n";
    			*nchansVSS = *nchansOut = 1;
				globs.fRemappedOutput = 0;
				}
#ifdef VSS_WINDOWS
    		else if (*nchansVSS > 2 && !vfMMIO)
				{
				vfNeededMMIO = 1;
    			cerr <<"vss warning: outputing to 2 channels (DirectSound's limit) instead of "
					 <<*nchansVSS
					 <<"\n\n";
    			*nchansVSS = *nchansOut = 2;
				globs.fRemappedOutput = 0;
				}
#endif	

			if (*nchansVSS > MaxNumChannels)
				{
				cerr <<"vss warning: limiting output to " << MaxNumChannels << " channels from "
					 <<*nchansVSS
					 <<"\n\n";
    			*nchansVSS = *nchansOut = 8;
				globs.fRemappedOutput = 0;
				// Otherwise Synth() causes a buffer overflow.
				}
    	}
    	else
    	{
			if (*argv)
				cerr <<"vss error: unknown flag \"" <<*argv <<"\"\n";
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

	if (*liveaudio == 0 && VssInputBuffer() != NULL)
		{
		cerr <<"vss warning: audio input doesn't work with -silent.\n";
		SetSoundIn(0);
		}
}

void assignvec(int n, float *out, int stride,
	 float *a, int astride)
{
	int i;
	for(i=0;i<n;++i)
	{
		out[i*stride] = a[i*astride];
	}	 
	 
}	 
void smulvec(float s, int n, float *out, int stride, float *in, int istride)
{
	int i;
	for(i=0;i<n;++i)
		out[i*stride] = s*in[i*istride];
}
void mulvec(int n, float *out, int stride, float *a, int astride, float *b, int bstride)
{
	int i;
	for(i=0;i<n;++i)
		out[i*stride] = a[i*astride]*b[i*bstride];
}
void us_mulvec(int n, float *out,  float *a,  float *b)
{
	int i;
	for(i=0;i<n;++i)
		out[i] = a[i]*b[i];
}
void kvec(float k, int n, float *out, int stride)
{
	int i;
	for(i=0;i<n;++i)
		out[i*stride] = k;
}
static unsigned long state;
void noisevec( int n, float *out, int stride)
{
	 unsigned long m = 5609937L; 
	 unsigned long a = 868765L;
	 unsigned long mask = 0xffffffL;
	 unsigned long st = state;
	 float sc = 1.0f/32767.0f;
	int i;
	
	for(i=0;i<n;++i)
	{
		st = (st*m+a)&mask;
		out[i*stride] = ((int)((st>>8)))*sc;
	}
	
	state = st;
}
#define NADD 16
void gaussian_noisevec( int n, float *out, int stride)
{
	 unsigned long m = 5609937L; 
	 unsigned long a = 868765L;
	 unsigned long mask = 0xffffffL;
	 unsigned long st = state;
	 float sc = 1.0f/((float)NADD*32767.0f);
	 int t;
	int i,j;
	
	for(i=0;i<n;++i)
	{
		t= 0;
		for(j=0;j<NADD;++j)
		{
			st = (st*m+a)&mask;
		 	t += (int)(st>>8);
		 }
		 out[i*stride] = t*sc;
	}
	
	state = st;
}
float dbtolin(float x)
{
	return exp(x*0.1151292546497f);
}	

/*___________________________ HANNING WINDOW ____________________________*/
#define hana	0.5f
#define	hanb	0.5f
void hanning(int n, float *sig, int stride)
{
	int	i;
	float k = 1.0f  /n ;
	double ka = PI * (2.0/n) ;
	for(i=0;i < n; ++i)
		sig[i*stride] = k*(hana + hanb * cos( i*ka -PI));
}

/*_____________________ BLACKMAN-HARRIS WINDOW __________________________*/

#define  a03   0.42323f
#define  a13   0.49755f
#define  a23   0.07922f
#define  a04   0.35875f
#define  a14   0.48829f
#define  a24   0.14128f
#define  a34   0.01168f
#define  a1h   0.54f
#define  a2h   0.46f

/*  Blackman-Harris Window  and window normalization */
void RvecBhwind(int n, float *wind,int stride, int k)
{
	float t,s;
	float ph1,dph1,ph2,dph2,ph3,dph3;
	long i;

	dph1 = 2.0*PI/n;
	dph2 = 2.0*dph1;
	ph1  = 0; 
	ph2  = 0; 
	s= 0.0f;

	switch(k)
	{
	 case 3:
		for (i=0; i<n; ++i)
		{
			t = a03 - a13*cos(ph1) + a23*cos(ph2);
			ph1 += dph1;
			ph2 += dph2;
			s+= t;
			wind[i*stride] =  t;
		}
		break;
	 case 4:
		dph3 = 3*dph1;
		ph3  = -3*PI-dph3;
		for (i=0; i<n; ++i)
		{
		  t = a04;
		  t += a14*cos(ph1);
		  t +=  a24*cos(ph2);
		  t += a34*cos(ph3);
			ph1 += dph1;
			ph2 += dph2;
			ph3 += dph3;
		  s += t;
		  wind[i*stride] = t;
		  }
		break;
	}
}

/*_________________________ RECTANGULAR WINDOW __________________________*/
void RvecRecwind(int n, float *sig, int stride)
{
	kvec(1.0f/n, n, sig,stride);	 
}

/*_________________________ TRIANGULAR  WINDOW __________________________*/
void RvecTriangularwind(int n, float *sig, int stride)
{
	int i = 0;
	float k = 2.0f/(n*n);

	while(i <= n/2.0)
	{
		sig[i*stride] = i * k;
		i++;
	}
	while(i <= n - 1)
	{
		sig[i*stride] = (n - i) * k;
		i++;
	}
}

/*___________________________ HAMMING WINDOW ____________________________*/
#define hama	0.5f
#define	hamb	0.426f
void RvecHammingwind(int n, float *sig, int stride)
{
	int	i = 0;
	float k = 1.0f/n;
	double ka = PI * (2.0/n) ;
	while(i < n)
	{
		sig[i*stride] = k*(hama + hamb * cos( i*ka -PI));
		i++;
	}
}
